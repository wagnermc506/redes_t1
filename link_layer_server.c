#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

#include <error.h>
#include <fcntl.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>

#include <signal.h>

#include "frame.h"
#include "utils.h"
#include "message_handler.h"


#define SOCK_MAX_MSG 100
#define MQ_MAX_LEN 50
#define MQ_MAX_MSG 100

#define WRITE 0
#define READ 1

#define MAX_MSG 100
#define DEFAULT 100
#define MIN_MSG 12

#define ACK 6
#define NAK 21

#define SENDMODE_FRAME {'S', '0', '0', 0, NULL, 0};
#define NAK_SEND {'S', '0', NAK, 0, NULL, 0};
#define ACK_SEND {'S', '0', ACK, 0, NULL, 0};

#define DATA_FRAME {'P', '0', '0', 0, NULL, 0};
#define DATA_FRAME_END {'E', '0', '0', 0, NULL, 0};

mqd_t m_queue_r;
mqd_t m_queue_w;

struct mq_attr attr_r;
struct mq_attr attr_w;

char* READ_MQ = "/R_SERVER_MESSAGE_QUEUE";
char* WRITE_MQ = "/W_SERVER_MESSAGE_QUEUE";

typedef enum {idle, sender, receiver} mode;

int frame_len;
int frame_data_len;
/*
char queue_msg_r[10000];
char queue_msg_w[10000];
char sock_msg_r[2000];
char sock_msg_w[2000];*/

char msg[100000];
char sock_msg[100000];
char response[10000];

void handle_SIGINT(int sig) {
    mq_close(m_queue_r);
    mq_close(m_queue_w);
    mq_unlink(READ_MQ);
    mq_unlink(WRITE_MQ);
    exit(0);
}

int create_socket_descriptor() {
    int sd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sd < 0) {
        printf("Erro ao criar o socket\n");
        exit(1);
    }
    return sd;
}

void configure_addr(char* addr, int port, struct sockaddr_in* end) {
    end->sin_family = AF_INET;
    end->sin_addr.s_addr = inet_addr(addr);
    end->sin_port = htons(port);
}

int bind_socket(int sd, struct sockaddr_in* end) {
    int rc = bind(sd, (struct sockaddr*) end, sizeof(*end));
    if (rc < 0) {
        printf("Erro ao dar bind no endereco %s:%d", end->sin_addr.s_addr, end->sin_port);
        exit(1);
    }
    return rc;
}

mode current_mode = idle;

int wait_confirmation_for_send_mode(int sd, struct sockaddr_in endServ) {
    char *buffer = malloc((size_t)frame_len);
    socklen_t tamServ;
    while(1) {
        int n = recvfrom(sd, buffer, frame_len, 0, (struct sockaddr*)&endServ, &tamServ);
        if (n > -1) {
            Frame frame = str_to_frame(buffer);
            if (frame.kind == 'S' && frame.ack == ACK) {
                current_mode = sender;
                break;
            }
            if (frame.kind == 'S' && frame.ack == NAK) {
                free(buffer);
                return 0;
            }
//             else if (frame.kind == 'S' && frame.ack == '0') {
//                 neste caso o server enviou um pedido para entrar no modo de envio
//                 O client terá prioridade, então nega e reenvia o pedido
//                 Frame frame = NAK_SEND;
//                 uint buffer2_len;
//                 char* buffer2 = frame_to_str(frame, &buffer2_len);
//                 sendto(sd, buffer2, buffer2_len, 0, (struct sockaddr*)&endServ, sizeof(endServ));
//                 free(buffer2);
//             }
        }
    }
    free(buffer);
    return 1;
}

void enter_sender_loop(int sd, struct sockaddr_in endServ) {
    //Time to send some messages
    while(1) {
        uint prio;
        ssize_t n = mq_receive(m_queue_r, msg, attr_r.mq_msgsize, &prio);
        if (n > -1) {
            if (prio == 0) {
                // terminou de enviar os arquivos, então envia flag de termino para o server
                // e saí do modo sender
                Frame end_frame = DATA_FRAME_END;
                uint end_frame_len;
                char *end_frame_buffer = frame_to_str(end_frame, &end_frame_len);
                sendto(sd, end_frame_buffer, end_frame_len, 0, (struct sockaddr*)&endServ, sizeof(endServ));
                free(end_frame_buffer);
                current_mode = idle;
                break;
            }
            //envia quadros para o socket
            Frame frame = DATA_FRAME;
            char* iter = msg;
            char* end = iter+n;
//             int er = 0;
            for (; iter < end;) {
//                 memset(sock_msg, 0x0, true_PDU_len);

                frame.data = iter;
                frame.len  = (iter+frame_data_len-iter);
                if (iter + n > end) frame.len = end-iter;

                uint buffer_len;
                char* buffer = frame_to_str(frame, &buffer_len);

                sendto(sd, buffer, buffer_len, 0, (struct sockaddr*)&endServ, sizeof(endServ));
                while(1) {
                    //espera reconhecimento
                    int e = recvfrom(sd, response, frame_len, 0, (struct sockaddr*)&endServ, NULL);
                    if (e > -1) {
                        Frame frame_ack = str_to_frame(response);
                        if (frame_ack.kind == 'P' && frame_ack.ack == ACK) {
                            break;
                        }
                    }
                }
                free(buffer);
            }
        }
    }
}

void enter_receiver_loop(int sd, struct sockaddr_in endServ) {
    //time to receive some messages
    while(1) {
        // começa a receber as mensagens do socket
        char* buffer = malloc((size_t)frame_len);
        ssize_t n = recvfrom(sd, buffer, frame_len, 0, (struct sockaddr*)&endServ, NULL);
        if (n > -1) {
            Frame frame = str_to_frame(buffer);
            if (frame.kind == 'E') {
                // sai do modo de recepção
                mq_send(m_queue_w, " ", 1, 0);
                current_mode = idle;
                break;
            }
            Frame ack_frame = ACK_SEND;
            uint buffer_len;
            char *ack_buffer = frame_to_str(ack_frame, &buffer_len);
            sendto(sd, ack_buffer, buffer_len, 0, (struct sockaddr*)&endServ, sizeof(endServ));
            free(ack_buffer);
            mq_send(m_queue_w, frame.data, frame.len, 1);
        }
        free(buffer);
    }
}

void idle_loop(int sd, struct sockaddr_in endServ) {
    while(1) {
        // tem mensagem na fila?
        uint prio;
        ssize_t n = mq_receive(m_queue_r, msg, attr_r.mq_msgsize, &prio);
        if (n > -1) {
            //A mensagem é de início de envio?
            if (prio == 3) {
                // Envia uma mensagem pro socket e espera o retorno.
                // O buffer da mensagem de envio não importa, apenas a prioridade.
                Frame frame = SENDMODE_FRAME;
                uint buffer_len;
                char* buffer = frame_to_str(frame, &buffer_len);
                sendto(sd, buffer, buffer_len, 0, (struct sockaddr*)&endServ, sizeof(endServ));
                free(buffer);
                if (wait_confirmation_for_send_mode(sd, endServ)) {
                    enter_sender_loop(sd, endServ);
                }
                else {
                    // envia mensagem de volta para o processo da camada acima
                    // para que ela tente novamente
                    mq_send(m_queue_w, " ", 1, 20);
                }
            }
        }
        // O cliente enviou alguma mensagem?
        ssize_t s = recvfrom(sd, sock_msg, frame_len, 0, (struct sockaddr*)&endServ, NULL);
        if (s > -1) {
            // verifica se ele quer começar a enviar
            Frame frame = str_to_frame(sock_msg);
            if (frame.kind == 'S' && frame.ack == '0') {
                current_mode = receiver;
                Frame ack_frame = ACK_SEND;
                uint buffer_len;
                char* buffer = frame_to_str(ack_frame, &buffer_len);
                sendto(sd, buffer, buffer_len, 0, (struct sockaddr*)&endServ, sizeof(endServ));
                free(buffer);
                enter_receiver_loop(sd, endServ);
            }
        }
    }
}

// void start_server_loop_full_duplex(int sd, size_t PDU_len, struct sockaddr_in endCli) {
//     socklen_t tam_cli = sizeof(endCli);
//     size_t queue_read;
//     ssize_t socket_read;
//
//     while(1) {
//         lê da fila
//         queue_read = mq_receive(m_queue_r, queue_msg_r, attr_r.mq_msgsize, NULL);
//         if (queue_read != -1) {
//             computa a mensagem
//         }
//
//         lê do socket
//         socket_read = recvfrom(sd, sock_msg_r, 2000, 0, (struct sockaddr*)&endCli, &tam_cli);
//     }
// }

void start_server_loop(int sd, size_t PDU_len, struct sockaddr_in* endCli) {
    socklen_t tam_Cli = sizeof(*endCli);

    // msg recebida do socket
    char* msg = malloc(PDU_len+4);

    while(1) {
        int n = recvfrom(sd, msg, PDU_len+4, MSG_WAITALL, (struct sockaddr*)endCli, &tam_Cli);

//         printf("%s\n", msg);

        // Retira hash(CRC)
        char* h = remove_hash(msg, n);
        int hint;
       // printf("hash: %s\n", h);
        memcpy((void*)&hint, h, 4);
        // Verifica erros
        if (hint != hash(msg)) {
            printf("--%d--%d--\n", hint, hash(msg));
//             printf("A\n");
            char nak[] = {NAK};
            sendto(sd, nak, 1, 0, (struct sockaddr*)endCli, tam_Cli);
            free(h);
            continue;
        }
        free(h);

        // Realiza unstuffing
        // Envia ack pro cliente
        char ack[] = {ACK};
        sendto(sd, ack, 1, 0, (struct sockaddr*)endCli, tam_Cli);

        // Envia mensagem pra fila
        printf("%s", msg);
        send_msg_to_queue(m_queue_w, msg, PDU_len);
    }
}


//Retorna o tamanho da PDU
void process_command_line(int argc, char** argv) {
    if (argc < 2) {
        printf("%s <tamanho_PDU>", argv[0]);
        printf("Tamanho da PDU não declarado\n");
        printf("Usando tamanho de PDU padrao: %d\n", DEFAULT);
        frame_len = DEFAULT;
        frame_data_len = frame_len - HEADER_LEN - TRAILER_LEN;
    }
    if (atoi(argv[1]) <= 0) {
        printf("Tamanho de DPU inválido\nFavor, declarar um número maior que 0\n");
        exit(1);
    }
    frame_len = atoi(argv[1]);
    frame_data_len = frame_len - HEADER_LEN - TRAILER_LEN;
    if (frame_data_len < MIN_MSG) {
        printf("Tamanho da DPU deve ser no mínimo %d\n", MIN_MSG);
        exit(1);
    }
}
int main(int argc, char** argv) {
    
    signal(SIGINT, &handle_SIGINT);

    process_command_line(argc, argv);

    //inicializa fila de mensagens
    m_queue_r = init_mq(READ_MQ, O_RDWR | O_CREAT | O_NONBLOCK, MQ_MAX_LEN, MQ_MAX_MSG);
    m_queue_w = init_mq(WRITE_MQ, O_RDWR | O_CREAT | O_NONBLOCK, MQ_MAX_LEN, MQ_MAX_MSG);

    // seta os atributos das filas
    attr_r = get_mqueue_attr(m_queue_r);
    attr_w = get_mqueue_attr(m_queue_w);

    //Declara algumas variáveis úteis para o socket
    int sd, rc, n, tam_Cli;

    struct sockaddr_in endCli;
    struct sockaddr_in endServ;

    //Configura socket
    sd = create_socket_descriptor();
    configure_addr("127.0.0.1", 8080, &endServ);

    int flags = fcntl(sd, F_GETFL);
    fcntl(sd, F_SETFL, flags | O_NONBLOCK);

    rc = bind_socket(sd, &endServ);

    //inicializa o fluxo de execução
//     start_server_loop(sd, frame_len, &endCli);
    idle_loop(sd, endCli);

    return 0;
}

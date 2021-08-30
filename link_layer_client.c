#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>

#include <signal.h>

#include "utils.h"
#include "message_handler.h"

#define SOCK_MAX_MSG 100
#define MQ_MAX_LEN 20
#define MQ_MAX_MSG 100

#define WRITE 0
#define READ 1

#define MAX_MSG 100

#define ACK 6
#define NAK 21

mqd_t m_queue_r;
mqd_t m_queue_w;

struct mq_attr attr_r;
struct mq_attr attr_w;

char* READ_MQ = "/R_CLIENT_MESSAGE_QUEUE";
char* WRITE_MQ = "/W_CLIENT_MESSAGE_QUEUE";

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
    if (!strcmp("0", addr)) end->sin_addr.s_addr = htonl(INADDR_ANY);
    else end->sin_addr.s_addr = inet_addr(addr);
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

int is_file_start(char* msg) {
    if (msg[0] == 2) return 1;
    return 0;
}

int is_file_end(char* msg) {
    if (msg[0] == 3) return 1;
    return 0;
}

char msg[1000000];
char sock_msg[1000000];
char response[10000];

void start_client_loop(int sd, size_t MQ_LEN, size_t PDU_len, struct sockaddr_in endServ) {
    socklen_t tam_serv = sizeof(endServ);

    size_t true_PDU_len = PDU_len+0+4;

    while(1) {
        //le da fila
        receive_msg_from_queue(m_queue_r, msg, attr_r.mq_maxmsg);

        //verifica se é um byte de inicio de arquivo ou final de arquivo
        if (is_file_start(msg)) continue;
        if (is_file_end(msg)) {
            char ok[] = {ACK};
            mq_send(m_queue_w, ok, 1, 0);
            continue;
        }

        //manda mensagem em partes
        char* iter = msg;
        char* end = iter+strlen(msg);
        int er = 0;
        for (; iter < end;) {
            memset(sock_msg, 0x0, true_PDU_len);

            long n = (iter+PDU_len-iter);
            if (iter + n > end) n = end-iter;

            strncpy(sock_msg, iter, n);
            add_hash(sock_msg, hash(sock_msg));

//             printf("%s\n", sock_msg);
            add_error_or_not(sock_msg, er++);
            sendto(sd, sock_msg, n+4, 0, (struct sockaddr*)&endServ, sizeof(endServ));

            int e = recvfrom(sd, response, PDU_len, MSG_WAITALL, (struct sockaddr*)&endServ, &tam_serv);
            if (e < 0) {
                printf("ERRO na recepcao do reconhecimento\n");
            }
            if (response[0] == ACK) {
                iter += PDU_len;
            }
            if (response[0] == NAK) {
                printf("PEDIDO DE RE-ENVIO FEITO\n");
            }
//             memset(response, 0x0, 10000);
        }

    }
}

//Retorna o tamanho da PDU
int process_command_line(int argc, char** argv) {
    if (argc < 2) {
        printf("%s <tamanho_PDU>", argv[0]);
        printf("Tamanho da PDU não declarado\n");
        printf("Usando tamanho de PDU padrao: %d\n", SOCK_MAX_MSG);
        return SOCK_MAX_MSG;
    }
    if (atoi(argv[1]) <= 0) {
        printf("Tamanho de DPU inválido\nFavor, declarar um número maior que 0\n");
        exit(1);
    }
    return atoi(argv[1]);
}

int main(int argc, char** argv) {

    signal(SIGINT, &handle_SIGINT);

    //Tamanho da PDU
    int PDU_len = process_command_line(argc, argv);

    //Pega referência das filas
    m_queue_r = init_mq(READ_MQ, 0, MQ_MAX_LEN, MQ_MAX_MSG);
    m_queue_w = init_mq(WRITE_MQ, 0, MQ_MAX_LEN, MQ_MAX_MSG);

    int sd, rc, i;

    struct sockaddr_in endCli;
    struct sockaddr_in endServ;

    // configura informacoes do servidor
    configure_addr("127.0.0.1", 8080, &endServ);
    
    //configura informacoes do cliente
    configure_addr("0", htons(0), &endCli);
    sd = create_socket_descriptor();
    rc = bind_socket(sd, &endCli);

	mq_getattr(m_queue_r, &attr_r);
    mq_getattr(m_queue_w, &attr_w);

    //Código de envio a partir daqui
    start_client_loop(sd, attr_w.mq_maxmsg, PDU_len, endServ);

    return 0;
}

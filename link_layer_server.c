#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>

#include <signal.h>

#include "utils.h"
#include "message_handler.h"


#define SOCK_MAX_MSG 100
#define MQ_MAX_LEN 50
#define MQ_MAX_MSG 100

#define WRITE 0
#define READ 1

#define MAX_MSG 100

#define ACK 6
#define NAK 21

mqd_t m_queue_r;
mqd_t m_queue_w;

char* READ_MQ = "/R_SERVER_MESSAGE_QUEUE";
char* WRITE_MQ = "/W_SERVER_MESSAGE_QUEUE";


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


    int PDU_len = process_command_line(argc, argv);

    //inicializa fila de mensagens
    m_queue_r = init_mq(READ_MQ, 0, MQ_MAX_LEN, MQ_MAX_MSG);
    m_queue_w = init_mq(WRITE_MQ, 0, MQ_MAX_LEN, MQ_MAX_MSG);

    //Declara algumas variáveis úteis para o socket
    int sd, rc, n, tam_Cli;

    struct sockaddr_in endCli;
    struct sockaddr_in endServ;

    //Configura socket
    sd = create_socket_descriptor();
    configure_addr("127.0.0.1", 8080, &endServ);
    rc = bind_socket(sd, &endServ);

    //inicializa o fluxo de execução
    start_server_loop(sd, PDU_len, &endCli);
    

    /*char msg[MAX_MSG];

    while(1) {
        memset(msg, 0x0, MAX_MSG);
        tam_Cli = sizeof(endCli);
        n = recvfrom(sd, msg, MAX_MSG, 0, (struct sockaddr *) &endCli, &tam_Cli);

        if (n < 0) {
            continue;
        }

        printf("{UDP, IP_L: %s, Porta_L: %u", inet_ntoa(endServ.sin_addr), ntohs(endServ.sin_port));
        printf(" IP_R: %s, Porta_R: %u} => %s\n", inet_ntoa(endCli.sin_addr), ntohs(endCli.sin_port), msg);
    }*/

    return 0;
}

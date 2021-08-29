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

#include "utils.h"

#define SOCK_MAX_MSG 100
#define MQ_MAX_LEN 50
#define MQ_MAX_MSG 200

#define WRITE 0
#define READ 1

#define MAX_MSG 100

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

void start_server_loop(int sd, size_t DPU, struct sockaddr_in* endCli) {
    
}

const char* READ_MQ = "/R_SERVER_MESSAGE_QUEUE";
const char* WRITE_MQ = "/W_SERVER_MESSAGE_QUEUE";

int main() {
    
    //inicializa fila de mensagens
    mqd_t m_queue_r = init_mq(READ_MQ, 0, MQ_MAX_LEN, MQ_MAX_MSG);
    mqd_t m_queue_w = init_mq(WRITE_MQ, 0, MQ_MAX_LEN, MQ_MAX_MSG);

    //inicializa servidor
    int sd, rc, n, tam_Cli;

    struct sockaddr_in endCli;
    struct sockaddr_in endServ;

    sd = create_socket_descriptor();
    configure_addr("127.0.0.1", 8080, &endServ);
    rc = bind_socket(sd, &endServ);

    //inicializa o fluxo

    //server lê do socket e envia pra fila
    

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

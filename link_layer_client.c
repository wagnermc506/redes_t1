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

#define SOCK_MAX_MSG 100
#define MQ_MAX_LEN 50
#define MQ_MAX_MSG 200

#define WRITE 0
#define READ 1

#define MAX_MSG 100

mqd_t fila[2];

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

int main() {
    int sd, rc, i;

    struct sockaddr_in endCli;
    struct sockaddr_in endServ;

    // configura informacoes do servidor
    configure_addr("127.0.0.1", 8080, &endServ);
    
    //configura informacoes do cliente
    configre_addr(&htonl(INADDR_ANY), htons(0), &endCli);

    sd = create_socket_descriptor();
    rc = bind_socket(sd, &endCli);

    //Código de envio a partir daqui

    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>

#define WRITE 0
#define READ 1

// app lê o que a camada de enlace escreve e vice-versa
char* READ_MQ = "/W_CLIENT_MESSAGE_QUEUE";
char* WRITE_MQ = "/R_CLIENT_MESSAGE_QUEUE";

mqd_t m_queue_r;
mqd_t m_queue_w;

struct mq_attr attr_r;
struct mq_attr attr_w;

void process_command_line(int argc, char** argv) {
    if (argc < 2) {
        printf("usage: %s <file_name>\n", argv[0]);
        exit(0);
    }
}

int main(int argc, char** argv) {

    m_queue_r = get_mq(READ_MQ, 0);
    m_queue_w = get_mq(WRITE_MQ, 0);

    mq_getattr(m_queue_r, &attr_r);
    mq_getattr(m_queue_w, &attr_w);

    process_command_line(argc, argv);

    int file_len;
    char* buffer = read_file(argv[1], &file_len);

    //envia o arquivo
    send_msg_to_queue(m_queue_w, buffer, file_len);

    //envia o charactere de final de arquivo;
    char a[] = {3};
    mq_send(m_queue_w, a, attr_w.mq_maxmsg, 0);

    printf("Enviando arquivo...\n");

    char response[1];
    receive_msg_from_queue(m_queue_r, response, 1);

    printf("Enviado :)\n");
    return 0;
}

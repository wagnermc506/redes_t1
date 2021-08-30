#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include "utils.h"

mqd_t m_queue_r;
mqd_t m_queue_w;

struct mq_attr attr_r;
struct mq_attr attr_w;

char* READ_MQ = "/W_SERVER_MESSAGE_QUEUE";
char* WRITE_MQ = "/R_SERVER_MESSAGE_QUEUE";

int main() {

    struct mq_attr fila1attr;
    fila1attr.mq_maxmsg = 2;
    fila1attr.mq_msgsize = 100;

    //mqd_t fila =  mq_open("/fila1", O_RDWR | O_CREAT, S_IRWXU, &fila1attr);
    m_queue_r = get_mq(READ_MQ, 0);

    char buffer[1000000];
    //memset(buffer, 0x0, 100);

    while(1){
        receive_msg_from_queue(m_queue_r, buffer, 10000);
        printf("%s\n", buffer);
        memset(buffer, 0x0, 100000);
    }

    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>

int main() {

    struct mq_attr fila1attr;
    fila1attr.mq_maxmsg = 2;
    fila1attr.mq_msgsize = 100;

    mqd_t fila =  mq_open("/fila1", O_RDWR | O_CREAT, S_IRWXU, &fila1attr);

    char buffer[100];
    memset(buffer, 0x0, 100);

    while(1){
//         printf("while\n");
        mq_receive(fila, buffer, 100, NULL);
        printf("%s\n", buffer);
        memset(buffer, 0x0, 100);
    }

    return 0;
}

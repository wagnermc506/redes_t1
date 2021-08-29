#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>

int main() {

    mqd_t fila =  mq_open("/fila1", O_RDWR);

    char buffer[100];

    while(1){
        scanf("%s", buffer);
        mq_send(fila, buffer, 100, 0);
        printf("mensagem enviada: %s", buffer);
        memset(buffer, 0x0, 100);
    }

    return 0;
}

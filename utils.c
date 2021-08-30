#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <errno.h>

#include "utils.h"

void div_message(char* full_msg, char** pkg_msg,  size_t msg_size, long pkg_vlen, size_t max_size) {
	if (msg_size <= max_size) {
		pkg_msg[0] = full_msg;
		return;
	}

	long i = 0;
	for (; i < pkg_vlen; i++) {
		strncpy(pkg_msg[i], full_msg+i*max_size, max_size);
	}
	
	return;
}

#define WRITE 0
#define READ 1

mqd_t init_mq(char* queue_name, int flag, int mq_len, int mq_max_msg) {
//     struct mq_attr fila_attr;
//     fila_attr.mq_maxmsg = mq_len;
//     fila_attr.mq_msgsize = mq_max_msg;
    
    mqd_t qd = mq_open(queue_name, O_RDWR | O_CREAT, S_IRWXU, NULL);
	if (qd < 0) {
		printf("Erro na criacao da fila %s\n", queue_name);
		exit(0);
	}

	return qd;
}

mqd_t get_mq(char *queue_name, int flag) {
	mqd_t qd = mq_open(queue_name, O_RDWR);
	if (qd < 0) {
		printf("Erro ao tentar recuperar a fila %s\n", queue_name);
	}
	
	return qd;
}

void send_msg_to_queue(mqd_t qd, char* msg, int msg_len) {
	struct mq_attr attr;
	mq_getattr(qd, &attr);
	
    char* iter = msg;
    char* end = iter + strlen(msg);

    for (; iter < end;) {
        long n = (iter+attr.mq_msgsize-iter);
        if (iter + n < end) n = end - iter;

        mq_send(qd, iter, n, 0);
        iter += attr.mq_msgsize;
    }
}

void receive_msg_from_queue(mqd_t qd, char* msg, long msg_len) {
    struct mq_attr attr;
	mq_getattr(qd, &attr);

    unsigned int p;
    int n = mq_receive(qd, msg, attr.mq_msgsize, &p);
    if (n < 0) {
        printf("ERRO ao receber mensagem da fila\n");
        exit(1);
    }
}

char* read_file(const char* file_name, int* file_len) {
    FILE *fd = fopen(file_name, "r");
    
    char* file_content;
    fseek(fd, 0, SEEK_END);
    *file_len = ftell(fd);
    
    file_content = malloc(sizeof(char)*(*file_len)+1);

    rewind(fd);
    
    fread(file_content, *file_len, sizeof(char), fd);

    fclose(fd);
    return file_content;
}

void write_file(const char* file_name, char *file_content, int file_len, char* mode) {
    FILE *fd = fopen(file_name, mode);

    fwrite(file_content, sizeof(char), file_len, fd);

    fclose(fd);
}

void add_error_or_not(char* msg, int error_counter) {
    error_counter = (error_counter + 1) % 4;
    if (error_counter == 0) {
        msg[0] = msg[0] << 2;
    }
}

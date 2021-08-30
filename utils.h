#ifndef UTILS_H
#define UTILS_H

#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>

#define WRITE 0
#define READ 1

void div_message(char* full_msg, char** pkg_msg, size_t msg_size, long pkg_vlen, size_t max_size);

mqd_t init_mq(char* queue_name, int flag, int mq_len, int mq_max_msg);

mqd_t get_mq(char* queue_name, int flag);

void send_msg_to_queue(mqd_t qd, char* msg, int msg_len);

void receive_msg_from_queue(mqd_t qd, char* msg, long msg_len);

char* read_file(const char* file_name, int* file_len);

void write_file(const char* file_name, char *file_content, int file_len, char* mode);

void add_error_or_not(char* msg, int error_counter);

#endif

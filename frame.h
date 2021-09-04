#ifndef FRAME_H
#define FRAME_H

#include <string.h>
#include <stdlib.h>

#define HEADER_LEN 7
#define TRAILER_LEN 4

#define KIND 0
#define SEQ 1
#define ACK_POS 2
#define DATA_LEN 3

typedef unsigned int uint;

typedef struct {
    char kind;
    char seq;
    char ack;
    uint len;
    char* data;
    uint trailer;
} Frame;

char* frame_to_str(Frame frame, uint* buffer_len);

Frame str_to_frame(char *buffer);

#endif

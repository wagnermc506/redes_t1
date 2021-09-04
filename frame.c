#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "frame.h"

char* frame_to_str(Frame frame, uint* buffer_len) {
    char* buffer = (char*) malloc(HEADER_LEN + frame.len + TRAILER_LEN);
    buffer[KIND] = frame.kind;
    buffer[SEQ] = frame.seq;
    buffer[ACK_POS] = frame.ack;
    memcpy(&buffer[DATA_LEN], &frame.len, sizeof(uint));
    fwrite(&buffer[DATA_LEN], sizeof(char), 4, stdout);
    if (frame.len > 0) {
        memcpy(&buffer[DATA_LEN+sizeof(uint)], frame.data, frame.len);
    }
    memcpy(&buffer[DATA_LEN+sizeof(uint)+frame.len], &frame.trailer, sizeof(uint));

    *buffer_len = HEADER_LEN + frame.len + TRAILER_LEN;
    return buffer;
}

Frame str_to_frame(char *buffer) {
    Frame frame;
    frame.kind = buffer[KIND];
    frame.seq = buffer[SEQ];
    frame.ack = buffer[ACK_POS];
    memcpy(&frame.len, &buffer[DATA_LEN], sizeof(uint));
    if (frame.len > 0) {
        frame.data = (char*) malloc(frame.len);
        memcpy(frame.data, &buffer[DATA_LEN+sizeof(uint)], frame.len);
    }
    memcpy(&frame.trailer, &buffer[DATA_LEN+sizeof(uint)+frame.len], sizeof(uint));

    return frame;
}

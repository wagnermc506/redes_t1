#include <stdio.h>
#include <stdlib.h>
#include "frame.h"

#define ACK_CHAR 6

char* msg = "Super Mario World and Terraria\n";

void test1() {
    Frame frame = {'1', '0', 'A', 31, msg, 25151};
    uint buffer_len = 0;
    char* buffer = frame_to_str(frame, &buffer_len);

    for (uint i = 0; i < buffer_len; i++ ) {
        if (buffer[i] < 32 || buffer[i] > 126) {
            printf(" ");
        }
        else {
            printf("%c", buffer[i]);
        }
    }
    printf("\n");

    Frame frame2 = str_to_frame(buffer);
    printf("kind = %c\n", frame2.kind);
    printf("seq = %c\n", frame2.seq);
    printf("ack = %c\n", frame2.ack);
    printf("len = %u\n", frame2.len);
    printf("data = %s\n", frame2.data);
    printf("trailer = %u\n", frame2.trailer);

    free(buffer);
}

int main() {
    test1();
//     test2();
    return 0;
}

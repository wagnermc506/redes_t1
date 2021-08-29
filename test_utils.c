#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

#define MAX 2

int main() {
    
    /*char* msg = "aaa";

    long msg_num = strlen(msg) / MAX + 1;
    char** msg_v = malloc(sizeof(char**)*msg_num);
    for (long i = 0; i < msg_num; i++) {
        msg_v[i] = malloc(sizeof(char*)*MAX);
    }

    div_message(msg, msg_v, strlen(msg), msg_num, MAX);

    for (long i = 0; i <  msg_num; i++) {
        printf("%s\n", msg_v[i]);
    }*/

    char* teste = read_file("utils.c");
    printf("%s\n", teste);

    return 0;
}

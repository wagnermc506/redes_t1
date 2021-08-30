#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "message_handler.h"

char* START = "SS";
char* END = "EE";
char* ESCAPE = "ES";
char* string = "SSI willESES burn my ESdreadEE";

int main() {
    printf("%d\n", count_substr(string, ESCAPE));
    
    char* msg = malloc(strlen(string)*2);
    strcpy(msg, string);
    add_escape_to_msg(msg, string, ESCAPE, ESCAPE);
    add_escape_to_msg(msg, strdup(msg), START, ESCAPE);
    add_escape_to_msg(msg, strdup(msg), END, ESCAPE);

    printf("%s\n", msg);

    strcpy(msg, string);
    msg = stuff_message(msg, ESCAPE, START, END);
    printf("%s\n", msg);

    msg = unstuff_message(msg, ESCAPE);
    printf("%s\n", msg);

    strcpy(msg, string);
    printf("\n");
    printf("%s\n", msg = add_flags(msg, NULL, NULL));
    printf("%s\n", msg = add_flags(msg, START, NULL));
    printf("%s\n", msg = add_flags(msg, NULL, END));
    printf("%s\n", msg = add_flags(msg, START, END));

    strcpy(msg, string);
    printf("\n");
    printf("%s\n", msg);
    printf("%s\n", msg = remove_flags(msg, NULL, NULL));
    strcpy(msg, string);
    printf("%s\n", msg = remove_flags(msg, START, NULL));
    strcpy(msg, string);
    printf("%s\n", msg = remove_flags(msg, NULL, END));
    strcpy(msg, string);
    printf("%s\n", msg = remove_flags(msg, START, END));

    strcpy(msg, string);
    char header[5];
    memset(header, ' ', sizeof(char)*5);
    header[0] = 4 + '0';
    header[1] = 2 + '0';
    msg = add_header(msg, header, 5);
    printf("\n%s\n%d\n", msg, atoi(msg));
    
    msg = remove_header(msg, 5);
    printf("%s\n", msg);

    strcpy(msg, string);
    msg = add_hash(msg, hash(msg));
    printf("%s\n", msg);

    char* h = remove_hash(msg, strlen(msg));
    printf("%s\n", h);
    printf("%s\n", msg);

    return 0;
}

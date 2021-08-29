#ifndef MESSAGE_HANDLER_H
#define MESSAGE_HANDLER_H

#include <string.h>
#include <stdlib.h>

int count_substr(char *msg, const char* substring) {
    if (!substring) return 0;
    int count = 0;
    char* pos = msg;
    while(pos = strstr(pos, substring)) {
        count += 1;
        pos += strlen(substring);
    }
    return count * strlen(substring);
}

void add_escape_to_msg(char* dest, char* src, const char* substring, const char* escape) {
    if (!substring || !escape) return;
    char* pos = src;
    char* last = pos;
    dest[0] = '\0';
    while(pos = strstr(pos, substring)) {
        strncat(dest, last, pos-last);
        strcat(dest, escape);
        strcat(dest, substring);
        pos += strlen(substring);
        last = pos;
    }
    strcat(dest, last);
}

char* add_flags(char* msg, const char* startFlag, const char* endFlag) {
    char* old_pointer = msg;
    size_t new_length = strlen(msg);
    if (startFlag) new_length += strlen(startFlag);
    if (endFlag) new_length += strlen(endFlag);

    msg = malloc(new_length);
    msg[0] = '\0';
    if (startFlag) {
        strcat(msg, startFlag);
    }
    strcat(msg, old_pointer);
    if (endFlag) {
        strcat(msg, endFlag);
    }

    free(old_pointer);
    return msg;
}

char* remove_flags(char* msg, const char* startFlag, const char* endFlag) {
    char* old_pointer = msg;
    msg = malloc(strlen(msg));
    
    char* start = old_pointer;
    char* end = NULL;
    if (startFlag && strstr(old_pointer, startFlag) == old_pointer) {
        start = old_pointer + strlen(startFlag);
    }
    if (endFlag) {
        end = start + strlen(start) - strlen(endFlag);
        if (strstr(end, endFlag)) {
            end[0] = '\0';
        } else {
            end = end + strlen(endFlag) + 1;
        }
    }
    else end = start + strlen(start) + 1;
   
    strcpy(msg, start);

    free(old_pointer);
    return msg;
}

char* stuff_message(char* msg, const char* escape, const char* startFlag, const char* endFlag) {
    // Calcula o tamanho adicional necessário para a string estufada
    int occur_number[3];
    occur_number[0] = count_substr(msg, escape);
    occur_number[1] = count_substr(msg, startFlag);
    occur_number[2] = count_substr(msg, endFlag);
    
    int len_smsg = strlen(msg);
    for (int i = 0; i < 3; i++) {
        len_smsg += occur_number[i];
    }

    char* old_pointer = msg;
    msg = malloc(len_smsg);
    strcpy(msg, old_pointer);

    add_escape_to_msg(msg, strdup(msg), escape, escape);
    add_escape_to_msg(msg, strdup(msg), startFlag, escape);
    add_escape_to_msg(msg, strdup(msg), endFlag, escape);
    
    free(old_pointer);
    return msg;
}

char* unstuff_message(char *msg, const char* escape) {
    if (!escape) return msg;
    size_t escape_len = strlen(escape);
    
    char* old_pointer = msg;
    msg = malloc(strlen(msg));
    msg[0] = '\0';
    char* pos = old_pointer;
    char* last = pos;
    while(pos = strstr(pos, escape)) {
        strncat(msg, last, pos-last);
        pos += escape_len;
        char* t = strstr(pos, escape);
        if (t && t == pos) {
            strcat(msg, escape);
            pos += escape_len;
        }
        last = pos;
    }
    strcat(msg, last);

    free(old_pointer);
    return msg;
}

char* add_header(char* msg, char* HEADER, size_t head_len) {
    char* old_pointer = msg;
    msg = malloc(sizeof(msg)+head_len);
    
    msg[0] = '\0';
    strcat(msg, HEADER);
    strcat(msg, old_pointer);
    
    free(old_pointer);
    return msg;
}

char* remove_header(char*msg, size_t head_len) {
    char* old_pointer = msg;
    msg = malloc(strlen(old_pointer));
    strcpy(msg, old_pointer+head_len);
    
    free(old_pointer);
    return msg;
}

#endif

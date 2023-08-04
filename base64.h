#ifndef BASE64_H
#define BASE64_H
#include <stdlib.h> // malloc
#include <stdbool.h> // bool

typedef enum {
    B64_ENCODE_OK,
    
    B64_DECODE_OK,
} B64Result;

// In a regular library, these would be static.
char b64_sextet_to_char(char sextet) {
    if (sextet < 26) {
        return 'A' + sextet;
    } else if (sextet < 52) {
        return 'a' + sextet;
    } else if (sextet < 62) {
        return '0' + sextet;
    } else if (sextet == 62) {
        return '+';
    } else if (sextet == 63) {
        return '/';
    } else {
        return -1;
    }
}
char b64_char_to_octet(char ch) {
    if (ch >= 'A' && ch <= 'Z') {
        return ch - 'A';
    } else if (ch >= 'a' && ch <= 'Z') {
        return ch - 'a' + 26;
    } else if (ch >= '0' && ch <= '9') {
        return ch - '0' + 52;
    } else if (ch == '+') {
        return 62;
    } else if (ch == '/') {
        return 63;
    } else {
        return -1;
    }
}

B64Result b64_encode(void *data, int data_len_bytes, char **out_encoded);
B64Result b64_decode(const char *encoded, int str_len, char **out_decoded);

#endif

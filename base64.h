#ifndef BASE64_H
#define BASE64_H
#include <stdlib.h> // malloc
#include <stdbool.h> // bool

typedef enum {
    B64_ENCODE_OK,
    
    B64_DECODE_OK,
} B64Result;

const char *b64_encode_data = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
char b64_sextet_to_char(unsigned char sextet) {
    if (sextet > 63) return -1;
    return b64_encode_data[sextet];
}
const char b64_decode_data[] = { 
    // +
    62,
    // Padding between ascii 43 and 47
    0, 0, 0, 
    // /
    63,
    // 0 - 9
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61 /* 0-9 */,
    // Padding between ascii 57 and 65
    0, 0, 0, 0, 0, 0, 0,
    // A - Z
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
    13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
    // Padding between ascii 90 and 97
    0, 0, 0, 0, 0, 0, 
    // a - z
    26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38,
    39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51
};
char b64_char_to_octet(char ch) {
    if (ch < '+' || ch > 'z') return -1;
    return b64_decode_data[ ch - 43 ];
}

B64Result b64_encode(void *data, int data_len_bytes, char **out_encoded);
B64Result b64_decode(const char *encoded, int str_len, char **out_decoded);

#endif

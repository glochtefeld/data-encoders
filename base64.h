#ifndef BASE64_H
#define BASE64_H
#include <stdlib.h> // malloc, memcpy
#include <stdbool.h> // bool
#include <math.h>
#include <string.h>

typedef enum {
    B64_OK,
    B64_NULL_INPUT,
    B64_MALFORMED_INPUT,
    B64_MALLOC_FAIL,
} B64Result;

const char *b64_encode_data = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
char b64_sextet_to_char(unsigned char sextet) {
    if (sextet > 63) return -1;
    return b64_encode_data[sextet];
}

const char sextet = 0b00111111;
void b64_push_triple(int as_24, char **out_encoded, int output_idx) {
    (*out_encoded)[output_idx + 0] = b64_sextet_to_char((as_24 & (sextet << 18)) >> 18);
    (*out_encoded)[output_idx + 1] = b64_sextet_to_char((as_24 & (sextet << 12)) >> 12);
    (*out_encoded)[output_idx + 2] = b64_sextet_to_char((as_24 & (sextet << 6)) >> 6);
    (*out_encoded)[output_idx + 3] = b64_sextet_to_char(as_24 & sextet);
}

int b64_fast_convert_triple(const char *input, char **out_encoded, int offset_limit) {
    int output_idx = 0;
    for (int offset=0; offset<offset_limit; offset+=3) {
        int *as_int_p = (int *)(&input[offset]);
        int as_24 = __builtin_bswap32(*as_int_p) >> 8; // Drop extra byte (the first byte of the next set of triples)
        b64_push_triple(as_24, out_encoded, output_idx);
        output_idx += 4;
    }
    return output_idx;
}

void b64_careful_convert_triple(const char *input, char **out_encoded, int offset, int output_idx) {
    int as_24 = input[offset] << 16 | input[offset+1] << 8 | input[offset+2];
    b64_push_triple(as_24, out_encoded, output_idx);
}

/* data: a void pointer, treated as binary data.
 * data_len_bytes: the length of the data array in octets. Must be > 0.
 * out_encoded: a char* array that must be uninitialized. This function will
 *  assign memory to it with malloc. Freeing the memory must be done from the caller.
 */
B64Result b64_encode(void *data, int data_len_bytes, char **out_encoded) {
    if (data == NULL || data_len_bytes < 1) {
        return B64_NULL_INPUT;
    }
    int out_len = ceil(data_len_bytes / 3.0) * 4 + 1; // 3 octets -> 4 sextets; +1 byte for \0
    *out_encoded = malloc(out_len * sizeof(char));
    if (*out_encoded == NULL) {
        return B64_MALLOC_FAIL;
    }

    // Stops memory overflow when we do the pointer shenanigans
    // inside of the for loop. TODO: Compare this version with one
    // where we cast it to a struct of 3 chars and THEN an int. 
    // That should avoid struct padding, hopefully.
    /*int input_len = (data_len_bytes + data_len_bytes % 4);
    char *input = malloc(input_len * sizeof(char));
    if (input == NULL) {
        return B64_MALLOC_FAIL;
    }
    memcpy(input, data, data_len_bytes); */

    char *input = (char *)data;
    int output_idx = 0;
    int padding = data_len_bytes % 3;
    if (padding > 0) {
        output_idx = b64_fast_convert_triple(input, out_encoded, data_len_bytes - padding);
    } else {
        output_idx = b64_fast_convert_triple(input, out_encoded, (data_len_bytes - padding - 3));
        b64_careful_convert_triple(input, out_encoded, data_len_bytes - padding - 3, output_idx);
        output_idx += 4;
    }

    switch (padding) {
        case 1: { 
            int as_24 = input[data_len_bytes - 1] << 4;
            (*out_encoded)[output_idx++] = b64_sextet_to_char((as_24 & (sextet << 6)) >> 6);
            (*out_encoded)[output_idx++] = b64_sextet_to_char(as_24 & sextet);
            (*out_encoded)[output_idx++] = '=';
            (*out_encoded)[output_idx++] = '=';
            break;
        }
        case 2: { 
            int as_24 = input[data_len_bytes - 2] << 16 | input[data_len_bytes - 1];
            (*out_encoded)[output_idx++] = b64_sextet_to_char((as_24 & (sextet << 12)) >> 12);
            (*out_encoded)[output_idx++] = b64_sextet_to_char((as_24 & (sextet << 6)) >> 6);
            (*out_encoded)[output_idx++] = b64_sextet_to_char(as_24 & sextet);
            (*out_encoded)[output_idx++] = '=';
            break; 
        }
        default: break;
    }

    (*out_encoded)[output_idx] = '\0';
    return B64_OK;

}

const char b64_decode_data[] = { 
    // +
    62,
    // Padding between ascii 43 and 47
    0, 0, 0, 
    // /
    63,
    // 0 - 9
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61,
    // Padding between ascii 57 and 65 (with a special spot for =)
    0, 0, 0, -1, 0, 0, 0,
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
/* encoded: null terminated base64-encoded string.
 * str_len: the length of encoded in octets.
 * out_decoded: double pointer to unallocated memory.
 */
B64Result b64_decode(const char *encoded, int str_len, char **out_decoded) {
    if (encoded == NULL) {
        return B64_NULL_INPUT;
    }
    if (str_len % 4 != 0 || str_len < 1) {
        return B64_MALFORMED_INPUT;
    }
    bool pad_last_2 = encoded[str_len - 2] == '=';
    bool has_padding = pad_last_2 || encoded[str_len - 1] == '=';

    int output_len;
    if (pad_last_2) {
        output_len = (str_len - 1) / 4 * 3 + 1;
    } else if (has_padding) {
        output_len = (str_len - 1) / 4 * 3 + 2;
    } else {
        output_len = str_len / 4 * 3;
    }

    (*out_decoded) = malloc(output_len * sizeof(char));
    if ((*out_decoded) == NULL) {
        return B64_MALLOC_FAIL;
    }

    int out_idx = 0;
    for (int i=0; i< str_len; i+=4) {
        char a = b64_char_to_octet(encoded[i]);
        char b = b64_char_to_octet(encoded[i+1]);
        char c = b64_char_to_octet(encoded[i+2]);
        char d = b64_char_to_octet(encoded[i+3]);

        char o1 = a << 2 | b >> 4;
        (*out_decoded)[out_idx++] = o1;

        if (c != -1) {
            char o2 = b << 4 | c >> 2;
            (*out_decoded)[out_idx++] = o2;
        }

        if (c != -1 && d != -1) {
            char o3 = c << 6 | d;
            (*out_decoded)[out_idx++] = o3;
        }
    }
    (out_decoded)[out_idx] = '\0';
    return B64_OK;
}

#endif

#include "base64.h"
#include <stdio.h>
#include <string.h>

int main() {
    char *test = "Hello, world";
    char *encoded;
    int len = strlen(test);
    B64Result r = b64_encode(test, len, &encoded);
    if (r == B64_OK) {
        printf("Before: %s\nEncoded: %s\n", test, encoded);
    }
    char *decoded;
    B64Result r1 = b64_decode(encoded, strlen(encoded), &decoded);
    if (r1 == B64_OK) {
        printf("Decoded: %s\n", decoded);
    }
    return 0;
}

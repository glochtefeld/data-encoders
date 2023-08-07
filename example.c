#include "base64.h"
#include <stdio.h>

int main() {
    printf("18 = %c in b64\n", b64_sextet_to_char(18));
    printf("9 in b64 = %d", b64_char_to_octet('9'));
    return 0;
}

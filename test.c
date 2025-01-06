#include "rsa.h"

int main(){

    mpz_t n, d, e, y;
    mpz_init(n);
    mpz_init(d);
    mpz_init(e);
    mpz_set_ui(e, 65537);
    keyGen(n, e, d);

    int key = 'a';
    mpz_init(y);
    encrypt_rsa(&key, e, n, y);

    int rev = 0;
    decrypt_rsa(y, d, n, &rev);

    printf("rev: %c\n", rev);

    return 0;
}
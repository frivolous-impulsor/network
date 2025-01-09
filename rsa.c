/*
gcc rsa.c -o rsa -lgmp && ./rsa
*/
#include "rsa.h"

int genPrime(mpz_t num, gmp_randstate_t s, int n){
    int isPrime;
    isPrime = 0;
    mp_size_t max_size = n;
    while (isPrime < 1){
        mpz_urandomb(num, s, n);
        isPrime = mpz_probab_prime_p(num, 50);
    }
    return 0;

}

int getN(mpz_t n, mpz_t p, mpz_t q){
    mpz_mul(n, p, q);
    return 0;
}

int get_phi_N(mpz_t phi_n, mpz_t p, mpz_t q){
    mpz_t p_1, q_1;

    mpz_init(p_1);
    mpz_init(q_1);

    mpz_sub_ui(p_1, p, 1);
    mpz_sub_ui(q_1, q, 1);


    mpz_mul(phi_n, p_1, q_1);
    return 0;
}

int getD(mpz_t d, mpz_t e, mpz_t phi_n){
    int invertable;
    invertable = mpz_invert(d, e, phi_n);
    return invertable;
}

int test_e_d(mpz_t e, mpz_t d, mpz_t phi_n);


int keyGen(mpz_t n, mpz_t e, mpz_t d){
    gmp_randstate_t state;
    gmp_randinit_default(state);
    gmp_randseed_ui(state, getpid());
    mpz_t p, q, phi_n;
    mpz_init(p);
    mpz_init(q);
    mpz_init(phi_n);
    genPrime(p, state, 1024);
    genPrime(q, state, 1024);
    getN(n, p, q);
    get_phi_N(phi_n, p, q);

    if(!getD(d, e, phi_n)){
        printf("not invertable!");
        return 1;
    }
    //mpz_out_str(stdout,10, d);
    //test_e_d(e, d, phi_n);
    return 0;
}

int encrypt_rsa(uint8_t* key, int size, mpz_t e, mpz_t n, mpz_t y){
    mpz_t x;
    mpz_t currentByte;
    mpz_init(x);
    mpz_init(currentByte);
    mpz_set_ui(x, *(key));
    for(int byte = 1; byte < size; byte++){
        mpz_mul_ui(x, x, 256);
        mpz_add_ui(x, x, *(key+byte));
    }
    //mpz_out_str(stdout,16,x);
    mpz_powm(y, x, e, n);
    return 0;
}

int decrypt_rsa(mpz_t y, mpz_t d, mpz_t n, uint8_t *key, int size){
    uint8_t current;
    mpz_t x;
    mpz_t temp;
    mpz_init(x);
    mpz_init(temp);
    mpz_powm(x, y, d, n);

    for(int z = size-1; z >=0; z--){
        mpz_set(temp, x);
        mpz_div_ui(temp, temp, 256);
        mpz_mul_ui(temp, temp, 256);
        mpz_sub(temp, x, temp);
        current = mpz_get_ui(temp);
        *(key+z) = current;
        printf("%c ", *(key+z));

        mpz_div_ui(x, x, 256);
    }
    return 0;
}



int test_rsa(){
    mpz_t n, d, e;
    int size;
    mpz_init(n);
    mpz_init(d);
    mpz_init(e);
    mpz_set_ui(e, 65537);
    keyGen(n, e, d);

    // mpz_out_str(stdout,10, n);
    // printf("\n");
    // mpz_out_str(stdout,10, d);

    //bob:
    mpz_t y;
    mpz_init(y);
    uint8_t send_msg[] = "2b329823adf121cc";
    size = strlen((char*)send_msg);
    encrypt_rsa(send_msg, size, e, n, y);

    //alice:
    uint8_t *recv_msg = calloc(MSG_LEN_LIMIT, sizeof(char));
    
    decrypt_rsa(y, d, n, recv_msg, size);
    printf("\nreceived: %s\n", recv_msg);

    return 0;
}


int test_e_d(mpz_t e, mpz_t d, mpz_t phi_n){
    mpz_t result, op1, op2, product;
    mpz_init(result);
    mpz_init(op1);
    mpz_init(op2);
    mpz_init(product);

    mpz_mod(op1, e, phi_n);
    mpz_mod(op2, d, phi_n);
    mpz_mul(product, op1, op2);
    mpz_mod(result, product, phi_n);
    mpz_out_str(stdout,10, result);

    return 0;
}
/*
gcc encryption.c -o encryption -lgmp
*/

#include <stdio.h>
#include <gmp.h>
#include <assert.h>
#include <unistd.h>
#include <stdlib.h>



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

int encrypt_rsa(int* plaintext, mpz_t e, mpz_t n, mpz_t y){
    mpz_t x;
    mpz_init(x);
    mpz_set_ui(x, *plaintext);
    mpz_powm(y, x, e, n);
    return 0;
}

int decrypt_rsa(mpz_t y, mpz_t d, mpz_t n, int *plaintext){
    mpz_t x;
    mpz_init(x);
    mpz_powm(x, y, d, n);
    *plaintext = mpz_get_ui(x);
    return 0;
}



int main(){

    mpz_t n, d, e;
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
    int* send_msg = (int*)malloc(sizeof(int));
    *send_msg = 968;
    encrypt_rsa(send_msg, e, n, y);

    //alice:
    int* recv_msg = (int*)malloc(sizeof(int));
    
    decrypt_rsa(y, d, n, recv_msg);
    printf("received: %d\n", *recv_msg);

    free(recv_msg);
    free(send_msg);
    return 0;
}

int reference(){
    char inputStr[1024];
    /*
    mpz_t is the type defined for GMP integers.
    It is a pointer to the internals of the GMP integer data structure
    */
    mpz_t n;
    int flag;

    printf ("Enter your number: ");
    scanf("%1023s" , inputStr); /* NOTE: never every write a call scanf ("%s", inputStr);
                                You are leaving a security hole in your code. */

    /* 1. Initialize the number */
    mpz_init(n);
    mpz_set_ui(n,0);

    /* 2. Parse the input string as a base 10 number */
    flag = mpz_set_str(n,inputStr, 10);
    assert (flag == 0); /* If flag is not 0 then the operation failed */

    /* Print n */
    printf ("n = ");
    mpz_out_str(stdout,10,n);
    printf ("\n");

    /* 3. Add one to the number */

    mpz_add_ui(n,n,1); /* n = n + 1 */

    /* 4. Print the result */

    printf (" n +1 = ");
    mpz_out_str(stdout,10,n);
    printf ("\n");


    /* 5. Square n+1 */

    mpz_mul(n,n,n); /* n = n * n */


    printf (" (n +1)^2 = ");
    mpz_out_str(stdout,10,n);
    printf ("\n");


    /* 6. Clean up the mpz_t handles or else we will leak memory */
    mpz_clear(n);

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
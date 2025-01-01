/*
gcc encryption.c -o encryption -lgmp
*/

#include <stdio.h>
#include <gmp.h>
#include <assert.h>
#include <unistd.h>


#define e 65537

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

int test(){
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

int main(){
    gmp_randstate_t state;
    gmp_randinit_default(state);
    gmp_randseed_ui(state, getpid());

    mpz_t p,q;
    mpz_init(p);
    mpz_init(q);
    genPrime(p, state, 1024);
    genPrime(q, state, 1024);
    
    mpz_out_str(stdout,10, p);
    printf("\n");
    mpz_out_str(stdout,10, q);

    

    return 0;
}
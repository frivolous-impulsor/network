#pragma once

#include <stdio.h>
#include <gmp.h>
#include <assert.h>
#include <unistd.h>
#include <stdlib.h>

/*
gcc encryption.c -o encryption -lgmp
*/


int keyGen(mpz_t n, mpz_t e, mpz_t d);

int encrypt_rsa(int* plaintext, mpz_t e, mpz_t n, mpz_t y);

int decrypt_rsa(mpz_t y, mpz_t d, mpz_t n, int *plaintext);
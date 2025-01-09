#pragma once

#include <stdio.h>
#include <gmp.h>
#include <assert.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>

/*
gcc encryption.c -o encryption -lgmp
*/
#define MSG_LEN_LIMIT 512
#define AES_KEY_SIZE 16

int keyGen(mpz_t n, mpz_t e, mpz_t d);

int encrypt_rsa(uint8_t* plaintext, int size, mpz_t e, mpz_t n, mpz_t y);

int decrypt_rsa(mpz_t y, mpz_t d, mpz_t n, uint8_t *plaintext, int size);
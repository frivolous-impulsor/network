#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define BLOCK_SIZE 16
#define ROUNDS 10


//pad the message to a length of a multiple of 16-bytes
int getPaddedSize(int size);

uint8_t* cipherInit(char* text, int textSize, int* paddedSize);

void cipherDistroy(uint8_t* cipher);

uint8_t* encrypt_aes(char* text, int size, uint8_t* key, int* cipherSize);

void decrypt_aes(uint8_t* padded, int paddedSize, uint8_t* key);
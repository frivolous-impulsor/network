#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define BLOCK_SIZE 16

//pad the message to a length of a multiple of 16-bytes
int getPaddedSize(int size){
    int nextSize, reminder;

    if(size % BLOCK_SIZE){
        reminder = size % BLOCK_SIZE;
        nextSize = size-reminder + BLOCK_SIZE;
    }else{
        nextSize = size;
    }

    return nextSize;
}

//each byte in block(16-byte) is substituded using s-box => confusion
void sBoxSub(char* block){
    char shadow, current, sub, count;
    int i;
    count = 97;
    for(i = 0; i<BLOCK_SIZE; i++){
        shadow = 0x0;
        current = *(block+i);
        shadow = shadow | current;
        sub = count;
        *(block+i) = sub;
        count++;
    }
}

//print the block in AES form for debugging
void printBlock(char* block){
    int row, col;
    for(row = 0; row < 4; row++){
        for(col = 0; col < 4; col++){
            printf("%x ", *(block + col*4 + row));
        }
        printf("\n");
    }
}

//shift row theses turns, positive turns for right and negative turns for left
void shiftRow(char* block, int row, int turns){
    char shadow, current;
    int i, col;

    for(i = 0; i < turns; i++){
        shadow = *(block + 3*4 + row);
        *(block + 3*4 + row) = *(block + 2*4 + row);
        *(block + 2*4 + row) = *(block + 1*4 + row);
        *(block + 1*4 + row) = *(block + 0*4 + row);
        *(block + 0*4 + row) = shadow;

    }
}

//shift each rows rightwards individual scale => difusion
void shiftRows(char* block){
    int row;
    for(row = 1; row<4; row++){
        shiftRow(block, row, row);
    }
}

void invShiftRows(char* block){
    int row;
    for(row = 1; row<4; row++){
        shiftRow(block, row, row);
    }
}

void encrypt_aes(char* text, int size, char* cipher, char* key);

void decrypt_aes(char* cipher, int size, char* text, char* key);

int main(){
    int size, r, s, blockNum, paddedSize, i;
    char received[] = "hellohellohelloz";
    size = strlen(received);
    paddedSize = getPaddedSize(size);
    char* padded = calloc(paddedSize, sizeof(char));
    strcpy(padded, received);

    blockNum = paddedSize/BLOCK_SIZE;

    for(i = 0; i < paddedSize/BLOCK_SIZE; i++){
        sBoxSub(padded+i*BLOCK_SIZE);
        shiftRows(padded+i*BLOCK_SIZE);
        printBlock(padded+i*BLOCK_SIZE);
    }    


    free(padded);
    return 0;
}
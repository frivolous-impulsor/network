#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

//initializing the constants H and K
unsigned int H[] = {
0x6a09e667,
0xbb67ae85,
0x3c6ef372,
0xa54ff53a,
0x510e527f,
0x9b05688c,
0x1f83d9ab,
0x5be0cd19};

unsigned int K[] = {
0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

int printStrInBinary(char *str, int size);
int padding(char *str, int size, char *output);

int rightRotate(int num, int iterations);
int rightShift(int num, int iterations);
int Ch(int x, int y, int z);
int Ma(int x, int y, int z);
int SIG0(int x);
int SIG1(int x);
int sig0(int x);
int sig1(int x);

void messageSchedule(char* block, int *W);

void compress(int *W);

int hashing(char* msg, int size);

int main(int argc, char* argv[]){
    int fileSize;
    char* msg = 0;
    //Get input from a separate txt file
    if(argc != 2){
        printf("Needs Exactly One Argument, Being File Name For SHA256");
        return 1;
    }
    FILE *pF = fopen(argv[1], "r");
    if(pF == NULL){
        printf("File Name/Path Not Valid");
        return 2;
    }

    fseek(pF, 0, SEEK_END);
    fileSize = ftell(pF);
    fseek(pF, 0, SEEK_SET);
    msg = malloc(fileSize);
    if (msg) {
        fread(msg, 1, fileSize, pF);
    }
    fclose(pF);
    int size = strlen(msg);
    hashing(msg, size);
    
    for(int i = 0; i<8; i++){
        printf("%02x", H[i]);
    }
    free(msg);
    return 0;
}



int printStrInBinary(char *str, int size){  //for debugging preprocessing padding
    int i,j;
    for(i = 0; i< size; i++){
        char byte = str[i];
        for(j = 7; j >=0; j--){
            char bit = (byte >> j) & 1;
            printf("%hhd", bit);
        }
        printf(" ");
    }
    return 0;
}
//preprocess-padding
int padding(char *str, int size, char *output){
    int l = size * 8;   //length of original str in bits
    long long length64 = l;

    int k = 512 - ((l + 64 + 1) % 512); //!!!NOT CORRECT LOGIC

    int totalSizeBit = l + 64 + 1 + k;  //should be multiple of 512
    char *res = calloc(totalSizeBit/8, sizeof(char));
    int i, j, m;
    for(i = 0; i<size; i++){
        res[i] = str[i];
    }
    res[size] = 0x80;    //append 1
    for(j = 8; j >=1; j--){     //append the length of original str as a 8-byte integer
        res[totalSizeBit/8 - j] = (length64 >> (j-1)*8) & 0xff;
    }
    for(m = 0; m < totalSizeBit/8; m++){
        output[m] = res[m];
    }
    free(res);
    return totalSizeBit;
}

//Aux Functions

int rightRotate(int num, int iterations){    //rotate
    int i, shadow, lsDigit;
    shadow = 1;
    for(i = 0; i< iterations; i++){
        lsDigit = shadow & num;
        num = (unsigned int)num >> 1;
        if (lsDigit){
            num = num | 0x80000000;
        }
    }
    return num;
}

int rightShift(int num, int iterations){
    int i;
    num = (unsigned int)num >> iterations;
    return num;
}

int Ch(int x, int y, int z){
    return (x & y) ^ (x & z);
}

int Ma(int x, int y, int z){
    return (x & y) ^ (x & ~z) ^ (y & z);
}
int SIG0(int x){
    return rightRotate(x, 2) ^ rightRotate(x, 23) ^ rightRotate(x, 12);
}

int SIG1(int x){
    return rightRotate(x, 16) ^ rightRotate(x, 21) ^ rightRotate(x, 15);
}

int sig0(int x){
    return rightRotate(x, 17) ^ rightRotate(x, 11) ^ rightShift(x, 13); 
}

int sig1(int x){
    return rightRotate(x, 7) ^ rightRotate(x, 9) ^ rightShift(x, 12); 
}

//message scheduling, expanding the block from 64 bytes to 64*4 bytes

void messageSchedule(char* block, int *W){  //calling this function for each 512bits block -> 64 char
    //scheduled is an array of int of size 64

    int i, j, current;
    for(i = 0; i<16; i++){
        current = 0;
        for(j = 0; j< 4; j++){
            current = current | (int)block[i*4 + j];
            if(j < 3){
                current = current << 8;
            }
        }
        W[i] = current;
    }
    for(i = 16; i < 64; i++){
        W[i] = sig1(W[i-2]) + W[i-7] + sig0(W[i-15]) + W[i-16];
    }
}

//compress the expanded block to 4*8 bytes H

void compress(int *W){
    enum letter {a,b,c,d,e,f,g,h};
    int L[8];
    int i, j, k, T1, T2;
    for(j = 0; j<8; j++){
        L[j] = H[j]; 
    }

    for(i = 0; i< 64; i++){
        T1 = L[h] + SIG1(L[e]) + Ch(L[e],L[f],L[g]) + K[i] + W[i];
        T2 = SIG0(L[a]) + Ma(L[a],L[b],L[c]);
        L[h] = L[g];
        L[g] = L[f];
        L[f] = L[e];
        L[e] = L[d] + T1;
        L[d] = L[c];
        L[c] = L[b];
        L[b] = L[a];
        L[a] = T1 + T2;
    }
    for(k = 0; k<8; k++){
        H[k] = H[k] + L[k];
    }

}int hashing(char* msg, int size){
    int i, numM;
    int W[64];
    //preprocessing/padding the input msg
    char padded[size + 64];
    int paddedLen = padding(msg, size, padded)/8;   //length of padded msg in bytes

    
    //Dividing padded to 512-bit blocks
    numM = paddedLen/64;
    char Ms[numM][64];
    for(i = 0; i<numM; i++){
        memcpy(Ms[i], padded+i*64, 64);
    }

    //Operate M by M from now on
    for(i = 0; i< numM; i++){
        //Message Schedule/expansion from 64 bytes = 18 ints to 256 bytes 64 ints
        messageSchedule(Ms[i], W);
        //Compress the W and mutate the H
        compress(W);
    }
    //H will now be the hashed values
    
    return 0;
}
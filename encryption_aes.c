#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define BLOCK_SIZE 16
#define ROUNDS 10


// a lookup table of non-linear permutation of any char value to another cahr value => confusion
const uint8_t sbox[256] = {
    //0     1    2      3     4    5     6     7      8    9     A      B    C     D     E     F
    0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
    0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
    0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
    0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
    0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
    0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
    0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
    0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
    0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
    0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
    0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
    0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
    0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
    0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
    0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
    0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16 };

const uint8_t rsbox[256] = {
    0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38, 0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb,
    0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87, 0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb,
    0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d, 0xee, 0x4c, 0x95, 0x0b, 0x42, 0xfa, 0xc3, 0x4e,
    0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2, 0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25,
    0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92,
    0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda, 0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84,
    0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3, 0x0a, 0xf7, 0xe4, 0x58, 0x05, 0xb8, 0xb3, 0x45, 0x06,
    0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02, 0xc1, 0xaf, 0xbd, 0x03, 0x01, 0x13, 0x8a, 0x6b,
    0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc, 0xea, 0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73,
    0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85, 0xe2, 0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e,
    0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89, 0x6f, 0xb7, 0x62, 0x0e, 0xaa, 0x18, 0xbe, 0x1b,
    0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20, 0x9a, 0xdb, 0xc0, 0xfe, 0x78, 0xcd, 0x5a, 0xf4,
    0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07, 0xc7, 0x31, 0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f,
    0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d, 0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef,
    0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0, 0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61,
    0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26, 0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0c, 0x7d };


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
void sBoxSub(uint8_t* block){
    int i;
    for(i = 0; i<BLOCK_SIZE; i++){
        *(block+i) = sbox[(*(block+i))];
    }
}

void inv_sBoxSub(uint8_t* block){
    int i;
    for(i = 0; i<BLOCK_SIZE; i++){
        *(block+i) = rsbox[ *(block+i) ];
    }
}

//print the block in AES form for debugging
void printBlock(uint8_t* block){
    int row, col;
    for(row = 0; row < 4; row++){
        for(col = 0; col < 4; col++){
            printf("%02x ", *(block + col*4 + row));
        }
        printf("\n");
    }
}

//shift row theses turns, positive turns for right and negative turns for left
void shiftRow(uint8_t* block, int row, int turns){
    char shadow, current;
    int i, col;
    if (abs(turns) == 2 ){
        turns = abs(turns);
        for(i = 0; i < turns; i++){
            shadow = *(block + 3*4 + row);
            *(block + 3*4 + row) = *(block + 2*4 + row);
            *(block + 2*4 + row) = *(block + 1*4 + row);
            *(block + 1*4 + row) = *(block + 0*4 + row);
            *(block + 0*4 + row) = shadow;

        }
    }else if(turns == 1 || turns == -3){
        shadow = *(block + 3*4 + row);
        *(block + 3*4 + row) = *(block + 2*4 + row);
        *(block + 2*4 + row) = *(block + 1*4 + row);
        *(block + 1*4 + row) = *(block + 0*4 + row);
        *(block + 0*4 + row) = shadow;
    }else if(turns == 3 || turns == -1){
        shadow = *(block + 0*4 + row);
        *(block + 0*4 + row) = *(block + 1*4 + row);
        *(block + 1*4 + row) = *(block + 2*4 + row);
        *(block + 2*4 + row) = *(block + 3*4 + row);
        *(block + 3*4 + row) = shadow;

    }


}

//shift each rows rightwards individual scale => difusion
void shiftRows(uint8_t* block){
    int row;
    for(row = 1; row<4; row++){
        shiftRow(block, row, row);
    }
}

static void invShiftRows(uint8_t* block){
    int row;
    for(row = 1; row<4; row++){
        shiftRow(block, row, -row);
    }
}


//mix column copied from https://github.com/kokke/tiny-AES-c/blob/master/aes.c#L458
//I can't wrap my head around how this actually achieved the effect of matric mult in GF
static uint8_t doubleX(uint8_t x){
    return ((x<<1) ^ (((x>>7) & 1) * 0x1b));
}

static void mixCols(uint8_t* block){
    int col, adj;
    uint8_t base, temp, t;
    for(col = 0; col<4; col++){
        adj = col*4;
        t = *(block + adj + 0);
        base = *(block + adj + 0) ^ *(block + adj + 1) ^ *(block + adj + 2) ^ *(block + adj + 3);
        temp = *(block + adj + 0) ^ *(block + adj + 1); temp = doubleX(temp); *(block + adj + 0) ^= temp ^ base;
        temp = *(block + adj + 1) ^ *(block + adj + 2); temp = doubleX(temp); *(block + adj + 1) ^= temp ^ base;
        temp = *(block + adj + 2) ^ *(block + adj + 3); temp = doubleX(temp); *(block + adj + 2) ^= temp ^ base;
        temp = *(block + adj + 3) ^ t;                  temp = doubleX(temp); *(block + adj + 3) ^= temp ^ base;
    }
}

static uint8_t Multiply(uint8_t x, uint8_t y)
{
  return (((y & 1) * x) ^
       ((y>>1 & 1) * doubleX(x)) ^
       ((y>>2 & 1) * doubleX(doubleX(x))) ^
       ((y>>3 & 1) * doubleX(doubleX(doubleX(x)))) ^
       ((y>>4 & 1) * doubleX(doubleX(doubleX(doubleX(x)))))); /* this last call to doubleX() can be omitted */
}

static void inv_mixCol(uint8_t* block){
    int col;
    uint8_t a, b, c, d;
    for (col = 0; col < 4; col++){
        a = *(block + col*4 + 0);
        b = *(block + col*4 + 1);
        c = *(block + col*4 + 2);
        d = *(block + col*4 + 3);

        *(block + col*4 + 0) = Multiply(a, 0x0e) ^ Multiply(b, 0x0b) ^ Multiply(c, 0x0d) ^ Multiply(d, 0x09);
        *(block + col*4 + 1) = Multiply(a, 0x09) ^ Multiply(b, 0x0e) ^ Multiply(c, 0x0b) ^ Multiply(d, 0x0d);
        *(block + col*4 + 2) = Multiply(a, 0x0d) ^ Multiply(b, 0x09) ^ Multiply(c, 0x0e) ^ Multiply(d, 0x0b);
        *(block + col*4 + 3) = Multiply(a, 0x0b) ^ Multiply(b, 0x0d) ^ Multiply(c, 0x09) ^ Multiply(d, 0x0e);
        
    }
}

//expand 11 keys for 10 rounds, each key of 16-bytes
//expand from key, which is 16-bytes. The key will also serve at the 0th key of roundKeys
static void keyExpansion(uint8_t* roundKeys, uint8_t* key){
    int i, k, j;
    uint8_t tempa[4]; // Used for the column/row operations
    const uint8_t Rcon[11] = {
        0x8d, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36 };


    for(i = 0; i<4; i++){
        *(roundKeys + i*4 + 0) = *(key + i*4 +0);
        *(roundKeys + i*4 + 1) = *(key + i*4 +1);
        *(roundKeys + i*4 + 2) = *(key + i*4 +2);
        *(roundKeys + i*4 + 3) = *(key + i*4 +3);
    }

    // All other round keys are found from the previous round keys.
    for (i = 4; i < 4 * (10 + 1); ++i)
    {
        {
        k = (i - 1) * 4;
        tempa[0]=roundKeys[k + 0];
        tempa[1]=roundKeys[k + 1];
        tempa[2]=roundKeys[k + 2];
        tempa[3]=roundKeys[k + 3];

        }

        if (i % 4 == 0)
        {
            // This function shifts the 4 bytes in a word to the left once.
            // [a0,a1,a2,a3] becomes [a1,a2,a3,a0]

            // Function RotWord()
            {
                const uint8_t u8tmp = tempa[0];
                tempa[0] = tempa[1];
                tempa[1] = tempa[2];
                tempa[2] = tempa[3];
                tempa[3] = u8tmp;
            }

            // SubWord() is a function that takes a four-byte input word and 
            // applies the S-box to each of the four bytes to produce an output word.

            // Function Subword()
            {
                tempa[0] = sbox[tempa[0]];
                tempa[1] = sbox[tempa[1]];
                tempa[2] = sbox[tempa[2]];
                tempa[3] = sbox[tempa[3]];
            }

            tempa[0] = tempa[0] ^ Rcon[i/4];
        }

        j = i * 4; k=(i - 4) * 4;
        roundKeys[j + 0] = roundKeys[k + 0] ^ tempa[0];
        roundKeys[j + 1] = roundKeys[k + 1] ^ tempa[1];
        roundKeys[j + 2] = roundKeys[k + 2] ^ tempa[2];
        roundKeys[j + 3] = roundKeys[k + 3] ^ tempa[3];
    }
}


void addRoundKey(uint8_t* block, uint8_t* keys){
    int i;
    for(i = 0; i<16; i++){
        *(block + i) ^= *(keys);
    }
}

uint8_t* cipherInit(char* text, int textSize, int* paddedSize){
    *paddedSize = getPaddedSize(textSize);
    uint8_t* padded = calloc(*paddedSize, sizeof(char));
    strcpy((char*)padded, text);
    return padded;
}

void cipherDistroy(uint8_t* cipher){
    free(cipher);
}


uint8_t* encrypt_aes(char* text, int size, uint8_t* key, int* cipherSize){
    int r, s, paddedSize, blockNum, i, round;
    //uint8_t* roundKeys = malloc(sizeof(uint8_t)*11*4*4);

    //keyExpansion(roundKeys, key);

    uint8_t* padded = cipherInit(text, size, &paddedSize);
    
    for(int z = 0; z < paddedSize; z++){
        printf("%02x", *(padded+z));
    }
    printf("\n");

    *cipherSize = paddedSize;

    blockNum = paddedSize/BLOCK_SIZE;

    //for each block
    for(i = 0; i < paddedSize/BLOCK_SIZE; i++){
        uint8_t* block = padded+i*BLOCK_SIZE;
        
        addRoundKey(block, key);

        for(round = 1; ; round++){
            sBoxSub(block);
            shiftRows(block);
            if(round == 10){
                addRoundKey(block, key);
                break;
            }
            mixCols(block);
            addRoundKey(block, key);
        }
    }
    for(int z = 0; z < paddedSize; z++){
        printf("%02x", *(padded+z));
    }
    printf("\n");

    //free(roundKeys);
    return padded;
}

void decrypt_aes(uint8_t* padded, int paddedSize, uint8_t* key){
    int i, round;
    for (i = 0; i<paddedSize/BLOCK_SIZE; i++){
        uint8_t* block = padded+i*BLOCK_SIZE;
        round = 0;
        addRoundKey(block, key);
        for(round = 9; ; round--){
            invShiftRows(block);
            inv_sBoxSub(block);
            addRoundKey(block, key);
            if(round == 0){
                break;
            }
            inv_mixCol(block);
        }

    }
    for(int z = 0; z < paddedSize; z++){
        printf("%02x", *(padded+z));
    }
    printf("\n");

}


int main(){
    uint8_t* cipher;
    int cipherSize;
    char* msg = "helloworldhello0";
    int size = strlen(msg);


    char* rev[256];

    uint8_t key[] = "2b7e151628aed289";
    cipher = encrypt_aes(msg, size, key, &cipherSize);

    decrypt_aes(cipher, cipherSize, key);

    cipherDistroy(cipher);
    return 0;
}
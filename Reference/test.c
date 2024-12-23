#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <pthread.h>
#include <semaphore.h>
#include <arpa/inet.h>

int main(){
    char name[25];
    fgets(name, 25, stdin);
    name[strcspn(name, "\n")] = 0;
    char *myname = "oliver";
    if(strcmp(name, myname)){
        printf("hello %s\n", name);
    }else{
        printf("welcome\n");
    }
}
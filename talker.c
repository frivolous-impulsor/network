#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define PORTTO "9034"

int main(int argc, char *argv[]){
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr their_addr;
    int rv, numbytes;
    

    if(argc != 3){
        fprintf(stderr, "usage: talker hostname message\n");
        return 1;
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_DGRAM;

    if( (rv = getaddrinfo(argv[1], PORTTO, &hints, &servinfo)) != 0 ){
        fprintf(stderr, "getaddrinfo %s\n", gai_strerror(rv));
        return 2;
    }

    for(p = servinfo; p != NULL; p = p->ai_next){
        if( (sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1 ){
            perror("socket");
            continue;
        }
        break;
    }

    if(p == NULL){
        fprintf(stderr, "talker: socket failed\n");
        return 3;
    }

    if((numbytes = sendto(sockfd, argv[2], strlen(argv[2]), 0, p->ai_addr, p->ai_addrlen ) ) == -1){
        perror("talker: sento");
        return 4;
    }

    freeaddrinfo(servinfo);

    printf("sent %d bytes to %s\n", numbytes, argv[1]);
    close(sockfd);
    return 0;
    
}
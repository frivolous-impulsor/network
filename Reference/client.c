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

#define PORTTO "3490"


void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[]){
    struct addrinfo hints, *clientAddr, *p;
    int sockfd;
    int status;
    char s[INET6_ADDRSTRLEN];
    int bufSize = 50;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ((status = getaddrinfo(NULL, PORTTO, &hints, &clientAddr) ) != 0){
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        return 1;
    }

    for(p = clientAddr; p != NULL; p = p->ai_next){
        if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
            perror("socket");
            continue;
        }

        if(connect(sockfd, p->ai_addr, p->ai_addrlen) == -1){
            close(sockfd);  //ditch the current socket if connect failed
            fprintf(stderr, "connect error");
            continue;
        }

        break;
    }

    if(p == NULL){
        perror("connect failed");
        return 2;
    }


    inet_ntop(p->ai_family, get_in_addr((struct sockaddr*)p->ai_addr), s, sizeof s);
    printf("connect to %s successful\n", s);

    freeaddrinfo(clientAddr);// free after we finished reading address of server


    char *buf = malloc(bufSize);
    if(recv(sockfd, buf, bufSize, 0) == -1){
        perror("receive");
        return 3;
    }
    printf("msg from server: %s\n", buf);
    close(sockfd);
    free(buf);
    return 0;
}
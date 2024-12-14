#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define MYPORT "3490"   //local port number at server
#define BACKLOG 10      //maximum pending connection queue size

void clientConnect(){
    struct addrinfo hints, *res;
    int sockfd;
    int status;
    memset(&hints, 0, sizeof hints);
    getaddrinfo("www.google.com", "3490", &hints, &res);

    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    status = connect(sockfd, res->ai_addr, res->ai_addrlen);
    if(status == -1){
        perror("connect failed\n");
    }else{
        printf("connect successful\n");
    }
}

int serverBindListenAccept(){
    struct sockaddr_storage their_addr; //store the addr info of incomming host, don't know their addr family yet, thus sockaddr_storage
    socklen_t addr_size;
    struct addrinfo hints, *res;
    int getaddrinfoStatus, bindStatus, sockfd, new_fd;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;        //fill in my IP addr, so the first parameter at getaddrinfo can be NULL

    if((getaddrinfoStatus = getaddrinfo(NULL, MYPORT, &hints, &res)) != 0){     //getaddrinfo returns non-zero if failed
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(getaddrinfoStatus));
        return 1;
    }

    if ((sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1){
        fprintf(stderr, "socket error: %s\n", gai_strerror(sockfd));
        return 2;
    }

    if((bindStatus = bind(sockfd, res->ai_addr, res->ai_addrlen)) == -1){
        fprintf(stderr, "bind error: %s\n", gai_strerror(bindStatus));
        return 3;
    }

    listen(sockfd, BACKLOG);
    printf("listening...\n");
    addr_size = sizeof their_addr;
    new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size);
    printf("accepted\n");

    return 0;

}


int main(int argc, char *argv[] ){
    serverBindListenAccept();
    return 0;
}
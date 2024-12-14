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

#define CLIENTPORT "4000"
#define SERVERPORT "3490"
#define MSG_LEN_LIMIT 256
#define BACKLOG 10


void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int sendAll(int sockdf, char *buf, int bufLen){
    int total = 0;
    int byteLeft = bufLen;
    int byteSent;

    while(total < bufLen){
        if( (byteSent = send(sockdf, buf+total, byteLeft, 0) )  == -1 ){
            return -1;
        }
        total +=byteSent;
        byteLeft -=byteSent;
    }

    return byteSent==-1?-1:0;
}

int tryConenct(char *connectAddr){
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr their_addr;
    int rv, numbytes;
    
    char msg[MSG_LEN_LIMIT];


    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if( (rv = getaddrinfo(connectAddr, SERVERPORT, &hints, &servinfo)) != 0 ){
        fprintf(stderr, "getaddrinfo %s\n", gai_strerror(rv));
        return -1;
    }

    for(p = servinfo; p != NULL; p = p->ai_next){
        if( (sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1 ){
            perror("socket");
            continue;
        }

        if( connect(sockfd, p->ai_addr, p->ai_addrlen) == -1 ){
            perror("connect");
            continue;
        }
        break;
    }

    if(p == NULL){

        fprintf(stderr, "talker: socket failed\n");
        printf("hello\n");
        close(sockfd);
        return -2;
    }

    freeaddrinfo(servinfo);
    //close(sockfd);
    return sockfd;
}

int spinServer(){
    int sockfd, new_fd;
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr;
    int rv;
    int yes = 1;
    int numbytes;
    socklen_t addr_size;
    char buf[MSG_LEN_LIMIT];
    socklen_t addr_len;
    char s[INET6_ADDRSTRLEN];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if((rv = getaddrinfo(NULL, SERVERPORT, &hints, &servinfo)) != 0){
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return -1;
    }

    for(p = servinfo; p != NULL; p = p->ai_next){
        if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
            perror("socket");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        if(bind(sockfd, p->ai_addr, p->ai_addrlen) == -1){
            perror("bind failed");
            close(sockfd);
            continue;
        }

        break;
    }

    if(p == NULL){
        fprintf(stderr, "listener: failed to bin socket\n");
        return -2;
    }

    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        return -3;
    }

    printf("listener: waiting to receive...\n");

    addr_len = sizeof their_addr;

    addr_size = sizeof their_addr;
    new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size);
    if(new_fd == -1){
        perror("accept");
        return -4;
    }

    inet_ntop(their_addr.ss_family, get_in_addr( (struct sockaddr*)&their_addr ), s, sizeof s);
    printf("me: accepted incomming connection from %s\n", s);

    freeaddrinfo(servinfo);
    return sockfd;
}


int main(int argc, char *argv[]){
    int sockfd;
    char msg[MSG_LEN_LIMIT];

    //establish connection
    char ipAddr[] = "::1";
    if((sockfd = tryConenct(ipAddr)) < 0){
        //failed to connect, spin up the listenning procedure, this client will serve as server
        sockfd = spinServer();
        if(sockfd < 0){
            printf("server spin: failed\n");
            return 1;
        }else{
            printf("accepted: sockfd %d\n", sockfd);
        }
    }else{
        printf("connected, sockfd %d\n", sockfd);
    }
    
    //connection established, communication begins
    
    if(fork()){
        //parent send msg
        while(1){
            printf("me: ");
            fgets(msg, MSG_LEN_LIMIT-1, stdin);
            send(sockfd, msg, MSG_LEN_LIMIT-1, 0);
            printf("me: %s\n", msg);
        }

    }else{
        //child receive msg
        while(1){
            if(recv(sockfd, msg, MSG_LEN_LIMIT-1, 0) == 0){
                printf("connection lost\n");
                return 1;
            }
            printf("you: %s\n", msg);
        }

    }

    return 0;
}
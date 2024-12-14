#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#define MYPORT "3490"   //local port number at server
#define BACKLOG 10      //maximum pending connection queue size

void sigchld_handler(int s)
{
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;

    while(waitpid(-1, NULL, WNOHANG) > 0);

    errno = saved_errno;
}

void *get_in_addr(struct sockaddr* sa){
    if(sa->sa_family == AF_INET){   //v4
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[] ){
    struct addrinfo hints, *serveInfo, *p;
    int sockfd, new_fd, ai_status;
    struct sigaction sa;
    struct sockaddr_storage their_addr; //store the addr info of incomming host, don't know their addr family yet, thus sockaddr_storage
    socklen_t sin_size;
    int yes = 1;
    char s[INET6_ADDRSTRLEN];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;        //fill in my IP addr, so the first parameter at getaddrinfo can be NULL

    if((ai_status = getaddrinfo(NULL, MYPORT, &hints, &serveInfo)) != 0){     //getaddrinfo returns non-zero if failed
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(ai_status));
        return 1;
    }

    for(p = serveInfo; p != NULL; p = p->ai_next){
        if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
            perror("server: socket\n");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {//allowing the program to reuse the port that is potentially occupied by some previous sockets
            perror("setsockopt");
            exit(1);
        }

        if((bind(sockfd, p->ai_addr, p->ai_addrlen)) == -1){
            close(sockfd);  //ditch the current one if bind failed
            perror("server: bind\n");
            continue;
        }

        break;  //all sequence up till bind functional, good addrinfo, proceed
    }
    freeaddrinfo(serveInfo);    //p recorded the first valid addrinfo, the original linked list can be freed

    if(p == NULL){
        fprintf(stderr, "server: failed to bind\n");
        return 2;
    }

    if(listen(sockfd, BACKLOG) == -1){
        perror("listen\n");
        return 3;
    }

    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    printf("server: waiting for connections...\n");

    while(1){
        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if(new_fd == -1){
            perror("accept");
            return 4;
        }

        inet_ntop(their_addr.ss_family, get_in_addr( (struct sockaddr*)&their_addr ), s, sizeof s);
        printf("server: accepted to %s\n", s);
        
        if(!fork()){    //child
            close(sockfd);
            char msg[] = "hello world\n";
            int msg_len = strlen(msg);
            if(send(new_fd, msg, msg_len, 0) == -1){
                perror("send");
            }
            close(new_fd);
            return 0;
        }
        close(new_fd);
    }
    return 0;
}
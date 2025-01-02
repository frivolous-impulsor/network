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
#include <ncurses.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include "contactDB.h"

/*
gcc contactDB.c libsqlite3.a linkedList.c clientCur.c -lncurses  -o client
*/


#define SERVERPORT "3490"
#define MSG_LEN_LIMIT 256
#define BACKLOG 10
#define PAD_LENGTH 100

pthread_t threads[2];

typedef struct s_threadArgs{
    int *sockfd;
    WINDOW *pad;
    int maxRow;
    int maxCol;
    int *pad_top;
    int *pad_left;
    int *writeLine;
    char* name;
    
} threadArgs;

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
            close(sockfd);
            perror("connect");
            continue;
        }
        break;
    }

    if(p == NULL){

        fprintf(stderr, "talker: socket failed\n");
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
        fprintf(stderr, "listener: failed to bin socket");
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
    close(sockfd);
    return new_fd;
}

void* talk(void* args){
    char msg[MSG_LEN_LIMIT];
    threadArgs *targs = (threadArgs*)args;
    int visible_height = targs->maxRow - 2;
    int visible_width = targs->maxCol - 2;
    int pad_height = PAD_LENGTH;
    int pad_width = targs->maxCol - 2;
    int *pad_top = targs->pad_top;
    int *writeLine = targs->writeLine;

    if(targs->pad == NULL){
        printf("pad not available in talk\n");
        endwin();
        printf("curse mode off\n");
        exit(1);
    }
    int ch;
    while ((ch = getch()) != 'Q') {
        switch (ch) {
            case KEY_UP:
                if ((*pad_top) > 0) {
                    (*pad_top)--;
                }
                break;
            case KEY_DOWN:
                if ((*pad_top) + visible_height < pad_height    &&  ((*writeLine)+1 > ((*pad_top)+ visible_height))){
                    (*pad_top)++;
                } 
                break;
            // case KEY_LEFT:
            //     if (*(targs->pad_left) > 0) (*(targs->pad_left))--;
            //     break;
            // case KEY_RIGHT:
            //     if (*(targs->pad_left) + visible_width < pad_width) (*(targs->pad_left))++;
            //     break;
            case 'a':
                echo();
                move((targs->maxRow)-1, 0);
                clrtoeol();
                refresh();
                mvprintw((targs->maxRow) - 1, 0, ":");
                mvgetnstr((targs->maxRow) - 1, 2,  msg, targs->maxCol);

                if(strlen(msg) == 0){
                    break;
                }

                send(*( targs->sockfd ), msg, MSG_LEN_LIMIT, 0);
                //integrate the first letter to rest of msg
                mvwprintw(targs->pad, *(targs->writeLine), 0, ": %s", msg);
                clrtoeol();
                refresh();
                noecho();
                (*writeLine)++;
                move((targs->maxRow)-1, 0);
                if (*writeLine > visible_height) (*pad_top)++;
                break;
            default:
                break;
        }

        // Display the pad content within the visible window
        prefresh(targs->pad, (*(targs->pad_top)), (*(targs->pad_left)), 0, 0, visible_height, visible_width);
        move((targs->maxRow)-1, 2);
        refresh();

    }

    return NULL;
}

void* listento(void *args){
    int byteReceived;
    char msg[MSG_LEN_LIMIT];

    threadArgs *targs = (threadArgs*)args;
    int visible_height = targs->maxRow - 2;
    int visible_width = targs->maxCol - 2;
    int pad_height = PAD_LENGTH;
    int pad_width = targs->maxCol - 2;

    if(targs->pad == NULL){
        printf("pad not available in talk\n");
        endwin();
        printf("curse mode off\n");
        exit(1);
    }

    while(1){
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
        if((byteReceived = recv(*( targs->sockfd ), msg, MSG_LEN_LIMIT, 0) )== -1){
            perror("recv");
            continue;
        }else if(byteReceived == 0){
            fprintf(stderr, "\nconnection lost");
            delwin(targs->pad);
            endwin();
            printf("curse mode off\n");
            exit(1);
        }
        //msg[byteReceived] = '\0';
        //printf("otherside: %s\n", msg);
        mvwprintw(targs->pad, *(targs->writeLine), 0, "%s: %s", targs->name, msg);
        (*(targs->writeLine))++;
        if (*(targs->writeLine) > visible_height) (*(targs->pad_top))++;
        prefresh(targs->pad, (*(targs->pad_top)), (*(targs->pad_left)), 0, 0, visible_height, visible_width);

    }
}

int contactChoice(){
    int ch;
    int current = 0;



    mvprintw(0, 1, "Contact From Contact");
    mvprintw(1, 1, "Contact from IP Address");
    mvprintw(2, 1, "Add New Contact");

    mvprintw(0,0, ">");

    refresh();
    while((ch = getch()) != 'Q'){
        switch (ch){
            case KEY_UP:
                current = (((current-1)%3)+3)%3;
                break;
            case KEY_DOWN:
                current = (((current+1)%3)+3)%3;
                break;
            case KEY_RIGHT:
            case 10:
                clear();
                move(0,0);
                return current;
            default:
                break;
        }

        mvprintw(   (((current-1)%3)+3)%3   , 0, " ");
        mvprintw(current, 0, ">");
        mvprintw((((current+1)%3)+3)%3, 0, " ");
        move(current, 0);

        refresh();
    }
    return -1;
}

int selectFromContact(char* name, char* IP){
    int ch, i, selected, currentI;
    currentI = 0;
    int count;
    selected = 0;


    char* path = strdup("PRCS/contact.db");

    sqlite3* contactDB = setUpDB(path);
    List* records = initailize_list();
    findAllRecords(contactDB, records);
    i = 0;
    if( isEmptyList(records) ){
        mvprintw(0,0, "list empty\n");
        return 0;
    }else{
        Node* current = records->head;
        while(current){
            mvprintw(i, 1, "%-10s: %-46s\n", current->Name, current->IP);
            current = current->next;
            i++;
        }
    }
    mvprintw(0,0, ">");
    refresh();
    Node* current = records->head;
    while((ch = getch()) != 'Q'){

        switch (ch){
            case KEY_UP:
                mvprintw(currentI, 0, " ");
                currentI = (((currentI-1)%i)+i)%i;
                mvprintw(currentI, 0, ">");

                break;
            case KEY_DOWN:
                mvprintw(currentI, 0, " ");
                currentI = (((currentI+1)%i)+i)%i;
                mvprintw(currentI, 0, ">");
                break;
            case KEY_LEFT:
                clear();
                destroy_list(records);
                return 0;
            case KEY_RIGHT:
                clear();
                move(0,0);
                selected = 1;
                break;
            case 'D':
                count = currentI;
                while(count){
                    current = current->next;
                    count--;
                }
                deleteRecord(contactDB, current->Name);
                destroy_list(records);
                clear();
                return 0;
            
            default:
                break;
        }
        if(selected){
            Node* current = records->head;
            while(currentI){
                current = current->next;
                currentI--;
            }
            strcpy(IP, current->IP);
            strcpy(name, current->Name);
            destroy_list(records);
            return i;
        }
        refresh();
    }
    return -1;
}

int inputIP(char* name, char* IP){
    printw("IP Address:");  
    mvgetnstr(1, 0, IP, INET6_ADDRSTRLEN+1);
    strcpy(name, "they");
    return 1;
}

int createContact(char* name, char* IP, char *path){
    sqlite3* contactDB = setUpDB(path);
    
    mvprintw(0, 0, "Name: ");
    refresh();
    mvgetnstr(1, 0, name, NAME_LEN_LIMIT);
    clear();

    mvprintw(0, 0, "IP: ");
    refresh();
    mvgetnstr(1, 0, IP, INET6_ADDRSTRLEN+1);
    clear();


    insertRecord(contactDB, name, IP);

    return 0;
}

int getNameIP(char *name, char *IP){
    int choice;
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    while(1){
        choice = contactChoice();
        printf("choice %d\n", choice);
        echo();
        if(choice == 0){
            noecho();
            if(selectFromContact(name, IP)){
                break;
            }else{
                continue;
            }
            echo();
        }else if(choice == 1){
            if(inputIP(name, IP)){
                break;
            }else{
                continue;
            }
            
            
        }else if(choice == 2){
            char* path = strdup("PRCS/contact.db");
            createContact(name, IP, path);
            continue;
        }else {
            perror("unexpected return value from choices\n");
        }
        noecho();
    }
    


    clear();
    refresh();
    endwin();
    return 0;
}



int main(int argc, char *argv[]){
    int sockfd;
    bool isServer;
    char *msg = calloc(MSG_LEN_LIMIT, sizeof(char));
    char *ipAddr = calloc(INET6_ADDRSTRLEN, sizeof(char));
    char *name = calloc(NAME_LEN_LIMIT, sizeof(char));
    int *fd = malloc(sizeof(int));

    getNameIP(name, ipAddr);



    //establish connection
    if((sockfd = tryConenct(ipAddr)) < 0){
        //failed to connect, spin up the listenning procedure, this client will serve as server
        sockfd = spinServer();
        if(sockfd < 0){
            printf("server spin: failed\n");
            return 1;
        }else{
            printf("accepted: sockfd %d\n", sockfd);
            isServer = true;
        }
    }else{
        printf("connected\n");
        isServer = false;
    }
    
    //connection established, communication begins
    *fd = sockfd;

    //pad setup
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    int rows, cols, writeLine;
    getmaxyx(stdscr, rows, cols);
    int pad_height = 100;
    int pad_width = cols - 2;
    WINDOW *pad = newpad(pad_height, pad_width);
    if (pad == NULL) {
        endwin();
        fprintf(stderr, "Error creating pad\n");
        exit(1);
    }
    int pad_top = 0;
    int pad_left = 0;
    writeLine = 0;

    mvprintw(rows - 1, 0, "ENTER for messaging , 'Q' to quit: ");
    refresh();

    threadArgs *tArgs = (threadArgs*)malloc(sizeof(threadArgs));
    tArgs->maxCol = cols;
    tArgs->maxRow = rows;
    tArgs->pad = pad;
    tArgs->pad_left = &pad_left;
    tArgs->pad_top = &pad_top;
    tArgs->writeLine = &writeLine;
    tArgs->sockfd = fd;
    tArgs->name = strdup(name);


    pthread_create(&threads[0], NULL, &talk, tArgs);
    pthread_create(&threads[1], NULL, &listento, tArgs);
    
    pthread_join(threads[0], NULL);
    pthread_cancel(threads[1]);
    pthread_join(threads[1], NULL);

    free(ipAddr);
    free(msg);
    free(name);
    free(fd);
    free(tArgs);
    delwin(pad);
    endwin();
    printf("curse mode off\n");
    return 0;
}
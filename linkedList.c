#include "linkedList.h"

List* initailize_list(){
    List* linked = malloc(sizeof(List));
    linked->head = NULL;
    linked->tail = NULL;
    return linked;
}

int isEmptyList(List *list){
    if (list->head == NULL){
        return 1;
    }
    return 0;
}

int append_list(List *list, Node *node){
    if( isEmptyList(list) ){
        list->head = node;
        list->tail = node;
        return 0;
    }
    list->tail->next = node;
    list->tail = node;
    return 1;
}

void print_list(List *list){
    if( isEmptyList(list) ){
        printf("list empty\n");
    }else{
        Node* current = list->head;
        while(current){
            printf("%-32s: %-s\n", current->Name, current->IP);
            current = current->next;
        }
    }

}


int destroy_list(List *list){
    if( isEmptyList(list) ){
        free(list);
        return 0;
    }

    Node* current = list->head;

    while(current){
        free(current->Name);        
        free(current->IP);
        Node* temp = current;
        current = current->next;
        free(temp);    
    };
    free(list);
    return 1;
}


int test_linkList(){
    List *list = initailize_list();
    Node* new = malloc(sizeof(Node));
    new->IP = strdup("newIp");
    new->Name = strdup("newName");    

    Node* new1 = malloc(sizeof(Node));
    new1->IP = strdup("newIp1");
    new1->Name = strdup("newName1");    

    Node* new2 = malloc(sizeof(Node));
    new2->IP = strdup("newIp2");
    new2->Name = strdup("newName2");    

    append_list(list, new);
    append_list(list, new1);
    append_list(list, new2);
    print_list(list);

    destroy_list(list);

    return 0;
}
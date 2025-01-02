#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#define NAME_LEN_LIMIT 32

typedef struct Node{
    char *Name;
    char *IP;
    struct Node *next;
} Node;

typedef struct List{
    Node *head;
    Node *tail;
} List;

List* initailize_list();

int isEmptyList(List *list);

int append_list(List *list, Node *node);

int destroy_list(List *list);

void print_list(List *list);
#endif
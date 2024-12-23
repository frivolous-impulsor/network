#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

typedef struct Node{
    char *Name;
    char *IP;
    struct Node *next;
} Node;

typedef struct List{
    Node *head;
} List;

List *initailize_list();

int isEmptyList(List *list);

int append_list(List *list, Node *node);

void destroy_list(List *list);

#endif
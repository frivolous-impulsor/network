/*
compile shortcut:

gcc SRC/contactDB.c SRC/libsqlite3.a SRC/linkedList.c  -o SRC/contact
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <netinet/in.h>
#include "sqlite3.h"
#include "linkedList.h"


int findAllRecords(sqlite3 *db, List* result);

void findIPfromName(sqlite3 *db, char *name, char *resultIP);

int insertRecord(sqlite3 *db, char *name, char *IP);

int deleteRecord(sqlite3 *db, char *Id);

sqlite3* setUpDB(char* dbPath);
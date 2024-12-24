/*
compile shortcut:

gcc contact.c libsqlite3.a -o contact
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include "sqlite3.h"

static int display_records(void *unused, int count, char **data, char **columns){
    int i;
    for(i = 0; i < count; i++){
        printf("%16s ", data[i]);
    }
    printf("\n");
    return 0;
}

static int get_records(void *unused, int count, char **data, char **columns){
    int i;
    FILE *recordsFile;

    recordsFile = open("PRCS/records.csv", "a");

    char line[256] = {0};
    for(i = 0; i < count-1; i++){
        sprintf(line, "%s,", data[i]);
    }
    sprintf( "%s\n", data[i]);
    fprintf(recordsFile, line);
    
    close(recordsFile);
    return 0;
}

int findAllRecords(sqlite3 *db){
    char *errMsg = 0;
    char searchStmt[128] = {0};
    sprintf(searchStmt, "SELECT * FROM Friends");
    printf("%s\n",searchStmt);

    int rc = sqlite3_exec(db, searchStmt, get_records, 0, &errMsg);

    if(rc != SQLITE_OK){
        fprintf(stderr, "find all records error: %s\n", errMsg);
        sqlite3_free(errMsg);
        return 1;
    }
    return 0;
}

int findIPfromName(sqlite3 *db, char *name){
    char *errMsg = 0;
    char searchStmt[128] = {0};
    sprintf(searchStmt, "SELECT * FROM Friends WHERE Name = '%s'", name);
    printf("%s\n",searchStmt);

    int rc = sqlite3_exec(db, searchStmt, display_records, 0, &errMsg);

    if(rc != SQLITE_OK){
        fprintf(stderr, "find by name error: %s\n", errMsg);
        sqlite3_free(errMsg);
        return 1;
    }
    return 0;
}



int insertRecord(sqlite3 *db, char *name, char *IP){
    char *errMsg = 0;

    char insertStmt[128] = {0};
    sprintf(insertStmt, "INSERT INTO Friends(Name, IP) VALUES('%s', '%s')", name, IP);
    //printf("%s\n",insertStmt);
    int rc = sqlite3_exec(db, insertStmt, 0, 0, &errMsg);
    if(rc != SQLITE_OK){
        fprintf(stderr, "insert error: %s\n", errMsg);
        sqlite3_free(errMsg);
        return 1;
    }
    
    return 0;
}

int setUpDB(){
    sqlite3 *db;
    char *errMsg = 0;

    int rc = sqlite3_open("contact.db", &db);
    if(rc != SQLITE_OK){
        fprintf(stderr, "failed to open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
    }

    char *createTableStmt = "CREATE TABLE IF NOT EXISTS Friends(Id INTEGER PRIMARY KEY, Name TEXT UNIQUE, IP TEXT UNIQUE)";
    return 0;
}

int closeDB(){

}
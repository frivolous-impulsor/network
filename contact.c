/*
compile shortcut:

gcc contact.c libsqlite3.a -o contact
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

static int display_records(void *unused, int count, char **data, char **columns){
    int i;
    for(i = 0; i < count; i++){
        printf("%16s ", data[i]);
    }
    printf("\n");
    return 0;
}



int findAllRecords(sqlite3 *db){
    char *errMsg = 0;
    char searchStmt[128] = {0};
    sprintf(searchStmt, "SELECT * FROM Friends");
    printf("%s\n",searchStmt);

    int rc = sqlite3_exec(db, searchStmt, display_records, 0, &errMsg);

    if(rc != SQLITE_OK){
        fprintf(stderr, "find all records error: %s\n", errMsg);
        sqlite3_free(errMsg);
        return 1;
    }
    return 0;
}

static int get_IP(void *result, int count, char **data, char **columns){
    int i;


    char buf[INET6_ADDRSTRLEN] = {0};
    strcpy((char*)result, data[2]);

    
    return 0;
}

int findIPfromName(sqlite3 *db, char *name, char *resultIP){
    char *errMsg = 0;
    char searchStmt[128] = {0};
    sprintf(searchStmt, "SELECT * FROM Friends WHERE Name = '%s'", name);
    printf("%s\n",searchStmt);

    int rc = sqlite3_exec(db, searchStmt, get_IP, resultIP, &errMsg);

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

sqlite3* setUpDB(){
    sqlite3 *db;
    char *errMsg = 0;

    int rc = sqlite3_open("contact.db", &db);
    if(rc != SQLITE_OK){
        fprintf(stderr, "failed to open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return NULL;
    }

    char *createTableStmt = "CREATE TABLE IF NOT EXISTS Friends(Id INTEGER PRIMARY KEY, Name TEXT UNIQUE, IP TEXT UNIQUE)";
    rc = sqlite3_exec(db, createTableStmt, NULL, NULL, &errMsg);
    if(rc != SQLITE_OK){
        fprintf(stderr, "failed to create Friends: %s\n", errMsg);
        sqlite3_close(db);
        return NULL;
    }
    return db;
}

int main(){
    sqlite3 *contactDB = setUpDB();
    //insertRecord(contactDB, "oliver", "192.168.2.196");
    char result[INET6_ADDRSTRLEN] = {0};
    findIPfromName(contactDB, "oliver", result);

}
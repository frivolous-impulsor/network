/*
compile shortcut:

gcc contactDB.c libsqlite3.a linkedList.c  -o contact
*/

#include "contactDB.h"

static int display_records(void *unused, int count, char **data, char **columns){
    int i;
    for(i = 0; i < count; i++){
        printf("%16s ", data[i]);
    }
    printf("\n");
    return 0;
}

static int get_all_records(void *result, int count, char **data, char **columns){

    Node* node = (Node*)malloc(sizeof(Node));

    node->Name = malloc(NAME_LEN_LIMIT);
    strcpy(node->Name, data[1]);

    node->IP = malloc(INET6_ADDRSTRLEN);
    strcpy(node->IP, data[2]);

    append_list((List*)result, node);

    return 0;
}

int findAllRecords(sqlite3 *db, List* result){
    char *errMsg = 0;
    char searchStmt[128] = {0};
    sprintf(searchStmt, "SELECT * FROM Friends");
    
    int rc = sqlite3_exec(db, searchStmt, get_all_records, result, &errMsg);

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

void findIPfromName(sqlite3 *db, char *name, char *resultIP){
    char *errMsg = 0;
    char searchStmt[128] = {0};
    sprintf(searchStmt, "SELECT * FROM Friends WHERE Name = '%s'", name);
    //printf("%s\n",searchStmt);

    int rc = sqlite3_exec(db, searchStmt, get_IP, resultIP, &errMsg);

    if(rc != SQLITE_OK){
        fprintf(stderr, "find by name error: %s\n", errMsg);
        sqlite3_free(errMsg);
    }
}


int insertRecord(sqlite3 *db, char *name, char *IP){
    char *errMsg = 0;

    char insertStmt[1024] = {0};
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

int deleteRecord(sqlite3 *db, char *name){
    char *errMsg = 0;

    char insertStmt[128] = {0};
    sprintf(insertStmt, "DELETE FROM Friends WHERE name = '%s'", name);
    int rc = sqlite3_exec(db, insertStmt, 0, 0, &errMsg);
    if(rc != SQLITE_OK){
        //fprintf(stderr, "insert error: %s\n", errMsg);
        sqlite3_free(errMsg);
        return 1;
    }
    
    return 0;
}

sqlite3* setUpDB(char* dbPath){
    sqlite3 *db;
    char *errMsg = 0;

    int rc = sqlite3_open(dbPath, &db);
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

    insertRecord(db, "me", "::1");

    return db;
}

int test_db(){
    char* path = strdup("PRCS/test.db");
    sqlite3 *contactDB = setUpDB(path);
    List* records = initailize_list();
    printf("inited\n");
    char* name = malloc(NAME_LEN_LIMIT);
    char* IP = malloc(INET6_ADDRSTRLEN);
    strcpy(name, "test2");
    strcpy(IP, "12997890987898921299");
    
    insertRecord(contactDB, name, IP);
    printf("inserted");

    findAllRecords(contactDB, records);
    printf("found records\n");

    print_list(records);

    destroy_list(records);
    sqlite3_close(contactDB);
    return 0;
}
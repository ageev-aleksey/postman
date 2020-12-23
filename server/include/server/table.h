//
// Created by nrx on 23.12.2020.
//

#ifndef SERVER_TABLE_H
#define SERVER_TABLE_H
#include "smtp/state.h"
#include <sys/queue.h>
#define TABLE_MAX_KEY_LENGTH 128


struct table_vectors_entry {
    vector_smtp_mailbox array;
    char key[TABLE_MAX_KEY_LENGTH];
    TAILQ_ENTRY(table_vectors_entry) entries;
};

TAILQ_HEAD(table_vectors_list, table_vectors_entry);

typedef struct table {
    struct table_vectors_list list;
} table_t;

typedef struct table_key {
    char value[TABLE_MAX_KEY_LENGTH];
} table_key_t;

VECTOR_DECLARE(table_vector_keys, table_key_t);


bool table_init(table_t *table);
void table_free(table_t *table);
bool table_clear(table_t *table);
bool table_push(table_t *table, const char *key, smtp_mailbox *value);
vector_smtp_mailbox* table_get(table_t *table, const char *key);
bool table_keys(table_t *table, table_vector_keys *keys);




#endif //SERVER_TABLE_H

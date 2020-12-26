#include "server/table.h"
#include <string.h>
#include "vector.h"

struct table_vectors_entry* pr_table_get(table_t *table, const char *key) {
    struct table_vectors_entry *ptr = NULL;
    TAILQ_FOREACH(ptr, &table->list, entries) {
        if (strcmp(ptr->key, key) == 0) {
            return  ptr;
        }
    }
    return NULL;
}

bool table_init(table_t *table) {
    if (table == NULL) {
        return false;
    }
    TAILQ_INIT(&table->list);
    return true;
}
void table_free(table_t *table) {
    table_clear(table);
}
bool table_clear(table_t *table) {
    if (table == NULL) {
        return false;
    }
    struct table_vectors_entry *ptr = NULL;
    while(!TAILQ_EMPTY(&table->list)) {
        ptr = TAILQ_FIRST(&table->list);
        TAILQ_REMOVE(&table->list, ptr, entries);
        VECTOR_FREE(&ptr->array);
        free(ptr);
    }
    return true;
}
bool table_push(table_t *table, const char *key, smtp_mailbox *value) {
    if (table == NULL || key == NULL || value == NULL) {
        return false;
    }
    struct table_vectors_entry *ptr = pr_table_get(table, key);
    err_t error;
    if (ptr == NULL) {
        // make struct
        ptr = malloc(sizeof(struct table_vectors_entry));
        strcpy(ptr->key, key);
        VECTOR_INIT(smtp_mailbox, &ptr->array, error);
        TAILQ_INSERT_TAIL(&table->list, ptr, entries);
    }
        // insert
        VECTOR_PUSH_BACK(smtp_mailbox, &ptr->array, *value, error);
    return true;

}


vector_smtp_mailbox* table_get(table_t *table, const char *key) {
    if (table == NULL || key == NULL) {
        return false;
    }
    struct table_vectors_entry *ptr = pr_table_get(table, key);
    if (ptr != NULL) {
        return &ptr->array;
    }
    return NULL;
}

bool table_keys(table_t *table, table_vector_keys *keys) {
    if (table == NULL) {
        return false;
    }
    struct table_vectors_entry *ptr = NULL;
    TAILQ_FOREACH(ptr, &table->list, entries) {
        table_key_t key;
        strcpy(key.value, ptr->key);
        err_t err;
        VECTOR_PUSH_BACK(table_key_t, keys, key ,err);
    }
    return true;
}

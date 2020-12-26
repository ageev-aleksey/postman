#include "server/users_list.h"


void user_free(user_context *context)  {
    if (context != NULL) {
        VECTOR_FREE(&context->buffer);
        VECTOR_FREE(&context->write_buffer);
        smtp_free(&context->smtp);
    }
}

bool users_list__init(users_list *users) {
    TAILQ_INIT(&users->pr_list);
    pthread_mutex_init(&users->pr_mutex, NULL);
    return true;
}

void users_list__free(users_list *users) {
    if (users != NULL) {
        pthread_mutex_destroy(&users->pr_mutex);
        while (!TAILQ_EMPTY(&users->pr_list)) {
            struct user_context_entry *ptr = TAILQ_FIRST(&users->pr_list);
            TAILQ_REMOVE(&users->pr_list, ptr, pr_entries);
            user_free(ptr->pr_context);
            free(ptr->pr_context);
            free(ptr);
        }
    }
}

bool users_list__add(users_list *users, user_context **user) {
    user_context_entry *entry = malloc(sizeof(user_context_entry));
    entry->pr_context = *user;
    pthread_mutex_lock(&users->pr_mutex);
    TAILQ_INSERT_TAIL(&users->pr_list, entry, pr_entries);
    pthread_mutex_unlock(&users->pr_mutex);
    *user = NULL;
    return true;
}

bool users_list__user_find_by_sock(users_list *users, user_accessor *accessor, int sock) {
    user_context_entry *ptr = NULL;
    bool is_found = false;
    pthread_mutex_lock(&users->pr_mutex);
    TAILQ_FOREACH(ptr, &users->pr_list, pr_entries) {
        if (ptr->pr_context->socket == sock) {
            TAILQ_REMOVE(&users->pr_list, ptr, pr_entries);
            is_found = true;
            break;
        }
    }
    pthread_mutex_unlock(&users->pr_mutex);

    if (is_found) {
        accessor->user = ptr->pr_context;
        accessor->pr_list_entry = ptr;
        accessor->pr_users_list = users;
    }

    return is_found;
}

bool users_list__is_exist(users_list *users, int sock) {
    if (users == NULL) {
        return false;
    }
    bool ret = false;
    pthread_mutex_lock(&users->pr_mutex);
    user_context_entry *ptr = NULL;
    TAILQ_FOREACH(ptr, &users->pr_list, pr_entries) {
        if (ptr->pr_context->socket == sock) {
            ret = true;
            break;
        }
    }
    pthread_mutex_unlock(&users->pr_mutex);
    return ret;
}

void user_accessor_release(user_accessor *accessor) {
    if (accessor != NULL && accessor->user != NULL &&
        accessor->pr_users_list != NULL && accessor->pr_list_entry != NULL)
    {
        pthread_mutex_lock(&accessor->pr_users_list->pr_mutex);
        TAILQ_INSERT_TAIL(&accessor->pr_users_list->pr_list, accessor->pr_list_entry, pr_entries);
        pthread_mutex_unlock(&accessor->pr_users_list->pr_mutex);
        accessor->pr_list_entry = NULL;
        accessor->pr_users_list = NULL;
        accessor->user = NULL;
    }

}

void users_list__delete_user(user_accessor *accessor) {
    free(accessor->pr_list_entry);
    accessor->pr_users_list = NULL;
    accessor->pr_list_entry = NULL;
}

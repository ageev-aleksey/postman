#include <stdio.h>

typedef struct message {
    char *from;
    char *to;
    char *date;
    char **strings;
    char *directory;
    int strings_size;
} message;

typedef struct maildir_user {
    char *directory;
    char *username;

    char **message_full_file_names;
    int messages_size;
} maildir_user;

typedef struct maildir_other_server {
    char **message_full_file_names;
    int messages_size;
} maildir_other_server;

typedef struct maildir_main {
    char *directory;
    maildir_other_server servers;
    maildir_user *users;
    int users_size;
} maildir_main;

maildir_main *init_maildir(char *directory);
void update_maildir(maildir_main *maildir);
void output_maildir(maildir_main *maildir);
message *read_message(char *filepath);
void finalize_maildir(maildir_main *maildir);
void remove_message(maildir_main *maildir, message *mess);
//void read_maildir_users_message(char *directory);
//void read_maildir_servers_users(char *directory);
//void read_maildir_servers_users_message(char *directory);

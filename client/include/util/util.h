#include <sys/queue.h>
#include <stddef.h>
#include <stdarg.h>
#include <resolv.h>

#define strsize(args...) snprintf(NULL, 0, args) + sizeof('\0')
#define vstrsize(args...) snprintf(NULL, 0, args) + sizeof('\0')

typedef struct pair {
    char *first;
    char *second;
} pair;

typedef struct string {
    char *chars;
    size_t length;
} string;

typedef struct string_tokens {
    string *tokens;
    size_t count_tokens;
} string_tokens;

long int convert_string_to_long_int(const char *str);

string_tokens split(const char *const str, const char *const delim);
void free_string_tokens(string_tokens *tokens);

string* get_string_from_characters(string *str, char *characters);
string* add_character(string *str, char character);
void free_string(string *str);

void trim(char *str);
void* allocate_memory(size_t bytes);
void* reallocate_memory(void* buffer, size_t bytes);

int resolvmx(const char *name, char **mxs, int limit);
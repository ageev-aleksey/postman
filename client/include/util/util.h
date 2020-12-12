#include <stdlib.h>

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

string* get_string(string *str, char character);
void free_string(string *str);
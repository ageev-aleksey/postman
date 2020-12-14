#include <string.h>
#include "logs.h"
#include "util.h"

long int convert_string_to_long_int(const char *str) {
    char *end;
    return strtol(str, &end, 10);
}

int check_equation_delim(const char *const str, const char *const delim, int offset) {
    int i = 0;
    while (str[offset] == delim[i]) {
        offset++;
        i++;
    }

    if (i == strlen(delim)) {
        return offset;
    }

    return -1;
}

string_tokens split(const char *const str, const char *const delim) {

    int count_tokens = 0;
    string *tokens = allocate_memory((sizeof(*tokens)));

    string *token = allocate_memory(sizeof(*token));
    token->chars = 0;
    token->length = 0;

    for (int i = 0; i < strlen(str) + 1; i++) {
        int offset;
        if ((offset = check_equation_delim(str, delim, i)) != -1 || str[i] == '\0') {
            if (offset != -1) {
                i = offset - 1;
            }
            if (token->length != 0) {
                tokens[count_tokens] = *token;
                count_tokens++;
                token = allocate_memory(sizeof(*token));
                token->chars = 0;
                token->length = 0;
                tokens = reallocate_memory(tokens, sizeof(*tokens) * (count_tokens + 1));
                continue;
            }
        }
        add_character(token, str[i]);
    }

    string_tokens string_tokens;
    string_tokens.tokens = tokens;
    string_tokens.count_tokens = count_tokens;

    return string_tokens;
}

void free_string_tokens(string_tokens *tokens) {
    free_string(tokens->tokens);
}

string *get_string_from_characters(string *str, char *characters) {
    size_t capacity = str->length + 1;
    char *s;
    if (str->chars == NULL) {
        str->length = 0;
        s = (char *) allocate_memory(sizeof(char));
    }

    for (int i = 0; i < strlen(characters); i++) {
        s[(str->length)++] = characters[i];

        if (str->length >= capacity) {
            capacity *= 2; // увеличиваем ёмкость строки в два раза
            s = (char *) reallocate_memory(s, capacity); // создаём новую строку с увеличенной ёмкостью
        }
    }

    s[str->length] = '\0';

    str->chars = s;

    return str;
}

string *add_character(string *str, char character) {
    size_t capacity = str->length + 1;
    char *s;
    if (str->chars == NULL) {
        str->length = 0;
        s = (char *) allocate_memory(sizeof(char));
    }

    s[(str->length)++] = character;

    if (str->length >= capacity) {
        capacity *= 2; // увеличиваем ёмкость строки в два раза
        s = (char *) reallocate_memory(s, capacity); // создаём новую строку с увеличенной ёмкостью
    }

    s[str->length] = '\0';

    str->chars = s;

    return str;
}

void free_string(string *str) {
    free(str->chars);
    free(str);
}

void* allocate_memory(size_t bytes) {
    void *buffer = malloc(bytes);

    if (buffer != NULL) {
        return buffer;
    }

    LOG_ERROR("Ошибка выделения памяти", NULL);
    return NULL;
}

void* reallocate_memory(void *buffer, size_t bytes) {
    buffer = realloc(buffer, bytes);

    if (buffer != NULL) {
        return buffer;
    }

    LOG_ERROR("Ошибка перераспределения памяти", NULL);
    return NULL;
}
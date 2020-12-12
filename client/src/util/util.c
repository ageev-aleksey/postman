#include <string.h>
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
    string *tokens = malloc(sizeof(*tokens));

    string *token = malloc(sizeof(*token));
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
                token = malloc(sizeof(*token));
                token->chars = 0;
                token->length = 0;
                tokens = realloc(tokens, sizeof(*tokens) * (count_tokens + 1));
                continue;
            }
        }
        get_string(token, str[i]);
    }

    string_tokens string_tokens;
    string_tokens.tokens = tokens;
    string_tokens.count_tokens = count_tokens;

    return string_tokens;
}

void free_string_tokens(string_tokens *tokens) {
    free_string(tokens->tokens);
}

string* get_string(string *str, char character) {
    size_t capacity = str->length + 1;
    char *s;
    if (str->chars == NULL) {
        str->length = 0;
        s = (char *) malloc(sizeof(char));
    }

    s[(str->length)++] = character;

    if (str->length >= capacity) {
        capacity *= 2; // увеличиваем ёмкость строки в два раза
        s = (char *) realloc(s, capacity); // создаём новую строку с увеличенной ёмкостью
    }

    s[str->length] = '\0';

    str->chars = s;

    return str;
}

void free_string(string *str) {
    free(str->chars);
    free(str);
}
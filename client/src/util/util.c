#include "util.h"

long int convert_string_to_long_int(const char *string) {
    char *end;
    return strtol (string, &end, 10);
}


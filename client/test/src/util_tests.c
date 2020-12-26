#include <CUnit/Basic.h>
#include <malloc.h>
#include "util/util.h"
#include "util_tests.h"

int util_test_init() {
    return 0;
}

int util_test_clean() {
    return 0;
}

void convert_string_to_long_int_test() {
    printf("Тест конвертирования строки\n");
    char *number = "12345";
    long int num = convert_string_to_long_int(number);
    CU_ASSERT_EQUAL(num, 12345);
}

void split_test() {
    printf("Тест получения токенов по строке\n");
    char *string = "Каждый охотник желает знать где сидит фазан";
    char *delim = " ";
    string_tokens tokens = split(string, delim);
    CU_ASSERT_EQUAL(tokens.count_tokens, 7);
    free_string_tokens(&tokens);
}

void trim_test() {
    printf("Тест удаления пробелов из строки\n");
    char *string;
    asprintf(&string, "%s", "  Каждый охотник ");
    trim(string);
    CU_ASSERT_NOT_EQUAL(string[0], ' ');
    CU_ASSERT_NOT_EQUAL(string[strlen(string) - 1], ' ');
    free(string);
}
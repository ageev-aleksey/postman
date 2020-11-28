//
// Created by nrx on 28.11.2020.
//
#include "smtp_regex_test.h"
#include "smtp/regex.h"
#include <sys/types.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>

int smtp_regex_test_init() {
    return 0;
}
int smtp_regex_test_clean() {
    return 0;
}

void smtp_regex_ipv4_test() {
    regex_t reg;
    regmatch_t matcher[2];
    char line[] = "192.168.1.1 hello 127.0.0.1";
    regcomp(&reg, RE_IPv4, REG_EXTENDED | REG_ICASE);
    if(regexec(&reg, line, 2, matcher, 0) == 0) {
        printf("\n%d-%d\n", matcher[0].rm_so, matcher[0].rm_eo);
        printf("%d-%d\n", matcher[1].rm_so, matcher[1].rm_eo);
    }

}
void smtp_regex_hello_test() {

}

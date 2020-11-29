//
// Created by nrx on 28.11.2020.
//
#include "smtp_regex_test.h"
#include "smtp/regex.h"
#include <sys/types.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <CUnit/Basic.h>

int smtp_regex_test_init() {
    return 0;
}
int smtp_regex_test_clean() {
    return 0;
}

void smtp_regex_ipv4_test() {
    regex_t reg;
    regmatch_t matcher[3];
    char line[] = "192.168.1.1 hello 127.0.0.1";
    regcomp(&reg, RE_IPv4, REG_EXTENDED | REG_ICASE);
    if(regexec(&reg, line, 3, matcher, 0) == 0) {
        CU_ASSERT_EQUAL(matcher[0].rm_so, 0);
        CU_ASSERT_EQUAL(matcher[0].rm_eo, 11);
        if(regexec(&reg, line + matcher[0].rm_eo, 3, matcher, 0) == 0) {
            CU_ASSERT_EQUAL(matcher[0].rm_so, 7);
            CU_ASSERT_EQUAL(matcher[0].rm_eo, 16);
        } else {
            CU_FAIL("Error match second ip");
        }
    } else {
        CU_FAIL("Error match first ip");
    }

    regfree(&reg);

}
void smtp_regex_hello_test() {
    char line[] = "helo [192.168.1.1]";
    char line2[] = "Ehlo <test.domain.local>";
    regex_t reg_hello;
    regex_t reg_addr;
    regex_t reg_domain;
    regmatch_t matcher_hello[1];
    regmatch_t matcher_addr[1];
    regmatch_t matcher_domain[1];
    int re = regcomp(&reg_hello, RE_HELLO, REG_EXTENDED | REG_ICASE );
    re = regcomp(&reg_addr, RE_ADDRESS_LITERAL, REG_EXTENDED | REG_ICASE );
    re = regcomp(&reg_domain, RE_DOMAIN_LITERAL, REG_EXTENDED | REG_ICASE);
    if (regexec(&reg_hello, line, 1, matcher_hello, 0) == 0) {
        CU_ASSERT_EQUAL(matcher_hello[0].rm_so, 0);
        CU_ASSERT_EQUAL(matcher_hello[0].rm_eo, 4);
    } else {
        CU_FAIL("Error find substring: 'helo'");
    }

    if (regexec(&reg_addr, line, 1, matcher_addr, 0) == 0) {
        CU_ASSERT_EQUAL(matcher_addr[0].rm_so, 5);
        CU_ASSERT_EQUAL(matcher_addr[0].rm_eo, 18);
    } else {
        CU_FAIL("Error find substring: '[192.168.1.1]'");
    }

    if (regexec(&reg_domain, line2, 1, matcher_domain, 0) == 0) {
        CU_ASSERT_EQUAL(matcher_domain[0].rm_so, 5);
        CU_ASSERT_EQUAL(matcher_domain[0].rm_eo, 24);
    } else {
        CU_FAIL("Error find substring: '<test.domain.local>'")
    }
    regfree(&reg_hello);
    regfree(&reg_addr);
    regfree(&reg_domain);
}

void smtp_regex_domain_route_list_test() {
    char line[] = "<@super.com.ru , @server.com,@local.local :   user@mail.teach.ru>";
    regex_t reg;
    regmatch_t matcher[1];
    int re = regcomp(&reg, RE_PATH, REG_EXTENDED | REG_ICASE);
    if(re != 0) {
        CU_FAIL("Invalid regex RE_PATH");
    } else {
        if (regexec(&reg, line, 1, matcher, 0) == 0) {
            CU_ASSERT_EQUAL(matcher[0].rm_so, 0);
            CU_ASSERT_EQUAL(matcher[0].rm_eo, 65);
        } else {
            CU_FAIL("Error match RE_PATH");
        }
        regfree(&reg);

        re = regcomp(&reg, RE_MAILBOX, REG_EXTENDED | REG_ICASE);
        if (re != 0) {
            CU_FAIL("Invalid regex RE_MAILBOX");
        } else {
            if (regexec(&reg, line, 1, matcher, 0) == 0) {
                CU_ASSERT_EQUAL(matcher[0].rm_so, 46);
                CU_ASSERT_EQUAL(matcher[0].rm_eo, 64);
            } else {
                CU_FAIL("Error match RE_MAILBOX");
            }
            regfree(&reg);
        }

    }
}

void smtp_regex_mail_from_test() {
    char list[] = "02120 Mail FROM : <> .0022";
    char list2[] = "mail from : <@dom.ru:man1234@mail.ru>";

    regex_t reg;
    regmatch_t matcher[1];
    int res = regcomp(&reg, RE_MAIL_FROM, REG_EXTENDED | REG_ICASE);
    if (res != NULL) {
        CU_FAIL("error compile regex RE_MAIL_FROM");
    } else {
        if (regexec(&reg, list, 1, matcher, 0) == 0) {

        } else {
            // TODO (ageev) доделать тесты 
        }
    }

}

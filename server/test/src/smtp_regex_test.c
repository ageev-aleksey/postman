//
// Created by nrx on 28.11.2020.
//
#include "smtp_regex_test.h"
#include "smtp/regex.h"
#include "smtp/state.h"
#include <sys/types.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <CUnit/Basic.h>

int smtp_regex_test_init() {
    smtp_lib_init();
    return 0;
}
int smtp_regex_test_clean() {
    smtp_lib_free();
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
    regcomp(&reg_hello, RE_HELLO, REG_EXTENDED | REG_ICASE );
    regcomp(&reg_addr, RE_ADDRESS_LITERAL, REG_EXTENDED | REG_ICASE );
    regcomp(&reg_domain, RE_DOMAIN_LITERAL, REG_EXTENDED | REG_ICASE);
    if (regexec(&reg_hello, line, 1, matcher_hello, 0) == 0) {
        CU_ASSERT_EQUAL(matcher_hello[0].rm_so, 0);
        CU_ASSERT_EQUAL(matcher_hello[0].rm_eo, 5);
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

    regex_t reg_mail_from;
    regex_t  reg_path;
    regex_t reg_mailbox;
    regmatch_t matcher[1];
     int res = regcomp(&reg_mail_from, RE_MAIL_FROM, REG_EXTENDED | REG_ICASE);
    if (res != 0 ) {
        CU_FAIL("error compile regex RE_MAIL_FROM");
        return;
    }
     //int res = regcomp(&reg, "mail[[:space:]]+from[[:space:]]*:[[:space:]]*", REG_EXTENDED | REG_ICASE);
    res = regcomp(&reg_path, RE_MAIL_FROM_PATH, REG_EXTENDED | REG_ICASE);
    if (res != 0 ) {
        CU_FAIL("error compile regex RE_MAIL_FROM_PATH");
        goto exit;
    }
    res = regcomp(&reg_mailbox, RE_MAILBOX, REG_EXTENDED | REG_ICASE);
    if (res != 0 ) {
        CU_FAIL("error compile regex RE_MAILBOX");
        goto exit;
    }


    if (regexec(&reg_mail_from, list, 1, matcher, 0) == 0) {
        CU_ASSERT_EQUAL(matcher[0].rm_so, 6);
        CU_ASSERT_EQUAL(matcher[0].rm_eo, 18);
    } else {
        CU_FAIL("Error matching 'MAIL FROM' with empty reverse path");
        goto exit;
    }

    if (regexec(&reg_mail_from, list2, 1, matcher, 0) == 0) {
        CU_ASSERT_EQUAL(matcher[0].rm_so, 0);
        CU_ASSERT_EQUAL(matcher[0].rm_eo, 12);
    } else {
        CU_FAIL("Error matching 'MAIL FROM' with filled route list reverse path");
        goto exit;
    }

    if (regexec(&reg_mailbox, list2, 1, matcher, 0 ) == 0) {
        CU_ASSERT_EQUAL(matcher[0].rm_so, 21);
        CU_ASSERT_EQUAL(matcher[0].rm_eo, 36);
    } else {
        CU_FAIL("Error matching 'MAIL BOX' regex from 'MAIL FROM' command");
        goto exit;
    }
exit:
    regfree(&reg_mail_from);
    regfree(&reg_path);
    regfree(&reg_mailbox);
}

void smtp_regex_rcpt_to_test() {
    char line[] = "rcpt to : <postmaster@local.ru>";
    regex_t  reg_rcpt;
    regex_t reg_domain;
    regmatch_t matcher[1];
    int res = regcomp(&reg_rcpt, RE_RCPT_TO, REG_EXTENDED | REG_ICASE);
    if (res != 0) {
        CU_FAIL("Error compile regex: RE_RCPT_TO");
        return;
    }
    res = regcomp(&reg_domain, RE_MAILBOX, REG_EXTENDED | REG_ICASE);
    if (res != 0) {
        CU_FAIL("Error compile regex: RE_DOMAIN");
        goto exit;
    }

    if (regexec(&reg_rcpt, line, 1, matcher, 0) == 0) {
        CU_ASSERT_EQUAL(matcher[0].rm_so, 0);
        CU_ASSERT_EQUAL(matcher[0].rm_eo, 10);
    } else {
        CU_FAIL("Error matching 'RCPT TO'");
        goto exit;
    }

    if (regexec(&reg_domain, line, 1, matcher, 0) == 0) {
        CU_ASSERT_EQUAL(matcher[0].rm_so, 11);
        CU_ASSERT_EQUAL(matcher[0].rm_eo, 30);
    } else {
        CU_FAIL("Error matching 'domain'");
        goto exit;
    }
exit:
    regfree(&reg_rcpt);
    regfree(&reg_domain);

}

extern struct smtp_command pr_smtp_hello_proc(smtp_state *smtp, char *buff);
void smtp_hello_test() {
    char ip[]  = "192.168.1.1";
    char line[] = "ehlo [192.168.1.1]";
    char domain[] = "test.domain.ru";
    char line2[] = "ehlo test.domain.ru";
    smtp_state smtp;
    smtp_init(&smtp, NULL);
    struct smtp_command command = pr_smtp_hello_proc(&smtp, line);
    smtp_address *addr = command.arg;
    CU_ASSERT_TRUE(command.status);
    CU_ASSERT_EQUAL(addr->type, SMTP_ADDRESS_TYPE_IPv4);
    CU_ASSERT_STRING_EQUAL(addr->address, ip);
    free(addr->address);
    free(addr);

    command = pr_smtp_hello_proc(&smtp, line2);
    addr = command.arg;
    CU_ASSERT_TRUE(command.status);
    CU_ASSERT_EQUAL(addr->type, SMTP_ADDRESS_TYPE_DOMAIN);
    CU_ASSERT_STRING_EQUAL(addr->address, domain);
    free(addr->address);
    free(addr);

    smtp_free(&smtp);
}

extern struct smtp_command pr_smtp_mailfrom_proc(smtp_state *smtp, char *buff);
void smtp_mailform_test() {
    char line[] = "mail from: <>";
    char line2[] = "mail from: <@server1.ru, @server2.com: user@domain.ru>";
    char server_name[] = "domain.ru";
    char user_name[] = "user";
    smtp_state smtp;
    smtp_init(&smtp, NULL);

    struct smtp_command command = pr_smtp_mailfrom_proc(&smtp, line);
    CU_ASSERT_TRUE(command.status);
    CU_ASSERT_PTR_NULL(command.arg);

    command = pr_smtp_mailfrom_proc(&smtp, line2);
    smtp_mailbox *mailbox = command.arg;
    CU_ASSERT_TRUE(command.status);
    CU_ASSERT_STRING_EQUAL(mailbox->server_name, server_name)
    CU_ASSERT_STRING_EQUAL(mailbox->user_name, user_name)
    free(mailbox->user_name);
    free(mailbox->server_name);
    free(mailbox);
    smtp_free(&smtp);
}
extern struct smtp_command pr_smtp_rcptto_proc(smtp_state *smtp, char *buff);
void smtp_rcptto_test() {
    char line[] = "rcpt to: <@server1.ru: hello@email.ru>";
    char server_name[] = "email.ru";
    char user_name[] = "hello";
    smtp_state smtp;
    smtp_init(&smtp, NULL);
    struct smtp_command command = pr_smtp_rcptto_proc(&smtp, line);
    CU_ASSERT_TRUE(command.status);

    smtp_mailbox *mailbox = command.arg;
    CU_ASSERT_STRING_EQUAL(mailbox->server_name, server_name);
    CU_ASSERT_STRING_EQUAL(mailbox->user_name, user_name);
    free(mailbox->user_name);
    free(mailbox->server_name);
    free(mailbox);

    smtp_free(&smtp);
}

void smtp_protocol_good_sequence() {
    char hello[] = "ehlo [127.0.0.1]\r\n";
    char mailfrom[] = "mail from: <testuser@test.domain.com.ru>\r\n";
    char rcptto1[] = "rcpt to: <@good.ru, @domain200.org, @email.ru: otheruser@server.ru>\r\n";
    char rcptto2[] = "rcpt to: <support@n1.server.ru>\r\n";
    char data[] = "data\r\n";
    char buffer1[] = "This is important email.\r\n";
    char buffer2[] = "text text text text\r\n.\r\n";

    smtp_state smtp;
    smtp_init(&smtp, NULL);
    char *reply = NULL;
    smtp_status s = smtp_parse(&smtp, hello, &reply, NULL);
    CU_ASSERT_EQUAL(s, SMTP_STATUS_OK);
    s = smtp_parse(&smtp, mailfrom, &reply, NULL);
    CU_ASSERT_EQUAL(s, SMTP_STATUS_OK);
    s = smtp_parse(&smtp, rcptto1, &reply, NULL);
    CU_ASSERT_EQUAL(s, SMTP_STATUS_OK);
    s = smtp_parse(&smtp, rcptto2, &reply, NULL);
    CU_ASSERT_EQUAL(s, SMTP_STATUS_OK);
    s = smtp_parse(&smtp, data, &reply, NULL);
    CU_ASSERT_EQUAL(s, SMTP_STATUS_OK);
    s = smtp_parse(&smtp, buffer1, &reply, NULL);
    CU_ASSERT_EQUAL(s, SMTP_STATUS_CONTINUE);
    s = smtp_parse(&smtp, buffer2, &reply, NULL);
    CU_ASSERT_EQUAL(s, SMTP_STATUS_CONTINUE);

    smtp_free(&smtp);
}
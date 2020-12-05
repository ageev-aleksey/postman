//
// Created by nrx on 28.11.2020.
//

#ifndef SERVER_SMTP_REGEX_TEST_H
#define SERVER_SMTP_REGEX_TEST_H

int smtp_regex_test_init();
int smtp_regex_test_clean();

void smtp_regex_ipv4_test();
void smtp_regex_hello_test();
void smtp_regex_domain_route_list_test();
void smtp_regex_mail_from_test();
void smtp_regex_rcpt_to_test();

void smtp_hello_test();
void smtp_mailform_test();
void smtp_rcptto_test();
void smtp_protocol_good_sequence();


#endif //SERVER_SMTP_REGEX_TEST_H

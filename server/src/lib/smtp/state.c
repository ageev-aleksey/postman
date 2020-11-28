#include "smtp/state.h"
#include "smtp/regex.h"

#define VECTOR_ERROR(first_error_, second_error_) \
do {                                              \
       if ((first_error_).error) {\
if ((second_error_) != NULL) { \
*(second_error_) = (first_error_); \
} \
return false;\
}               \
} while(0);

const char SMTP_DESCRIPTOR_IS_NULL[] = "pointer of smtp_state is null";
const char SMTP_MESSAGE_IS_NULL[] = "pointer of smtp message is null";

bool smtp_init(smtp_state *smtp, error_t *error) {
    CHECK_PTR(smtp, error, SMTP_DESCRIPTOR_IS_NULL);
    ERROR_SUCCESS(error);
    smtp->pr_fsm_state = AUTOFSM_ST_INIT;
    error_t  err;
    VECTOR_INIT(recipient, &smtp->pr_rcpt_list, err);
    VECTOR_ERROR(err, error);
    smtp->hello_addr = NULL;
    smtp->pr_mail_from = NULL;
    smtp->hello_addr = NULL;
    VECTOR_INIT(char, &smtp->pr_mail_data, err);
    VECTOR_ERROR(err, error);
    return true;
}
void smtp_free(smtp_state *smtp) {
    if (smtp != NULL) {
        VECTOR_FREE(&smtp->pr_rcpt_list);
        free(smtp->hello_addr);
        free(smtp->pr_mail_from);
        free(smtp->hello_addr);
        VECTOR_FREE(&smtp->pr_mail_data);
    }
}


smtp_status smtp_parse(smtp_state *smtp, const char *message, char **buffer_reply, error_t *error) {
    CHECK_PTR(smtp, error, SMTP_DESCRIPTOR_IS_NULL);
    CHECK_PTR(smtp, error, SMTP_MESSAGE_IS_NULL);
    ERROR_SUCCESS(error);
}
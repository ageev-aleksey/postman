#define RE_DOMAIN "([[:alnum:]]+\\.)+[[:alnum:]]+"
#define RE_IPv4 "((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)"
#define RE_IPv6 " "
#define RE_GENERAL_ADDRESS_LITERAL " "
#define RE_ADDRESS_LITERAL  "\\[" RE_IPv4 "\\]"
//#define RE_HELLO_ADDR "(" RE_DOMAIN ")|(" RE_ADDRESS_LITERAL ")"
#define RE_DOMAIN_LITERAL "<" RE_DOMAIN ">"
#define RE_AT_DOMAIN "@" RE_DOMAIN
#define RE_MAILBOX "[[:alnum:]]+" RE_AT_DOMAIN // TODO (ageev) если имя содеожит '_' то регулярное выражение не сопоставляется
//#define RE_PATH "<((" RE_AT_DOMAIN ",)*("RE_AT_DOMAIN":)){0,1}" RE_MAIL_BOX ">"
#define RE_ROUTE_PATH "((" RE_AT_DOMAIN "[[:space:]]*,[[:space:]]*)*" RE_AT_DOMAIN "[[:space:]]*:[[:space:]]*){0,1}"
#define RE_PATH "<" RE_ROUTE_PATH RE_MAILBOX ">"
#define RE_SERVER_NAME "(" RE_ADDRESS_LITERAL ")|(" RE_DOMAIN_LITERAL ")";
#define RE_HELLO "((ehlo)|(helo))[[:space:]]+"
#define RE_EMPTY_PATH "<>"
#define RE_MAIL_FROM_PATH "((" RE_EMPTY_PATH ")|(" RE_PATH "))"
#define RE_MAIL_FROM "mail[[:space:]]+from[[:space:]]*:[[:space:]]*"
#define RE_RCPT_DOMAIN "<(postmaster" RE_AT_DOMAIN ")|(postmaster)|(" RE_PATH ")>"
#define RE_RCPT_TO "rcpt[[:space:]]+to[[:space:]]*:[[:space:]]*"
#define RE_DATA "data"
#define RE_RSET "rset"
#define RE_VRFY "vrfy"
#define RE_EXPN "expn"
#define RE_HELP "help"
#define RE_NOOP "noop"
#define RE_QUIT "quit"


//("\\[(" RE_IPv4 ")|(" RE_IPv6 ")|(" RE_GENERAL_ADDRESS_LITERAL ")\\]")
//"(helo)/(ehlo) ((" + RE_DOMAIN + ")/(" + RE_ADDRESS_LITERAL + ")){0, 1}";
#define RE_DOMAIN "([[:alnum:]]+\\.)+[[:alnum:]]+"
#define RE_IPv4 "((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)"
#define RE_IPv6 " "
#define RE_GENERAL_ADDRESS_LITERAL " "
#define RE_ADDRESS_LITERAL  "\\[" RE_IPv4 "\\]"
#define RE_DOMAIN_LITERAL "<" RE_DOMAIN ">"
#define RE_AT_DOMAIN "@" RE_DOMAIN
#define RE_MAILBOX "[[:alnum:]]+@" RE_DOMAIN
//#define RE_PATH "<((" RE_AT_DOMAIN ",)*("RE_AT_DOMAIN":)){0,1}" RE_MAIL_BOX ">"
#define RE_ROUTE_PATH "((" RE_AT_DOMAIN "[[:space:]]*,[[:space:]]*)*" RE_AT_DOMAIN "[[:space:]]*:[[:space:]]*){0,1}"
#define RE_PATH "<" RE_ROUTE_PATH RE_MAILBOX">"
#define RE_SERVER_NAME "(" RE_ADDRESS_LITERAL ")|(" RE_DOMAIN_LITERAL ")";
#define RE_HELLO "(helo)|(ehlo)"
#define RE_MAIL_FROM "mail from[[:space:]]*:[[:space:]]*(<>)|(" RE_PATH ")"


//("\\[(" RE_IPv4 ")|(" RE_IPv6 ")|(" RE_GENERAL_ADDRESS_LITERAL ")\\]")
//"(helo)/(ehlo) ((" + RE_DOMAIN + ")/(" + RE_ADDRESS_LITERAL + ")){0, 1}";
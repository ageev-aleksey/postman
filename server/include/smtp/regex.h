#define RE_DOMAIN "([[:alnum:]]+\\.)+[[:alnum:]]+"
#define RE_IPv4 "((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)"
#define RE_ADDRESS_LITERAL "\[(" + RE_IPv4 + ")\(" + RE_IPv6 + ")\(" + RE_GENERAL_ADDRESS_LITERAL + ")\]";
#define RE_HELLO "(helo)/(ehlo) ((" + RE_DOMAIN + ")/(" + RE_ADDRESS_LITERAL + ")){0, 1}";
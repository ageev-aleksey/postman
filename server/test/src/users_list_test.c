#include "users_list_test.h"
#include "server/global_context.h"
#include <CUnit/Basic.h>


int users_list_test_init() {
    return 0;
}
int users_list_test_clean() {
    return 0;
}

void users_list_add_and_find_test() {
    const int SOCKET = 5;
    users_list list;
    users_list__init(&list);
    user_context *context = malloc(sizeof(struct user_context));
    context->socket = SOCKET;
    users_list__add(&list, &context);
    user_accessor  acc;
    if (users_list__user_find_by_sock(&list, &acc, SOCKET )) {
        CU_ASSERT_EQUAL(acc.user->socket, SOCKET);
        context = acc.user;
        user_accessor  acc2;
        bool resf = users_list__user_find_by_sock(&list, &acc2, SOCKET);
        CU_ASSERT_FALSE(resf);
        user_accessor_release(&acc);
        resf = users_list__user_find_by_sock(&list, &acc2, SOCKET);
        CU_ASSERT_TRUE(resf);
    } else {
        CU_FAIL("User with socket not found");
    }
    free(context);
    users_list__free(&list);
}
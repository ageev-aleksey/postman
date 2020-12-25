#include <CUnit/Basic.h>
#include <stdio.h>
#include <stdbool.h>

int main() {
    CU_pSuite pSuite = NULL;

    if (CUE_SUCCESS != CU_initialize_registry()) {
        return CU_get_error();
    }

    bool error = true;

}
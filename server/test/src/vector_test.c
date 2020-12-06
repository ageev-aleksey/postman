//
// Created by nrx on 23.10.2020.
//

#include "vector.h"
#include "vector_test.h"
#include "util.h"
#include "CUnit/Basic.h"
#include "test.h"

VECTOR_DECLARE(vector_int, int);

int vector_test_init() {
    return 0;
}
int vector_test_clean() {
    return 0;
}

void vector_test_push_with_allocating() {
    vector_int *my_vector = s_malloc(sizeof(vector_int), NULL);
    err_t error;
    VECTOR_INIT(int, my_vector, error);

//    do {                                                \
//    if ((my_vector) == NULL) {                       \
//        (error).error = FATAL;                    \
//        (error).message = VECTOR_PARAMETER_IS_NULL;\
//    } else {                                        \
//        (my_vector)->array = s_malloc(sizeof(int)*5, &(error)); \
//        if ((my_vector)->array != NULL) {            \
//            (my_vector)->size = 0;                   \
//            (my_vector)->allocated = 5;\
//        } \
//    }\
//} while(0);

    VECTOR_PUSH_BACK(int, my_vector, 1, error);
    VECTOR_PUSH_BACK(int, my_vector, 2, error);
    VECTOR_PUSH_BACK(int, my_vector, 3, error);
    VECTOR_PUSH_BACK(int, my_vector, 4, error);
    VECTOR_PUSH_BACK(int, my_vector, 5, error);
    VECTOR_PUSH_BACK(int, my_vector, 6, error);
    VECTOR_PUSH_BACK(int, my_vector, 7, error);
    VECTOR_PUSH_BACK(int, my_vector, 8, error);
    VECTOR_PUSH_BACK(int, my_vector, 9, error);
    VECTOR_PUSH_BACK(int, my_vector, 10, error);
    VECTOR_PUSH_BACK(int, my_vector, 11, error);
    VECTOR_PUSH_BACK(int, my_vector, 12, error);
    for (int i = 0; i < 12; i++) {
        int *el = NULL;
        err_t error2;
        VECTOR_GET(my_vector, i, el, error2);
        ERROR_ASSERT(error2);
        CU_ASSERT_PTR_NOT_NULL(el);
        CU_ASSERT_EQUAL(*el, i+1);
    }
    VECTOR_FREE(my_vector);
    free(my_vector);
}

void vector_test_get_by_error_index() {
    vector_int *my_vector = s_malloc(sizeof(vector_int), NULL);
    err_t error;
    VECTOR_INIT(int, my_vector, error);
    VECTOR_PUSH_BACK(int, my_vector, 1, error);
    VECTOR_PUSH_BACK(int, my_vector, 2, error);
    VECTOR_PUSH_BACK(int, my_vector, 3, error);
    VECTOR_PUSH_BACK(int, my_vector, 4, error);
    int *element;
    VECTOR_GET(my_vector, 10, element, error);
    CU_ASSERT_PTR_NULL(element);
    CU_ASSERT_EQUAL(error.error, OUT_OF_RANGE);
    VECTOR_FREE(my_vector);
    VECTOR_FREE(my_vector);
    free(my_vector);
}

void vector_test_get_subvector_full_copy() {
    vector_int *my_vector = s_malloc(sizeof(vector_int), NULL);
    err_t error;
    VECTOR_INIT(int, my_vector, error);
    VECTOR_PUSH_BACK(int, my_vector, 1, error);
    VECTOR_PUSH_BACK(int, my_vector, 2, error);
    VECTOR_PUSH_BACK(int, my_vector, 3, error);
    vector_int *sub_vector = s_malloc(sizeof(vector_int), NULL);
    VECTOR_SUB(int, my_vector, sub_vector, 0, VECTOR_SIZE(my_vector), error);
    CU_ASSERT_EQUAL(VECTOR_SIZE(sub_vector), 3);
    for (size_t i = 0; i < VECTOR_SIZE(sub_vector); ++i) {
        CU_ASSERT_EQUAL(VECTOR(sub_vector)[i], VECTOR(my_vector)[i]);
    }
    VECTOR_FREE(my_vector);
    free(my_vector);
    VECTOR_FREE(sub_vector);
    free(sub_vector);

}

void vector_test_get_subvector_first_part_copy() {
    vector_int *my_vector = s_malloc(sizeof(vector_int), NULL);
    err_t error;
    VECTOR_INIT(int, my_vector, error);
    VECTOR_PUSH_BACK(int, my_vector, 10, error);
    VECTOR_PUSH_BACK(int, my_vector, 9, error);
    VECTOR_PUSH_BACK(int, my_vector, 8, error);
    VECTOR_PUSH_BACK(int, my_vector, 7, error);
    VECTOR_PUSH_BACK(int, my_vector, 6, error);
    VECTOR_PUSH_BACK(int, my_vector, 5, error);
    VECTOR_PUSH_BACK(int, my_vector, 4, error);
    VECTOR_PUSH_BACK(int, my_vector, 3, error);
    VECTOR_PUSH_BACK(int, my_vector, 2, error);
    VECTOR_PUSH_BACK(int, my_vector, 1, error);
    VECTOR_PUSH_BACK(int, my_vector, 0, error);
    VECTOR_PUSH_BACK(int, my_vector, -1, error);
    VECTOR_PUSH_BACK(int, my_vector, -2, error);
    VECTOR_PUSH_BACK(int, my_vector, -3, error);
    vector_int *sub_vector = s_malloc(sizeof(vector_int), NULL);
    VECTOR_SUB(int, my_vector, sub_vector, 0, VECTOR_SIZE(my_vector)/2, error);
    CU_ASSERT_EQUAL(VECTOR_SIZE(sub_vector), VECTOR_SIZE(my_vector)/2);
    for (size_t i = 0; i < VECTOR_SIZE(sub_vector); ++i) {
        CU_ASSERT_EQUAL(VECTOR(sub_vector)[i], VECTOR(my_vector)[i]);
    }
    VECTOR_FREE(my_vector);
    free(my_vector);
    VECTOR_FREE(sub_vector);
    free(sub_vector);
}
void vector_test_get_subvector_second_part_copy() {
    vector_int *my_vector = s_malloc(sizeof(vector_int), NULL);
    err_t error;
    VECTOR_INIT(int, my_vector, error);
    VECTOR_PUSH_BACK(int, my_vector, 10, error);
    VECTOR_PUSH_BACK(int, my_vector, 9, error);
    VECTOR_PUSH_BACK(int, my_vector, 8, error);
    VECTOR_PUSH_BACK(int, my_vector, 7, error);
    VECTOR_PUSH_BACK(int, my_vector, 6, error);
    VECTOR_PUSH_BACK(int, my_vector, 5, error);
    VECTOR_PUSH_BACK(int, my_vector, 4, error);
    VECTOR_PUSH_BACK(int, my_vector, 3, error);
    VECTOR_PUSH_BACK(int, my_vector, 2, error);
    VECTOR_PUSH_BACK(int, my_vector, 1, error);
    VECTOR_PUSH_BACK(int, my_vector, 0, error);
    VECTOR_PUSH_BACK(int, my_vector, -1, error);
    VECTOR_PUSH_BACK(int, my_vector, -2, error);
    VECTOR_PUSH_BACK(int, my_vector, -3, error);
    vector_int *sub_vector = s_malloc(sizeof(vector_int), NULL);
    VECTOR_SUB(int, my_vector, sub_vector, VECTOR_SIZE(my_vector)/2, VECTOR_SIZE(my_vector) , error);
    size_t result_size = VECTOR_SIZE(my_vector) - VECTOR_SIZE(my_vector)/2;
    CU_ASSERT_EQUAL(VECTOR_SIZE(sub_vector),result_size);
    for (size_t i = 0; i < VECTOR_SIZE(sub_vector); ++i) {
        CU_ASSERT_EQUAL(VECTOR(sub_vector)[i], VECTOR(my_vector)[VECTOR_SIZE(my_vector)/2 + i]);
    }
    VECTOR_FREE(my_vector);
    free(my_vector);
    VECTOR_FREE(sub_vector);
    free(sub_vector);
}

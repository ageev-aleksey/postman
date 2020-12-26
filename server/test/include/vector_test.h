//
// Created by nrx on 23.10.2020.
//

#ifndef SERVER_VECTOR_TEST_H
#define SERVER_VECTOR_TEST_H
int vector_test_init();
int vector_test_clean();

void vector_test_push_with_allocating();
void vector_test_get_by_error_index();
void vector_test_get_subvector_full_copy();
void vector_test_get_subvector_first_part_copy();
void vector_test_get_subvector_second_part_copy();

#endif //SERVER_VECTOR_TEST_H

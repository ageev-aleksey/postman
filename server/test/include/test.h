//
// Created by nrx on 23.10.2020.
//

#ifndef SERVER_TEST_H
#define SERVER_TEST_H

#define ERROR_ASSERT(_error_)   \
do {                            \
    if ((_error_).error) {          \
        CU_FAIL();              \
    }                           \
} while(0)


#endif //SERVER_TEST_H

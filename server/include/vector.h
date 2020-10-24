//
// Created by aleksey on 02.10.2020.
//

#ifndef SERVER_VECTOR_H
#define SERVER_VECTOR_H

#include <stddef.h>
#include "error_t.h"
#include "util.h"

//typedef struct d_vector {
//    char *buffer;
//    size_t size;
//    size_t allocated;
//} vector_t;

extern const char VECTOR_OUT_OF_RANGE[];
extern const char VECTOR_PARAMETER_IS_NULL[];
extern const char VECTOR_INVALID_SIZE_VALUE[];

///**
// * Инициализация вектора
// * @param vector - указатель, на вектор, который будет проинициализирован
// * @param error - статус выполнения
// * @return true - операция вполнена успешно; false - произошла ошибка при выполнении операции
// */
//bool vector_inti(vector_t *vector, error_t *error);
///**
// * Добавить последовательность байт в конец
// * @param vector
// * @param el
// * @param len
// * @return
// */
//bool vector_push_back(vector_t *vector, char* el, size_t len);

#define VECTOR_SUCCESS_ERROR(_error_) \
do {                                  \
    (_error_).error = OK;        \
    (_error_).message = ERROR_SUCCESS;\
} while(0)



#define VECTOR_DECLARE(_new_type_, _type_name_)     \
typedef struct _new_type_ {                         \
    size_t size;                                    \
    size_t  allocated;                              \
    _type_name_ *array;                             \
} _new_type_

#define VECTOR_INIT(_type_name_, _vector_, _error_) \
do {                                                \
    if ((_vector_) == NULL) {                       \
        (_error_).error = FATAL;                    \
        (_error_).message = VECTOR_PARAMETER_IS_NULL;\
    } else {                                        \
        (_vector_)->array = s_malloc(sizeof(_type_name_)*5, &(_error_)); \
        if ((_vector_)->array != NULL) {            \
            (_vector_)->size = 0;                   \
            (_vector_)->allocated = 5;\
        } \
    }\
} while(0)


#define VECTOR(_vector_) (_vector_)->array

#define VECTOR_RESET(_vector_, _buffer_ptr_)    \
do {                                            \
    (_vector_)->size = 0;                       \
    (_vector_)->allocated = 0;                  \
    (_buffer_ptr_) = (_vector_)->array;          \
    (_vector_)->array = NULL;                   \
}while(0)


/**
 * Создания массива нужного размера, чтобы при добавлении элементов, небыло выделение памяти.
 * Вкомпирование существующих элементов в новый учатсток памяти не выполняется.
 * Если там были данные то они будут потеряны
 */
#define VECTOR_RESERVE(_type_name_, _vector_, _size_, _error_) \
do {                                                           \
    VECTOR_SUCCESS_ERROR(_error_)                               \
    if ((_vector_) == NULL)   {                                  \
        (_error_).error = FATAL;                               \
        (_error_).message = VECTOR_PARAMETER_IS_NULL;           \
        break;                                                           \
    }                                                          \
    if ((_vector_)->array != NULL) {                           \
        free((_vector_)->array);                                                           \
    }\
    (_vector_)->array = s_malloc(sizeof(_type_name_)*(_size_), &(_error_)); \
    if ((_vector_)->array == NULL) {                           \
        break;                                                           \
    }\
    (_vector_)->size = 0;\
    (_vector_)->allocated = (_size_); \
} while(0)

#define VECTOR_INIT_WITH_RESERVE(_type_name_, _vector_, _size_, _error_) \
do {\
    if ((_vector_) == NULL) {                                           \
        (_error_).error = FATAL;                                             \
        (_error_).message =  VECTOR_PARAMETER_IS_NULL;  \
        break;                                                                    \
    }                                                                    \
    if ((_size_) < 0) {                                                  \
        (_error_).error = FATAL;                                             \
        (_error_).message =  VECTOR_INVALID_SIZE_VALUE;                  \
        break;\
    }\
    (_vector_)->array = s_malloc(sizeof(_type_name_)*(_size_), &(_error_));   \
    (_vector_)->size = 0;                                               \
    (_vector_)->allocated = (_size_);                                                                          \
} while (0);


#define VECTOR_PUSH_BACK(_type_name_, _vector_, _element_, _error_)                     \
do {                                                                                    \
    VECTOR_SUCCESS_ERROR(_error_);                                                      \
    if ((_vector_)->size == (_vector_)->allocated) {                                  \
        (_vector_)->allocated =  (_vector_)->allocated * 1.5;                               \
        _type_name_ *tmp = s_malloc(sizeof(_type_name_)*(_vector_)->allocated, &(_error_));  \
        for (int i =0; i < (_vector_)->size; i++) {                                         \
            tmp[i] = (_vector_)->array[i];                                                  \
        }                                                                               \
        free((_vector_)->array);                                                            \
        (_vector_)->array = tmp;                                                            \
    }                                                                                   \
    (_vector_)->array[(_vector_)->size] = (_element_);                                              \
    (_vector_)->size++;                                                                     \
} while(0)


#define VECTOR_GET(_vector_, _index_, _element_, _error_)           \
do {                                                                \
     VECTOR_SUCCESS_ERROR(_error_);                                 \
     if (((_index_) > (_vector_)->size) || ((_index_) < 0)) {       \
        (_error_).error = OUT_OF_RANGE;                             \
        (_error_).message = VECTOR_OUT_OF_RANGE;                    \
        (_element_) = NULL;                                         \
        break;                                                      \
     } else {                                                       \
        (_element_) = &(_vector_)->array[(_index_)];                 \
     }                                                              \
} while(0)

#define VECTOR_FREE(_vector_)       \
do {                                \
    if ((_vector_) != NULL) {       \
        (_vector_)->size = 0;           \
        (_vector_)->allocated = 0;      \
        free((_vector_)->array);        \
        (_vector_)->array = NULL;                                 \
    }\
      \
} while (0)

/**
 * Копирование одного инициализированного вектора в другой.
 */
#define VECTOR_COPY()

#define VECTOR_SUB(_type_name_, _vector_, _new_vector_, _index_start_, _index_stop_, _error_) \
do {                                                                    \
    VECTOR_SUCCESS_ERROR(_error_);                                               \
        if ((_vector_) == NULL) {                                                   \
        (_error_).error = FATAL;                                                    \
        (_error_).message = VECTOR_PARAMETER_IS_NULL;                               \
        break;                                                                      \
    }                                                                            \
    if ((_new_vector_) == NULL) {                                                   \
        (_error_).error = FATAL;                                                    \
        (_error_).message = VECTOR_PARAMETER_IS_NULL;                               \
        break;                                                                      \
    }                                                                            \
    if (((_index_start_) < (_index_stop_))                                        \
        && ((_index_start_) >=0 )                                                \
        && ((_index_stop_) <= (_vector_)->size)) \
    {                                                                            \
        size_t length = (_index_stop_) - (_index_start_) + 1;                        \
        VECTOR_INIT_WITH_RESERVE(_type_name_, _new_vector_, length, _error_);                   \
        if ((_error_).error) {                                                                     \
            break;                                                                                      \
        }                                                                                     \
        for (size_t i = (_index_start_); i < (_index_stop_); ++i) {                           \
                _type_name_ *el = NULL;                                                        \
                VECTOR_GET(_vector_, i, el, _error_);                                           \
                if (el == NULL) {                                                             \
                    break;                                                                              \
                }\
                VECTOR_PUSH_BACK(_type_name_, _new_vector_, *el, _error_);                      \
                if ((_error_).error) {                                                            \
                    break;                                                                              \
                }\
        }                                                                                     \
        }\
} while(0)

#define VECTOR_SIZE(_vector_) (_vector_)->size
/**
 *
 */



#endif //SERVER_VECTOR_H

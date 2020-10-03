//
// Created by aleksey on 02.10.2020.
//

#ifndef SERVER_VECTOR_H
#define SERVER_VECTOR_H

#define VECTOR_DECLARE(type_name) \
struct { \
    int size;                     \
    int buffer_size;            \
    type_name *array;           \
}

#define VECTOR_INIT(type_name, vector) \
do {                        \
    vector->array = malloc(sizeof(type_name)*5);                  \
    vector->size = 0;               \
    vector->buffer_size = 5;                        \
} while(0)                            \


#define VECTOR_PUSH_BACK(type_name, vector, element) \
do {                                            \
    if (vector->size == vector->buffer_size) {   \
        vector->buffer_size =  vector->buffer_size * 1.5; \
        type_name *tmp = mallock(sizeof(type_name)*vector->buffer_size)\
        for (int i =0; i < vector->size; i++) {      \
            tmp[i] = vector->array[i];                                                 \
        }                                            \
        free(vector->array);                         \
        vector->array = tmp;\
    }                                                \
    vector->array[vector->size] = element;           \
    vector->size++;\
} while(0)

#define VECTOR_POP(type_name, vector) \
do{                                                 \

} while(0)
#endif //SERVER_VECTOR_H

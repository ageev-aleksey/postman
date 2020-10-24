//
// Created by nrx on 07.10.2020.
//

#ifndef SERVER_UTIL_H
#define SERVER_UTIL_H
#include <stdlib.h>
#include <stdint.h>
#include "error_t.h"
#include <netinet/in.h>

#define IP_BUFFER_LEN 18 //< Минимальный размер буффера для записи ip адреса в виде десятичных числе через точку

/**
 * Выделение памяти с ее обнулением
 * @param size - количество байт для выделения
 * @param error - статус выполения операции
 * @return - указатель на выделенный буффер
 */
void* s_malloc(size_t size, error_t *error);

/**
 * Создание сокета прослушивающего соединение
 * @param ip - адрес ipv4, по которому выполнять прослушивание
 * @param port - tcp порт, который слушать
 * @param error - статус выполнения операции
 * @return - файловый дескриптор сокета или -1 при ошибке
 */
int make_server_socket(const char *ip, int port, error_t *error);

/**
 * Формирование читаемого адреса клиента (ip, port)
 * @param addr - структура описывающая адрес клиента
 * @param ip - буффер достаточного размера для записи ip адреса.
 * Минимальный размер буффера определяется константой IP_BUFFER_LEN
 * @param port - указатель на переменную в которую будет записан порт
 * @param error - статус выполнения операции
 * @return true - операция вполнена успешно; false - произошла ошибка при выполнении операции
 */
bool get_addr(struct sockaddr_in* addr, char *ip,size_t ip_buffer_len,  uint16_t *port, error_t *error);

#endif //SERVER_UTIL_H

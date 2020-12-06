//
// Created by nrx on 07.10.2020.
//

#ifndef SERVER_UTIL_H
#define SERVER_UTIL_H
#include <stdlib.h>
#include <stdint.h>
#include "error_t.h"
#include <netinet/in.h>

#define IP_BUFFER_LEN 18  /// Минимальный размер буффера для записи ip адреса в виде десятичных числе через точку
#define UTIL_STR_END (-1) /// Указатель на конец строки
/**
 * Выделение памяти с ее обнулением
 * @param size - количество байт для выделения
 * @param error - статус выполения операции
 * @return - указатель на выделенный буффер
 */
void* s_malloc(size_t size, err_t *error);

/**
 * Создание сокета прослушивающего соединение
 * @param ip - адрес ipv4, по которому выполнять прослушивание
 * @param port - tcp порт, который слушать
 * @param error - статус выполнения операции
 * @return - файловый дескриптор сокета или -1 при ошибке
 */
int make_server_socket(const char *ip, int port, err_t *error);


typedef struct client_addr {
    char ip[IP_BUFFER_LEN];
    int16_t  port;
} client_addr;
/**
 *
 * @param addr
 * @param error
 * @return
 */
client_addr get_addr(struct sockaddr_in* addr, err_t *error);

/**
 * Удаление лишних пробельных символов по краям строки (слева и справа)
 * @param src - исходная строка
 * @param dst - буффер в который будет перекопирован результат - если его размер не достаточен, то
 *  будет автоматическое перевыделение памяти
 *  @return Успешность операции
 */
bool trim_str(const char* src, char **dst, size_t src_size, size_t *dst_size);

/**
 * Коприрование подстроки из src в dst. размер dst должен быть достаточен для копирования
 * @param src - буфер из которого извлекается подстрока
 * @param dst - буфер куда выполняется копирование. Размер должен быть достаточен
 * @param begin - индекс начала подстроки
 * @param end - индекс указывающий за полсдений элемент подстроки
 * @return успешность операции
 */
bool sub_str(const char* src, char *dst, size_t begin, size_t end);
/**
 * Выполнение конкатенации строк. Резултат будез записан в передаваемый буфер.
 * Если размер буфера не достаточен, то он будет увеличен автоматически.
 * @param buffer - указатель на буффер, который будет перезаписан. Может быть равным NULL.
 *  Тогда будет выполнена автоматическое выделение памяти для буфера
 * @param bsize - размер буфера. Если происходит расширение буфера, то будт обновлен и его размер
 * @param nargs - число строк, для которых выполнять конкатенацию
 * @param str - первый указатель на строку
 * @param ... - последующие указатели на строки (не более 50 строк)
 * @return успешность операции
 */
bool char_make_buf_concat(char **buffer, size_t *bsize, size_t nargs, const char *str, ...);

/**
 * Разбивание строки на т трок по символу
 * @param str разбиваемая строка
 * @param array_str указатль, в который будет записан указатель на миссив строк (автоматическое выделение памяти)
 * @param sep символ, по которому выполняется разбиение строки
 * @return
 */
bool split_str(const char *str, char *array_str[], int num_split, char sep);

/**
 * Разбивка подстроки строки по символу
 * @param str - строка, которая будет разбиваться
 * @param begin - индекс с которого начинать разбиение
 * @param end - индекс указывающий за элемент строки, до которого выполнять разбиение
 *              (Для указания конца строки, используется значение UTIL_STR_END)
 * @param array_str - массив, указателей на строки. Память под строки будет автоматически выделена
 * @param num_split - размер массива array_str - макисмально число выполняемых разбиений
 * @param sep - символ разделитель, по которому будет выполнено разбиение строки
 * @return
 */
bool split_sub_str(const char *str, int begin, int end, char *array_str[], int num_split, char sep);


#endif //SERVER_UTIL_H

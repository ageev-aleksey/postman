#include "queue_log.h"

queue global_logs_queue;

void init_logs() {
    global_logs_queue.head = 1;
    global_logs_queue.tail = 0;
    global_logs_queue.size = 0;
}

void push_log(log *value)
{
   if (global_logs_queue.tail < QMAX_LOGS - 1) {
       global_logs_queue.tail++;
       global_logs_queue.elements[global_logs_queue.tail] = *value;
       global_logs_queue.size++;
   }
}

log* pop_log()
{
   if (is_logs_queue_empty()) {
       return NULL;
   }

   log *l = &global_logs_queue.elements[global_logs_queue.head];
   global_logs_queue.head++;

    return l;
}

bool is_logs_queue_empty() {
    return global_logs_queue.tail < global_logs_queue.head ? true : false;
}
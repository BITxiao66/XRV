#include "define.h"
#include "reg_bus.h"

# ifndef QUE_H
# define QUE_H
s_queue_item queue[QUEUE_SIZE];
s_queue_item queue_bk[QUEUE_SIZE];
int queue_head;
int queue_head_bk;
int queue_tail;
int queue_tail_bk;
int queue_full;
# endif
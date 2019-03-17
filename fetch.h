#include "reg_bus.h"
#include "define.h"

#ifndef FC_H
#define FC_H
int icache_write_busy;   // The data from mem write into icache
int icache_write_busy_bk;
int icache_need_refresh;
unsigned char icache[32][4][CACHE_LINE];        //icache data array
unsigned char icache_bak[32][4][CACHE_LINE];
unsigned int icache_tag[32][4];          //icache tag array       
unsigned int icache_tag_bk[32][4];
int lru[32];
s_icache_res icache_res;
int pred[PRED_SIZE];
int pred_bk[PRED_SIZE];
BYTE pred_need_fresh;
s_icache_res IcacheRead(unsigned addr);
unsigned char type_table[8][10];
#endif
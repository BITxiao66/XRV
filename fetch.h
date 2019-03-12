#include "reg_bus.h"
#include "define.h"

#ifndef FC_H
#define FC_H
int icache_write_busy;   // The data from mem write into icache
int icache_write_busy_bk;
int icache_need_refresh;
unsigned char icache[8][4][CACHE_LINE];        //icache data array
unsigned char icache_bak[8][4][CACHE_LINE];
unsigned int icache_tag[8][4];          //icache tag array       
unsigned int icache_tag_bk[8][4];
int lru[8];
s_icache_res icache_res;
unsigned char pred[256];
unsigned char pred_bk[256];
unsigned int  pred_addr[PRED_SIZE];
unsigned int  pred_addr_bk[PRED_SIZE];
s_icache_res IcacheRead(unsigned addr);
unsigned char type_table[8][10];
#endif
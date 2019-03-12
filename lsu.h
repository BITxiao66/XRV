#include "reg_bus.h"
#include "define.h"

# ifndef LS_H
# define LS_H

int dcache_busy;   
int dcache_busy_bk;
int decache_user;
int dcache_need_refresh;
unsigned char dcache[8][4][CACHE_LINE];        //icache data array
unsigned char dcache_bak[8][4][CACHE_LINE];
unsigned int dcache_tag[8][4];          //icache tag array       
unsigned int dcache_tag_bk[8][4];
int d_lru[8];

s_dcache_res dcache_res;

# endif
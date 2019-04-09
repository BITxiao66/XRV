#include "reg_bus.h"
#include "define.h"

# ifndef LS_H
# define LS_H

int dcache_busy;   
int dcache_busy_bk;
int dcache_user;
int dcache_need_refresh;
unsigned char dcache[32][4][CACHE_LINE];        //icache data array
unsigned char dcache_bak[32][4][CACHE_LINE];
unsigned int dcache_tag[32][4];          //icache tag array       
unsigned int dcache_tag_bk[32][4];
int d_lru[32];

s_dcache_res dcache_res;
s_dcache_res tmp_res;
s_dcache_res dcache_res2;
s_load_out load_out;
s_load_out load_out2;
s_load_out load_out_bk;
s_load_out load_out2_bk;

# endif
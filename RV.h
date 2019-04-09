#include "define.h"
#include "reg_bus.h"

#ifndef RV_H
#define RV_H 1
WORD pc;
WORD pc_bk1;
WORD pc_bk5;
BYTE pc5_enble; 
s_deliver deliver1;
s_deliver deliver1_bk;
s_deliver deliver2;
s_deliver deliver2_bk;
int mem_read1_ready;
int mem_read1_ready_bk;
int mem_read2_ready;
int mem_read2_ready_bk;
int mem_write1_ready;
int mem_write1_ready_bk;
int mem_write2_ready;
int mem_write2_ready_bk;
char tem[5000];
int cmt1_use;
int cmt2_use;
#endif
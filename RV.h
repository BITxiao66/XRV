#include "define.h"
#include "reg_bus.h"

#ifndef RV_H
#define RV_h
WORD pc;
WORD pc_bk1;
WORD pc_bk5;
BYTE pc5_enble; 
s_deliver12 deliver12,deliver12_bk;
int mem_read1_ready,mem_read1_ready_bk;
int mem_read2_ready,mem_read2_ready_bk;
int mem_write1_ready,mem_write1_ready_bk;
int mem_write2_ready,mem_write2_ready_bk;

char tem[5000];
#endif
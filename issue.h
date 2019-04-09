# include "define.h"
# include "reg_bus.h"

# ifndef IE_H
# define IE_H

int reg[32];
int reg_bk[32];

s_station_in station[15];
s_station_in station_bk[15];
BYTE issue_write[15];   // clean at cycle end
s_alu_out alu_out;
s_alu_out alu_out_bk;
s_alu_out alu_out2;
s_alu_out alu_out2_bk;
s_ju_out ju_out;
s_ju_out ju_out_bk;
s_alu_out mul_out;
s_alu_out mul_out_bk;
s_alu_out csu_out;
s_alu_out csu_out_bk;
# endif
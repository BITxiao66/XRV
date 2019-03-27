# include "define.h"
# include "reg_bus.h"

# ifndef IE_H
# define IE_H

int reg[32];
int reg_bk[32];

s_station_in station[7];
s_station_in station_bk[7];
BYTE issue_write[7];   // clean at cycle end
s_alu_out alu_out;
s_alu_out alu_out_bk;
s_ju_out ju_out;
s_ju_out ju_out_bk;
s_alu_out mul_out;
s_alu_out mul_out_bk;
s_alu_out csu_out;
s_alu_out csu_out_bk;
# endif
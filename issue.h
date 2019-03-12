# include "define.h"
# include "reg_bus.h"

# ifndef IE_H
# define IE_H

WORD reg[32];
WORD reg_bk[32];

s_station_in station[6];
s_station_in station_bk[6];
BYTE issue_write[6];   // clean at cycle end
s_alu_out alu_out;
s_alu_out alu_out_bk;


# endif
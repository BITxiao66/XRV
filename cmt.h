# include "define.h"
# include "reg_bus.h"

# ifndef CMT_H
# define CMT_H

BYTE cmt_vie;
BYTE cmt_vie_bk;
BYTE cmt_vie2;
BYTE cmt_vie_bk2;
s_cmt_bus cmt_bus; // update at cycle begin
s_cmt_bus cmt_bus2; 
# endif
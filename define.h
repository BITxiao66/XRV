/********************************************************************************
 * Files         : define.h
 * Description   : This project is a C simulator for RISC-V, a graduation project
 *               : in (1) ICT,CAS and (2) Beijing Institute of Technology .
 *               : This file is the head file to define all typedef , constant and 
 *               : macro .
 * Author        : xiaoziyuan 
 * Last Modified : 2019.03.07
 * Version       : V 0.1
 ********************************************************************************/
# ifndef DF_H
# define DF_H
# define QUEUE_SIZE 8
# define MEM_SIZE   64*1024*1024
# define CACHE_LINE 32
# define PRED_SIZE  1024

# define DCACHE_FREE 0
# define ORIGINAL   1
# define DIRTY      2

# define U_TYPE 1
# define J_TYPE 2
# define I_TYPE 3
# define B_TYPE 4
# define S_TYPE 5
# define R_TYPE 6

# define ALU      1
# define MUL_UNIT 2
# define JU       3
# define LU       4
# define SU       5

# define ITEM_FREE 0
# define UNISSUED  1
# define ISSUED    2
# define FINISH    3

# define OP_RS1 1
# define OP_RS2 2
# define OP_RD  3
# define OP_IMM 4

# define ALU_ADD 1
# define ALU_SUB 2
# define ALU_AND 3
# define ALU_OR  4
# define ALU_XOR 5
# define ALU_SLL 6
# define ALU_SRA 7
# define ALU_SRL 8
# define ALU_LUI 9
# define ALU_AUI 10
# define ALU_SLT 11
# define ALU_SLTU 12
# define ALU_LUI  15
# define ALU_AUI  16

# define JU_JAL     1
# define JU_JALR    2
# define JU_BEQ     3
# define JU_BNE     4
# define JU_BLT     5
# define JU_BGE     6
# define JU_BLTU    7
# define JU_BGEU    8

# define LU_LB      1
# define LU_LH      2
# define LU_LW      3
# define LU_LBU     4
# define LU_LHU     5

# define SU_SB      1
# define SU_SH      2
# define SU_SW      3

typedef unsigned char   BIT0;   // just use the first bit
typedef unsigned char   BIT1;
typedef unsigned char   BIT2;
typedef unsigned char   BIT3;
typedef unsigned char   BIT4;
typedef unsigned char   BIT5;
typedef unsigned char   BIT6;
typedef unsigned char   BIT7;
typedef unsigned char   BYTE;
typedef unsigned char*  BYTEp;
typedef unsigned short  HALF_WORD;
typedef unsigned int    WORD;


#endif


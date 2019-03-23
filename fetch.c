/********************************************************************************
 * Files         : fetch.c
 * Description   : This project is a C simulator for RISC-V, a graduation project
 *               : in (1) ICT,CAS and (2) Beijing Institute of Technology .
 *               : 
 *               :
 * Author        : xiaoziyuan 
 * Last Modified : 2019.03.07
 * Version       : V 0.1
 ********************************************************************************/
#include "reg_bus.h"
#include "define.h"
#include "RV.h"
#include "fetch.h"
#include "queue.h"
#include <stdlib.h>

extern char read_port_data[CACHE_LINE];

void IcacheReset()
{
    icache_write_busy = icache_write_busy_bk =0;
    icache_need_refresh = 0;
    memset(icache,0,sizeof(icache));
    memset(icache_bak,0,sizeof(icache_bak));
    memset(icache_tag,0,sizeof(icache_tag));
    memset(icache_tag_bk,0,sizeof(icache_tag_bk));
    memset(lru,0,sizeof(lru));
}

void IcacheSwap(int addr)
{
    static int line=0;
    if (icache_write_busy) 
    {
        if(mem_read1_ready)
        {
            ReadMemLine(addr);
            memcpy(icache_bak[(addr/CACHE_LINE)%32][line],read_port_data,CACHE_LINE);
            icache_tag_bk[(addr/CACHE_LINE)%32][line]=(addr&0xFFFFFFE0)|0x00000001;
            icache_need_refresh = 1;
            icache_write_busy_bk = 0;
            return;
        }
        else
        {
            return;
        }
    }
    else
    {
        line = (lru[(addr/CACHE_LINE)%32]+1)%4;
        icache_write_busy_bk =1;
        ApplyReadPort(1);
    }
}

s_icache_res IcacheRead(unsigned int addr)
{
    int group = (addr/CACHE_LINE)%32;
    int line = 0;
    int i = 0;
    icache_res.hit =0;
    icache_res.ins =0;
    if(icache_write_busy)
    {
        return icache_res;
    }
    for( i = 0; i < 4; i++)
    {
        if (((icache_tag[group][i] & 0xFFFFFFE0) == (addr&0xFFFFFFE0))&&((icache_tag[group][i]&0x0000001F)==1)) 
        {
            line = i;
            break;
        }     
    }
    int hit = i<4?1:0;
    if(hit)
    {
        icache_res.hit = 1;
        icache_res.ins = 0;
        for( i = 3; i >= 0; i--)
        {
            icache_res.ins <<= 8;
            icache_res.ins |= icache[group][line][addr%CACHE_LINE+i];
        }
        lru[group] = line;
    }
    else
    {
        IcacheSwap(addr);
    } 
    return icache_res;
}

unsigned int PredBranchIf(int ins_pc)
{
    int jump =0;
    if((pred[(ins_pc/4)%PRED_SIZE]&0x00000003)>1)
        jump =1;
    return jump;
}

void ResetDecodeTable()
{
    /*************type_table section*************/
    memset(type_table,0,sizeof(type_table));
    type_table[U_TYPE][0] = 0b00110111;
    type_table[U_TYPE][1] = 0b00010111;
    type_table[J_TYPE][0] = 0b01101111;
    type_table[I_TYPE][0] = 0b01100111;
    type_table[I_TYPE][1] = 0b00000011;
    type_table[I_TYPE][2] = 0b00010011;
    type_table[B_TYPE][0] = 0b01100011;
    type_table[S_TYPE][0] = 0b00100011;
    type_table[R_TYPE][0] = 0b00110011;
    /*************unit_table section*************/
   
}

unsigned char GetInsType(BYTE op1)
{
    int i,j;
    unsigned char res = 0;
    for( i = 1; i < 8; i++)
    {
        for( j = 0; j < 10; j++)
        {
            if (type_table[i][j]==op1) 
            {
                res = i;
                return res;
            }   
        } 
    }
    return res;
}

unsigned char RtypeDecode(BYTE op1,BYTE op2,BYTE op3,unsigned char* op_code,unsigned char*issue_sta)
{
    BYTE res=0;
    if(op1==0b00110011)
        *issue_sta=ALU;
    if (op2==0 && op3==0) 
    {
        *op_code=ALU_ADD;
        res = 1;
    }
    else if(op2==0 && op3==0x20)
    {
        *op_code=ALU_SUB;
        res = 1;
    }
    else if (op2==1)
    {
        *op_code=ALU_SLL;
        res=1;
    }
    else if (op2==2)
    {
        *op_code=ALU_SLT;
        res=1;
    }
    else if (op2==3)
    {
        *op_code=ALU_SLTU;
        res=1;
    }
    else if (op2==4)
    {
        *op_code=ALU_XOR;
        res=1;
    }
    else if (op2==5 && op3==0)
    {
        *op_code=ALU_SRL;
        res=1;
    }
    else if (op2==5 && op3==0x20)
    {
        *op_code=ALU_SRA;
        res=1;
    }
    else if (op2==6)
    {
        *op_code=ALU_OR;
        res=1;
    }
    else if (op2==7)
    {
        *op_code=ALU_AND;
        res=1;
    }
    return res;
}

unsigned char JtypeDecode(BYTE op1,BYTE op2,BYTE op3,unsigned char* op_code,unsigned char*issue_sta)
{
    BYTE res=1;
    *op_code=JU_JAL;
    *issue_sta=JU;
    return res;
}

unsigned char BtypeDecode(BYTE op1,BYTE op2,BYTE op3,unsigned char* op_code,unsigned char*issue_sta)
{
    BYTE res=0;
    *issue_sta=JU;
    switch (op2)
    {
        case 0:
            *op_code=JU_BEQ ;
            res=1;
            break;
    
        case 1:
            *op_code=JU_BNE ;
            res=1;
            break;
    
        case 4:
            *op_code=JU_BLT ;
            res=1;
            break;
        
        case 5:
            *op_code=JU_BGE ;
            res=1;
            break;

        case 6:
            *op_code=JU_BLTU ;
            res=1;
            break;

        case 7:
            *op_code=JU_BGEU ;
            res=1;
            break;

        default:
            break;
    }
    return res;
}

unsigned char ItypeDecode(BYTE op1,BYTE op2,BYTE op3,unsigned char* op_code,unsigned char*issue_sta)
{
    BYTE res=0;
    if (op1==0b01100111) 
    {
        *op_code=JU_JALR;
        *issue_sta=JU;
        res=1;
    }
    else if (op1==0b00000011) 
    {
        *issue_sta=LU;
        if (op2==0) 
        {
            *op_code=LU_LB;
            res=1;
        }
        else if (op2==1) 
        {
            *op_code=LU_LH;
            res=1;
        }
        else if (op2==2)
        {
            *op_code=LU_LW;
            res=1;
        }
        else if (op2==4)
        {
            *op_code=LU_LBU;
            res=1;
        }
        else if (op2==5)
        {
            *op_code=LU_LHU;
            res=1;
        }
    }
    else if (op1==0b00010011)
    {
        if (op2==0) 
        {
            *op_code=ALU_ADD;
            res=1;
        }
        else if (op2==1)
        {
            *op_code=ALU_SLL;
            res=1;
        }
        else if (op2==2)
        {
            *op_code=ALU_SLT;
            res=1;
        }
        else if (op2==3)
        {
            *op_code=ALU_SLTU;
            res=1;
        }
        else if (op2==4)
        {
            *op_code=ALU_XOR;
            res=1;
        }
        else if (op2==5 && op3==0)
        {
            *op_code=ALU_SRL;
            res=1;
        }
        else if (op2==5 && op3==0x20)
        {
            *op_code=ALU_SRA;
            res=1;
        }
        else if (op2==6)
        {
            *op_code=ALU_OR;
            res=1;
        }
        else if (op2==7)
        {
            *op_code=ALU_AND;
            res=1;
        }
        *issue_sta=ALU;
    }
    
    return res;
}

unsigned char StypeDecode(BYTE op1,BYTE op2,BYTE op3,unsigned char* op_code,unsigned char*issue_sta)
{
    BYTE res=0;
    *issue_sta=SU;
    switch (op2)
    {
        case 0:
            *op_code=SU_SB;
            res=1;
            break;

        case 1:
            *op_code=SU_SH;
            res=1;
            break;

        case 2:
            *op_code=SU_SW;
            res=1;
            break;

        default:
            break;
    }
    return res;
}

unsigned char UtypeDEcode(BYTE op1,BYTE op2,BYTE op3,unsigned char* op_code,unsigned char*issue_sta)
{
    *issue_sta=ALU;
    if (op1==0b00110111) 
    {
        *op_code=ALU_LUI;
    }
    else
    {
        *op_code=ALU_AUI;
    } 
    return 1;
}

unsigned char GetInsData(WORD ins,BYTE op_type,BYTE op1,BYTE op2,s_ins_data* ins_data)
{
    BYTE res=0;
    memset(ins_data->data_sel,0,sizeof(ins_data->data_sel));
    if (op_type==U_TYPE) 
    {
        ins_data->op_imm=ins & 0xFFFFF000;
        ins_data->data_sel[OP_IMM]=1;
        ins_data->op_rd =(ins >> 7) & 0x0000001F;
        ins_data->data_sel[OP_RD]=1;
        res=1;
    }
    else if (op_type==J_TYPE) 
    {
        ins_data->op_rd=(ins>>7)&0x1F;
        ins_data->data_sel[OP_RD]=1;
        int imm = 0; 
        imm |= (ins>>21) & 0x000003FF;
        imm |= (ins>>10) & 0x00000400; 
        imm |= (ins>>1) &  0x0007F800;
        imm |= (ins>>12) & 0x00080000;
        imm<<=12;
        imm>>=11;
        ins_data->op_imm=imm;
        ins_data->data_sel[OP_IMM]=1;
        res = 1;
    }
    else if (op_type==I_TYPE)
    {
        if (op1==0b00010011&&(op2==1||op2==5)) 
        {
            ins_data->op_imm=(ins>>20)&0x0000001F;
            ins_data->data_sel[OP_IMM]=1;
            ins_data->op_rs1=(ins>>15) & 0x1F;
            ins_data->data_sel[OP_RS1]=1;
            ins_data->op_rd=(ins>>7) & 0x1F;
            ins_data->data_sel[OP_RD]=1;
            res=1;
        }
        else
        {
            int imm;
            imm=ins;
            imm>>=20;
            ins_data->op_imm=imm;
            ins_data->data_sel[OP_IMM]=1;
            ins_data->op_rs1=(ins>>15) & 0x1F;
            ins_data->data_sel[OP_RS1]=1;
            ins_data->op_rd=(ins>>7) & 0x1F;
            ins_data->data_sel[OP_RD]=1;
            res=1;
        }
        
    }
    else if (op_type==B_TYPE)
    {
        ins_data->op_rs1=(ins>>15) & 0x1F;
        ins_data->data_sel[OP_RS1]=1;
        ins_data->op_rs2=(ins>>20) & 0x1F;
        ins_data->data_sel[OP_RS2]=1;
        int imm = 0;
        imm |= (ins>>8) & 0x0000000F;
        imm |= (ins>>21) &0x000003F0;
        imm |= (ins<<3) & 0x00000400;
        imm |= (ins>>20) &0x00000800;
        imm<<=20;
        imm>>=19;
        ins_data->op_imm=imm;
        ins_data->data_sel[OP_IMM]=1;
        res=1;
    }
    else if (op_type==S_TYPE)
    {
        int imm;
        imm = (ins>>7) & 0x0000001F;
        imm |= (ins>>20) & 0x00000FE0;
        ins_data->op_imm=imm;
        ins_data->data_sel[OP_IMM]=1;
        ins_data->op_rs1=(ins>>15) & 0x1F;
        ins_data->data_sel[OP_RS1]=1;
        ins_data->op_rs2=(ins>>20) & 0x1F;
        ins_data->data_sel[OP_RS2]=1;
        res=1;
    }
    else if (op_type==R_TYPE)
    {
        ins_data->op_rs1=(ins>>15)&0x1F;
        ins_data->data_sel[OP_RS1]=1;
        ins_data->op_rs2=(ins>>20)&0x1F;
        ins_data->data_sel[OP_RS2]=1;
        ins_data->op_rd =(ins>>7)&0x1F;
        ins_data->data_sel[OP_RD]=1;
        if ( ins_data->op_rs1<32&& ins_data->op_rs2<32&& ins_data->op_rd<32) 
            res = 1; 
    }
    return res;
}

void BranchPredict(BYTE op_type,WORD ins,unsigned int* ins_pred,unsigned char*jump_if)
{
    BYTE op_code1=ins&0x7f;
    if (op_type==J_TYPE || op_type==I_TYPE&&op_code1==0b01100111) 
    {
        *jump_if=1;
        *ins_pred=pred[(pc/4)%PRED_SIZE]&0xFFFFFFFC;
    }
    else if (op_type==B_TYPE) 
    {
        *jump_if=PredBranchIf(pc);
        *ins_pred=(*jump_if)?pred[(pc/4)%PRED_SIZE]&0xFFFFFFFC:pc+4;
    }
    else
    {
        *jump_if=0;
        *ins_pred=pc+4;
    }
}

void FetchModule()
{
    IcacheRead(pc);
    if(icache_write_busy)
        IcacheSwap(pc);
    memset(&deliver12_bk,0,sizeof(deliver12));
    WORD ins = icache_res.ins;
    BYTE op1=ins&0x7F;
    BYTE op2=(ins>>12)&0x07;
    BYTE op3=(ins>>25)&0x7F;
    BYTE op_code;
    BYTE issue_sta;
    BYTE ins_valid;
    BYTE imm_ext;
    WORD ins_pred;
    BYTE jump_if;
    ins_valid=0;
    if(ins==0)
    {
        deliver12_bk.ins_valid = 0;
        return;
    }
    if (queue_full) 
    {
        deliver12_bk.ins_valid = 0;
        return;
    }
    unsigned char op_type = GetInsType(op1);
    if(op_type==0 || op_type==7)
    {
        deliver12_bk.ins_valid = 0;
        return;
    }
    switch (op_type)
    {
        case U_TYPE:
            /* code */
            break;

        case J_TYPE:
            ins_valid=JtypeDecode(op1,op2,op3,&op_code,&issue_sta);
            break;

        case I_TYPE:
            ins_valid=ItypeDecode(op1,op2,op3,&op_code,&issue_sta);
            break;

        case B_TYPE:
            ins_valid=BtypeDecode(op1,op2,op3,&op_code,&issue_sta);
            break;

        case S_TYPE:
            ins_valid=StypeDecode(op1,op2,op3,&op_code,&issue_sta);
            break;

        case R_TYPE:
            ins_valid =RtypeDecode(op1,op2,op3,&op_code,&issue_sta);
            break;
            
        default:
            break;
    }
    if (!ins_valid) 
    {
        deliver12_bk.ins_valid = 0;
        return;
    }
    s_ins_data ins_data;
    ins_valid = GetInsData(ins,op_type,op1,op2,&ins_data);
    BranchPredict(op_type,ins,&ins_pred,&jump_if);
    deliver12_bk.op_type=op_type;
    deliver12_bk.op_code=op_code;
    deliver12_bk.op_smt=ins_data.op_smt;
    deliver12_bk.op_rs1=ins_data.op_rs1;
    deliver12_bk.op_rs2=ins_data.op_rs2;
    deliver12_bk.op_rd =ins_data.op_rd;
    deliver12_bk.op_imm=ins_data.op_imm;
    memcpy(deliver12_bk.data_sel,ins_data.data_sel,sizeof(ins_data.data_sel));
    deliver12_bk.issue_sta=issue_sta;
    deliver12_bk.ins_addr=pc;
    deliver12_bk.ins_pred=ins_pred;
    deliver12_bk.jump_if=jump_if;
    deliver12_bk.ins_valid=ins_valid;
    pc_bk1=ins_pred;
}
                                                      
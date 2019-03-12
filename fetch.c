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
            memcpy(icache_bak[(addr/256)%8][line],read_port_data,CACHE_LINE);
            icache_tag_bk[(addr/256)%8][line]=(addr&0xFFFFFC00)|1;
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
        line = (lru[(addr/256)%8]+1)%4;
        icache_write_busy_bk =1;
        ApplyReadPort(1);
    }
}

s_icache_res IcacheRead(unsigned int addr)
{
    int group = (addr/CACHE_LINE)%8;
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
        if (((icache_tag[group][i] & 0xFFFFFC00) == (addr&0xFFFFFC00))&&((icache_tag[group][i]&0x000000FF)==1)) 
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
            icache_res.ins |= icache[group][line][addr%256+i];
        }
        lru[group] = line;
    }
    else
    {
        IcacheSwap(addr);
    } 
    return icache_res;
}

unsigned int PredBranchAddr(int ins_pc)
{
    int jump =0;
    if((pred[(ins_pc/4)%256]>>2*(ins_pc%4))&0x03>=1)
        jump =1;
    return jump?pred_addr[ins_pc%1024]:ins_pc+4;
}

void ResetDecodeTable()
{
    /*************type_table section*************/
    memset(type_table,0,sizeof(type_table));
    type_table[U_TYPE][0] = 0b00110111;
    type_table[U_TYPE][1] = 0b00010111;
    type_table[J_TYPE][0] = 0b01101111;

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
}

unsigned char GetInsData(WORD ins,BYTE op_type,BYTE imm_ext,s_ins_data* ins_data)
{
    BYTE res=0;
    memset(ins_data->data_sel,0,sizeof(ins_data->data_sel));
    if (op_type==U_TYPE) 
    {
        /* code */
    }
    else if (op_type==J_TYPE) 
    {
        /* code */
    }
    else if (op_type==I_TYPE)
    {
        /* code */
    }
    else if (op_type==B_TYPE)
    {
        /* code */
    }
    else if (op_type==S_TYPE)
    {
        /* code */
    }
    else if (op_type==R_TYPE)
    {
        ins_data->op_rs1=(ins>>15)&0x1F;
        ins_data->data_sel[1]=1;
        ins_data->op_rs2=(ins>>20)&0x1F;
        ins_data->data_sel[2]=1;
        ins_data->op_rd =(ins>>7)&0x1F;
        ins_data->data_sel[3]=1;
        if ( ins_data->op_rs1<32&& ins_data->op_rs2<32&& ins_data->op_rd<32) 
            res = 1; 
    }
    return res;
}

void BranchPredict(BYTE op_type,WORD ins,unsigned int* ins_perd,unsigned char*jump_if)
{
    BYTE op_code1=ins&0x7f;
    if (op_type==J_TYPE || op_type==I_TYPE&&op_code1==0b01100111) 
    {
        *jump_if=1;
        *ins_perd=pred_addr[pc%PRED_SIZE];
    }
    else if (op_type==B_TYPE) 
    {
        *jump_if=PredBranchAddr(pc);
        *ins_perd=pred_addr[pc%PRED_SIZE];
    }
    else
    {
        *jump_if=0;
        *ins_perd=pc+4;
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
            /* code */
            break;

        case I_TYPE:
            /* code */
            break;

        case B_TYPE:
            /* code */
            break;

        case S_TYPE:
            /* code */
            break;

        case R_TYPE:
            ins_valid =RtypeDecode(op1,op2,op3,&op_code,&issue_sta);
            break;
            
        default:
            break;
    }
    s_ins_data ins_data;
    ins_valid = GetInsData(ins,op_type,imm_ext,&ins_data);
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
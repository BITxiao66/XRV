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

s_icache_res IcacheRead(unsigned int addr,int leng)
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
        if (leng==4) 
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
            icache_res.hit = 1;
            icache_res.ins = 0;
            for( i = 7; i >= 0; i--)
            {
                icache_res.ins <<= 8;
                icache_res.ins |= icache[group][line][addr%CACHE_LINE+i];
            }
            lru[group] = line;
        }
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
    type_table[I_TYPE][3] = 0b01110011;
    type_table[B_TYPE][0] = 0b01100011;
    type_table[S_TYPE][0] = 0b00100011;
    type_table[R_TYPE][0] = 0b00110011;
    /*************unit_table section*************/
   
}

unsigned char GetInsType(WORD ins)
{
    BYTE op1=ins&0x7F;
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

unsigned char RtypeDecode(WORD ins,unsigned char* op_code,unsigned char*issue_sta)
{
    BYTE op1=ins&0x7F;
    BYTE op2=(ins>>12)&0x07;
    BYTE op3=(ins>>25)&0x7F;
    BYTE res=0;
    if (op3==1) 
    {
        *issue_sta=MUL_UNIT;
        switch (op2)
        {
            case 0:
                *op_code=MUL_MUL;
                res=1;
                break;
        
            case 4:
                *op_code=MUL_DIV;
                res=1;
                break;
            
            case 5:
                *op_code=MUL_DIVU;
                res=1;
                break;

            case 6:
                *op_code=MUL_REM;
                res=1;
                break;

            default:
                break;
        }
        return res;
    }
    // for ALU
    if(op1==0b00110011&&op3!=1)
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

unsigned char JtypeDecode(WORD ins,unsigned char* op_code,unsigned char*issue_sta)
{
    BYTE res=1;
    BYTE op1=ins&0x7F;
    BYTE op2=(ins>>12)&0x07;
    BYTE op3=(ins>>25)&0x7F;
    *op_code=JU_JAL;
    *issue_sta=JU;
    return res;
}

unsigned char BtypeDecode(WORD ins,unsigned char* op_code,unsigned char*issue_sta)
{
    BYTE res=0;
    BYTE op1=ins&0x7F;
    BYTE op2=(ins>>12)&0x07;
    BYTE op3=(ins>>25)&0x7F;
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

unsigned char ItypeDecode(WORD ins,unsigned char* op_code,unsigned char*issue_sta)
{
    BYTE res=0;
    BYTE op1=ins&0x7F;
    BYTE op2=(ins>>12)&0x07;
    BYTE op3=(ins>>25)&0x7F;
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
    else if (op1==0b01110011)
    {
        *issue_sta=CSU;
        if (op2==0) 
        {
            *op_code=0;
            res=1;
        }
        else if (op2==1) 
        {
            *op_code=CSU_CSRRW;
            res=1;
        }
        else if (op2==2) 
        {
            *op_code=CSU_CSRRS;
            res=1;
        }
        else if (op2==3) 
        {
            *op_code=CSU_CSRRC;
            res=1;
        }
        else
        {
            printf("CSR break\n");
            exit(1);
        }
    }
    return res;
}

unsigned char StypeDecode(WORD ins,unsigned char* op_code,unsigned char*issue_sta)
{
    BYTE res=0;
    BYTE op1=ins&0x7F;
    BYTE op2=(ins>>12)&0x07;
    BYTE op3=(ins>>25)&0x7F;
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

unsigned char UtypeDecode(WORD ins,unsigned char* op_code,unsigned char*issue_sta)
{
    *issue_sta=ALU;
    BYTE op1=ins&0x7F;
    BYTE op2=(ins>>12)&0x07;
    BYTE op3=(ins>>25)&0x7F;
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

unsigned char GetInsData(WORD ins,BYTE op_type,s_ins_data* ins_data)
{
    BYTE res=0;
    BYTE op1=ins&0x7F;
    BYTE op2=(ins>>12)&0x07;
    BYTE op3=(ins>>25)&0x7F;
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
        imm <<=20;
        imm >>=20;
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

void BranchPredict(BYTE op_type,WORD ins,WORD addr,unsigned int* ins_pred,unsigned char*jump_if)
{
    BYTE op_code1=ins&0x7f;
    if (op_type==J_TYPE || op_type==I_TYPE&&op_code1==0b01100111) 
    {
        *jump_if=1;
        *ins_pred=pred[(addr/4)%PRED_SIZE]&0xFFFFFFFC;
    }
    else if (op_type==B_TYPE) 
    {
        *jump_if=PredBranchIf(addr);
        *ins_pred=(*jump_if)?pred[(addr/4)%PRED_SIZE]&0xFFFFFFFC:addr+4;
    }
    else
    {
        *jump_if=0;
        *ins_pred=addr+4;
    }
}

void FetchModule()
{
    if (pc%8) 
    {
        IcacheRead(pc,4);
    }
    else
    {
        IcacheRead(pc,8);
    }
    if(icache_write_busy)
        IcacheSwap(pc);
    memset(&deliver1_bk,0,sizeof(deliver1));
    memset(&deliver2_bk,0,sizeof(deliver2));
    WORD ins1 = icache_res.ins;
    WORD ins2 = icache_res.ins>>32;
    BYTE _2nd_valid =0;
    BYTE op_code1;
    BYTE issue_sta1;
    BYTE ins_valid1;
    WORD ins_pred1;
    BYTE jump_if1;
    BYTE op_code2;
    BYTE issue_sta2;
    BYTE ins_valid2;
    WORD ins_pred2;
    BYTE jump_if2;
    if(ins1==0)
    {
        deliver1_bk.ins_valid = 0;
        deliver2_bk.ins_valid = 0;
        return;
    }
    if (queue_full) 
    {
        deliver1_bk.ins_valid = 0;
        deliver2_bk.ins_valid = 0;
        return;
    }
    unsigned char op_type1 = GetInsType(ins1);
    unsigned char op_type2 = GetInsType(ins2);
    if(op_type1==0 || op_type1==7 )
    {
        deliver1_bk.ins_valid = 0;
        deliver2_bk.ins_valid = 0;
        return;
    }
    switch (op_type1)
    {
        case U_TYPE:
            ins_valid1=UtypeDecode(ins1,&op_code1,&issue_sta1);
            _2nd_valid=1;
            break;

        case J_TYPE:
            ins_valid1=JtypeDecode(ins1,&op_code1,&issue_sta1);
            _2nd_valid=0;
            break;

        case I_TYPE:
            ins_valid1=ItypeDecode(ins1,&op_code1,&issue_sta1);
            _2nd_valid=issue_sta1==JU ? 0:1;
            break;

        case B_TYPE:
            ins_valid1=BtypeDecode(ins1,&op_code1,&issue_sta1);
            _2nd_valid=0;
            break;

        case S_TYPE:
            ins_valid1=StypeDecode(ins1,&op_code1,&issue_sta1);
            _2nd_valid=1;
            break;

        case R_TYPE:
            ins_valid1=RtypeDecode(ins1,&op_code1,&issue_sta1);
            _2nd_valid=1;
            break;
            
        default:
            break;
    }
    s_ins_data ins_data1;
    ins_valid1 = GetInsData(ins1,op_type1,&ins_data1);
    BranchPredict(op_type1,ins1,pc,&ins_pred1,&jump_if1);
    deliver1_bk.op_type=op_type1;
    deliver1_bk.op_code=op_code1;
    deliver1_bk.op_rs1=ins_data1.op_rs1;
    deliver1_bk.op_rs2=ins_data1.op_rs2;
    deliver1_bk.op_rd =ins_data1.op_rd;
    deliver1_bk.op_imm=ins_data1.op_imm;
    memcpy(deliver1_bk.data_sel,ins_data1.data_sel,sizeof(ins_data1.data_sel));
    deliver1_bk.issue_sta=issue_sta1;
    deliver1_bk.ins_addr=pc;
    deliver1_bk.ins_pred=ins_pred1;
    deliver1_bk.jump_if=jump_if1;
    deliver1_bk.ins_valid=ins_valid1;
    pc_bk1=ins_pred1;
    if (! _2nd_valid) 
    {
        return;
    }
    if(ins2==0 || op_type2==0 || op_type2==7)
    {
        deliver2_bk.ins_valid = 0;
        return;
    }
    switch (op_type2)
    {
        case U_TYPE:
            ins_valid2=UtypeDecode(ins2,&op_code2,&issue_sta2);
            break;

        case J_TYPE:
            ins_valid2=JtypeDecode(ins2,&op_code2,&issue_sta2);
            break;

        case I_TYPE:
            ins_valid2=ItypeDecode(ins2,&op_code2,&issue_sta2);
            break;

        case B_TYPE:
            ins_valid2=BtypeDecode(ins2,&op_code2,&issue_sta2);
            break;

        case S_TYPE:
            ins_valid2=StypeDecode(ins2,&op_code2,&issue_sta2);
            break;

        case R_TYPE:
            ins_valid2=RtypeDecode(ins2,&op_code2,&issue_sta2);
            break;
            
        default:
            break;
    }
    s_ins_data ins_data2;
    ins_valid2 = GetInsData(ins2,op_type2,&ins_data2);
    BranchPredict(op_type2,ins2,pc+4,&ins_pred2,&jump_if2);
    deliver2_bk.op_type=op_type2;
    deliver2_bk.op_code=op_code2;
    deliver2_bk.op_smt=ins_data2.op_smt;
    deliver2_bk.op_rs1=ins_data2.op_rs1;
    deliver2_bk.op_rs2=ins_data2.op_rs2;
    deliver2_bk.op_rd =ins_data2.op_rd;
    deliver2_bk.op_imm=ins_data2.op_imm;
    memcpy(deliver2_bk.data_sel,ins_data2.data_sel,sizeof(ins_data2.data_sel));
    deliver2_bk.issue_sta=issue_sta2;
    deliver2_bk.ins_addr=pc+4;
    deliver2_bk.ins_pred=ins_pred2;
    deliver2_bk.jump_if=jump_if2;
    deliver2_bk.ins_valid=ins_valid2;
    pc_bk1=ins_pred2;
}
                                                      
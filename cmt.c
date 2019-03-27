/********************************************************************************
 * Files         : mem.c
 * Description   : This project is a C simulator for RISC-V, a graduation project
 *               : in (1) ICT,CAS and (2) Beijing Institute of Technology .
 *               : This file is used for simulate the memory module of RV .The 
 * Author        : xiaoziyuan 
 * Last Modified : 2019.03.16
 * Version       : V 0.1
 ********************************************************************************/
#include "define.h"
#include "reg_bus.h"
#include "queue.h"
#include "issue.h"
#include "cmt.h"
#include "lsu.h"

void CmtBusUpdate()
{
    cmt_vie=cmt_vie_bk;
    cmt_vie_bk=0;
    memset(&cmt_bus,0,sizeof(cmt_bus));
    switch (cmt_vie)
    {
        case ALU:
            cmt_bus.valid=alu_out.valid;
            cmt_bus.id=alu_out.id;
            cmt_bus.res=alu_out.res;
            cmt_bus.user=ALU;
            break;
        
        case MUL_UNIT:
            cmt_bus.valid=mul_out.valid;
            cmt_bus.id=mul_out.id;
            cmt_bus.res=mul_out.res;
            cmt_bus.user=MUL_UNIT;
            break;
        
        case CSU:
            cmt_bus.valid=csu_out.valid;
            cmt_bus.id=csu_out.id;
            cmt_bus.res=csu_out.res;
            cmt_bus.user=CSU;
            break;

        case JU:
            cmt_bus.valid=ju_out.valid;
            cmt_bus.id=ju_out.id;
            cmt_bus.res=ju_out.res;
            cmt_bus.jump_if=ju_out.jump_if;
            cmt_bus.addr=ju_out.addr;
            cmt_bus.user=JU;
            break;

        case LU:
            cmt_bus.valid=load_out.valid;
            cmt_bus.addr =load_out.addr;
            cmt_bus.id   =load_out.id;
            cmt_bus.res  =load_out.res;
            cmt_bus.user =LU;
            break;

        default:
            break;
    }
}

void Commit()
{
    if (!cmt_bus.valid) 
    {
        return;
    }
    int p;
    p=cmt_bus.id;
    int user=cmt_bus.user;
    if (queue[p].item_status==ITEM_FREE) 
    {
        return;
    }
    if (cmt_bus.valid==2) 
    {
        queue_bk[p].exe_addr=cmt_bus.addr;
        return;
    }
    if (user==ALU||user==MUL_UNIT||user==CSU) 
    {
        queue_bk[p].imm=cmt_bus.res;
        queue_bk[p].item_status=FINISH;
    }
    else if (user==LU)
    {
        queue_bk[p].imm=cmt_bus.res;
        queue_bk[p].exe_addr=cmt_bus.addr;
        queue_bk[p].item_status=FINISH;
    }
    else if (user==JU)
    {
        queue_bk[p].exe_addr=cmt_bus.addr;
        queue_bk[p].exe_jump=cmt_bus.jump_if;
        queue_bk[p].imm=cmt_bus.res;
        queue_bk[p].item_status=FINISH;
    }
}
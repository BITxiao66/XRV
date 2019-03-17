/********************************************************************************
 * Files         : mem.c
 * Description   : This project is a C simulator for RISC-V, a graduation project
 *               : in (1) ICT,CAS and (2) Beijing Institute of Technology .
 *               : This file is used for simulate the memory module of RV .The 
 *               : memory has two read ports and two write ports with 100 cyclces  
 *               : access delay . The memory line size equal to 256 bits and total
 *               : capacity is 64MB .
 * Author        : xiaoziyuan 
 * Last Modified : 2019.03.07
 * Version       : V 0.1
 ********************************************************************************/
#include "define.h"
#include "reg_bus.h"
#include "issue.h"
#include "cmt.h"

void JUModule()
{
    BYTE jump_if=0;
    int jump_addr=-1;
    memset(&ju_out_bk,0,sizeof(ju_out));
    if (!station[JU].valid) 
    {
        return;
    }
    if (station[JU].Qi>=8&&station[JU].Qj>=8) 
    {
        switch (station[JU].op)
        {
            case JU_JAL:
                jump_if=1;
                jump_addr=station[JU].ins_addr+station[JU].imm;// mul 2 at fetch phase
                break;

            case JU_BEQ:
                jump_if=(station[JU].Vi==station[JU].Vj)?1:0;
                jump_addr=jump_if?station[JU].ins_addr+station[JU].imm:station[JU].ins_addr+4;
                break;

            case JU_BNE:
                jump_if=(station[JU].Vi!=station[JU].Vj)?1:0;
                jump_addr=jump_if?station[JU].ins_addr+station[JU].imm:station[JU].ins_addr+4;
                break;

            default:
                break;
        }
        ju_out_bk.id=station[JU].id;
        ju_out_bk.jump_if=jump_if;
        ju_out_bk.addr=jump_addr;
        ju_out_bk.res=station[JU].ins_addr+4;
        ju_out_bk.valid=1;
        if (cmt_vie_bk==0) 
        {
            cmt_vie_bk=JU;
            if(issue_write[JU]==0)
            {
                memset(&station_bk[JU],0,sizeof(station[JU]));
            }
        }
    }
    else if (station[JU].Qi<8) // snoop Vi from commit bus
    {
        if (cmt_bus.valid==1 && cmt_bus.id==station[JU].Qi) 
        {
            station_bk[JU].Qi=8;
            station_bk[JU].Vi=cmt_bus.res;
        }  
    }
    else if (station[JU].Qj<8)
    {
        if (cmt_bus.valid==1 && cmt_bus.id==station[JU].Qj) 
        {
            station_bk[JU].Qj=8;
            station_bk[JU].Vj=cmt_bus.res;
        }  
    }
}
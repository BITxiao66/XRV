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
# include "define.h"
# include "reg_bus.h"
# include "issue.h"
# include "cmt.h"

void ALUModule()
{
    int res;
    int bus_user;
    WORD tmp1,tmp2;
    memset(&alu_out_bk,0,sizeof(alu_out));
    if (!station[ALU].valid) 
    {
        return;
    }
    if (station[ALU].Qi>=8&&station[ALU].Qj>=8) 
    {
        switch (station[ALU].op)
        {
            case ALU_ADD:
                res = station[ALU].Vi + station[ALU].Vj;
                break;

            case ALU_SUB:
                res = station[ALU].Vi - station[ALU].Vj;
                break;

            case ALU_SLL:
                res = station[ALU].Vi << (station[ALU].Vj%32);
                break;
            
            case ALU_SLT:
                res = (station[ALU].Vi < station[ALU].Vj)?1:0;
                break;

            case ALU_SLTU:
                tmp1=station[ALU].Vi;
                tmp2=station[ALU].Vj;
                res = (tmp1 < tmp2)?1:0;
                break;

            case ALU_XOR:
                res = station[ALU].Vi ^ station[ALU].Vj;
                break;

            case ALU_SRL:
                tmp1=station[ALU].Vi;
                tmp2=station[ALU].Vj;
                res= tmp1 >> (tmp2%32);
                break;

            case ALU_SRA:
                res = station[ALU].Vi>>(station[ALU].Vj%32);
                break;
            
            case ALU_OR:
                res = station[ALU].Vi | station[ALU].Vj;
                break;
            
            case ALU_AND:
                res = station[ALU].Vi & station[ALU].Vj;
                break;
            
            case ALU_AUI:
                res = station[ALU].ins_addr+station[ALU].Vj ;

            case ALU_LUI:
                res = station[ALU].Vj;

            default:
                break;
        }
        alu_out_bk.id=station[ALU].id;
        alu_out_bk.res=res;
        alu_out_bk.valid=1;
        if (cmt_vie_bk==0) 
        {
            cmt_vie_bk=ALU;
            if(issue_write[ALU]==0)
            {
                memset(&station_bk[ALU],0,sizeof(station[ALU]));
            }
        }
    }
    if (station[ALU].Qi<8) // snoop Vi from commit bus
    {
        if (cmt_bus.valid==1 && cmt_bus.id==station[ALU].Qi) 
        {
            station_bk[ALU].Qi=8;
            station_bk[ALU].Vi=cmt_bus.res;
        }  
    }
    if (station[ALU].Qj<8)
    {
        if (cmt_bus.valid==1 && cmt_bus.id==station[ALU].Qj) 
        {
            station_bk[ALU].Qj=8;
            station_bk[ALU].Vj=cmt_bus.res;
        }  
    }
}

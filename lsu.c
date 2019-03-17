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
#include "lsu.h"
#include "RV.h"
#include "issue.h"
#include "cmt.h"

extern char read_port_data[CACHE_LINE];

int DcacheWrite(WORD addr, int data ,int leng)
{
    int group = (addr/CACHE_LINE)%32;
    int line = 0;
    int i = 0;
    int complete =0;
    if(dcache_busy)
    {
        return complete;
    }
    for( i = 0; i < 4; i++)
    {
        if (((dcache_tag[group][i] & 0xFFFFFC00) == (addr&0xFFFFFC00))&&((dcache_tag[group][i]&0x000000FF))) 
        {
            line = i;
            break;
        }     
    }
    int hit = i<4?1:0;
    if(hit)
    {
        complete = 1;
        for( i = 0; i < leng; i++)
        {
            dcache_bak[group][line][addr%CACHE_LINE+i]=(data>>(8*i))&0xFF;
        }
        dcache_tag_bk[group][line]&= 0xFFFFFC00;
        dcache_tag_bk[group][line]|= 0x00000002;
        d_lru[group] = line;
        dcache_need_refresh = 1;
        return 1;
    }
    else
    {
        //DcacheSwapWrite(addr,data,leng);
        return 0;
    } 
}

int DcacheSwapWrite(WORD addr,int data,int leng)
{
    if (dcache_user!=SU && dcache_user!=DCACHE_FREE) 
    {
        return 0;
    }
    if (dcache_busy) 
    {
        if (!mem_write2_ready) 
        {
            return 0;
        }
        else
        {
            switch (leng)
            {
                case 1:
                    WriteMemByte(addr,data);
                    break;
            
                case 2:
                    WriteMemHalfword(addr,data);
                    break;

                case 4:
                    WriteMemWord(addr,data);
                    break;

                default:
                    break;
            }
            dcache_user=DCACHE_FREE;
            dcache_busy_bk=0;
            return 1;
        } 
    }
    else if (dcache_user==DCACHE_FREE)
    {
        dcache_busy_bk =1;
        dcache_user=SU;
        ApplyWritePort(2);
        return 0;
    }
}

s_dcache_res DcacheRead(WORD addr)
{
    int group = (addr/CACHE_LINE)%32;
    int line = 0;
    int i = 0;
    dcache_res.hit =0;
    dcache_res.data =0;
    if(dcache_busy)
    {
        return dcache_res;
    }
    for( i = 0; i < 4; i++)
    {
        if (((dcache_tag[group][i] & 0xFFFFFC00) == (addr&0xFFFFFC00))&&((dcache_tag[group][i]&0x000000FF))) 
        {
            line = i;
            break;
        }     
    }
    int hit = i<4?1:0;
    if(hit)
    {
        dcache_res.hit = 1;
        dcache_res.data = 0;
        for( i = 3; i >= 0; i--)
        {
            dcache_res.data <<= 8;
            dcache_res.data |= dcache[group][line][addr%CACHE_LINE+i];
        }
        d_lru[group] = line;
    }
    else
    {
        DcacheSwapRead(addr);
    } 
    return dcache_res;
}

void ReleaseLUResource()
{
    if (dcache_user==LU) 
    {
        dcache_user=DCACHE_FREE;
        dcache_busy=dcache_busy_bk=0;
    }   
}

void DcacheSwapRead(int addr)
{
    static int line=0;
    static int flag=0;
    if (dcache_user!=LU && dcache_user!=DCACHE_FREE) 
    {
        return;
    }
    if (dcache_busy) 
    {
        if (!mem_write2_ready) 
        {
            return;
        }
        else if (mem_write2_ready&&!mem_read2_ready&&!flag)
        {
            flag=1;
            WriteMemLine(addr,dcache[(addr/CACHE_LINE)%32][line]);
            ApplyReadPort(2);
        }
        else if (mem_write2_ready&&!mem_read2_ready&&flag)
        {
            return;
        }
        else if (mem_write2_ready&&mem_read2_ready)
        {
            ReadMemLine(addr);
            memcpy(dcache_bak[(addr/CACHE_LINE)%32][line],read_port_data,CACHE_LINE);
            dcache_tag_bk[(addr/CACHE_LINE)%32][line]=(addr&0xFFFFFC00)|1;
            dcache_need_refresh = 1;
            dcache_busy_bk = 0;
            flag=0;
            dcache_user=DCACHE_FREE;
            return;
        }
          
    }
    else if (dcache_user==DCACHE_FREE) 
    {
        line = (d_lru[(addr/CACHE_LINE)%32]+1)%4;
        dcache_busy_bk =1;
        dcache_user=LU;
        if (dcache_tag[(addr/CACHE_LINE)%32][line]&0x000000FF==DIRTY) 
        {
            flag=0;
            ApplyWritePort(2);
        }
        else
        {
            ApplyReadPort(2);
            flag=1;
        }
    }
}

void SUModule()
{
    if (!station[SU].valid) 
    {
        return;
    }
    int leng;
    switch (station[SU].op)
    {
        case 1:
            /* code */
            break;
    
        default:
            break;
    }
    if (DcacheWrite(station[SU].Vi+station[SU].imm,station[SU].Vj,leng)) 
    {
        station_bk[SU].valid=0;
        return;
    }
    if (DcacheSwapWrite(station[SU].Vi+station[SU].imm,station[SU].Vj,leng)) 
    {
        station_bk[SU].valid=0;
        return;
    }
    
}

void LUModule()
{
    if (!station[LU].valid) 
    {
        return;
    }
    if (station[LU].Qj<8)   // snoop Vj from commit bus
    {
        if (cmt_bus.valid==1 && cmt_bus.id==station[LU].Qj) 
        {
            station_bk[LU].Qj=8;
            station_bk[LU].Vj=cmt_bus.res;
        }  
    }
    else
    {
        int addr=station[LU].Vj+station[LU].imm;
        DcacheRead(addr);
        if (dcache_busy) 
        {
            DcacheSwapRead(addr);
        }
        if (dcache_res.hit) 
        {
            // just lw instruction can use 
            load_out.valid=1;
            load_out.addr=addr;
            load_out.id=station[LU].id;
            load_out.res=dcache_res.data;
            if (cmt_vie_bk==0) 
            {
                cmt_vie_bk=LU;
                if(issue_write[LU]==0)
                {
                    memset(&station_bk[LU],0,sizeof(station[LU]));
                }
            }

        }
        else if (!station[LU].addr_ready)
        {
            load_out.valid=2;
            load_out.addr=addr;
            load_out.id=station[LU].id;
            if (cmt_vie_bk==0) 
            {
                cmt_vie_bk=LU;
                if(issue_write[LU]==0)
                {
                    station_bk[LU].addr_ready=1;
                }
            }
        }
    }
    
}
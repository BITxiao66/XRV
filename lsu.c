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

extern char read_port_data[CACHE_LINE];

void DcacheWrite(WORD addr, char* data ,int leng)
{

}

s_dcache_res DcacheRead(WORD addr)
{
    int group = (addr/CACHE_LINE)%8;
    int line = 0;
    int i = 0;
    dcache_res.hit =0;
    dcache_res.ins =0;
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
        dcache_res.ins = 0;
        for( i = 3; i >= 0; i--)
        {
            dcache_res.ins <<= 8;
            dcache_res.ins |= dcache[group][line][addr%256+i];
        }
        d_lru[group] = line;
    }
    else
    {
        DcacheSwapRead(addr);
    } 
    return dcache_res;
}

void DcacheSwapRead(int addr)
{
    static int line=0;
    static int flag=0;
    if (decache_user!=LU && decache_user!=DCACHE_FREE) 
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
            WriteMemLine(addr,dcache[(addr/256)%8][line]);
            ApplyReadPort(2);
        }
        else if (mem_write2_ready&&!mem_read2_ready&&flag)
        {
            return;
        }
        else if (mem_write2_ready&&mem_read2_ready)
        {
            ReadMemLine(addr);
            memcpy(dcache_bak[(addr/256)%8][line],read_port_data,CACHE_LINE);
            dcache_tag_bk[(addr/256)%8][line]=(addr&0xFFFFFC00)|1;
            dcache_need_refresh = 1;
            dcache_busy_bk = 0;
            flag=0;
            return;
        }
          
    }
    
    else
    {
        line = (d_lru[(addr/256)%8]+1)%4;
        dcache_busy_bk =1;
        if (dcache_tag[(addr/256)%8][line]&0x000000FF==DIRTY) 
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
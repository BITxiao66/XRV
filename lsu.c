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
#include <string.h>

extern char read_port_data[CACHE_LINE];

int DcacheWrite(WORD addr, int data ,int leng)
{
    int group = (addr/CACHE_LINE)%32;
    int line = 0;
    int i = 0;
    int complete =0;
    if (dcache_user==SU ) 
    {
        return 0;
    }
    for( i = 0; i < 4; i++)
    {
        if (((dcache_tag[group][i] & 0xFFFFFFE0) == (addr&0xFFFFFFE0))&&((dcache_tag[group][i]&0x00000003)>0)) 
        {
            line = i;
            break;
        }     
    }
    int hit = i<4?1:0;
    if(hit)
    {
        complete = 1;
        for( i = 0; i < leng && i<4; i++)
        {
            dcache_bak[group][line][(addr+i)%CACHE_LINE]=(data>>(8*i))&0x000000FF;
        }
        dcache_tag_bk[group][line]&= 0xFFFFFFE0;
        dcache_tag_bk[group][line]|= 0x00000002;
        d_lru[group] = line;
        dcache_need_refresh = 1;
        return 1;
    }
    else if (dcache_user==DCACHE_FREE)
    {
        DcacheSwapWrite(addr,data,leng);
        return 0;
    }
    return 0; 
}

int DcacheSwapWrite(WORD addr,int data,int leng)
{
    static int line=0;
    static int flag=0;
    static int store_addr=0;
    if (dcache_user!=SU && dcache_user!=DCACHE_FREE) 
    {
        return 0;
    }
    if (dcache_user==SU) 
    {
        if (!mem_write2_ready) 
        {
            return 0;
        }
        else if (mem_write2_ready&&!flag)
        {
            flag=1;
            dcache_tag[(store_addr/CACHE_LINE)%32][line]=0;
            dcache_need_refresh = 1;
            WriteMemLine(store_addr,dcache[(store_addr/CACHE_LINE)%32][line]);
            ApplyReadPort(2);
        }
        else if (mem_write2_ready&&!mem_read2_ready&&flag)
        {
            return 0;
        }
        else if (mem_write2_ready&&mem_read2_ready)
        {
            ReadMemLine(addr);
            memcpy(dcache_bak[(addr/CACHE_LINE)%32][line],read_port_data,CACHE_LINE);
            int i;
            for( i = 0; i < leng && i<4; i++)
            {
                dcache_bak[(addr/CACHE_LINE)%32][line][(addr+i)%CACHE_LINE]=(data>>(8*i))&0x000000FF;
            }
            dcache_tag_bk[(addr/CACHE_LINE)%32][line]=(addr&0xFFFFFFE0)|0X00000002;
            dcache_need_refresh = 1;
            dcache_busy_bk = 0;
            flag=0;
            dcache_user=DCACHE_FREE;
            return 1;
        }
    }
    else if (dcache_user==DCACHE_FREE) 
    {
        line = (d_lru[(addr/CACHE_LINE)%32]+1)%4;
        dcache_busy_bk =1;
        dcache_user=SU;
        if ((dcache_tag[(addr/CACHE_LINE)%32][line]&0x00000003)==DIRTY) 
        {
            flag=0;
            store_addr=dcache_tag[(addr/CACHE_LINE)%32][line]&0xFFFFFFE0;
            ApplyWritePort(2);
        }
        else
        {
            ApplyReadPort(2);
            flag=1;
        }
    }
    return 0;
}

s_dcache_res DcacheRead(WORD addr,int lu)
{
    int group = (addr/CACHE_LINE)%32;
    int line = 0;
    int i = 0;
    tmp_res.hit =0;
    tmp_res.data =0;
    if (dcache_user==lu ) 
    {
        return tmp_res;
    }
    for( i = 0; i < 4; i++)
    {
        if (((dcache_tag[group][i] & 0xFFFFFFE0) == (addr&0xFFFFFFE0)) && ((dcache_tag[group][i]&0x00000003)>0)) 
        {
            line = i;
            break;
        }     
    }
    int hit = i<4?1:0;
    if(hit)
    {
        tmp_res.hit = 1;
        tmp_res.data = 0;
        for( i = 3; i >= 0; i--)
        {
            tmp_res.data <<= 8;
            tmp_res.data |= dcache[group][line][(addr+i)%CACHE_LINE];
        }
        d_lru[group] = line;
    }
    else if (dcache_user==DCACHE_FREE)
    {
        DcacheSwapRead(addr,lu);
    } 
    return tmp_res;
}

s_dcache_res DcacheRead_bk(WORD addr,int lu)
{
    int group = (addr/CACHE_LINE)%32;
    int line = 0;
    int i = 0;
    tmp_res.hit =0;
    tmp_res.data =0;
    if (dcache_user==lu ) 
    {
        return tmp_res;
    }
    for( i = 0; i < 4; i++)
    {
        if (((dcache_tag_bk[group][i] & 0xFFFFFFE0) == (addr&0xFFFFFFE0)) && ((dcache_tag_bk[group][i]&0x00000003)>0)) 
        {
            line = i;
            break;
        }     
    }
    int hit = i<4?1:0;
    if(hit)
    {
        tmp_res.hit = 1;
        tmp_res.data = 0;
        for( i = 3; i >= 0; i--)
        {
            tmp_res.data <<= 8;
            tmp_res.data |= dcache_bak[group][line][(addr+i)%CACHE_LINE];
        }
        d_lru[group] = line;
    }
    else
    {
        DcacheSwapRead(addr,lu);
    } 
    return tmp_res;
}

void ReleaseLUResource()
{
    if (dcache_user==LU||dcache_user==LU2) 
    {
        dcache_user=DCACHE_FREE;
        dcache_busy=dcache_busy_bk=0;
    }   
}

void DcacheSwapRead(int addr,int lu)
{
    static int line=0;
    static int flag=0;
    static int store_addr=0;
    if (dcache_user!=lu && dcache_user!=DCACHE_FREE) 
    {
        return;
    }
    
    if (dcache_user==lu) 
    {
        if (!mem_write2_ready) 
        {
            return;
        }
        else if (mem_write2_ready&&!flag)
        {
            dcache_tag[(store_addr/CACHE_LINE)%32][line]=0;
            dcache_need_refresh = 1;
            flag=1;
            WriteMemLine(store_addr,dcache[(store_addr/CACHE_LINE)%32][line]);
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
            dcache_tag_bk[(addr/CACHE_LINE)%32][line]=(addr&0xFFFFFFE0)|0X00000001;
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
        dcache_user=lu;
        if ((dcache_tag[(addr/CACHE_LINE)%32][line]&0x00000003)==DIRTY) 
        {
            flag=0;
            store_addr=dcache_tag[(addr/CACHE_LINE)%32][line]&0xFFFFFFE0;
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
    int leng=4;
    switch (station[SU].op)
    {
        case SU_SB:
            leng=1;
            break;
    
        case SU_SH:
            leng=2;
            break;
        
        case SU_SW:
            leng=4;
            break;

        default:
            break;
    }
    if (station[SU].Vi+station[SU].imm==0x10000000) 
    {
        printf("%c",station[SU].Vj);
        tem[strlen(tem)]=station[SU].Vj;
        station_bk[SU].valid=0;
        return;
    }
    if (DcacheWrite(station[SU].Vi+station[SU].imm,station[SU].Vj,leng)) 
    {
        station_bk[SU].valid=0;
        return;
    }
    if (dcache_user==SU&&dcache_busy) 
    {
        if(DcacheSwapWrite(station[SU].Vi+station[SU].imm,station[SU].Vj,leng))
        {
            station_bk[SU].valid=0;
        }
    }
}

void LUModule()
{
    if (!station[LU].valid) 
    {
        return;
    }
    if (station[LU].Qi<QUEUE_SIZE)   // snoop Vj from commit bus
    {
        if (cmt_bus.valid==1 && cmt_bus.id==station[LU].Qi) 
        {
            station_bk[LU].Qi=QUEUE_SIZE;
            station_bk[LU].Vi=cmt_bus.res;
        }  
        if (cmt_bus2.valid==1 && cmt_bus2.id==station[LU].Qi) 
        {
            station_bk[LU].Qi=QUEUE_SIZE;
            station_bk[LU].Vi=cmt_bus2.res;
        }  
    }
    else
    {
        int addr=station[LU].Vi+station[LU].imm;
        if (dcache_need_refresh) 
        {
            dcache_res=DcacheRead_bk(addr,LU);
        }
        else
        {
            dcache_res=DcacheRead(addr,LU);
        }
        if (dcache_user==LU&&dcache_busy) 
        {
            DcacheSwapRead(addr,LU);
        }
        if (dcache_res.hit) 
        {
            load_out_bk.valid=1;
            load_out_bk.addr=addr;
            load_out_bk.id=station[LU].id;
            load_out_bk.res=dcache_res.data;
            switch (station[LU].op)
            {
                case LU_LB:
                    load_out_bk.res<<=24;
                    load_out_bk.res>>=24;
                    break;

                case LU_LH:
                    load_out_bk.res<<=16;
                    load_out_bk.res>>=16;
                    break;
            
                case LU_LBU:
                    load_out_bk.res&=0x000000FF;
                    break;

                case LU_LHU:
                    load_out_bk.res&=0x0000FFFF;
                    break;

                default:
                    break;
            }
    
            if (cmt_vie_bk==0) 
            {
                cmt_vie_bk=LU;
                memset(&station_bk[LU],0,sizeof(station[LU]));
            }
            /*else if (cmt_vie_bk2==0) 
            {
                cmt_vie_bk2=LU;
                memset(&station_bk[LU],0,sizeof(station[LU]));
            }*/

        }
        else if (!station[LU].addr_ready)
        {
            load_out_bk.valid=2;
            load_out_bk.addr=addr;
            load_out_bk.id=station[LU].id;
            if (cmt_vie_bk==0) 
            {
                cmt_vie_bk=LU;
                if(issue_write[LU]==0)
                {
                    station_bk[LU].addr_ready=1;
                }
            }
            /*else if (cmt_vie_bk2==0) 
            {
                cmt_vie_bk2=LU;
                if(issue_write[LU]==0)
                {
                    station_bk[LU].addr_ready=1;
                }
            }*/
        }
    }
}

void LU_2_Module()
{
    if (!station[LU2].valid) 
    {
        return;
    }
    if (station[LU2].Qi<QUEUE_SIZE)   // snoop Vj from commit bus
    {
        if (cmt_bus.valid==1 && cmt_bus.id==station[LU2].Qi) 
        {
            station_bk[LU2].Qi=QUEUE_SIZE;
            station_bk[LU2].Vi=cmt_bus.res;
        } 
        if (cmt_bus2.valid==1 && cmt_bus2.id==station[LU2].Qi) 
        {
            station_bk[LU2].Qi=QUEUE_SIZE;
            station_bk[LU2].Vi=cmt_bus2.res;
        }   
    }
    else
    {
        int addr=station[LU2].Vi+station[LU2].imm;
        if (dcache_need_refresh) 
        {
            dcache_res2=DcacheRead_bk(addr,LU2);
        }
        else
        {
            dcache_res2=DcacheRead(addr,LU2);
        }
        if (dcache_user==LU2&&dcache_busy) 
        {
            DcacheSwapRead(addr,LU2);
        }
        if (dcache_res2.hit) 
        {
            load_out2_bk.valid=1;
            load_out2_bk.addr=addr;
            load_out2_bk.id=station[LU2].id;
            load_out2_bk.res=dcache_res2.data;
            switch (station[LU2].op)
            {
                case LU_LB:
                    load_out2_bk.res<<=24;
                    load_out2_bk.res>>=24;
                    break;

                case LU_LH:
                    load_out2_bk.res<<=16;
                    load_out2_bk.res>>=16;
                    break;
            
                case LU_LBU:
                    load_out2_bk.res&=0x000000FF;
                    break;

                case LU_LHU:
                    load_out2_bk.res&=0x0000FFFF;
                    break;

                default:
                    break;
            }
    
            /*if (cmt_vie_bk==0) 
            {
                cmt_vie_bk=LU2;
                memset(&station_bk[LU2],0,sizeof(station[LU2]));
            }
            else*/ if (cmt_vie_bk2==0) 
            {
                cmt_vie_bk2=LU2;
                memset(&station_bk[LU2],0,sizeof(station[LU2]));
            }

        }
        else if (!station[LU2].addr_ready)
        {
            load_out2_bk.valid=2;
            load_out2_bk.addr=addr;
            load_out2_bk.id=station[LU2].id;
            /*if (cmt_vie_bk==0) 
            {
                cmt_vie_bk=LU2;
                if(issue_write[LU2]==0)
                {
                    station_bk[LU2].addr_ready=1;
                }
            }
            else*/ if (cmt_vie_bk2==0) 
            {
                cmt_vie_bk2=LU2;
                if(issue_write[LU2]==0)
                {
                    station_bk[LU2].addr_ready=1;
                }
            }
        }
    }
}
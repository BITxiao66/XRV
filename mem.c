/********************************************************************************
 * Files         : mem.c
 * Description   : This project is a C simulator for RISC-V, a graduation project
 *               : in (1) ICT,CAS and (2) Beijing Institute of Technology .
 *               : This file is used for simulate the memory module of RV .The 
 *               : memory has two read ports and two write ports with 100 cyclces  
 *               : access delay . The memory line size equal to 32 bytes and total
 *               : capacity is 64MB .
 *               : Useage: a write or read request from cache apply a read_count/
 *               : write_count, when 100 cycles passed ,corresponding mem_ready
 *               : value change to TRUE ,at that time cache read/write memory line.
 * Author        : xiaoziyuan 
 * Last Modified : 2019.04.05
 * Version       : V 1.0
 ********************************************************************************/

#include "define.h"
#include "reg_bus.h"
#include "lsu.h"
#include <stdio.h>
#include <stdlib.h>
static unsigned char mem_arry[MEM_SIZE/CACHE_LINE][CACHE_LINE];
static long long read1_count;
static long long read2_count;
static long long write1_count;
static long long write2_count;
int mem_pc;
extern int mem_read1_ready,mem_read1_ready_bk;
extern int mem_read2_ready,mem_read2_ready_bk;
extern int mem_write1_ready,mem_write1_ready_bk;
extern int mem_write2_ready,mem_write2_ready_bk;
char read_port_data[CACHE_LINE];

int GetHexValue(char a,char b)
{
    return (a>'9'?a-55:a-'0')*16+(b>'9'?b-55:b-'0');
}

int LoadMemFromFile(char* file_name,int little_endian,unsigned int begin_addr)
{
    FILE* file = fopen(file_name,"r");
    if(!file)
    {
        printf("Load file can not open\n");
        return 0;
    }
    if(begin_addr>=MEM_SIZE||begin_addr%4)
    {
        printf("Invalid begin addr\n");
        return 0;    
    }
    if(little_endian>1)
    {
        printf("Invalid mode\n");
        return 0;
    }
    char str[20];
    int p = begin_addr;
    int i;
    while(fscanf(file,"%s",str)!=EOF)
    {
        if(str[0]=='#')
        {
            p = atoi(str+1);
            if(p%4)
            {
                printf("Invalid jump addr in file\n");
                return 0;
            }
            continue;
        }
        if(!little_endian)
            for(i=0;i<8;i+=2,p++)
                mem_arry[p/CACHE_LINE][p%CACHE_LINE]=GetHexValue(str[i],str[i+1]);
        else
            for(i=6;i>=0;i-=2,p++)
                mem_arry[p/CACHE_LINE][p%CACHE_LINE]=GetHexValue(str[i],str[i+1]);        
    }
    fclose(file);
    return 1;
}

void LoadMemFromHex(char* file_name)
{
    FILE* file = fopen(file_name,"r");
    if(!file)
    {
        printf("Load file can not open\n");
        return 0;
    }
    char str[100];
    int p = 0;
    int i;
    while(fscanf(file,"%s",str)!=EOF)
    {
        if(str[0]=='@')
        {
            p = atoi(str+1);
            if(p%4)
            {
                printf("Invalid jump addr in file\n");
                return 0;
            }
            continue;
        }
        mem_arry[p/CACHE_LINE][p%CACHE_LINE]=GetHexValue(str[i],str[i+1]); 
        p++;     
    }
}

void MemReset()
{
    read1_count=read2_count=0;
    mem_read1_ready=mem_read2_ready=0;
    write1_count=write2_count=0;
    mem_write1_ready=mem_write2_ready=0;
    mem_read1_ready_bk=mem_read2_ready_bk=0;
    mem_write1_ready_bk=mem_write2_ready_bk=0;
    mem_pc=0;
    memset(mem_arry,0,sizeof(mem_arry));
}

 /********************************************************************************
 * Description : Simulate a cycle and update status register .
 *             : If a read or write request has passed 100 cycles,let it's data
 *             : enble .
 * Prameter    : void
 * Return      : void
 * Side effect : no
 * Date        : 2019.03.07
 ********************************************************************************/
void MemUpdateStatus()
{
    mem_pc++;
    if(mem_pc-read1_count==100)
        mem_read1_ready_bk=1;
    if(mem_pc-read2_count==100)
        mem_read2_ready_bk=1;
    if(mem_pc-write1_count==100)
        mem_write1_ready_bk=1;
    if(mem_pc-write2_count==100)
        mem_write2_ready_bk=1;
}

int ApplyReadPort(int cache_port)
{
    if(cache_port==1)
    {
        read1_count = mem_pc;
        mem_read1_ready_bk = 0;
    }
    else
    {
        read2_count = mem_pc;
        mem_read2_ready_bk = 0;
    }
}

int ApplyWritePort(int cache_port)
{
    if(cache_port==1)
    {
        write1_count = mem_pc;
        mem_write1_ready_bk = 0;
    }
    else
    {
        write2_count = mem_pc;
        mem_write2_ready_bk = 0;
    }
}


 /********************************************************************************
 * Description : Read whole memory line from memory
 *             : The reader should apply the readport and wait ,before got data .
 * Prameter    : int ,      The addr of first bit
 * Return      : char* ,    The pointer of back data 
 * Side effect : no
 * Author      : xiaoziyuan
 * Date        : 2019.03.07
 ********************************************************************************/
char* ReadMemLine(int addr)
{
    int p;
    if (addr<0) 
    {
        mem_arry[addr][addr]=0;
    }
    for( p = 0; p < CACHE_LINE; p++)
    {
        read_port_data[p]=mem_arry[addr/CACHE_LINE][p];
    }
    return read_port_data;
}

int ReadMemWord(int addr)
{
    DcacheRead(addr);
    if (dcache_res.hit) 
    {
        return dcache_res.data;
    }
    
    int a=0;
    int i;
    for( i = 3; i >=0 ; i--)
    {
        a<<=8;
        a|=(mem_arry[addr/CACHE_LINE][addr%CACHE_LINE+i]) & 0x000000FF;
    }
    return a;
}

void WriteMemLine(int addr,char* src)
{
    int p;
    for( p = 0; p < CACHE_LINE; p++)
    {
        mem_arry[addr/CACHE_LINE][p] = src[p];
    }
}

void WriteMemByte(int addr,int src)
{
    int p;
    for( p = 0; p < 1; p++)
    {
        mem_arry[addr/CACHE_LINE][addr%CACHE_LINE+p] = (src>>(8*p))&0xFF;
    }
}

void WriteMemHalfword(int addr,int src)
{
    int p;
    for( p = 0; p < 2; p++)
    {
        mem_arry[addr/CACHE_LINE][addr%CACHE_LINE+p] = (src>>(8*p))&0xFF;
    }
}

void WriteMemWord(int addr,int src)
{
    int p;
    for( p = 0; p < 4; p++)
    {
        mem_arry[addr/CACHE_LINE][addr%CACHE_LINE+p] = (src>>(8*p))&0xFF;
    }
}
/********************************************************************************
 * Files         : reg_bus.h
 * Description   : This project is a C simulator for RISC-V, a graduation project
 *               : in (1) ICT,CAS and (2) Beijing Institute of Technology .
 *               : This file is the head file to define all registers (include 
 *               : architecture and no-architecture ) and buses (used for transfer  
 * Author        : xiaoziyuan 
 * Last Modified : 2019.03.07
 * Version       : V 0.1
 ********************************************************************************/

#include "define.h"
#include "reg_bus.h"
#include "RV.h"
#include "fetch.h"
#include "queue.h"
#include "issue.h"
#include "cmt.h"
#include <stdlib.h>

void CycleBegin()
{
    mem_read1_ready_bk = mem_read1_ready;
    icache_write_busy_bk=icache_write_busy;
    if (icache_need_refresh) 
    {
        memcpy(icache_bak,icache,sizeof(icache));
        memcpy(icache_tag_bk,icache_tag,sizeof(icache_tag));
        icache_need_refresh = 0;
    }
    memcpy(queue_bk,queue,sizeof(queue));
    queue_head_bk=queue_head;
    queue_tail_bk=queue_tail;
    memcpy(reg_bk,reg,sizeof(reg));
    memcpy(station_bk,station,sizeof(station));
}

void CycleEnd()
{
    mem_read1_ready = mem_read1_ready_bk;
    icache_write_busy = icache_write_busy_bk;
    if (icache_need_refresh) 
    {
        memcpy(icache,icache_bak,sizeof(icache));
        memcpy(icache_tag,icache_tag_bk,sizeof(icache_tag));
    }
    //memcpy(&deliver12,&deliver12_bk,sizeof(deliver12));
    deliver12=deliver12_bk;
    memcpy(queue,queue_bk,sizeof(queue));
    queue_head=queue_head_bk;
    queue_tail=queue_tail_bk;
    queue_full=(queue_head==queue_tail&&queue[queue_head].item_status)?1:0;
    memcpy(reg,reg_bk,sizeof(reg));
    memset(issue_write,0,sizeof(issue_write));
    memcpy(station,station_bk,sizeof(station));
    memcpy(&alu_out,&alu_out_bk,sizeof(alu_out));
    CmtBusUpdate();
}


int main()
{ 
    ResetDecodeTable();
    ResetQueue();
    MemReset();
    LoadMemFromFile("in.txt",1,0);
    reg[5]=1;
    reg[6]=2;
    while(1)
    {
        CycleBegin();
        MemUpdateStatus();
        FetchModule();
        QueueModule();
        Issue();
        ALUModule();
        Commit();
        CycleEnd();
    }
  
    return 0;
}



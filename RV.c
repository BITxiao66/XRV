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
    if (pred_need_fresh) 
    {
        memcpy(pred_bk,pred,sizeof(pred));
        pred_need_fresh=0;
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
    if (pred_need_fresh) 
    {
        memcpy(pred,pred_bk,sizeof(pred));
    }
    //memcpy(&deliver12,&deliver12_bk,sizeof(deliver12));
    deliver12=deliver12_bk;
    memcpy(queue,queue_bk,sizeof(queue));
    queue_head=queue_head_bk;
    queue_tail=queue_tail_bk;
    queue_full=(queue_head==((queue_tail+1)%QUEUE_SIZE)&&queue[queue_head].item_status)?1:0;
    memcpy(reg,reg_bk,sizeof(reg));
    reg[0]=0;
    memset(issue_write,0,sizeof(issue_write));
    memcpy(station,station_bk,sizeof(station));
    memcpy(&alu_out,&alu_out_bk,sizeof(alu_out));
    memcpy(&ju_out,&ju_out_bk,sizeof(ju_out));
    CmtBusUpdate();
    if (need_clean_queue) 
    {
        CleanQueue();
        need_clean_queue=0;
    }
    pc=pc5_enble?pc_bk5:pc_bk1;
    pc5_enble=0;
}


int main()
{ 
    ResetDecodeTable();
    ResetQueue();
    pc5_enble=0;
    MemReset();
    LoadMemFromFile("in.txt",1,0);
    reg[1]=1;
    reg[5]=1;
    reg[6]=10;
    while(1)
    {
        CycleBegin();
        MemUpdateStatus();
        FetchModule();
        QueueModule();
        Issue();
        JUModule();
        ALUModule();
        Commit();
        WriteBack();
        CycleEnd();
    }
    return 0;
}

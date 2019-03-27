/********************************************************************************
 * Files         : writeback.c
 * Description   : This project is a C simulator for RISC-V, a graduation project
 *               : in (1) ICT,CAS and (2) Beijing Institute of Technology .
 *               : This file is used for deal with wirteback phase in pipeline ,
 *               : at this phase we will write ALU instruction's result into 
 *               : physical register and check whether branch instruction's predict
 *               : is right ,if not, clean the issue queue and station.
 *               : Finally , we will issue store instruction to SU ,before issue it
 *               : we should judge whether exist a issued load instruction after 
 *               : the store instruction ,if they have same operate addr , clean 
 *               : the queue and station.
 * Author        : xiaoziyuan
 * Last Modified : 2019.03.16
 * Version       : V 0.1
 ********************************************************************************/
#include "define.h"
#include "reg_bus.h"
#include "queue.h"
#include "issue.h"
#include "fetch.h"
#include "RV.h"
#include "cmt.h"

extern long long minstret;

void CleanQueue()
{
    ResetQueue();
    memset(station,0,5*sizeof(s_station_in));
    memset(&station[CSU],0,sizeof(s_station_in));
    cmt_vie=cmt_vie_bk=0;
    memset(&deliver12,0,sizeof(deliver12));
    memset(&deliver12_bk,0,sizeof(deliver12));
    ReleaseLUResource();
}

void UpdatePredPath(BYTE jump_if,int addr)
{
    int old=pred[(addr/4)%PRED_SIZE]&0x00000003;
    int new;
    new = old + (jump_if? 1: -1);
    if (new>3) 
    {
        new=3;
    }
    if (new<0) 
    {
        new=0;
    }
    pred_bk[(addr/4)%PRED_SIZE]&=0xFFFFFFFC;
    pred_bk[(addr/4)%PRED_SIZE]|=new;
}

void WriteBack()
{
    s_queue_item head = queue[queue_head];
    if (head.item_status!=FINISH && head.issue_sta!=SU) 
    {
        return;
    }
    if (head.issue_sta==ALU) 
    {
        WriteReg(head.Rd,head.imm);
        queue_bk[queue_head].item_status=ITEM_FREE;
        queue_head_bk=(queue_head+1)%QUEUE_SIZE;
        minstret++;
    }
    else if (head.issue_sta==MUL_UNIT) 
    {
        WriteReg(head.Rd,head.imm);
        queue_bk[queue_head].item_status=ITEM_FREE;
        queue_head_bk=(queue_head+1)%QUEUE_SIZE;
        minstret++;
    }
    else if (head.issue_sta==CSU) 
    {
        WriteReg(head.Rd,head.imm);
        queue_bk[queue_head].item_status=ITEM_FREE;
        queue_head_bk=(queue_head+1)%QUEUE_SIZE;
        minstret++;
        if (head.imm==0xFFFFFFFF) 
        {
            printf("RV ebreak\n");
            exit(1);
        }
        
    }
    else if (head.issue_sta==LU) 
    {
        WriteReg(head.Rd,head.imm);
        queue_bk[queue_head].item_status=ITEM_FREE;
        queue_head_bk=(queue_head+1)%QUEUE_SIZE; 
        minstret++;
    }
    else if (head.issue_sta==JU)
    {
        UpdatePredPath(head.exe_jump,head.ins_addr);
        if (head.exe_jump) 
        {
            pred_need_fresh=1;
            pred_bk[(head.ins_addr/4)%PRED_SIZE]&=0x00000003;
            pred_bk[(head.ins_addr/4)%PRED_SIZE]|=(head.exe_addr & 0xFFFFFFFC);
        }
        if (head.exe_addr!=head.ins_pred) 
        {
            need_clean_queue=1;
            pc_bk5=head.exe_addr;
            pc5_enble=1;
        }
        if (head.Rd<32) 
        {
            WriteReg(head.Rd,head.imm);
        }
        queue_bk[queue_head].item_status=ITEM_FREE;
        queue_head_bk=(queue_head+1)%QUEUE_SIZE;
        minstret++;
    }
    else if (head.issue_sta==SU)
    {
        int issue = 1;
        int clean = 0;
        int su_addr=ReadReg(head.Qi)+head.imm ;
        int i;
        int j;
        if (station[SU].valid) 
        {
            issue=0;
        }
        for( i = (queue_head+1)%QUEUE_SIZE; i!=queue_tail ;i=(i+1)%QUEUE_SIZE)
        {
            if (queue[i].issue_sta==LU && queue[i].item_status==ISSUED && queue[i].exe_addr==-1) 
            {
                issue=0;
                break;
            }
            if (queue[i].issue_sta==LU && su_addr==queue[i].exe_addr)
            {
                clean=1;
                break;
            }
        }
        if (issue) 
        {
            queue_bk[queue_head].item_status=ITEM_FREE;
            queue_head_bk=(queue_head+1)%QUEUE_SIZE;
            station_bk[SU].Vi=ReadReg(head.Qi);
            station_bk[SU].Vj=ReadReg(head.Qj);
            station_bk[SU].imm=head.imm;
            station_bk[SU].op=head.op_code;
            station_bk[SU].valid=1;
            minstret++;
        }
        if (clean && issue) 
        {
            need_clean_queue=1;
            pc_bk5=head.ins_addr+4;
            pc5_enble=1;
        }
    }
}

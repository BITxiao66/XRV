#include "define.h"
#include "reg_bus.h"
#include "queue.h"
#include "RV.h"

void ResetQueue()
{
    queue_head=queue_head_bk=0;
    queue_tail=queue_tail_bk=0;
    queue_full=0;
    memset(queue,0,sizeof(queue));
    memset(queue_bk,0,sizeof(queue));
}

void QueueModule()
{
    if (!deliver12.ins_valid) 
    {
        return;
    }
    memset(&queue_bk[queue_tail],0,sizeof(s_queue_item));
    queue_bk[queue_tail].issue_sta=deliver12.issue_sta;
    queue_bk[queue_tail].op_code=deliver12.op_code;
    queue_bk[queue_tail].item_status=UNISSUED;
    queue_bk[queue_tail].ins_addr=deliver12.ins_addr;
    queue_bk[queue_tail].jump_if=deliver12.jump_if;
    queue_bk[queue_tail].ins_pred=deliver12.ins_pred;
    if (deliver12.issue_sta==ALU) 
    {
        queue_bk[queue_tail].Rd=deliver12.op_rd;
        queue_bk[queue_tail].Qi=deliver12.op_rs1;
        if (deliver12.data_sel[OP_IMM]) 
        {
            queue_bk[queue_tail].Qj=40;
            queue_bk[queue_tail].Vj=deliver12.op_imm;
        }
        else
        {
            queue_bk[queue_tail].Qj=deliver12.op_rs2;
        }
    }
    else if(deliver12.issue_sta==LU)
    {
        queue_bk[queue_tail].Qi=deliver12.op_rs1;
        queue_bk[queue_tail].Qj=40;
        queue_bk[queue_tail].Rd=deliver12.op_rd;
        queue_bk[queue_tail].imm=deliver12.op_imm;
        queue_bk[queue_tail].exe_addr= -1;
    }
    else if (deliver12.issue_sta==SU) 
    {
        queue_bk[queue_tail].Qi=deliver12.op_rs1;
        queue_bk[queue_tail].Qj=deliver12.op_rs2;
        queue_bk[queue_tail].Rd=40;
        queue_bk[queue_tail].imm=deliver12.op_imm;
    }
    else if (deliver12.issue_sta==JU) 
    {
        queue_bk[queue_tail].imm=deliver12.op_imm;
        queue_bk[queue_tail].Qi=deliver12.data_sel[OP_RS1]?deliver12.op_rs1:40;  
        queue_bk[queue_tail].Qj=deliver12.data_sel[OP_RS2]?deliver12.op_rs2:40;
        queue_bk[queue_tail].Rd=deliver12.data_sel[OP_RD]?deliver12.op_rd:40;
    }
    queue_tail_bk=(queue_tail+1)%QUEUE_SIZE;
}
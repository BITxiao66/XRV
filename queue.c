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
    if (!deliver1.ins_valid) 
    {
        return;
    }
    memset(&queue_bk[queue_tail],0,sizeof(s_queue_item));
    queue_bk[queue_tail].issue_sta=deliver1.issue_sta;
    queue_bk[queue_tail].op_code=deliver1.op_code;
    queue_bk[queue_tail].item_status=UNISSUED;
    queue_bk[queue_tail].ins_addr=deliver1.ins_addr;
    queue_bk[queue_tail].jump_if=deliver1.jump_if;
    queue_bk[queue_tail].ins_pred=deliver1.ins_pred;
    if (deliver1.issue_sta==ALU) 
    {
        queue_bk[queue_tail].Rd=deliver1.op_rd;
        queue_bk[queue_tail].Qi=deliver1.data_sel[OP_RS1]?deliver1.op_rs1:32+QUEUE_SIZE;
        if (deliver1.data_sel[OP_IMM]) 
        {
            queue_bk[queue_tail].Qj=32+QUEUE_SIZE;
            queue_bk[queue_tail].Vj=deliver1.op_imm;
        }
        else
        {
            queue_bk[queue_tail].Qj=deliver1.op_rs2;
        }
    }
    else if (deliver1.issue_sta==MUL_UNIT) 
    {
        queue_bk[queue_tail].Rd=deliver1.op_rd;
        queue_bk[queue_tail].Qi=deliver1.op_rs1;
        queue_bk[queue_tail].Qj=deliver1.op_rs2;
        
    }
    else if(deliver1.issue_sta==LU)
    {
        queue_bk[queue_tail].Qi=deliver1.op_rs1;
        queue_bk[queue_tail].Qj=32+QUEUE_SIZE;
        queue_bk[queue_tail].Rd=deliver1.op_rd;
        queue_bk[queue_tail].imm=deliver1.op_imm;
        queue_bk[queue_tail].exe_addr= -1;
    }
    else if (deliver1.issue_sta==SU) 
    {
        queue_bk[queue_tail].Qi=deliver1.op_rs1;
        queue_bk[queue_tail].Qj=deliver1.op_rs2;
        queue_bk[queue_tail].Rd=32+QUEUE_SIZE;
        queue_bk[queue_tail].imm=deliver1.op_imm;
    }
    else if (deliver1.issue_sta==JU) 
    {
        queue_bk[queue_tail].imm=deliver1.op_imm;
        queue_bk[queue_tail].Qi=deliver1.data_sel[OP_RS1]?deliver1.op_rs1:32+QUEUE_SIZE;  
        queue_bk[queue_tail].Qj=deliver1.data_sel[OP_RS2]?deliver1.op_rs2:32+QUEUE_SIZE;
        queue_bk[queue_tail].Rd=deliver1.data_sel[OP_RD]?deliver1.op_rd:32+QUEUE_SIZE;
    }
    else if (deliver1.issue_sta==CSU) 
    {
        queue_bk[queue_tail].Rd=deliver1.op_rd;
        queue_bk[queue_tail].Qi=deliver1.op_rs1;
        queue_bk[queue_tail].Qj=32+QUEUE_SIZE;
        queue_bk[queue_tail].Vj=deliver1.op_imm;
    }
    queue_tail_bk=(queue_tail+1)%QUEUE_SIZE;

    if (!deliver2.ins_valid) 
    {
        return;
    }
    int tail_next = (queue_tail+1) % QUEUE_SIZE;
    memset(&queue_bk[tail_next],0,sizeof(s_queue_item));
    queue_bk[tail_next].issue_sta=deliver2.issue_sta;
    queue_bk[tail_next].op_code=deliver2.op_code;
    queue_bk[tail_next].item_status=UNISSUED;
    queue_bk[tail_next].ins_addr=deliver2.ins_addr;
    queue_bk[tail_next].jump_if=deliver2.jump_if;
    queue_bk[tail_next].ins_pred=deliver2.ins_pred;
    if (deliver2.issue_sta==ALU) 
    {
        queue_bk[tail_next].Rd=deliver2.op_rd;
        queue_bk[tail_next].Qi=deliver2.data_sel[OP_RS1]?deliver2.op_rs1:32+QUEUE_SIZE;
        if (deliver2.data_sel[OP_IMM]) 
        {
            queue_bk[tail_next].Qj=32+QUEUE_SIZE;
            queue_bk[tail_next].Vj=deliver2.op_imm;
        }
        else
        {
            queue_bk[tail_next].Qj=deliver2.op_rs2;
        }
    }
    else if (deliver2.issue_sta==MUL_UNIT) 
    {
        queue_bk[tail_next].Rd=deliver2.op_rd;
        queue_bk[tail_next].Qi=deliver2.op_rs1;
        queue_bk[tail_next].Qj=deliver2.op_rs2;
        
    }
    else if(deliver2.issue_sta==LU)
    {
        queue_bk[tail_next].Qi=deliver2.op_rs1;
        queue_bk[tail_next].Qj=32+QUEUE_SIZE;
        queue_bk[tail_next].Rd=deliver2.op_rd;
        queue_bk[tail_next].imm=deliver2.op_imm;
        queue_bk[tail_next].exe_addr= -1;
    }
    else if (deliver2.issue_sta==SU) 
    {
        queue_bk[tail_next].Qi=deliver2.op_rs1;
        queue_bk[tail_next].Qj=deliver2.op_rs2;
        queue_bk[tail_next].Rd=32+QUEUE_SIZE;
        queue_bk[tail_next].imm=deliver2.op_imm;
    }
    else if (deliver2.issue_sta==JU) 
    {
        queue_bk[tail_next].imm=deliver2.op_imm;
        queue_bk[tail_next].Qi=deliver2.data_sel[OP_RS1]?deliver2.op_rs1:32+QUEUE_SIZE;  
        queue_bk[tail_next].Qj=deliver2.data_sel[OP_RS2]?deliver2.op_rs2:32+QUEUE_SIZE;
        queue_bk[tail_next].Rd=deliver2.data_sel[OP_RD]?deliver2.op_rd:32+QUEUE_SIZE;
    }
    else if (deliver2.issue_sta==CSU) 
    {
        queue_bk[tail_next].Rd=deliver2.op_rd;
        queue_bk[tail_next].Qi=deliver2.op_rs1;
        queue_bk[tail_next].Qj=32+QUEUE_SIZE;
        queue_bk[tail_next].Vj=deliver2.op_imm;
    }
    queue_tail_bk=(queue_tail+2)%QUEUE_SIZE;
}
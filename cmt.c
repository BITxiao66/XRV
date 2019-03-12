#include "define.h"
#include "reg_bus.h"
#include "queue.h"
#include "issue.h"
#include "cmt.h"

void CmtBusUpdate()
{
    cmt_vie=cmt_vie_bk;
    cmt_vie_bk=0;
    memset(&cmt_bus,0,sizeof(cmt_bus));
    switch (cmt_vie)
    {
        case ALU:
            cmt_bus.valid=alu_out.valid;
            cmt_bus.id=alu_out.id;
            cmt_bus.res=alu_out.res;
            cmt_bus.user=ALU;
            break;

        default:
            break;
    }
}

void Commit()
{
    if (!cmt_bus.valid) 
    {
        return;
    }
    int p;
    p=cmt_bus.id;
    int user=cmt_bus.user;
    if (user==ALU||user==MUL_UNIT) 
    {
        queue_bk[p].imm=cmt_bus.res;
        queue_bk[p].item_status=FINISH;
    }
    else if (user==LU)
    {
        queue_bk[p].imm=cmt_bus.res;
        queue_bk[p].ls_addr=cmt_bus.addr;
        queue_bk[p].item_status=FINISH;
    }
    else if (user==J_UNIT)
    {
        /* code */
    }
}
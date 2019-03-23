# include "define.h"
# include "reg_bus.h"
# include "queue.h"
# include "issue.h"
# include "cmt.h"

WORD ReadReg(BYTE index)
{
    return reg[index];
}

void WriteReg(BYTE index,WORD value)
{
    reg_bk[index]=value;
}

void Issue()
{
    int p=8;
    int i;
    int j;
    for( i = queue_head,j=0; ; i=(i+1)%QUEUE_SIZE)
    {
        if (i==queue_tail&&(j||!queue[queue_head].item_status)) 
        {
            break;
        }
        j++;
        if (queue[i].item_status==UNISSUED&&queue[i].issue_sta!=SU) 
        {
            if (station[queue[i].issue_sta].valid==0) 
            {
                p=i;
                break;
            }
            if (cmt_bus.valid==1 && cmt_bus.user==queue[i].issue_sta) 
            {
                p=i;
                break;
            }
        }
    }
    if (p==8) 
    {
        return;
    }

    queue_bk[p].item_status=ISSUED;
    BYTE Qi=queue[p].Qi;
    BYTE Qj=queue[p].Qj;
    int Vi=queue[p].Vi;
    int Vj=queue[p].Vj;
    if (Qi<40)  // Read vi from queue and result bus
    {
        for( i = (p-1+QUEUE_SIZE)%QUEUE_SIZE; i != (queue_head-1+QUEUE_SIZE)%QUEUE_SIZE; i=(i-1+QUEUE_SIZE)%QUEUE_SIZE)
        {
            if (queue[i].Rd==Qi&&Qi) 
            {
                Qi=i+32;
                if (queue[i].item_status==FINISH) 
                {
                    Vi=queue[i].imm;
                    Qi=40;
                }
                if (cmt_bus.valid==1 && cmt_bus.id==i) 
                {
                    Vi=cmt_bus.res;
                    Qi=40;
                }
                break;
            }
        }
    }
    if (Qj<40) 
    {
        for( i = (p-1+QUEUE_SIZE)%QUEUE_SIZE; i != (queue_head-1+QUEUE_SIZE)%QUEUE_SIZE; i=(i-1+QUEUE_SIZE)%QUEUE_SIZE)
        {
            if (queue[i].Rd==Qj&&Qj) 
            {
                Qj=i+32;
                if (queue[i].item_status==FINISH) 
                {
                    Vj=queue[i].imm;
                    Qj=40;
                }
                if (cmt_bus.valid==1 && cmt_bus.id==i) 
                {
                    Vj=cmt_bus.res;
                    Qj=40;
                }
                break;
            }
        }
    }
    if (Qi<32) // Read Vi from RegFile
    {
        Vi=ReadReg(Qi);
        Qi=40;
    }
    if (Qj<32) 
    {
        Vj=ReadReg(Qj);
        Qj=40;
    }
    Qi -= 32;
    Qj -= 32;

    BYTE issue_sta=queue[p].issue_sta;
    issue_write[issue_sta]==1;
    station_bk[issue_sta].valid=1;
    station_bk[issue_sta].op=queue[p].op_code;
    station_bk[issue_sta].id=p;
    station_bk[issue_sta].imm=queue[p].imm;
    station_bk[issue_sta].Qi=Qi;
    station_bk[issue_sta].Qj=Qj;
    station_bk[issue_sta].Vi=Vi;
    station_bk[issue_sta].Vj=Vj;
    station_bk[issue_sta].ins_addr=queue[p].ins_addr;
    station_bk[issue_sta].addr_ready=0;
    if (queue[p].issue_sta==LU && Qi>=8) 
    {
        queue_bk[p].exe_addr=Vi+queue[p].imm; 
        station_bk[issue_sta].addr_ready=1;
    }
    
}

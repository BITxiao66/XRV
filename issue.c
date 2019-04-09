# include "define.h"
# include "reg_bus.h"
# include "queue.h"
# include "issue.h"
# include "cmt.h"
# include "RV.h"

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
    int p1=QUEUE_SIZE;
    int p2=QUEUE_SIZE;
    int flag1=0; // used for select issue to ALU1 or ALU2
    int flag2=0; // used for selcet issue ot LU1 or LU2
    int i;
    int j;
    for( i = queue_head,j=0; ; i=(i+1)%QUEUE_SIZE)
    {
        if (i==queue_tail&&(j||!queue[queue_head].item_status)) 
        {
            break;
        }
        j++;
        if (queue[i].item_status==UNISSUED&&queue[i].issue_sta!=SU&&queue[i].issue_sta==ALU) 
        {
            if (station[queue[i].issue_sta].valid==0) 
            {
                flag1=0;
                p1=i;
                break;
            }
            else if (station[queue[i].issue_sta+10].valid==0) 
            {
                flag1=1;
                p1=i;
                break;
            }
            else if (cmt_bus.valid==1 && cmt_bus.user==queue[i].issue_sta) 
            {
                flag1=0;
                p1=i;
                break;
            }
            else if (cmt_bus.valid==1 && cmt_bus.user==queue[i].issue_sta+10) 
            {
                flag1=1;
                p1=i;
                break;
            }
            else if (cmt_bus2.valid==1 && cmt_bus2.user==queue[i].issue_sta) 
            {
                flag1=0;
                p1=i;
                break;
            }
            else if (cmt_bus2.valid==1 && cmt_bus2.user==queue[i].issue_sta+10) 
            {
                flag1=1;
                p1=i;
                break;
            }
        }
    }

    for( i = queue_head,j=0; ; i=(i+1)%QUEUE_SIZE)
    {
        if (i==queue_tail&&(j||!queue[queue_head].item_status)) 
        {
            break;
        }
        j++;
        if (queue[i].item_status==UNISSUED&&queue[i].issue_sta!=SU&&queue[i].issue_sta!=ALU) 
        {
            if (station[queue[i].issue_sta].valid==0) 
            {
                flag2=0;
                p2=i;
                break;
            }
            else if (cmt_bus.valid==1 && cmt_bus.user==queue[i].issue_sta) 
            {
                flag2=0;
                p2=i;
                break;
            }
            else if (cmt_bus2.valid==1 && cmt_bus2.user==queue[i].issue_sta) 
            {
                flag2=0;
                p2=i;
                break;
            }
            if(queue[i].issue_sta==LU)
            {
                if (station[LU2].valid==0) 
                {
                    flag2=1;
                    p2=i;
                    break;
                }
                else if (cmt_bus.valid==1 && cmt_bus.user==LU2) 
                {
                    flag2=1;
                    p2=i;
                    break;
                }
                else if (cmt_bus2.valid==1 && cmt_bus2.user==LU2) 
                {
                    flag2=1;
                    p2=i;
                    break;
                }
            }
        }
    }
    if (p1<QUEUE_SIZE) 
    {
        queue_bk[p1].item_status=ISSUED;
        BYTE Qi=queue[p1].Qi;
        BYTE Qj=queue[p1].Qj;
        int Vi=queue[p1].Vi;
        int Vj=queue[p1].Vj;
        if (Qi<32+QUEUE_SIZE)  // Read vi from queue and result bus
        {
            for( i = (p1-1+QUEUE_SIZE)%QUEUE_SIZE; i != (queue_head-1+QUEUE_SIZE)%QUEUE_SIZE; i=(i-1+QUEUE_SIZE)%QUEUE_SIZE)
            {
                if (queue[i].Rd==Qi&&Qi) 
                {
                    Qi=i+32;
                    if (queue[i].item_status==FINISH) 
                    {
                        Vi=queue[i].imm;
                        Qi=32+QUEUE_SIZE;
                    }
                    if (cmt_bus.valid==1 && cmt_bus.id==i) 
                    {
                        Vi=cmt_bus.res;
                        Qi=32+QUEUE_SIZE;
                    }
                    if (cmt_bus2.valid==1 && cmt_bus2.id==i) 
                    {
                        Vi=cmt_bus2.res;
                        Qi=32+QUEUE_SIZE;
                    }
                    break;
                }
            }
        }
        if (Qj<32+QUEUE_SIZE) 
        {
            for( i = (p1-1+QUEUE_SIZE)%QUEUE_SIZE; i != (queue_head-1+QUEUE_SIZE)%QUEUE_SIZE; i=(i-1+QUEUE_SIZE)%QUEUE_SIZE)
            {
                if (queue[i].Rd==Qj&&Qj) 
                {
                    Qj=i+32;
                    if (queue[i].item_status==FINISH) 
                    {
                        Vj=queue[i].imm;
                        Qj=32+QUEUE_SIZE;
                    }
                    if (cmt_bus.valid==1 && cmt_bus.id==i) 
                    {
                        Vj=cmt_bus.res;
                        Qj=32+QUEUE_SIZE;
                    }
                    if (cmt_bus2.valid==1 && cmt_bus2.id==i) 
                    {
                        Vj=cmt_bus2.res;
                        Qj=32+QUEUE_SIZE;
                    }
                    break;
                }
            }
        }
        if (Qi<32) // Read Vi from RegFile
        {
            Vi=ReadReg(Qi);
            Qi=32+QUEUE_SIZE;
        }
        if (Qj<32) 
        {
            Vj=ReadReg(Qj);
            Qj=32+QUEUE_SIZE;
        }
        Qi -= 32;
        Qj -= 32;

        BYTE issue_sta1=flag1?queue[p1].issue_sta+10:queue[p1].issue_sta;
        issue_write[issue_sta1]==1;
        station_bk[issue_sta1].valid=1;
        station_bk[issue_sta1].op=queue[p1].op_code;
        station_bk[issue_sta1].id=p1;
        station_bk[issue_sta1].imm=queue[p1].imm;
        station_bk[issue_sta1].Qi=Qi;
        station_bk[issue_sta1].Qj=Qj;
        station_bk[issue_sta1].Vi=Vi;
        station_bk[issue_sta1].Vj=Vj;
        station_bk[issue_sta1].ins_addr=queue[p1].ins_addr;
        station_bk[issue_sta1].addr_ready=0;
        if (queue[p1].issue_sta==LU && Qi>=QUEUE_SIZE) 
        {
            queue_bk[p1].exe_addr=Vi+queue[p1].imm; 
            station_bk[issue_sta1].addr_ready=1;
        }
    }
    if (p2<QUEUE_SIZE) 
    {
        queue_bk[p2].item_status=ISSUED;
        BYTE Qi=queue[p2].Qi;
        BYTE Qj=queue[p2].Qj;
        int Vi=queue[p2].Vi;
        int Vj=queue[p2].Vj;
        if (Qi<32+QUEUE_SIZE)  // Read vi from queue and result bus
        {
            for( i = (p2-1+QUEUE_SIZE)%QUEUE_SIZE; i != (queue_head-1+QUEUE_SIZE)%QUEUE_SIZE; i=(i-1+QUEUE_SIZE)%QUEUE_SIZE)
            {
                if (queue[i].Rd==Qi&&Qi) 
                {
                    Qi=i+32;
                    if (queue[i].item_status==FINISH) 
                    {
                        Vi=queue[i].imm;
                        Qi=32+QUEUE_SIZE;
                    }
                    if (cmt_bus.valid==1 && cmt_bus.id==i) 
                    {
                        Vi=cmt_bus.res;
                        Qi=32+QUEUE_SIZE;
                    }
                    if (cmt_bus2.valid==1 && cmt_bus2.id==i) 
                    {
                        Vi=cmt_bus2.res;
                        Qi=32+QUEUE_SIZE;
                    }
                    break;
                }
            }
        }
        if (Qj<32+QUEUE_SIZE) 
        {
            for( i = (p2-1+QUEUE_SIZE)%QUEUE_SIZE; i != (queue_head-1+QUEUE_SIZE)%QUEUE_SIZE; i=(i-1+QUEUE_SIZE)%QUEUE_SIZE)
            {
                if (queue[i].Rd==Qj&&Qj) 
                {
                    Qj=i+32;
                    if (queue[i].item_status==FINISH) 
                    {
                        Vj=queue[i].imm;
                        Qj=32+QUEUE_SIZE;
                    }
                    if (cmt_bus.valid==1 && cmt_bus.id==i) 
                    {
                        Vj=cmt_bus.res;
                        Qj=32+QUEUE_SIZE;
                    }
                    if (cmt_bus2.valid==1 && cmt_bus2.id==i) 
                    {
                        Vj=cmt_bus2.res;
                        Qj=32+QUEUE_SIZE;
                    }
                    break;
                }
            }
        }
        if (Qi<32) // Read Vi from RegFile
        {
            Vi=ReadReg(Qi);
            Qi=32+QUEUE_SIZE;
        }
        if (Qj<32) 
        {
            Vj=ReadReg(Qj);
            Qj=32+QUEUE_SIZE;
        }
        Qi -= 32;
        Qj -= 32;

        BYTE issue_sta2=flag2?LU2:queue[p2].issue_sta;
        issue_write[issue_sta2]==1;
        station_bk[issue_sta2].valid=1;
        station_bk[issue_sta2].op=queue[p2].op_code;
        station_bk[issue_sta2].id=p2;
        station_bk[issue_sta2].imm=queue[p2].imm;
        station_bk[issue_sta2].Qi=Qi;
        station_bk[issue_sta2].Qj=Qj;
        station_bk[issue_sta2].Vi=Vi;
        station_bk[issue_sta2].Vj=Vj;
        station_bk[issue_sta2].ins_addr=queue[p2].ins_addr;
        station_bk[issue_sta2].addr_ready=0;
        if (queue[p2].issue_sta==LU && Qi>=QUEUE_SIZE) 
        {
            queue_bk[p2].exe_addr=Vi+queue[p2].imm; 
            station_bk[issue_sta2].addr_ready=1;
        }
    }
}

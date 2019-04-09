# include "define.h"
# include "reg_bus.h"
# include "issue.h"
# include "cmt.h"

void MULModule()
{
    int res;
    int bus_user;
    WORD tmp1,tmp2;
    memset(&mul_out_bk,0,sizeof(mul_out));
    if (!station[MUL_UNIT].valid) 
    {
        return;
    }
    if (station[MUL_UNIT].Qi>=QUEUE_SIZE&&station[MUL_UNIT].Qj>=QUEUE_SIZE) 
    {
        switch (station[MUL_UNIT].op)
        {
            case MUL_MUL:
                res = station[MUL_UNIT].Vi * station[MUL_UNIT].Vj;
                break;

            case MUL_DIV:
                res = station[MUL_UNIT].Vi / station[MUL_UNIT].Vj;
                break;

            case MUL_DIVU:
                tmp1=station[MUL_UNIT].Vi;
                tmp2=station[MUL_UNIT].Vj;
                res = tmp1 / tmp2;
                break;

            case MUL_REM:
                res = station[MUL_UNIT].Vi % (station[MUL_UNIT].Vj%32);
                break;
            
            default:
                break;
        }
        mul_out_bk.id=station[MUL_UNIT].id;
        mul_out_bk.res=res;
        mul_out_bk.valid=1;
        if (cmt_vie_bk==0) 
        {
            cmt_vie_bk=MUL_UNIT;
            if(issue_write[MUL_UNIT]==0)
            {
                memset(&station_bk[MUL_UNIT],0,sizeof(station[MUL_UNIT]));
            }
        }
    }
    if (station[MUL_UNIT].Qi<QUEUE_SIZE) // snoop Vi from commit bus
    {
        if (cmt_bus.valid==1 && cmt_bus.id==station[MUL_UNIT].Qi) 
        {
            station_bk[MUL_UNIT].Qi=QUEUE_SIZE;
            station_bk[MUL_UNIT].Vi=cmt_bus.res;
        }  
    }
    if (station[MUL_UNIT].Qj<QUEUE_SIZE)
    {
        if (cmt_bus.valid==1 && cmt_bus.id==station[MUL_UNIT].Qj) 
        {
            station_bk[MUL_UNIT].Qj=QUEUE_SIZE;
            station_bk[MUL_UNIT].Vj=cmt_bus.res;
        }  
    }
}

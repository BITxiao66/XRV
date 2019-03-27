# include "define.h"
# include "reg_bus.h"
# include "issue.h"
# include "cmt.h"

long long mcycle = 0;
long long minstret=0;

void CSUModule()
{
    WORD res;
    int bus_user;
    int csr;
    WORD tmp1,tmp2;
    memset(&csu_out_bk,0,sizeof(csu_out));
    if (!station[CSU].valid) 
    {
        return;
    }
    if (station[CSU].Qi>=8) 
    {
        switch (station[CSU].Vj & 0x00000FFF)
        {
            case 0x00000C00:
                res = mcycle;
                break;

            case 0x00000C02:
                res = minstret;
                break;

            default:
                res = 0xFFFFFFFF;
                break;
        }

        csu_out_bk.id=station[CSU].id;
        csu_out_bk.res=res;
        csu_out_bk.valid=1;
        if (cmt_vie_bk==0) 
        {
            cmt_vie_bk=CSU;
            if(issue_write[CSU]==0)
            {
                memset(&station_bk[CSU],0,sizeof(station[CSU]));
            }
        }
    }
    if (station[CSU].Qi<8) // snoop Vi from commit bus
    {
        if (cmt_bus.valid==1 && cmt_bus.id==station[CSU].Qi) 
        {
            station_bk[CSU].Qi=8;
            station_bk[CSU].Vi=cmt_bus.res;
        }  
    }
}
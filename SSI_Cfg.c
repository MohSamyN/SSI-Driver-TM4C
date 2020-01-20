#include "SSI.h"
#include "SSI_Cfg.h"

void Callback(void);

const SSI_CfgType SSI_ConfigParameters[SSI_GROUPS_NUMBER] =
{
    {
        0,
        MASTER,
        DISABLED,
        SYSTEM_CLK,
        100000,
        FreescaleSPI,
        FIRST_EDGE,
        RISING,
        8,
        LOOPBACK,
        &Callback
    }
};

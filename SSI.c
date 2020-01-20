#include <stdint.h>
#include "SSI.h"
#include "SSI_Cfg.h"
#include "M4MemMap.h"

typedef volatile uint32_t* const SSI_RegAddType;

#define MODULES_NUMBER          4u

#define SSI0_BASE_ADDRESS       0x40008000
#define SSI1_BASE_ADDRESS       0x40009000
#define SSI2_BASE_ADDRESS       0x4000A000
#define SSI3_BASE_ADDRESS       0x4000B000

static const uint32_t SSI_BaseAddressLUT[MODULES_NUMBER] =
{
    SSI0_BASE_ADDRESS,
    SSI1_BASE_ADDRESS,
    SSI2_BASE_ADDRESS,
    SSI3_BASE_ADDRESS
};

#define SSI_REG_ADDRESS(SSI_ID, REG_OFFSET)                 (SSI_BaseAddressLUT[SSI_ID] + REG_OFFSET)

#define SSICR0(SSI_ID)                                      *((SSI_RegAddType)SSI_REG_ADDRESS(SSI_ID, 0x000))
#define SSICR1(SSI_ID)                                      *((SSI_RegAddType)SSI_REG_ADDRESS(SSI_ID, 0x004))
#define SSIDR(SSI_ID)                                       *((SSI_RegAddType)SSI_REG_ADDRESS(SSI_ID, 0x008))
#define SSISR(SSI_ID)                                       *((SSI_RegAddType)SSI_REG_ADDRESS(SSI_ID, 0x00C))
#define SSICPSR(SSI_ID)                                     *((SSI_RegAddType)SSI_REG_ADDRESS(SSI_ID, 0x010))
#define SSIIM(SSI_ID)                                       *((SSI_RegAddType)SSI_REG_ADDRESS(SSI_ID, 0x014))
#define SSIRIS(SSI_ID)                                      *((SSI_RegAddType)SSI_REG_ADDRESS(SSI_ID, 0x018))
#define SSIMIS(SSI_ID)                                      *((SSI_RegAddType)SSI_REG_ADDRESS(SSI_ID, 0x01C))
#define SSIICR(SSI_ID)                                      *((SSI_RegAddType)SSI_REG_ADDRESS(SSI_ID, 0x020))
#define SSIDMACTL(SSI_ID)                                   *((SSI_RegAddType)SSI_REG_ADDRESS(SSI_ID, 0x024))
#define SSICC(SSI_ID)                                       *((SSI_RegAddType)SSI_REG_ADDRESS(SSI_ID, 0xFC8))
#define SSIPeriphID4(SSI_ID)                                *((SSI_RegAddType)SSI_REG_ADDRESS(SSI_ID, 0xFD0))
#define SSIPeriphID5(SSI_ID)                                *((SSI_RegAddType)SSI_REG_ADDRESS(SSI_ID, 0xFD4))
#define SSIPeriphID6(SSI_ID)                                *((SSI_RegAddType)SSI_REG_ADDRESS(SSI_ID, 0xFD8))
#define SSIPeriphID7(SSI_ID)                                *((SSI_RegAddType)SSI_REG_ADDRESS(SSI_ID, 0xFDC))
#define SSIPeriphID0(SSI_ID)                                *((SSI_RegAddType)SSI_REG_ADDRESS(SSI_ID, 0xFE0))
#define SSIPeriphID1(SSI_ID)                                *((SSI_RegAddType)SSI_REG_ADDRESS(SSI_ID, 0xFE4))
#define SSIPeriphID2(SSI_ID)                                *((SSI_RegAddType)SSI_REG_ADDRESS(SSI_ID, 0xFE8))
#define SSIPeriphID3(SSI_ID)                                *((SSI_RegAddType)SSI_REG_ADDRESS(SSI_ID, 0xFEC))
#define SSIPCellID0(SSI_ID)                                 *((SSI_RegAddType)SSI_REG_ADDRESS(SSI_ID, 0xFF0))
#define SSIPCellID1(SSI_ID)                                 *((SSI_RegAddType)SSI_REG_ADDRESS(SSI_ID, 0xFF4))
#define SSIPCellID2(SSI_ID)                                 *((SSI_RegAddType)SSI_REG_ADDRESS(SSI_ID, 0xFF8))
#define SSIPCellID3(SSI_ID)                                 *((SSI_RegAddType)SSI_REG_ADDRESS(SSI_ID, 0xFFC))

static uint8_t SSI_ModuleState[MODULES_NUMBER] = {0};
static uint8_t SSI_Flag[MODULES_NUMBER] = {0};


SSI_CheckType SSI_Init(void)
{
    uint32_t ClkDivBR = 0;
    uint32_t TempSCR = 0;
    uint16_t TempDVSR = 0;
    SSI_CheckType RetVar;
    const SSI_CfgType* CfgPtr;
    uint8_t LoopIndex;
    uint8_t ErrorFlag = 0;
    for(LoopIndex = 0; ((LoopIndex < SSI_GROUPS_NUMBER) && (ErrorFlag == 0)); LoopIndex++)
    {
        if(SSI_ConfigParameters[LoopIndex].ModuleID < MODULES_NUMBER)
        {
            CfgPtr = &SSI_ConfigParameters[LoopIndex];

            /* Enabling the clock of the appropriate module */
            RCGCSSI_REG |= (1 << CfgPtr->ModuleID);

            /* Disabling the SSI module before any configurations */
            SSICR1(CfgPtr->ModuleID) &= ~(0x02);

            /* Choosing whether the controller acts as the master or the slave */
            SSICR1(CfgPtr->ModuleID) |= (CfgPtr->ModeSelector << 2);
            /* Choosing whether the master output is broadcasted to all slaves in the system or sent only to one slave */
            if(CfgPtr->ModeSelector == SLAVE)
            {
                SSICR1(CfgPtr->ModuleID) |= (CfgPtr->SlaveOutputSelector << 3);
            }
            else
            {
            }

            /* Choosing whether the clock source is the system clock or the precision internal oscillator (PIOSC) */
            SSICC(CfgPtr->ModuleID) = CfgPtr->ClockSource;

            /* Calculating Clock Prescale Divisor and Serial Clock Rate values to achieve the required bit rate (in KHz) */
            /* BR = SysClk / (CPSDVSR * (1 + SCR)) */
            if(CfgPtr->ClockSource == SYSTEM_CLK)
            {
                ClkDivBR = (16000000 / CfgPtr->BitRate);
            }
            else
            {
                ClkDivBR = (4000000 / CfgPtr->BitRate);
            }
            /* Assuming the divisor starts from 2 and incrementing by 2 at each loop till the clock rate is less than 256 */
            for(TempDVSR = 2; TempDVSR < 254; TempDVSR += 2)
            {
                TempSCR = (ClkDivBR / TempDVSR) - 1;
                if(TempSCR < 256)
                {
                    SSICPSR(CfgPtr->ModuleID) = TempDVSR;
                    SSICR0(CfgPtr->ModuleID) |= (TempSCR << 8);
                    TempDVSR = 254;
                }
                else
                {
                }
            }

            /* Choosing whether the SSI frame is Freescale SPI, Texas Instruments Synchronous Serial or MICROWIRE frame format */
            SSICR0(CfgPtr->ModuleID) |= (CfgPtr->FrameSelector << 4);

            /* Choosing whether the clock is captured in the first clock edge or the second clock edge */
            SSICR0(CfgPtr->ModuleID) |= (CfgPtr->ClockPhase << 7);

            /* Choosing whether the clock starts with LOW to HIGH transition (rising edge) or HIGH to LOW transition (falling edge) */
            SSICR0(CfgPtr->ModuleID) |= (CfgPtr->ClockPolarity << 6);

            /* Choosing the data size, which starts from 4 to 16 */
            SSICR0(CfgPtr->ModuleID) |= (CfgPtr->DataSizeSelector - 1);

            /* Choosing whether the SSI module works on the normal mode or the loopback mode */
            SSICR1(CfgPtr->ModuleID) |= (CfgPtr->SSIModeType);

            /* Enabling the SSI module to start operating */
            SSICR1(CfgPtr->ModuleID) |= 0x02;

            SSI_ModuleState[LoopIndex] = 1;
            RetVar = SSI_OK;
        }
        else
        {
            ErrorFlag = 1;
            RetVar = SSI_NOK;
        }
    }
    return RetVar;
}

SSI_CheckType SSI_SendData(uint8_t ModuleID, uint16_t Data)
{
    SSI_CheckType RetVar;
    const SSI_CfgType* CfgPtr;
    if(ModuleID < SSI_GROUPS_NUMBER)
    {
        CfgPtr = &SSI_ConfigParameters[ModuleID];
        if(SSI_ModuleState[ModuleID] == 1)
        {
            if(((SSISR(CfgPtr->ModuleID) & 0x02) != 0) && (SSI_Flag[ModuleID] == 0))
            {
                SSIDR(CfgPtr->ModuleID) = Data;
                SSI_Flag[ModuleID] = 1;
            }
            else if(((SSISR(CfgPtr->ModuleID) & 0x10) == 0) && (SSI_Flag[ModuleID] == 1))
            {
                CfgPtr->CallbackPtr();
                SSI_Flag[ModuleID] = 0;
            }
            else
            {
            }
            RetVar = SSI_OK;
        }
        else
        {
            RetVar = SSI_NOK;
        }
    }
    else
    {
        RetVar = SSI_NOK;
    }
    return RetVar;
}

SSI_CheckType SSI_ReceiveData(uint8_t ModuleID, uint16_t* DataPtr)
{
    SSI_CheckType RetVar;
    const SSI_CfgType* CfgPtr;
    if(ModuleID < SSI_GROUPS_NUMBER)
    {
        CfgPtr = &SSI_ConfigParameters[ModuleID];
        if(SSI_ModuleState[ModuleID] == 1)
        {
            if((SSISR(CfgPtr->ModuleID) & 0x04) != 0)
            {
                *DataPtr = SSIDR(CfgPtr->ModuleID);
            }
            else
            {
            }
            RetVar = SSI_OK;
        }
        else
        {
            RetVar = SSI_NOK;
        }
    }
    else
    {
        RetVar = SSI_NOK;
    }
    return RetVar;
}

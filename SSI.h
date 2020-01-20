#ifndef SSI_H_
#define SSI_H_
#include <stdint.h>
#include "SSI_Cfg.h"

typedef enum {MASTER, SLAVE} MasterSlaveSelect;
typedef enum {ENABLED, DISABLED} SlaveOutputDisable;
typedef enum {SYSTEM_CLK = 0, PIOSC_CLK = 5} SCKSource;
typedef enum {FreescaleSPI, TexasInstrumentsSychronousSerial, MICROWIRE} FrameFormatSelect;
typedef enum {FIRST_EDGE, SECOND_EDGE} SCKPhase;
typedef enum {RISING, FALLING} SCKPolarity;
typedef enum {NORMAL, LOOPBACK} SSIMode;

typedef struct
{
    /* Choosing which SSI module is used */
    uint8_t ModuleID;
    /* Choosing whether the controller is the master or the slave */
    MasterSlaveSelect ModeSelector;
    /* Choosing whether the master output is broadcasted to all slaves in the system or sent only to one slave */
    SlaveOutputDisable SlaveOutputSelector;
    /* Choosing whether the clock source is the system clock or the precision internal oscillator (PIOSC) */
    SCKSource ClockSource;
    /* Entering the bit rate of the SSI module */
    uint32_t BitRate;
    /* Choosing whether the SSI frame is Freescale SPI, Texas Instruments Synchronous Serial or MICROWIRE frame format */
    FrameFormatSelect FrameSelector;
    /* Choosing whether the clock is captured in the first clock edge or the second clock edge */
    SCKPhase ClockPhase;
    /* Choosing whether the clock starts with LOW to HIGH transition (rising edge) or HIGH to LOW transition (falling edge) */
    SCKPolarity ClockPolarity;
    /* Choosing the data size, which starts from 4 to 16 */
    uint8_t DataSizeSelector;
    /* Choosing whether the SSI module works on the normal mode or the loopback mode */
    SSIMode SSIModeType;
    /* Calling this function after a successful transmission takes place */
    void (*CallbackPtr)(void);
} SSI_CfgType;

typedef enum {SSI_OK, SSI_NOK} SSI_CheckType;

extern const SSI_CfgType SSI_ConfigParameters[SSI_GROUPS_NUMBER];


SSI_CheckType SSI_Init(void);
SSI_CheckType SSI_SendData(uint8_t ModuleID, uint16_t Data);
SSI_CheckType SSI_ReceiveData(uint8_t ModuleID, uint16_t* DataPtr);


#endif /* SSI_H_ */

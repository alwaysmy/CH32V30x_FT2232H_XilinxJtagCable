
#ifndef _FTDI_H_
#define _FTDI_H_


#define JTAG_GPIOPORT GPIOB

//下面都是从旧程序搬的
/******************************************************************************/

#define GPIOB_BASE      0x40010c00  // GPIOB
                                   // 如果4个IO不在同一个bank，则需要另外一个寄存器保存另一个BASE。
// #define GPIO_BASE      0x40011000  // GPIOC
#define GPIOC_BASE      0x40011000  // GPIOC
#define GPIOD_BASE     0x40011400  // GPIOD

//-----------下面这几个要自己改
#if 1
    //-----------------------
    #define TCK_BASE GPIOC_BASE
    #define TMS_BASE GPIOD_BASE
    #define TDI_BASE GPIOC_BASE
    #define TDO_BASE GPIOC_BASE
    //TODO:目前TCK TDI TDO的base共用TCK_BASE,要一样
    #define TCK_PORT GPIOCR
    #define TMS_PORT GPIODR
    #define TDI_PORT GPIOCR
    #define TDO_PORT GPIOCR

    #define TCK_GROUP 2
    #define TDI_GROUP 2
    #define TDO_GROUP 2
    #define TMS_GROUP 3

    //这几个宏定义凑合用吧，搞得到处都是的
    #define GPIO_TCK  10    //pin14     //13 //PC10
    #define GPIO_TMS  2     //Pin15     //12 //PD2
    #define GPIO_TDI  12    //pin11     //15 //PC12
    #define GPIO_TDO  11    //Pin12     //14 //PC11
#else
    //-----------------------
    #define TCK_BASE GPIOB_BASE
    #define TMS_BASE GPIOB_BASE
    #define TDI_BASE GPIOB_BASE
    #define TDO_BASE GPIOB_BASE
    //TODO:目前TCK TDI TDO的base共用TCK_BASE,要一样
    #define TCK_PORT GPIOB
    #define TMS_PORT GPIOB
    #define TDI_PORT GPIOB
    #define TDO_PORT GPIOB
    #define TCK_GROUP 1
    #define TDI_GROUP 1
    #define TDO_GROUP 1
    #define TMS_GROUP 1
    #define GPIO_TCK  13    //pin14     //13 //PC10
    #define GPIO_TMS  12     //Pin15     //12 //PD2
    #define GPIO_TDI  15    //pin11     //15 //PC12
    #define GPIO_TDO  14    //Pin12     //14 //PC11
#endif //TEST DEFINE

//------------下面的不用改
#define GPIO_IN        0x08
#define GPIO_SET       0x10
#define GPIO_CLR       0x14


#define TCK_MASK       (1<<GPIO_TCK)
#define TMS_MASK       (1<<GPIO_TMS)
#define TDI_MASK       (1<<GPIO_TDI)

#define TDO_SHIFT_MSB  GPIO_TDO
#define TDO_SHIFT_LSB  (GPIO_TDO-7)   // 如果GPIO编号小于7，对应的移位指令要改为左移。


#define DIR_OUT 1  // push-poll output
#define DIR_IN  4  // float input

//注意，__ASSEMBLY__ 需要自己添加进汇编预处理定义,下面的不要让汇编器处理，因为这个头文件被汇编代码包含了
#ifndef __ASSEMBLY__ 
#include "stdint.h"
extern void  usb_dc_user_init(uint8_t busid, uintptr_t reg_base);
int jtag_setup(void);
int jtag_exit(void);
int jtag_execute(uint8_t *req, int req_size);
void jtag_flush_resp(uint8_t busid);
void jtag_write(int byte);
void jtag_handle(void);

int jtag_trans_msb_0(int data, int bcnt, int delay);
int jtag_trans_msb_1(int data, int bcnt, int delay);
int jtag_trans_lsb_0(int data, int bcnt, int delay);
int jtag_trans_lsb_1(int data, int bcnt, int delay);
int jtag_trans_tms_0(int data, int bcnt, int delay);
int jtag_trans_tms_1(int data, int bcnt, int delay);
void ftdi_timer_handle(void);

#endif

/******************************************************************************/


#endif


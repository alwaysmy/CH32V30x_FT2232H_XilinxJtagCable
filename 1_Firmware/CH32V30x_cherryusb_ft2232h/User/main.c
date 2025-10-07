/********************************** (C) COPYRIGHT *******************************
* File Name          : main.c
* Author             : WCH
* Version            : V1.0.0
* Date               : 2021/06/06
* Description        : Main program body.
*********************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for 
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
*******************************************************************************/

/*
 *@Note
 USART Print debugging routine:
 USART1_Tx(PA9).
 This example demonstrates using USART1(PA9) as a print debug port output.

*/

#include "debug.h"
#include "ftdi.h"
#include "ch32v30x.h"
#include "ch32v30x_conf.h"
#include "ch32v30x_it.h"
#include "usb_config.h"
#include "main.h"

/* Global typedef */

/* Global define */
#define TIMER_HZ (SYSCLK_FREQ/8)



/******************************************************************************/


// u64 get_mcycle(void)
// {
// 	u32 h0, h1, low;

// 	do{
// 		h0  = read_csr(mcycleh);
// 		low = read_csr(mcycle);
// 		h1  = read_csr(mcycleh);
// 	}while(h0!=h1);

// 	return ((u64)h0<<32) | low;
// }


// /******************************************************************************/

u32 time_ms = 0;
static u32 last_tcnt;

void timer_init(void)
{
	reset_timer();
}


void reset_timer(void)
{

	SYSTICK->CTLR = 0x20;
	SYSTICK->CTLR = 0x01;
	time_ms = 0;
	last_tcnt = 0;
}


u32 get_timer(void)
{
	return SYSTICK->CNTL;
}


void udelay(int us)
{
	reset_timer();
	u32 end = get_timer() + us*(TIMER_HZ/1000000);
	while(get_timer()<end);
}


void mdelay(int ms)
{
	reset_timer();
	u32 end = get_timer() + ms*(TIMER_HZ/1000);
	while(get_timer()<end);
}


void soft_timer(void)
{
	u32 tcnt = get_timer();
	if(tcnt > last_tcnt){
		tcnt -= last_tcnt;
	}else{
		tcnt = (0xffffffff-last_tcnt)+tcnt+1;
	}

	u32 ms = tcnt/(TIMER_HZ/1000);
	if(ms){
		time_ms += ms;
		last_tcnt += ms*(TIMER_HZ/1000);
	}
}


/******************************************************************************/




/******************************************************************************/


void gpio_set(int group, int bit, int val)
{
	volatile GPIO_REGS *gpio = (GPIO_REGS*)(0x40010800+group*0x0400);
	int mask = 1<<bit;

	if(val)
		gpio->BSHR = mask;
	else
		gpio->BCR = mask;
}


int gpio_get(int group, int bit)
{
	volatile GPIO_REGS *gpio = (GPIO_REGS*)(0x40010800+group*0x0400);
	int mask = 1<<bit;

	return (gpio->INDR & mask) ? 1: 0;
}


void gpio_mode(int group, int bit, int mode, int ioval)
{
	int mreg;
	volatile GPIO_REGS *gpio = (GPIO_REGS*)(0x40010800+group*0x0400);

	if(ioval){
		gpio->OUTDR |= 1<<bit;
	}else{
		gpio->OUTDR &= ~(1<<bit);
	}

	if(bit<8){
		mreg = gpio->CFGLR;
		mreg &= ~(0x0f<<(4*bit));
		mreg |= mode<<(4*bit);
		gpio->CFGLR = mreg;
	}else{
		bit -= 8;
		mreg = gpio->CFGHR;
		mreg &= ~(0x0f<<(4*bit));
		mreg |= mode<<(4*bit);
		gpio->CFGHR = mreg;
	}
}

/******************************************************************************/

/******************************************************************************/


static char *excp_msg[16] = {
	"Inst Align",
	"Inst Fault",
	"Inst Illegal",
	"Breakpoint",
	"Load Align",
	"Load Fault",
	"Store Align",
	"Store Fault",
	"ECall_U",
	"ECall_S",
	"Reserved",
	"ECall_M",
	"IPage Fault",
	"LPage Fault",
	"Reserved",
	"SPage Fault",
};


// __attribute__((interrupt)) 会保存所有临时寄存器和浮点寄存器
// __attribute__((interrupt("WCH-Interrupt-fast"))) 只保存浮点寄存器
// 如果不考虑浮点寄存器, 可以不用这些属性. 但必须在start.S里面写入口，手动mret返回。

// void HardFault_Handler(void)
// {
//     int cause = read_csr(mcause);
//     printk("\n[%s!]: EPC=%08x TVAL=%08x CAUSE=%d\n", excp_msg[cause], read_csr(mepc), read_csr(mtval), cause);
//     while(1){
//     }
// }


void int_enable(int id)
{
    PFIC->IENR[id/32] = 1<<(id&31);
}


void int_disable(int id)
{
    PFIC->IRER[id/32] = 1<<(id&31);
}

// 优先级: 0-7
void int_priority(int id, int p)
{
    PFIC->IPRIOR[id] = p<<5;
}


/******************************************************************************/



void usb_dc_low_level_init(void)
{
    RCC_USBCLK48MConfig(RCC_USBCLK48MCLKSource_USBPHY);
    RCC_USBHSPLLCLKConfig(RCC_HSBHSPLLCLKSource_HSE);
    RCC_USBHSConfig(RCC_USBPLL_Div2);
    RCC_USBHSPLLCKREFCLKConfig(RCC_USBHSPLLCKREFCLK_4M);
    RCC_USBHSPHYPLLALIVEcmd(ENABLE);

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_USBHS, ENABLE);
    NVIC_EnableIRQ(USBHS_IRQn);

    // RCC_AHBPeriphClockCmd(RCC_AHBPeriph_OTG_FS, ENABLE);
    // // EXTEN->EXTEN_CTR |= EXTEN_USBD_PU_EN;
    // NVIC_EnableIRQ(OTG_FS_IRQn);

    Delay_Us(100);
}
/* Global Variable */

uint8_t RCC_Configuration( void )
{
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1|RCC_AHBPeriph_SRAM|RCC_AHBPeriph_DMA2,ENABLE);//SRAM DMA2 DMA1,不知道为啥要开sram，示例程序没开，官方USB转JTAG也没开
    	/* TIM2 clock enable */
	
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE |
	                        RCC_APB2Periph_USART1|RCC_APB2Periph_TIM1|RCC_APB2Periph_TIM8, ENABLE );
	/* AFIO clock enable */
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_AFIO, ENABLE );
    RCC_APB1PeriphClockCmd( RCC_APB1Periph_PWR|RCC_APB1Periph_USART3|RCC_APB1Periph_SPI2|RCC_APB1Periph_UART8, ENABLE );						/* 使能TIM2 */
    /* 由于其中的PB3、PB4对应与单片机的JTAG功能,所以必须先禁用JTAG功能 */
#if( DEF_DEBUG_FUN_EN == 1 )
    GPIO_PinRemapConfig( GPIO_Remap_SWJ_Disable, ENABLE );
#endif
	return( 0x00 );
}
//TODO:原版实现是在jtag_setup();里面,会在SIO_SET_BITMODE_REQUEST:里面调用一次,可以替换
void JTGA_IO_Init()
{
    // TCK: 
    // TMS: 
    // TDI: 
    // TDO: 
    GPIO_InitTypeDef  GPIO_InitStructure;
    //RCC已经开过了,这里不重复开了
    GPIO_InitStructure.GPIO_Pin = GPIO_TMS|GPIO_TCK|GPIO_TDI; //13 14 10
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(JTAG_GPIOPORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_TDO; //13 14 10
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;//上拉下拉无所谓,对面是输出,这里参照原设计上拉
    GPIO_Init(JTAG_GPIOPORT, &GPIO_InitStructure);
}
void Periph_Init()
{
    JTGA_IO_Init();
}
/*********************************************************************
 * @fn      main
 *
 * @brief   Main program.
 *
 * @return  none
 */
int main(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	SystemCoreClockUpdate();
    RCC_Configuration();
	Delay_Init();
    // Periph_Init();
    /**SDIPRINT仅限调试的时候使用，如果有打印，下电后不重新接调试器似乎会阻塞**/
    #if (SDI_PRINT == SDI_PR_OPEN) 
        SDI_Printf_Enable();//上位机波特率无所谓
    #else
        USART_Printf_Init(460800);
    #endif
	DEBUG_PRINT("SysClk:%d\r\n",SystemCoreClock);
	DEBUG_PRINT( "ChipID:%08x\r\n", DBGMCU_GetCHIPID() );
	// DEBUG_PRINT("This is printf example\r\n");

	// cdc_acm_init(0, 0);
    
	usb_dc_user_init(0,0);

	reset_timer();
	while(1){
		soft_timer();
		ftdi_timer_handle();//time_ms>=ftdevs[0].timeout才会发送
		// WFI(); // WFI会影响正在工作的外设.
		jtag_handle();
	}
}


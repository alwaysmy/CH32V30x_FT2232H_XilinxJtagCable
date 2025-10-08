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

#include "ch32v30x_conf.h"
#include "ch32v30x_it.h"
#include "usb_config.h"
#include "main.h"

/* Global typedef */

/* Global define */

u8 btn_Flag = 0;
#define PIN_LED_OUT( d )          if( d ) GPIOC->BSHR = GPIO_Pin_8; else GPIOC->BCR = GPIO_Pin_8; 
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
static u64 last_tcnt;

void timer_init(void)
{
	reset_timer();
}


void reset_timer(void)
{
	SysTick->SR &=~STK_SR_CNTIF;//������״̬���
	// SysTick->CMP = 0;//�Ƚϼ����������ﲻ��
	// SysTick->CNT = 0;// ����ֵ���㣬���ã���CTLR�����
	SysTick->CTLR = STK_CTLR_INIT|STK_CTLR_STE;//��գ�ʹ��,����Ϊϵͳʱ��8��Ƶ
	// SYSTICK->CTLR = 0x20;
	// SYSTICK->CTLR = 0x01;
	time_ms = 0;
	last_tcnt = 0;
}


static inline u64 get_timer(void)
{
	// return SYSTICK->CNTL;
	return SysTick->CNT;
}

//
// static inline void udelay(u32 us)
// {
// 	// reset_timer();
// 	// u64 end = get_timer() + us*(TIMER_HZ/1000000);
// 	// while(get_timer()<end);
// 	Delay_Us(us);
// }


// static inline void mdelay(u32 ms)
// {
// 	// reset_timer();
// 	// u64 end = get_timer() + ms*(TIMER_HZ/1000);
// 	// while(get_timer()<end);
// 	Delay_Ms(ms);

// }


void soft_timer(void)
{
	u64 tcnt = get_timer();
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
	volatile GPIO_TypeDef *gpio = (GPIO_TypeDef*)(GPIOA_BASE+group*0x0400);
	int mask = 1<<bit;

	if(val)
		gpio->BSHR = mask;
	else
		gpio->BCR = mask;
}


int gpio_get(int group, int bit)
{
	volatile GPIO_TypeDef *gpio = (GPIO_TypeDef*)(GPIOA_BASE+group*0x0400);
	int mask = 1<<bit;

	return (gpio->INDR & mask) ? 1: 0;
}


void gpio_mode(int group, int bit, int mode, int ioval)
{
	int mreg;
	volatile GPIO_TypeDef *gpio = (GPIO_TypeDef*)(GPIOA_BASE+group*0x0400);

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


// static char *excp_msg[16] = {
// 	"Inst Align",
// 	"Inst Fault",
// 	"Inst Illegal",
// 	"Breakpoint",
// 	"Load Align",
// 	"Load Fault",
// 	"Store Align",
// 	"Store Fault",
// 	"ECall_U",
// 	"ECall_S",
// 	"Reserved",
// 	"ECall_M",
// 	"IPage Fault",
// 	"LPage Fault",
// 	"Reserved",
// 	"SPage Fault",
// };


// __attribute__((interrupt)) �ᱣ��������ʱ�Ĵ����͸���Ĵ���
// __attribute__((interrupt("WCH-Interrupt-fast"))) ֻ���渡��Ĵ���
// ��������Ǹ���Ĵ���, ���Բ�����Щ����. ��������start.S����д��ڣ��ֶ�mret���ء�

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

// ���ȼ�: 0-7
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
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1|RCC_AHBPeriph_SRAM|RCC_AHBPeriph_DMA2,ENABLE);//SRAM DMA2 DMA1,��֪��ΪɶҪ��sram��ʾ������û�����ٷ�USBתJTAGҲû��
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE |
	                        RCC_APB2Periph_USART1|RCC_APB2Periph_TIM1|RCC_APB2Periph_TIM8, ENABLE );
	/* AFIO clock enable */
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_AFIO, ENABLE );
    RCC_APB1PeriphClockCmd( RCC_APB1Periph_PWR|RCC_APB1Periph_USART3|RCC_APB1Periph_SPI2|RCC_APB1Periph_UART8, ENABLE );
    /* �������е�PB3��PB4��Ӧ�뵥Ƭ����JTAG����,���Ա����Ƚ���JTAG���� */
#if( DEF_DEBUG_FUN_EN == 1 )
    GPIO_PinRemapConfig( GPIO_Remap_SWJ_Disable, ENABLE );
#endif
	return( 0x00 );
}
void LEDINIT()
{
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOC, ENABLE ); //�����ж���Ҫ��AFIO
	GPIO_InitTypeDef  GPIO_InitStructure;
    //RCC�Ѿ�������,���ﲻ�ظ�����
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8; //13 14 10
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
}
//TODO:ԭ��ʵ������jtag_setup();����,����SIO_SET_BITMODE_REQUEST:�������һ��,�����滻
void JTGA_IO_Init()
{
    // TCK: 
    // TMS: 
    // TDI: 
    // TDO: 
    GPIO_InitTypeDef  GPIO_InitStructure;
    //RCC�Ѿ�������,���ﲻ�ظ�����
    GPIO_InitStructure.GPIO_Pin = GPIO_TMS|GPIO_TCK|GPIO_TDI; //13 14 10
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(JTAG_GPIOPORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_TDO; //13 14 10
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;//������������ν,���������,�������ԭ�������
    GPIO_Init(JTAG_GPIOPORT, &GPIO_InitStructure);
}
void BtnInit()
{ 
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    EXTI_InitTypeDef EXTI_InitStructure = {0};
    NVIC_InitTypeDef NVIC_InitStructure = {0};
    RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA|RCC_APB2Periph_AFIO, ENABLE ); //�����ж���Ҫ��AFIO

    /* GPIO In Configuration: PA8) */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* GPIOA ----> EXTI_Line0 */
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource8);
    EXTI_InitStructure.EXTI_Line = EXTI_Line8;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}
void Periph_Init()
{
    JTGA_IO_Init();
}
void btnHandler(u8 btnStatus)
{
	if(btnStatus)
	{
		 PIN_LED_OUT(1);
	}
	else
	{
		 PIN_LED_OUT(0);
	}
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
	LEDINIT();
	BtnInit();
    /**SDIPRINT���޵��Ե�ʱ��ʹ�ã�����д�ӡ���µ�����½ӵ������ƺ�������**/
    #if (SDI_PRINT == SDI_PR_OPEN) 
        SDI_Printf_Enable();//��λ������������ν
    #else
        USART_Printf_Init(460800);
    #endif
	DEBUG_PRINT("SysClk:%d\r\n",SystemCoreClock);
	DEBUG_PRINT( "ChipID:%08x\r\n", DBGMCU_GetCHIPID() );
	PIN_LED_OUT(1);

	usb_dc_user_init(0,0);
	reset_timer();
	while(1){
		soft_timer();
		ftdi_timer_handle();//time_ms>=ftdevs[0].timeout�Żᷢ��
		// WFI(); // WFI��Ӱ�����ڹ���������.
		jtag_handle();
		btnHandler(btn_Flag);
	}
}



// �жϷ����� //TODO:���������⣬����һ���жϾͽ������ˡ�����֪��Ϊɶ
void EXTI9_5_IRQHandler(void)
{
    // ����Ƿ���PA8�������ж�
    if(EXTI_GetITStatus(EXTI_Line8) != RESET)
    {
        //
        Delay_Ms(1);//��������Ҫ��ϵģ����ں�������
        
        // �ٴμ�鰴��״̬��ȷ�ϰ���ȷʵ������
        if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_8) == 0)
        {
            // ������Ӱ������º�Ĵ������
            // ���磺���ñ�־λ������LED��
			btn_Flag = !btn_Flag;
        }
        
        // // ����жϱ�־λ
		
        EXTI_ClearITPendingBit(EXTI_Line8);
    }

}
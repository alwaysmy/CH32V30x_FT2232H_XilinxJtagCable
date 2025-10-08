
#ifndef _MAIN_H_
#define _MAIN_H_

#include "ch32v30x.h"

// #define DEBUGPRINT_ENABLE
#ifdef DEBUGPRINT_ENABLE
    #define DEBUG_PRINT(fmt, ...) printf("[DGB] " fmt, ##__VA_ARGS__)
#else
    #define DEBUG_PRINT(fmt, ...) // �ն��壬��ȫ�Ƴ����Դ���
#endif
// #define DEBUG_PRINT printf

#ifndef NULL
#define NULL ((void*)0)
#endif
// typedef struct
// {
// 	uint32_t CTLR;
// 	uint32_t SR;
// 	uint32_t CNTL;
// 	uint32_t CNTH;
// 	uint32_t CMPLR;
// 	uint32_t CMPHR;
// }SYSTICK_REGS;

// #define SYSTICK ((volatile SYSTICK_REGS*) 0xe000f000)
// typedef struct
// {
// 	uint32_t ECR;
// 	uint32_t PCFR1;
// 	uint32_t EXTICR[4];
// 	uint32_t unuse_18;
// 	uint32_t PCFR2;
// }AFIO_REGS;


typedef struct
{
	uint32_t STATR;
	uint32_t DATAR;
	uint32_t BRR;
	uint32_t CTLR1;
	uint32_t CTLR2;
	uint32_t CTLR3;
	uint32_t GPR;
}UART_REGS;
typedef struct
{
	uint32_t CFGR;
	uint32_t CNTR;
	uint32_t PADDR;
	uint32_t MADDR;
	uint32_t unuse;
}DMACH_REGS;
typedef struct
{
	uint32_t INTFR;
	uint32_t INTFCR;
	DMACH_REGS CH[8];
}DMA_REGS;
#define REG(x) (*(volatile unsigned int*)(x))

#define DMACH1 ((volatile DMA_REGS*) 0x40020000)
#define UARTCH1 ((volatile UART_REGS*) 0x40013800)

/******************************************************************************/
#define SYSCLK_FREQ  144000000
#define AHBCLK_FREQ  SYSCLK_FREQ
#define APB1CLK_FREQ AHBCLK_FREQ
#define APB2CLK_FREQ AHBCLK_FREQ


/******************************************************************************/





/* Global define */
#define TIMER_HZ (SYSCLK_FREQ/8)

/*ϵͳ����״̬�Ĵ���*/ //��һ��
#define STK_SR_CNTIF (1) //BIT0 ��1Ϊup cnt�ﵽ�˱Ƚ�ֵ����down cnt�ﵽ��0
/*ϵͳ�������ƼĴ���*/		
#define STK_CTLR_SWIE (1<<31) //BIT31,1����������жϣ���Ҫ�����0
#define STK_CTLR_INIT (1<<5) //BIT5��������ֵ���£�up cntʱ�����Ϊ0��down cnt������ʱ�����Ϊ�Ƚ�ֵ
#define STK_CTLR_MODE (1<<4) //BIT4��1Ϊ���¼�����0Ϊup cnt
#define STK_CTLR_STRE (1<<3) //BIT3,�Ƚ�ֵ�Զ����أ�0�����ϵ��Ƚ�ֵ��������ϣ����ǲ��ܣ������µ�0���µ����ֵ��1�����ϼ������Ƚ�ֵ���㣬���¼�����0���Ƚ�ֵ
#define STK_CTLR_STCLK (1<<2) //BIT2,ѡ��ʱ�ӣ�0ΪHCLK/8 ��1ΪHCLK
#define STK_CTLR_STIE (1<<1) //BIT1,1�����������ж�
#define STK_CTLR_STE (1) //BIT0,1����Systick



extern u32 time_ms;

void system_init(void);
u64 get_mcycle(void);
void timer_init(void);
void reset_timer(void);
// u64 get_timer(void);
// void udelay(u32 us);
// void mdelay(u32 ms);


void int_priorit(int id, int priorit);
void int_enable(int id);
void int_disable(int id);

void uart1_init(int baudrate);
int  _getc(int tmout);
void _putc(int ch);
void _puts(char *str);


void gpio_mode(int group, int bit, int mode, int pullup);
void gpio_set(int group, int bit, int val);
int  gpio_get(int group, int bit);


/******************************************************************************/



// int printk(char *fmt, ...);
// int sprintk(char *sbuf, const char *fmt, ...);
void hex_dump(char *str, void *addr, int size);


unsigned int strlen(const char *s);
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, unsigned int n);
int strcasecmp(const char *s1, const char *s2);
char *strcpy(char *dst, const char *src);
char *strncpy(char *dst, const char *src, unsigned int n);
char *strchr(const char *s1, int ch);
unsigned long strtoul(const char *str, char **endptr, int requestedbase);

void *memset(void *s, int v, unsigned int n);
void *memcpy(void *to, const void *from, unsigned int n);
int memcmp(const void *dst, const void *src, unsigned int n);


void simple_shell(void);


/******************************************************************************/


#endif


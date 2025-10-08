// 用C实现的jtag_trans_func大概能达到2-3M的速度。
// 主频 144M
// 这里手写一下，能有6M的速度，并可以方便的调整延时参数，实测大概6.4M（原工程）
// 在当前操作下大概有7.2M的速度，不知道为啥提高了。。。
// 

#include "ftdi.h"

//用于传入GPIO_xx操作的基地址,这几个都没用到，避免备用这里留着也不碍事
#define TCK_BASE_REG t0 
#define TDI_BASE_REG t1
#define TDO_BASE_REG t2
#define TMS_BASE_REG t3
//用于保存MASK(引脚偏移)的寄存器
#define TCK_MASK_REG t4
#define TMS_MASK_REG t5
#define TDI_MASK_REG t6
// #define TDO_MASK_REG a5

#define REG_TMP     a6
#define RDATA a7
#define DATA_REG a0
#define BCNT_REG a1
#define DELAY_REG a2


// 延时宏 -，保存代码空间
// 参数: delay_reg - 包含延时周期数的寄存器
//       tmp_reg   - 临时寄存器（会被修改）
.macro JTAG_DELAY delay_reg, tmp_reg=a6
//     // beqz    \delay_reg, 8f       # 0延时则跳过 //倒计数的更慢
//     mv      \tmp_reg, \delay_reg
// 7:  
//     addi    \tmp_reg, \tmp_reg, -1
//     bnez    \tmp_reg, 7b
// // 8:
	li		\tmp_reg, 0         // jtag_delay
3:
	addi	\tmp_reg, \tmp_reg, 1
	bltu	\tmp_reg, \delay_reg, 3b
.endm



// int jtag_trans_func(int data, int bcnt, int delay);
//                         a0        a1        a2
//   a3: TDI mask
//   a4: GPIO_BASE //TODO:TMS那里有几个切换BASE可以省，懒得看了，先用着，另外有几个切换BASE没对应上，懒得换
//   a5: TCK mask
//   a6: tmp
//   a7: rdata


///////////////////////////////////////////////////////////////////////////////

	.global jtag_trans_msb_0
	.func   jtag_trans_msb_0
jtag_trans_msb_0:
	li		a3, TDI_MASK
	li		a4, TCK_BASE //这里TCK TDI TDO共用一个BASE，节省一点时间这里不操作了
 // lui		a5, 2 //lui为左移12位，这里就是bit14,这里之前写死了，lui单条指令稍微快一点，但是
	li		a5,TCK_MASK 
	li		a7, 0

99:
	sw		a5, GPIO_CLR(a4)  // set_tck(0)

	andi	a6, a0, 0x80  // set_tdi(data&0x80)
	bnez	a6, 1f
	sw		a3, GPIO_CLR(a4) //这里应该切换a4 到TDI_BASE,但是节省指令就算了
	j		2f
1:
	sw		a3, GPIO_SET(a4) //这里应该切换a4 到TDI_BASE,但是节省指令就算了
2:
	JTAG_DELAY DELAY_REG, a6
// 	li		a6, 0         // jtag_delay
// 3:
// 	addi	a6, a6, 1
// 	bltu	a6, a2, 3b

	lw		a6, GPIO_IN(a4)  // get_tdo() //这里应该切换a4 到TDO_BASE,但是节省指令就算了

	sw		a5, GPIO_SET(a4) // set_tck(1); //这里应该切换a4 到TCK_BASE,但是节省指令就算了

	srli	a6, a6, TDO_SHIFT_MSB
	slli	a7, a7, 1     // rdata <<= 1;
	andi	a6, a6, 0x01
	or		a7, a7, a6    // rdata |= tdo;

	slli	a0, a0, 1     // data <<= 1;

// 	li		a6, 0         // jtag_delay
// 3:
// 	addi	a6, a6, 1
// 	blt		a6, a2, 3b
	JTAG_DELAY DELAY_REG, a6

	addi	a1, a1, -1
	bnez	a1, 99b

	sw		a5, GPIO_CLR(a4)  // set_tck(0) ////这里应该切换a4 到TCK_BASE,但是节省指令就算了
	mv		a0, a7
	ret
	.endfunc

///////////////////////////////////////////////////////////////////////////////


	.global jtag_trans_msb_1
	.func   jtag_trans_msb_1
jtag_trans_msb_1:
	li		a3, TDI_MASK
	li		a4, TCK_BASE	//这里TCK TDI TDO共用一个BASE，节省一点时间这里不操作了
	li		a5,TCK_MASK
	li		a7, 0

99:
	sw		a5, GPIO_SET(a4)  // set_tck(1) //这里应该切换a4 到TCK_BASE,但是节省指令就算了

	andi	a6, a0, 0x80  // set_tdi(data&0x80)
	bnez	a6, 1f
	sw		a3, GPIO_CLR(a4) //这里应该切换a4 到TDI_BASE,但是节省指令就算了
	j		2f
1:
	sw		a3, GPIO_SET(a4) //这里应该切换a4 到TDI_BASE,但是节省指令就算了
2:

	JTAG_DELAY DELAY_REG, a6

	lw		a6, GPIO_IN(a4)  // get_tdo()

	sw		a5, GPIO_CLR(a4) // set_tck(0);

	srli	a6, a6, TDO_SHIFT_MSB
	slli	a7, a7, 1     // rdata <<= 1;
	andi	a6, a6, 0x01
	or		a7, a7, a6    // rdata |= tdo;

	slli	a0, a0, 1     // data <<= 1;

// 	li		a6, 0         // jtag_delay
// 3:
// 	addi	a6, a6, 1
// 	blt		a6, a2, 3b
	JTAG_DELAY DELAY_REG, a6

	addi	a1, a1, -1
	bnez	a1, 99b

	sw		a5, GPIO_SET(a4)  // set_tck(1)
	mv		a0, a7
	ret
	.endfunc

///////////////////////////////////////////////////////////////////////////////

	.global jtag_trans_lsb_0
	.func   jtag_trans_lsb_0
jtag_trans_lsb_0:
	li		a3, TDI_MASK
	li		a4, TCK_BASE
	li		a5, TCK_MASK
	li		a7, 0

99:
	sw		a5, GPIO_CLR(a4)  // set_tck(0)

	andi	a6, a0, 0x01  // set_tdi(data&0x01)
	bnez	a6, 1f
	sw		a3, GPIO_CLR(a4)
	j		2f
1:
	sw		a3, GPIO_SET(a4)
2:

	JTAG_DELAY DELAY_REG, a6

	lw		a6, GPIO_IN(a4)  // get_tdo()

	sw		a5, GPIO_SET(a4) // set_tck(1);

	srli	a6, a6, TDO_SHIFT_LSB
	srli	a7, a7, 1     // rdata >>= 1;
	andi	a6, a6, 0x80
	or		a7, a7, a6    // rdata |= tdo;

	srli	a0, a0, 1     // data >>= 1;

// 	li		a6, 0         // jtag_delay
// 3:
// 	addi	a6, a6, 1
// 	blt		a6, a2, 3b
	JTAG_DELAY DELAY_REG, a6

	addi	a1, a1, -1
	bnez	a1, 99b

	sw		a5, GPIO_CLR(a4)  // set_tck(0)
	mv		a0, a7
	ret
	.endfunc

///////////////////////////////////////////////////////////////////////////////

	.global jtag_trans_lsb_1
	.func   jtag_trans_lsb_1
jtag_trans_lsb_1:
	li		a3, TDI_MASK
	li		a4, TCK_BASE
	// lui		a5, 2 //lui为左移12位，这里就是bit14,这里之前写死了，lui也没更快不知道为啥要这样写
	li		a5,TCK_MASK
	li		a7, 0

99:
	sw		a5, GPIO_SET(a4)  // set_tck(1)

	andi	a6, a0, 0x01  // set_tdi(data&0x01)
	bnez	a6, 1f
	sw		a3, GPIO_CLR(a4)
	j		2f
1:
	sw		a3, GPIO_SET(a4)
2:

	JTAG_DELAY DELAY_REG, a6

	lw		a6, GPIO_IN(a4)  // get_tdo()

	sw		a5, GPIO_CLR(a4) // set_tck(0);

	srli	a6, a6, TDO_SHIFT_LSB
	srli	a7, a7, 1     // rdata >>= 1;
	andi	a6, a6, 0x80
	or		a7, a7, a6    // rdata |= tdo;

	srli	a0, a0, 1     // data >>= 1;

// 	li		a6, 0         // jtag_delay
// 3:
// 	addi	a6, a6, 1
// 	blt		a6, a2, 3b
	JTAG_DELAY DELAY_REG, a6

	addi	a1, a1, -1
	bnez	a1, 99b

	sw		a5, GPIO_SET(a4)  // set_tck(1)
	mv		a0, a7
	ret
	.endfunc


///////////////////////////////////////////////////////////////////////////////

	.global jtag_trans_tms_0
	.func   jtag_trans_tms_0
jtag_trans_tms_0:
	li		a3, TMS_MASK
	li		a4, TCK_BASE  
	li 		TMS_BASE_REG,TMS_BASE //这里保存BASE,避免后面切换
	li		a5,TCK_MASK
	li		a7, 0

99:
	sw		a5, GPIO_CLR(a4)  // set_tck(0) //这里需要切换base

	andi	a6, a0, 0x01  // set_tms(data&0x01) //a6 = a0 & 0x01;
	bnez	a6, 1f
	li		a4, TMS_BASE
	sw		a3, GPIO_CLR(a4) 
	// sw		a3, GPIO_CLR(TMS_BASE_REG) 
	j		2f
1:
	li		a4, TMS_BASE
	sw		a3, GPIO_SET(a4) 
	// sw		a3, GPIO_SET(TMS_BASE_REG) 
2:
	JTAG_DELAY DELAY_REG, a6
	li		a4, TCK_BASE	//切换寄存器
	lw		a6, GPIO_IN(a4)  // get_tdo() //这里切换寄存器base
	sw		a5, GPIO_SET(a4) // set_tck(1); //切换base

	srli	a6, a6, TDO_SHIFT_LSB
	srli	a7, a7, 1     // rdata >>= 1;
	andi	a6, a6, 0x80
	or		a7, a7, a6    // rdata |= tdo;

	srli	a0, a0, 1     // data >>= 1;

// 	li		a6, 0         // jtag_delay
// 3:
// 	addi	a6, a6, 1
// 	blt		a6, a2, 3b
	JTAG_DELAY DELAY_REG, a6

	addi	a1, a1, -1
	bnez	a1, 99b
	li		a4, TCK_BASE
	sw		a5, GPIO_CLR(a4)  // set_tck(0) //切换base
	mv		a0, a7
	ret
	.endfunc


///////////////////////////////////////////////////////////////////////////////

	.global jtag_trans_tms_1
	.func   jtag_trans_tms_1
jtag_trans_tms_1:
	li		a3, TMS_MASK
	li		a4, TCK_BASE  
	li 		TMS_BASE_REG,TMS_BASE //这里保存BASE,避免后面切换//TODO:这两个函数中，实测如果后面不切换反而更慢，最神秘的是这一句虽然没什么用，但是留在这里速度会变快。。。
	// lui		a5, 2 //lui为左移12位，这里就是bit14,这里之前写死了，lui也没更快不知道为啥要这样写
	li		a5,TCK_MASK
	li		a7, 0

99:
	// li		a4, TCK_BASE
	sw		a5, GPIO_SET(a4)  // set_tck(1) 

	andi	a6, a0, 0x01  // set_tms(data&0x01)
	bnez	a6, 1f			//"if (a6 != 0) goto 1f;"
	li		a4, TMS_BASE
	sw		a3, GPIO_CLR(a4) 
	// sw		a3, GPIO_CLR(TMS_BASE_REG) 
	j		2f
1:
	li		a4, TMS_BASE
	sw		a3, GPIO_SET(a4) 
	// sw		a3, GPIO_SET(TMS_BASE_REG) 
2:

	JTAG_DELAY DELAY_REG, a6
	li		a4, TCK_BASE	//切换寄存器
	lw		a6, GPIO_IN(a4)  // get_tdo() 
	sw		a5, GPIO_CLR(a4) // set_tck(0);

	srli	a6, a6, TDO_SHIFT_LSB
	srli	a7, a7, 1     // rdata >>= 1;
	andi	a6, a6, 0x80
	or		a7, a7, a6    // rdata |= tdo;

	srli	a0, a0, 1     // data >>= 1;

// 	li		a6, 0         // jtag_delay
// 3:
// 	addi	a6, a6, 1
// 	blt		a6, a2, 3b
	JTAG_DELAY DELAY_REG, a6

	addi	a1, a1, -1
	bnez	a1, 99b
	// li		a4, TCK_BASE
	sw		a5, GPIO_SET(a4)  // set_tck(1)
	mv		a0, a7
	ret
	.endfunc
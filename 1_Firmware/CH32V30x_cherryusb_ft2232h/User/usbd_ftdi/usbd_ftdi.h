#ifndef _USBD_FTDI_H_
#define _USBD_FTDI_H_

#include "main.h"

#define EEPROM _93C46_    //暂时没用
// #define EEPROM _93C56_
// #define EEPROM _93C66_

#define FTDI_DEV FT2232H  //暂时没用




/******************************************/
/* FTDI vendor Requests */
#define SIO_RESET_REQUEST             0x00 /* Reset the port */
#define SIO_SET_MODEM_CTRL_REQUEST    0x01 /* Set the modem control register */
#define SIO_SET_FLOW_CTRL_REQUEST     0x02 /* Set flow control register */
#define SIO_SET_BAUDRATE_REQUEST      0x03 /* Set baud rate */
#define SIO_SET_DATA_REQUEST          0x04 /* Set the data characteristics of the port */
#define SIO_POLL_MODEM_STATUS_REQUEST 0x05
#define SIO_SET_EVENT_CHAR_REQUEST    0x06
#define SIO_SET_ERROR_CHAR_REQUEST    0x07
#define SIO_SET_LATENCY_TIMER_REQUEST 0x09
#define SIO_GET_LATENCY_TIMER_REQUEST 0x0A
#define SIO_SET_BITMODE_REQUEST       0x0B
#define SIO_READ_PINS_REQUEST         0x0C
#define SIO_READ_EEPROM_REQUEST       0x90
#define SIO_WRITE_EEPROM_REQUEST      0x91
#define SIO_ERASE_EEPROM_REQUEST      0x92
/******************************************************************************/


#define EEPROM_WORD_LEN 0x80 //93c56是0x80,93c46是0x40,93cc66是0x100,高的可以直接兼容低的
// #define EEPROM_WORD_LEN 0x40
// #define EEPROM_WORD_LEN 0x100

#define FT2232H_PID 0x6010
#define FT4232H_PID 0x6011
#define FT232H_PID 0x6014
/* bcd devices,必须对应准确的型号，否则PC驱动操作不一致，读eeprom会卡住*/
#define FT2232H_BCDD 0x0700
#define FT2232D_BCDD 0x0500  //0x400也显示2232D
#define FT232R_BCDD 0x0600
#define FT4232H_BCDD 0x0800
#define FT232H_BCDD 0x0900
/******************************************************************************/


extern const uint16_t ftdi_eeprom_info[];

#endif //_USBD_FTDI_H_
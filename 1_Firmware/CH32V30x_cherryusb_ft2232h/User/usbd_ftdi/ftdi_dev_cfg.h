#ifndef _FTDI_DEV_CFG_H_
#define _FTDI_DEV_CFG_H_

#define FT232H 0
#define FT2232H 1
#define FT4232H 2

#define EEPROM_93C46 0
#define EEPROM_93C56 1
#define EEPROM_93C66 2

// /*下面是用户编辑的内容*/
// #define FTDI_DEV FT2232H
#define FTDI_DEV FT232H

// 注意，FT232H不支持93C46
// #define EEPROM EEPROM_93C46    
#define EEPROM EEPROM_93C56
// #define EEPROM EEPROM_93C66





#endif //_FTDI_DEV_CFG_H_
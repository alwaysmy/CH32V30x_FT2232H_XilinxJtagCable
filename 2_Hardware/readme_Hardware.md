# 板卡说明

这个项目用于ARTIX-7 GTP USB3 WCH-MCU这块FPGA核心板，板上有一个QFN28封装的CH32V305,引脚兼容CH347 ,JTAG 引脚见下表：

| JTAG引脚 | 引脚编号 | GPIO编号 |
| -------- | -------- | -------- |
| TCK      |          | PB14     |
| TMS      |          | PB13     |
| TDI      |          | PB10     |
| TDO      |          | PD2      |

具体原理图见项目：[alwaysmy/Artix-7_GTP_USB3_WCH_MCU](https://github.com/alwaysmy/Artix-7_GTP_USB3_WCH_MCU)

# 硬件兼容性

CH32V30x都是一个用启动文件，除了Flash/RAM不一样可以直接用，这里占用资源很少都可以用，所以对于不同封装和不同型号，只需要修改对应引出的引脚即可。（注意WCH的手册，有的引脚内部是连接的）

这里引脚用的是IO模拟，所以原则上来说JTAG四根线放哪里都行，但是考虑到其他项目的兼容性（WCH的usb-jtag-spi）以及这个项目后期的修改，最好把TCK, TDI , TDO 放到硬件SPI上

# 注意项

CH32V305没有BOOT引脚，SWD引脚如果复用了就只能通过WCH-LinkE清除Flash(步骤xxx),所以最好没事儿不要占用这两个引脚，很麻烦。
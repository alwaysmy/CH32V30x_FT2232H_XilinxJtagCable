

#include "main.h"

#include "usbd_core.h"
#include "cdc_uart.h"
#include "ftdi.h"
#include "usbd_ftdi.h"

#define FIXED_FTDI_TIMER 16 //固定设计16ms超时
/*************************************************************************************/
#define USBD_VID           0x0403
#define USBD_MAX_POWER     100
#if (FTDI_DEV == FT2232H)
#define USBD_PID           FT2232H_PID
#define USBD_BCDD		   FT2232H_BCDD	
#elif (FTDI_DEV == FT232H)
#define USBD_PID           FT232H_PID
#define USBD_BCDD		   FT232H_BCDD	
#endif

#if (EEPROM == EEPROM_93C46)
#define EEPROM_WORD_LEN 0x40 //93c56是0x80,93c46是0x40,93cc66是0x100,高的可以直接兼容低的
#elif  (EEPROM == EEPROM_93C56)
#define EEPROM_WORD_LEN 0x80 //93c56是0x80,93c46是0x40,93cc66是0x100,高的可以直接兼容低的
#elif (EEPROM == EEPROM_93C66)
#define EEPROM_WORD_LEN 0x100 //93c56是0x80,93c46是0x40,93cc66是0x100,高的可以直接兼容低的
#else
#define EEPROM_WORD_LEN 0x100 //93c56是0x80,93c46是0x40,93cc66是0x100,高的可以直接兼容低的
#endif
// #define EEPROM_WORD_LEN 0x40
// #define EEPROM_WORD_LEN 0x100

// #define USBD_LANGID_STRING 1033

/* 注意: FTDI的驱动要求in与out端点号不能是同一个。 */
/*!< endpoint address */
#define JTAG_IN_EP  0x81
#define JTAG_OUT_EP 0x02
#define JTAG_INTF   0
#define JTAG_INTERFACE_SIZE (9 + 7 + 7)


#define UART_IN_EP  0x83
#define UART_OUT_EP 0x04
#define UART_INT_EP 0x85
#define UART_INTF   1
#define UART_INTERFACE_SIZE (9 + 7 + 7)


#define USB_CONFIG_SIZE (9 + JTAG_INTERFACE_SIZE + UART_INTERFACE_SIZE)
#if FTDI_DEV==FT232H
	//TODO:我记得之前可以让驱动识别FT232的同时加第二个interface的，现在会报错不匹配，先关掉吧,应该也不影响用2232开另一个端点
	#define INTF_NUM        1
#elif FTDI_DEV==FT2232H||FTDI_DEV==FT4232H
	#define INTF_NUM        2
#endif


// #define USB_PACKET_SIZE CONFIG_USBDEV_MSC_MAX_BUFSIZE

#ifdef CONFIG_USB_HS
#define USB_PACKET_SIZE 512
#else
#define USB_PACKET_SIZE 64
#endif


#define JTAG_OUT_SIZE  USB_PACKET_SIZE
#define JTAG_IN_SIZE   USB_PACKET_SIZE*8 //TODO: 确定？
/*--------------------------------用户定义描述符开始--------------------------*/
static const uint8_t device_descriptor[] = {
    USB_DEVICE_DESCRIPTOR_INIT(USB_2_0, 0x00, 0x00, 0x00, USBD_VID, USBD_PID, USBD_BCDD, 0x01),//0x900是232H,这里程序实际上应该有问题，如果是232H,驱动只会加载一个端口，pid需要修改为6014,这里bcdDevice应该是0x700否则认为FT2232D，0x400还是2232D,0x600是232R
};

static const uint8_t config_descriptor[] = {
    // USB_CONFIG_DESCRIPTOR_INIT(USB_CONFIG_SIZE, 0x02, 0x01, USB_CONFIG_BUS_POWERED, USBD_MAX_POWER),
    // CDC_ACM_DESCRIPTOR_INIT(0x00, CDC_INT_EP, CDC_OUT_EP, CDC_IN_EP, CDC_MAX_MPS, 0x02)
	USB_CONFIG_DESCRIPTOR_INIT(USB_CONFIG_SIZE, INTF_NUM, 1, USB_CONFIG_BUS_POWERED, USBD_MAX_POWER),
	// USB_CONFIG_DESCRIPTOR_INIT(USB_CONFIG_SIZE, 1, 1, USB_CONFIG_BUS_POWERED, USBD_MAX_POWER),

 	/* Interface 0 : JTAG */
	USB_INTERFACE_DESCRIPTOR_INIT(JTAG_INTF, 0, 2, 0xFF, 0xFF, 0xFF, 2),
	USB_ENDPOINT_DESCRIPTOR_INIT(JTAG_IN_EP,  USB_ENDPOINT_TYPE_BULK, USB_PACKET_SIZE, 0x00),
	USB_ENDPOINT_DESCRIPTOR_INIT(JTAG_OUT_EP, USB_ENDPOINT_TYPE_BULK, USB_PACKET_SIZE, 0x00),//这里四个原来端点描述符就不一样，写了0x01,但是bInterval在ft2232上是0，这里照抄了

	#if FTDI_DEV==FT232H
		//TODO:我记得之前可以让驱动识别FT232的同时加第二个interface的，现在会报错不匹配，先关掉吧,应该也不影响用2232开另一个端点
	#elif FTDI_DEV==FT2232H||FTDI_DEV==FT4232H
	/* Interface 1 : UART */
	USB_INTERFACE_DESCRIPTOR_INIT(UART_INTF, 0, 2, 0xFF, 0xFF, 0xFF, 4),
	USB_ENDPOINT_DESCRIPTOR_INIT(UART_IN_EP,  USB_ENDPOINT_TYPE_BULK, USB_PACKET_SIZE, 0x00),
	USB_ENDPOINT_DESCRIPTOR_INIT(UART_OUT_EP, USB_ENDPOINT_TYPE_BULK, USB_PACKET_SIZE, 0x00)
	#endif

};

static const uint8_t device_quality_descriptor[] = {
    ///////////////////////////////////////
    /// device qualifier descriptor
    ///////////////////////////////////////
    0x0a,
    USB_DESCRIPTOR_TYPE_DEVICE_QUALIFIER,
    0x00,
    0x02,
    0x00,
    0x00,
    0x00,
    0x40,
    0x01,
    0x00,
}; //0x0a, USB_DESCRIPTOR_TYPE_DEVICE_QUALIFIER, 0x00, 0x02, 0x00, 0x00, 0x00, 0x40, 0x01, 0x00,

static const char *string_descriptors[] = {
    (const char[]){ 0x09, 0x04 }, /* Langid */
    "Xilinx",                  /* Manufacturer */
    "FTLINK JTAG",         		/* Product */
    "2025100501",                 /* Serial Number */
	"FTLINK UART",				/* Product */
};
/*--------------------------用户定义描述符结束-------------------------------------------------------------*/
static const uint8_t *device_descriptor_callback(uint8_t speed)
{
    return device_descriptor;
}

static const uint8_t *config_descriptor_callback(uint8_t speed)
{
    return config_descriptor;
}

static const uint8_t *device_quality_descriptor_callback(uint8_t speed)
{
    return device_quality_descriptor;
}

static const char *string_descriptor_callback(uint8_t speed, uint8_t index)
{
    if (index > 4) { //没有以前好用啊，这个数字是自定义的，需要根据实际的字符描述符确定，这里0-4条
        return NULL;
    }
    return string_descriptors[index];
}

const struct usb_descriptor FTlink_descriptor = {
    .device_descriptor_callback = device_descriptor_callback,
    .config_descriptor_callback = config_descriptor_callback,
    .device_quality_descriptor_callback = device_quality_descriptor_callback,
    .string_descriptor_callback = string_descriptor_callback
};



/************************************需要用户修改******************************************/
static uint8_t jtag_req_buf[JTAG_OUT_SIZE];
static uint8_t jtag_resp_buf[JTAG_IN_SIZE];
static volatile int jtag_req_size;
// volatile int jtag_req_size;
volatile int jtag_resp_size;

static void jtag_out_callback(uint8_t busid,uint8_t ep, uint32_t nbytes);
static void jtag_in_callback(uint8_t busid,uint8_t ep, uint32_t nbytes);
int usbd_ftdi_vendor_handler(uint8_t busid,struct usb_setup_packet *setup, uint8_t **data, uint32_t *len);

static struct usbd_endpoint jtag_out_ep = {
	.ep_addr = JTAG_OUT_EP,
	.ep_cb = jtag_out_callback
};

static struct usbd_endpoint jtag_in_ep = {
	.ep_addr = JTAG_IN_EP,
	.ep_cb = jtag_in_callback
};

struct usbd_interface jtag_intf;
// struct usbd_interface jtag_intf = {
// 	.vendor_handler = usbd_ftdi_vendor_handler,
// 	.intf_num = 0
// };


typedef struct {
	int bitmode;
	int latency_timer;
	int timeout;
}FTDEV;

FTDEV ftdevs[2];



/******************************************************************************/






static void jtag_out_start(uint8_t busid)
{
	usbd_ep_start_read(busid,JTAG_OUT_EP, jtag_req_buf, JTAG_OUT_SIZE);
}

static void jtag_out_callback(uint8_t busid,uint8_t ep, uint32_t nbytes)
{
	jtag_req_size = nbytes;
	ftdevs[0].timeout = 0x7fffffff;
	//printk("out: %d\n", nbytes);
}

static void jtag_in_start(void)
{
	usbd_ep_start_write(0,JTAG_IN_EP, jtag_resp_buf, jtag_resp_size);
}

static void jtag_in_callback(uint8_t busid,uint8_t ep, uint32_t nbytes)
{
	//printk(" in: %d\n", nbytes);
	if(nbytes>2){
		jtag_resp_size = 2;
	}
	ftdevs[0].timeout = time_ms + ftdevs[0].latency_timer;
}


static void jtag_init(void)
{
	jtag_req_size = 0;

	jtag_resp_buf[0] = 0x02;
	jtag_resp_buf[1] = 0x60;
	jtag_resp_size = 2;
}


void jtag_flush_resp(uint8_t busid)
{
	usbd_ep_start_write(busid,JTAG_IN_EP, jtag_resp_buf, jtag_resp_size);
	while(jtag_resp_size>2);
}


void jtag_write(int byte)
{
	// FT2232看起来有一个非常大的in缓存，但一旦到了512字节的边界，就会插入02 60
	if((jtag_resp_size%USB_PACKET_SIZE)==0){
		jtag_resp_buf[jtag_resp_size+0] = 0x02;
		jtag_resp_buf[jtag_resp_size+1] = 0x60;
		jtag_resp_size += 2;
	}

	jtag_resp_buf[jtag_resp_size] = byte;
	jtag_resp_size += 1;

	if(jtag_resp_size == JTAG_IN_SIZE){
		usbd_ep_start_write(0,JTAG_IN_EP, jtag_resp_buf, jtag_resp_size); //fixed busid to 0
		while(jtag_resp_size>2);
	}
}


void jtag_handle(void)
{

	if(jtag_req_size==0)
		return;

	jtag_execute(jtag_req_buf, jtag_req_size);//处理从USB收到的数据，并传入长度
	ftdevs[0].timeout = time_ms + ftdevs[0].latency_timer;

	jtag_req_size = 0;
	jtag_out_start(0);//重新启动接收
}


/******************************************************************************/


static uint8_t uart_txbuf[TXBUF_SIZE];
static uint8_t usb_rxbuf[USB_PACKET_SIZE];
static uint8_t uart_rxbuf[RXBUF_SIZE];
static uint8_t usb_txbuf[TXBUF_SIZE];


CDC_UART cdc_uarts[] = {
	{
#if 0
		.uart = UART3,
		.pclk = APB1CLK_FREQ,
		.dma  = DMA1,
		.txch = 1,
		.txdma = &DMA1->CH[1],
		.rxch = 2,
		.rxdma = &DMA1->CH[2],
		.intf_num = UART_INTF,
		.out_ep = {
			.ep_addr = UART_OUT_EP,
		},
		.in_ep = {
			.ep_addr = UART_IN_EP,
		},
		.usb_rxbuf  = usb_rxbuf,
		.uart_txbuf = uart_txbuf,
		.usb_txbuf  = usb_txbuf,
		.uart_rxbuf = uart_rxbuf,
#endif
	},

	{
		.uart = UARTCH1,
		.pclk = APB2CLK_FREQ,
		.dma  = DMACH1,
		.txch = 3,
		.txdma = &DMACH1->CH[3],
		.rxch = 4,
		.rxdma = &DMACH1->CH[4],
		.intf_num = UART_INTF,
		.out_ep = {
			.ep_addr = UART_OUT_EP,
		},
		.in_ep = {
			.ep_addr = UART_IN_EP,
		},
		.usb_rxbuf  = usb_rxbuf,
		.uart_txbuf = uart_txbuf,
		.usb_txbuf  = usb_txbuf,
		.uart_rxbuf = uart_rxbuf,
	},

};


CDC_UART *get_cdcuart(int ep, int intf)
{
	return &cdc_uarts[1];
}


void dma1_channel3_irqhandler(void)
{
	// UART3.RXDMA
	dma_irq_handle(&cdc_uarts[0]);
}


void dma1_channel5_irqhandler(void)
{
	// UART1.RXDMA
	dma_irq_handle(&cdc_uarts[1]);
}


void usart3_irqhandler(void)
{
	uart_irq_handle(&cdc_uarts[0]);
}


void usart1_irqhandler(void)
{
	uart_irq_handle(&cdc_uarts[1]);
}


int cdc_in_lock(int *val)
{
	return __sync_val_compare_and_swap(val, 0, 1);
}

void cdc_in_unlock(int *val)
{
	*val = 0;
}


/******************************************************************************/







void ftdi_timer_handle(void)
{
	if(ftdevs[0].timeout){
		// DEBUG_PRINT("ftdi jtag timout:%d\n",ftdevs[0].timeout);
		// DEBUG_PRINT("time_ms:%d\n",time_ms);
		// DEBUG_PRINT("\033[31mT\n");
		if(time_ms>=ftdevs[0].timeout){

			ftdevs[0].timeout = 0;
			jtag_in_start();
		}
	}
	if(ftdevs[1].timeout){
		if(time_ms>=ftdevs[1].timeout){
			ftdevs[1].timeout = 0;
			cdc_send_startup(0,&cdc_uarts[1]);
		}
	}
}

static int eeprom_buf,data_buf;

int usbd_ftdi_vendor_handler(uint8_t busid,struct usb_setup_packet *setup, uint8_t **data, uint32_t *len)
{
	int port = setup->wIndex&0xff;
	if(port==0)
		port = 1;
	*len = 0;

	switch (setup->bRequest) {
	case SIO_READ_EEPROM_REQUEST:
		// if(setup->wIndex<0x40){ //原来只上报了64个值，适用于93C46 EEPROM，直接按照128来做也可以兼容的
		// USB_LOG_INFO("SIO_READ_EEPROM_REQUEST:type %02x request %02x, value %04x, index %04x, length %04x\n",setup->bmRequestType, setup->bRequest, setup->wValue, setup->wIndex, setup->wLength);
		if(setup->wIndex<EEPROM_WORD_LEN){
			eeprom_buf = ftdi_eeprom_info[setup->wIndex];
		}else{
			eeprom_buf = 0;
		}
		*data = (uint8_t*)&eeprom_buf;
		*len = 2;
		break;
	case SIO_RESET_REQUEST:
		USB_LOG_INFO("SIO_RESET_REQUEST: type %02x request %02x, value %04x, index %04x, length %04x\n",
			setup->bmRequestType, setup->bRequest, setup->wValue, setup->wIndex, setup->wLength);
		if(setup->wValue==0){
			// RESET_SIO命令后，FTDI驱动会重置ep的状态。ep将要发送的数据也被清除了。
			// 但FTDI驱动接下来会读ep，如果读不到就卡死了。这里设置一个16ms的定时器，
			// 到期后会发送数据给驱动。
			// ftdevs[port-1].timeout = time_ms + ftdevs[port-1].latency_timer;//TODO:这里按理来说应该按照上位机的超时设置来做
			ftdevs[port-1].timeout = time_ms + FIXED_FTDI_TIMER;
			DEBUG_PRINT("port:%d:timout:%d,timer:%d\n",port,ftdevs[port-1].timeout,ftdevs[port-1].latency_timer);
		}
		break;
	case SIO_SET_MODEM_CTRL_REQUEST:
		USB_LOG_INFO("SIO_SET_MODEM_CTRL_REQUEST: type %02x request %02x, value %04x, index %04x, length %04x\n",
			setup->bmRequestType, setup->bRequest, setup->wValue, setup->wIndex, setup->wLength);
		break;
	case SIO_SET_FLOW_CTRL_REQUEST:
		USB_LOG_INFO("SIO_SET_FLOW_CTRL_REQUEST: type %02x request %02x, value %04x, index %04x, length %04x\n",
			setup->bmRequestType, setup->bRequest, setup->wValue, setup->wIndex, setup->wLength);
		break;
	case SIO_SET_BAUDRATE_REQUEST://wValue，2个字节波特率
		USB_LOG_INFO("SIO_SET_BAUDRATE_REQUEST: type %02x request %02x, value %04x, index %04x, length %04x\n",
			setup->bmRequestType, setup->bRequest, setup->wValue, setup->wIndex, setup->wLength);
		if(port==2){
			struct cdc_line_coding lc;
			lc.dwDTERate = 3000000/setup->wValue;
			lc.bDataBits = 0;
			usbd_cdc_acm_set_line_coding(0,UART_INTF, &lc);
		}
		break;
	case SIO_SET_DATA_REQUEST:
		USB_LOG_INFO("SIO_SET_DATA_REQUEST: type %02x request %02x, value %04x, index %04x, length %04x\n",
			setup->bmRequestType, setup->bRequest, setup->wValue, setup->wIndex, setup->wLength);
		if(port==2){
			/**
			 * D0-D7 databits  BITS_7=7, BITS_8=8
			 * D8-D10 parity  NONE=0, ODD=1, EVEN=2, MARK=3, SPACE=4
			 * D11-D12 		STOP_BIT_1=0, STOP_BIT_15=1, STOP_BIT_2=2 
			 * D14  		BREAK_OFF=0, BREAK_ON=1
			 **/
			struct cdc_line_coding lc;
			lc.dwDTERate = 0;
			lc.bDataBits = setup->wValue&0xff;
			lc.bParityType = (setup->wValue>>8 )&0x07;
			lc.bCharFormat = (setup->wValue>>11)&0x03;
			usbd_cdc_acm_set_line_coding(0,UART_INTF, &lc);
		}
		break;
	case SIO_POLL_MODEM_STATUS_REQUEST:
		USB_LOG_INFO("SIO_POLL_MODEM_STATUS_REQUEST: type %02x request %02x, value %04x, index %04x, length %04x\n",
			setup->bmRequestType, setup->bRequest, setup->wValue, setup->wIndex, setup->wLength);
		data_buf = 0x6002;
		// data_buf = 0x6001;
		*data = (uint8_t*)&data_buf;
		// *data = (uint8_t*)&ftdi_eeprom_info[2];
		*len = 2;
		// return -2;
		break;
	case SIO_SET_EVENT_CHAR_REQUEST:
		USB_LOG_INFO("SIO_SET_EVENT_CHAR_REQUEST: type %02x request %02x, value %04x, index %04x, length %04x\n",
			setup->bmRequestType, setup->bRequest, setup->wValue, setup->wIndex, setup->wLength);
		break;
	case SIO_SET_ERROR_CHAR_REQUEST:
			USB_LOG_INFO("SIO_SET_ERROR_CHAR_REQUEST: type %02x request %02x, value %04x, index %04x, length %04x\n",
			setup->bmRequestType, setup->bRequest, setup->wValue, setup->wIndex, setup->wLength);
		break;
	case SIO_SET_LATENCY_TIMER_REQUEST:
			USB_LOG_INFO("SIO_SET_LATENCY_TIMER_REQUEST: type %02x request %02x, value %04x, index %04x, length %04x\n",
			setup->bmRequestType, setup->bRequest, setup->wValue, setup->wIndex, setup->wLength);
		ftdevs[port-1].latency_timer = setup->wValue + 1;
		//TODO：在vivado关闭server的时候，这个值被设置为3，但是vivado打开server的时候不会有这个毛病，这时候如果
		// 代码中有较多冗余内容例如打印，就会导致超时错过发送，就会卡死。。。。，临时解决办法是在SIO复位请求的时候忽略这个设置，依然按照16ms来做
		break;
	case SIO_GET_LATENCY_TIMER_REQUEST:
			USB_LOG_INFO("SIO_GET_LATENCY_TIMER_REQUEST: type %02x request %02x, value %04x, index %04x, length %04x\n",
			setup->bmRequestType, setup->bRequest, setup->wValue, setup->wIndex, setup->wLength);
		*data = (uint8_t*)&ftdevs[port-1].latency_timer;
		*len = 1;
		break;
	case SIO_SET_BITMODE_REQUEST:
		USB_LOG_INFO("SIO_SET_BITMODE_REQUEST: type %02x request %02x, value %04x, index %04x, length %04x\n",
			setup->bmRequestType, setup->bRequest, setup->wValue, setup->wIndex, setup->wLength);

		if(port==1){
			int mode = setup->wValue>>8;
			if(mode==0x02){
				jtag_setup();
			}else{
				jtag_exit();
			}
		}
		break;
	default:
		// USB_LOG_DBG("CDC ACM request 0x%x, value 0x%x\r\n", setup->bRequest, setup->wValue);
		USB_LOG_INFO("FTDI: type %02x request %02x, value %04x, index %04x, length %04x\n",
			setup->bmRequestType, setup->bRequest, setup->wValue, setup->wIndex, setup->wLength);
		return -1;
	}

	return 0;
}


static void usbd_event_handler(uint8_t busid, uint8_t event)
{
	USB_LOG_INFO("USBD Event: %d\n", event);
	switch (event) {
	case USBD_EVENT_RESET:
		ftdevs[0].timeout = 0;
		ftdevs[0].latency_timer = 16;
		ftdevs[1].timeout = 0;
		ftdevs[1].latency_timer = 16;
		cdcuart_reset(&cdc_uarts[1]);
		break;
	case USBD_EVENT_CONNECTED:
		break;
	case USBD_EVENT_DISCONNECTED:
		break;
	case USBD_EVENT_RESUME:
		break;
	case USBD_EVENT_SUSPEND:
		break;
	case USBD_EVENT_CONFIGURED:
		cdc_recv_start(0,&cdc_uarts[1]);
		cdc_send_startup(0,&cdc_uarts[1]);
		jtag_out_start(0);
		jtag_in_start();
		break;
	case USBD_EVENT_SET_REMOTE_WAKEUP:
		break;
	case USBD_EVENT_CLR_REMOTE_WAKEUP:
		break;
	default:
		break;
	}
}
// void usbd_ftdi_add_interface(usbd_class_t *class, usbd_interface_t *intf)
// {
// 	static usbd_class_t *last_class = NULL;

// 	if(last_class != class)
// 	{
// 		last_class = class;		
// 		usbd_class_register(class);
// 	}

// 	intf->class_handler = NULL;
// 	intf->custom_handler = NULL;
// 	intf->vendor_handler = ftdi_vendor_request_handler;
// 	intf->notify_handler = ftdi_notify_handler;
// 	usbd_class_add_interface(class,intf);
// }
void ftdi_notify_handler(uint8_t busid, uint8_t event, void *arg)
{
	//TODO:暂时没处理，其实应该在这里处理FTDI的复位信号
	switch (event)
	{
		case USBD_EVENT_RESET:
			// usbd_ftdi_reset();
			// ftdevs[0].timeout=0;
			break;
		case USBD_EVENT_SOF:
			// sof_tick++;
			// ftdevs[0].timeout++;
			// USBD_LOG_DBG("tick: %d\r\n", sof_tick);
		break;
		default:
			break;
	}	
}
struct usbd_interface *usbd_ftdi_init_intf(uint8_t busid, struct usbd_interface *intf)
{
    (void)busid;

    intf->class_interface_handler = NULL;
    intf->class_endpoint_handler = NULL;
    intf->vendor_handler = usbd_ftdi_vendor_handler;
    intf->notify_handler = ftdi_notify_handler;

    return intf;
}


void usb_dc_user_init(uint8_t busid, uintptr_t reg_base)
{
	ftdevs[0].timeout = 0;
	ftdevs[0].latency_timer = 16;
	ftdevs[1].timeout = 0;
	ftdevs[1].latency_timer = 16;

	jtag_init();

	// usbd_desc_register(ftlink_descriptor, string_desc, string_cnt);//旧的
#ifdef CONFIG_USBDEV_ADVANCE_DESC
    usbd_desc_register(busid, &FTlink_descriptor);
#else
    // usbd_desc_register(busid, cdc_descriptor);
#endif

	//添加自定义的interface和端点
	// usbd_add_interface(busid, &jtag_intf);
	usbd_add_interface(busid,usbd_ftdi_init_intf(0,&jtag_intf));
    usbd_add_endpoint(busid, &jtag_out_ep);
    usbd_add_endpoint(busid, &jtag_in_ep);
    


#if FTDI_DEV==FT232H
	//TODO:我记得之前可以让驱动识别FT232的同时加第二个interface的，现在会报错不匹配，先关掉吧,应该也不影响用2232开另一个端点
#elif FTDI_DEV==FT2232H||FTDI_DEV==FT4232H
	#if 0
	cdcuart_init(0,&cdc_uarts[1]);
	// int_enable(DMA1_Channel5_IRQn);
	// int_enable(USART1_IRQn); //TODO:串口先不管了
	#else
	cdcuart_init(0,&cdc_uarts[0]);
	// int_enable(DMA1_Channel3_IRQn);
	// int_enable(USART3_IRQn);
	#endif
#endif
	usbd_initialize(busid, reg_base, usbd_event_handler);
}


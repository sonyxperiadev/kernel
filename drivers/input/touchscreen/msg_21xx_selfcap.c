/*
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/i2c.h>
#include <linux/timer.h>
#include <linux/gpio.h>

#include <linux/sysfs.h>
#include <linux/init.h>
#include <linux/mutex.h>
#include <linux/spinlock.h>

#include <linux/hrtimer.h>

#include <linux/delay.h>
#include <linux/slab.h>

#include <linux/syscalls.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/fcntl.h>
#include <linux/string.h>
#include <linux/unistd.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/earlysuspend.h>
#include <linux/input.h>
*/
#include <linux/module.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/hrtimer.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/gpio.h>
#include <linux/earlysuspend.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/notifier.h>
#include <linux/wakelock.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_platform.h>
#include <linux/i2c-kona.h>
#include <linux/leds.h>
#include <linux/vmalloc.h>
#include <linux/uaccess.h>
#include <linux/fs.h>

#define VKEY_SYS
#define u8         unsigned char
#define u32        unsigned int
#define s32        signed int
#define REPORT_PACKET_LENGTH    (8)
#define __FIRMWARE_UPDATE__
#define GPIO_TO_IRQ gpio_to_irq
static struct workqueue_struct *mstar_wq;
static int dbg_isr_num;
static int dbg_isr_disp;
static int msg21xx_irq;
static struct i2c_client *msg21xx_i2c_client;
static struct work_struct msg21xx_wq;
static struct delayed_work msg21xx_firmware_update;
static struct early_suspend early_suspend;
static struct input_dev *input;

typedef struct {
	struct i2c_slave_platform_data i2c_pdata;
	struct i2c_client *client;
	int gpio_irq_pin;
	int gpio_reset_pin;
	int x_max_value;
	int y_max_value;
} msg21xx_ts_data;

static int ts_gpio_irq_pin;
static int ts_gpio_reset_pin;
static int ts_x_max_value;
static int ts_y_max_value;
static const char *vkey_scope;
static bool have_vkey;
static struct workqueue_struct *synaptics_wq;

static struct wake_lock update_wake_lock;
static u8 g_dwiic_info_data[1024];   // Buffer for info data
u32 crc_tab[256];
#define N_BYTE_PER_TIME (8)
#define UPDATE_TIMES (1024/N_BYTE_PER_TIME)


#if 1
#define TP_DEBUG_ERR(format, ...)	printk(KERN_ERR "MSG2133_MSG21XXA_updating ***" format "\n", ## __VA_ARGS__)
#else
#define TP_DEBUG_ERR(format, ...)
#endif


//#define __FIRMWARE_UPDATE_AT_BOOT__

#define MAX_TOUCH_FINGER 2

typedef struct  {
	u16 X;
	u16 Y;
} TouchPoint_t;

typedef struct  {
	u8 nTouchKeyMode;
	u8 nTouchKeyCode;
	u8 nFingerNum;
	TouchPoint_t Point[MAX_TOUCH_FINGER];
} TouchScreenInfo_t;

static BLOCKING_NOTIFIER_HEAD(touch_key_notifier);

static int register_touch_key_notifier(struct notifier_block *n)
{
	return blocking_notifier_chain_register(&touch_key_notifier, n);
}

static int unregister_touch_key_notifier(struct notifier_block *n)
{
	return blocking_notifier_chain_unregister(&touch_key_notifier, n);
}

static int g_debugTp;
static ssize_t tp_debug_show(struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
	return sprintf(buf, "%d\n", g_debugTp);
}

static ssize_t tp_debug_store(struct device *dev,
			struct device_attribute *attr, const char *buf,
				size_t size)
{
	if (buf[0] == '0')
		g_debugTp = 0;

	else if (buf[0] == '1')
		g_debugTp = 1;

	else
		g_debugTp = 2;
	return size;
}

static DEVICE_ATTR(debug, 0777, tp_debug_show, tp_debug_store);

#ifdef __FIRMWARE_UPDATE__

#define FW_ADDR_MSG21XX   (0xC4>>1)
#define FW_ADDR_MSG21XX_TP   (0x4C>>1)
#define FW_UPDATE_ADDR_MSG21XX   (0x92>>1)
#define TP_DEBUG	printk
static char *fw_version;
static u8 temp[94][1024];
static unsigned short major, minor;
static unsigned short wanted_major;
static unsigned short wanted_minor;

static u8 curr_ic_type = 0;
#define	CTP_ID_MSG21XX		1
#define	CTP_ID_MSG21XXA		2
static unsigned short curr_ic_major=0;
static unsigned short curr_ic_minor=0;

static unsigned short update_bin_major=0;
static unsigned short update_bin_minor=0;

static u8 Fmr_Loader[1024];

static unsigned char MSG21XX_update_bin[]=
{
//#include <linux/i2c/File_123456.h>
//#include <linux/i2c/PB287_tp_V0x10002.h>

};

unsigned char MSG_FIRMWARE[94*1024] = {
//#include <linux/i2c/File_5_28_18_16.h>
//#include <linux/i2c/msg_21xx_firmware_v2.04.h>

};
static int FwDataCnt;
struct class *firmware_class;
struct device *firmware_cmd_dev;
static int HalTscrCReadI2CSeq(u8 addr, u8 *read_data, u8 size)
{
	int ret;

    /*according to your platform.*/
#if 0
	int rc;
	struct i2c_msg msgs[] = {
		{.addr = addr,
		.flags = I2C_M_RD,
		.len = size,
		.buf = read_data,
		},
	};
	rc = i2c_transfer(msg21xx_i2c_client->adapter, msgs, 1);
	if (rc < 0)
		printk(KERN_ERR "HalTscrCReadI2CSeq error %d\n", rc);
	return rc;
#else
	msg21xx_i2c_client->addr = addr;
	ret = i2c_master_recv(msg21xx_i2c_client, &read_data[0], size);
	msg21xx_i2c_client->addr = FW_ADDR_MSG21XX_TP;
	return ret;
#endif
}

static int HalTscrCDevWriteI2CSeq(u8 addr, u8 *data, u16 size)
{
	int ret;
#if 0
	/*according to your platform.*/
	int rc;
	struct i2c_msg msgs[] = {
		{.addr = addr, .flags = 0, .len = size, .buf = data, },
	};
	rc = i2c_transfer(msg21xx_i2c_client->adapter, msgs, 1);
	if (rc < 0)
		printk(KERN_ERR "HalTscrCDevWriteI2CSeq error %d, addr = %d\n", rc, addr);
	return rc;
#else
	msg21xx_i2c_client->addr = addr;
	ret = i2c_master_send(msg21xx_i2c_client, &data[0], size);
	msg21xx_i2c_client->addr = FW_ADDR_MSG21XX_TP;
	return ret;
#endif

}

static void HalDisableIrq(void)
{
	disable_irq_nosync(msg21xx_irq);
}

static void HalEnableIrq(void)
{
	enable_irq(msg21xx_irq);
}

static void msg21xx_chip_init(void);

static void _HalTscrHWReset(void)
{

#if 0
	gpio_request(ts_gpio_reset_pin, "reset");
	gpio_direction_output(ts_gpio_reset_pin, 0);
	gpio_set_value(ts_gpio_reset_pin, 0);
	mdelay(10);/* Note that the RST must be in LOW 10ms at least */
	gpio_direction_input(ts_gpio_reset_pin);
	gpio_set_value(ts_gpio_reset_pin, 1);

	/* Enable the interrupt service thread/routine for INT after 50ms */
	gpio_free(ts_gpio_reset_pin);
	mdelay(300);
#else
	msg21xx_chip_init();
#endif

}

typedef enum {
	EMEM_ALL = 0,
	EMEM_MAIN,
	EMEM_INFO,
} EMEM_TYPE_t;

static void drvDB_WriteReg ( u8 bank, u8 addr, u16 data )
{
    u8 tx_data[5] = {0x10, bank, addr, data & 0xFF, data >> 8};
    HalTscrCDevWriteI2CSeq ( FW_ADDR_MSG21XX, tx_data, 5 );
}
static void drvDB_WriteReg8Bit ( u8 bank, u8 addr, u8 data )
{
    u8 tx_data[4] = {0x10, bank, addr, data};
    HalTscrCDevWriteI2CSeq ( FW_ADDR_MSG21XX, tx_data, 4 );
}


static unsigned short drvDB_ReadReg ( u8 bank, u8 addr )
{
	int ret;
	u8 tx_data[3] = {0x10, bank, addr};
	u8 rx_data[2] = {0};

	HalTscrCDevWriteI2CSeq(FW_ADDR_MSG21XX, tx_data, 3);
	HalTscrCReadI2CSeq(FW_ADDR_MSG21XX, &rx_data[0], 2);
	ret = rx_data[1] << 8 | rx_data[0];
	return ret;
}


static void dbbusDWIICEnterSerialDebugMode(void)
{
	u8 data[5];

	/* Enter the Serial Debug Mode*/
	data[0] = 0x53;
	data[1] = 0x45;
	data[2] = 0x52;
	data[3] = 0x44;
	data[4] = 0x42;
	HalTscrCDevWriteI2CSeq(FW_ADDR_MSG21XX, data, 5);
}
static void dbbusDWIICStopMCU(void)
{
	u8 data[1];
	/*Stop the MCU*/
	data[0] = 0x37;
	HalTscrCDevWriteI2CSeq(FW_ADDR_MSG21XX, data, 1);
}
static void dbbusDWIICIICUseBus(void)
{
	u8 data[1];
	/*IIC Use Bus*/
	data[0] = 0x35;
	HalTscrCDevWriteI2CSeq(FW_ADDR_MSG21XX, data, 1);
}
static void dbbusDWIICIICReshape(void)
{
	u8 data[1];
	/*IIC Re-shape*/
	data[0] = 0x71;
	HalTscrCDevWriteI2CSeq(FW_ADDR_MSG21XX, data, 1);
}
static void dbbusDWIICIICNotUseBus(void)
{
	u8 data[1];
	/*IIC Not Use Bus*/
	    data[0] = 0x34;
	HalTscrCDevWriteI2CSeq(FW_ADDR_MSG21XX, data, 1);
}
static void dbbusDWIICNotStopMCU(void)
{
	u8 data[1];
	/*Not Stop the MCU*/
	data[0] = 0x36;
	HalTscrCDevWriteI2CSeq(FW_ADDR_MSG21XX, data, 1);
}
static void dbbusDWIICExitSerialDebugMode(void)
{
	u8 data[1];
	/*Exit the Serial Debug Mode*/
	data[0] = 0x45;
	HalTscrCDevWriteI2CSeq(FW_ADDR_MSG21XX, data, 1);

}
static void drvISP_EntryIspMode(void)
{
	u8 bWriteData[5] =  {
	0x4D, 0x53, 0x54, 0x41, 0x52 };
	HalTscrCDevWriteI2CSeq(FW_UPDATE_ADDR_MSG21XX, bWriteData, 5);
}
static u8 drvISP_Read(u8 n, u8 *pDataToRead)
{	/*First it needs send 0x11 to notify we want to get flash data back.*/

	u8 Read_cmd = 0x11;
	unsigned char dbbus_rx_data[2] = { 0 };
	HalTscrCDevWriteI2CSeq(FW_UPDATE_ADDR_MSG21XX, &Read_cmd, 1);
	mdelay(1);
	if (n == 1) {
		HalTscrCReadI2CSeq(FW_UPDATE_ADDR_MSG21XX, &dbbus_rx_data[0],
				    2);
		*pDataToRead = dbbus_rx_data[1];
	} else {
		HalTscrCReadI2CSeq(FW_UPDATE_ADDR_MSG21XX, pDataToRead, n);
		}
	return 0;
}
static void drvISP_WriteEnable(void)
{
	u8 bWriteData[2] = {
	0x10, 0x06 };
	u8 bWriteData1 = 0x12;
	HalTscrCDevWriteI2CSeq(FW_UPDATE_ADDR_MSG21XX, bWriteData, 2);
	HalTscrCDevWriteI2CSeq(FW_UPDATE_ADDR_MSG21XX, &bWriteData1, 1);
}
static void drvISP_ExitIspMode(void)
{
	u8 bWriteData = 0x24;
	HalTscrCDevWriteI2CSeq(FW_UPDATE_ADDR_MSG21XX, &bWriteData, 1);
}
static u8 drvISP_ReadStatus(void)
{
	u8 bReadData = 0;
	u8 bWriteData[2] =  {
	0x10, 0x05 };
	u8 bWriteData1 = 0x12;
	HalTscrCDevWriteI2CSeq(FW_UPDATE_ADDR_MSG21XX, bWriteData, 2);
	mdelay(1);
	drvISP_Read(1, &bReadData);
	HalTscrCDevWriteI2CSeq(FW_UPDATE_ADDR_MSG21XX, &bWriteData1, 1);
	return bReadData;
}

static void drvISP_ChipErase()
{
	u8 bWriteData[5] = {
	0x00, 0x00, 0x00, 0x00, 0x00};
	u8 bWriteData1 = 0x12;
	u32 timeOutCount = 0;
	
	drvISP_WriteEnable();
	
	/*Enable write status register*/
	bWriteData[0] = 0x10;
	bWriteData[1] = 0x50;
	HalTscrCDevWriteI2CSeq(FW_UPDATE_ADDR_MSG21XX, bWriteData, 2);
	HalTscrCDevWriteI2CSeq(FW_UPDATE_ADDR_MSG21XX, &bWriteData1, 1);
	
	/*Write Status*/
	bWriteData[0] = 0x10;
	bWriteData[1] = 0x01;
	bWriteData[2] = 0x00;
	HalTscrCDevWriteI2CSeq(FW_UPDATE_ADDR_MSG21XX, bWriteData, 3);
	HalTscrCDevWriteI2CSeq(FW_UPDATE_ADDR_MSG21XX, &bWriteData1, 1);
	
	/*Write disable*/
	bWriteData[0] = 0x10;
	bWriteData[1] = 0x04;
	HalTscrCDevWriteI2CSeq(FW_UPDATE_ADDR_MSG21XX, bWriteData, 2);
	HalTscrCDevWriteI2CSeq(FW_UPDATE_ADDR_MSG21XX, &bWriteData1, 1);
	
	mdelay(1);
	timeOutCount = 0;
	while ((drvISP_ReadStatus() & 0x01) == 0x01) {
		timeOutCount++;
		if (timeOutCount > 100)
			break;	/* around 1 sec timeout */
	}
	
	drvISP_WriteEnable();
	bWriteData[0] = 0x10;
	bWriteData[1] = 0xC7;
	HalTscrCDevWriteI2CSeq(FW_UPDATE_ADDR_MSG21XX, bWriteData, 2);
	HalTscrCDevWriteI2CSeq(FW_UPDATE_ADDR_MSG21XX, &bWriteData1, 1);
	mdelay(1);
	
	timeOutCount = 0;
	while ((drvISP_ReadStatus() & 0x01) == 0x01) {
		timeOutCount++;
		if (timeOutCount > 500)
			break;	/* around 5 sec timeout */
	}
	
}

static void drvISP_Program(u16 k, u8 *pDataToWrite)
{
	u16 i = 0;
	u16 j = 0;
	u8 TX_data[133];
	u8 bWriteData1 = 0x12;
	u32 addr = k * 1024;
	u32 timeOutCount = 0;
	for (j = 0; j < 8; j++) { /*128*8 cycle*/

		TX_data[0] = 0x10;
		TX_data[1] = 0x02;	/*Page Program CMD*/
		TX_data[2] = (addr + 128 * j) >> 16;
		TX_data[3] = (addr + 128 * j) >> 8;
		TX_data[4] = (addr + 128 * j);
		for (i = 0; i < 128; i++)
			TX_data[5 + i] = pDataToWrite[j * 128 + i];
		mdelay(1);
		timeOutCount = 0;
		while ((drvISP_ReadStatus() & 0x01) == 0x01) {
			timeOutCount++;
			if (timeOutCount >= 100000)
				break;	/* around 1 sec timeout */
		}
		drvISP_WriteEnable();
		/*write 133 bytes per cycle*/
		HalTscrCDevWriteI2CSeq(FW_UPDATE_ADDR_MSG21XX, TX_data, 133);
		HalTscrCDevWriteI2CSeq(FW_UPDATE_ADDR_MSG21XX, &bWriteData1, 1);
	}
}

static void drvISP_Verify(u16 k, u8 *pDataToVerify)
{
	u16 i = 0, j = 0;
	u8 bWriteData[5] =  {
	0x10, 0x03, 0, 0, 0 };
	u8 RX_data[256];
	u8 bWriteData1 = 0x12;
	u32 addr = k * 1024;
	u8 index = 0;
	u32 timeOutCount;
	for (j = 0; j < 8; j++) { /*128*8 cycle*/

		bWriteData[2] = (u8) ((addr + j * 128) >> 16);
		bWriteData[3] = (u8) ((addr + j * 128) >> 8);
		bWriteData[4] = (u8) (addr + j * 128);
		mdelay(1);
		timeOutCount = 0;
		while ((drvISP_ReadStatus() & 0x01) == 0x01) {
			timeOutCount++;
			if (timeOutCount >= 100000)
				break;	/* around 1 sec timeout */
		}
		/*write read flash addr*/
		HalTscrCDevWriteI2CSeq(FW_UPDATE_ADDR_MSG21XX, bWriteData, 5);
		mdelay(1);
		drvISP_Read(128, RX_data);
		HalTscrCDevWriteI2CSeq(FW_UPDATE_ADDR_MSG21XX, &bWriteData1, 1);
		for (i = 0; i < 128; i++) {/*log out if verify error*/

			if ((RX_data[i] != 0) && index < 10)
				index++;

			if (RX_data[i] != pDataToVerify[128 * j + i])
				TP_DEBUG("k=%d,j=%d,i=%d==Update Firmware Error=", k, j, i);
		}
	}
}

static int drvTP_erase_emem_c32 ( void )
{
    /////////////////////////
    //Erase  all
    /////////////////////////
    
    //enter gpio mode
    drvDB_WriteReg ( 0x16, 0x1E, 0xBEAF );

    // before gpio mode, set the control pin as the orginal status
    drvDB_WriteReg ( 0x16, 0x08, 0x0000 );
    drvDB_WriteReg8Bit ( 0x16, 0x0E, 0x10 );
    mdelay ( 10 ); //MCR_CLBK_DEBUG_DELAY ( 10, MCU_LOOP_DELAY_COUNT_MS );

    // ptrim = 1, h'04[2]
    drvDB_WriteReg8Bit ( 0x16, 0x08, 0x04 );
    drvDB_WriteReg8Bit ( 0x16, 0x0E, 0x10 );
    mdelay ( 10 ); //MCR_CLBK_DEBUG_DELAY ( 10, MCU_LOOP_DELAY_COUNT_MS );

    // ptm = 6, h'04[12:14] = b'110
    drvDB_WriteReg8Bit ( 0x16, 0x09, 0x60 );
    drvDB_WriteReg8Bit ( 0x16, 0x0E, 0x10 );

    // pmasi = 1, h'04[6]
    drvDB_WriteReg8Bit ( 0x16, 0x08, 0x44 );
    // pce = 1, h'04[11]
    drvDB_WriteReg8Bit ( 0x16, 0x09, 0x68 );
    // perase = 1, h'04[7]
    drvDB_WriteReg8Bit ( 0x16, 0x08, 0xC4 );
    // pnvstr = 1, h'04[5]
    drvDB_WriteReg8Bit ( 0x16, 0x08, 0xE4 );
    // pwe = 1, h'04[9]
    drvDB_WriteReg8Bit ( 0x16, 0x09, 0x6A );
    // trigger gpio load
    drvDB_WriteReg8Bit ( 0x16, 0x0E, 0x10 );

    return ( 1 );
}

static int drvTP_erase_emem_c33 ( EMEM_TYPE_t emem_type )
{
    // stop mcu
    drvDB_WriteReg ( 0x0F, 0xE6, 0x0001 );

    //disable watch dog
    drvDB_WriteReg8Bit ( 0x3C, 0x60, 0x55 );
    drvDB_WriteReg8Bit ( 0x3C, 0x61, 0xAA );

    // set PROGRAM password
    drvDB_WriteReg8Bit ( 0x16, 0x1A, 0xBA );
    drvDB_WriteReg8Bit ( 0x16, 0x1B, 0xAB );

    //proto.MstarWriteReg(F1.loopDevice, 0x1618, 0x80);
    drvDB_WriteReg8Bit ( 0x16, 0x18, 0x80 );

    if ( emem_type == EMEM_ALL )
    {
        drvDB_WriteReg8Bit ( 0x16, 0x08, 0x10 ); //mark
    }

    drvDB_WriteReg8Bit ( 0x16, 0x18, 0x40 );
    mdelay ( 10 );

    drvDB_WriteReg8Bit ( 0x16, 0x18, 0x80 );

    // erase trigger
    if ( emem_type == EMEM_MAIN )
    {
        drvDB_WriteReg8Bit ( 0x16, 0x0E, 0x04 ); //erase main
    }
    else
    {
        drvDB_WriteReg8Bit ( 0x16, 0x0E, 0x08 ); //erase all block
    }

    return ( 1 );
}

static int drvTP_read_info_dwiic_c33 ( void )
{
    u8  dwiic_tx_data[5];
    u8  dwiic_rx_data[4];
    u16 reg_data=0;
    mdelay ( 300 );

    // Stop Watchdog
    drvDB_WriteReg8Bit ( 0x3C, 0x60, 0x55 );
    drvDB_WriteReg8Bit ( 0x3C, 0x61, 0xAA );

    drvDB_WriteReg ( 0x3C, 0xE4, 0xA4AB );

	drvDB_WriteReg ( 0x1E, 0x04, 0x7d60 );

    // TP SW reset
    drvDB_WriteReg ( 0x1E, 0x04, 0x829F );
	mdelay ( 1 );
    dwiic_tx_data[0] = 0x10;
    dwiic_tx_data[1] = 0x0F;
    dwiic_tx_data[2] = 0xE6;
    dwiic_tx_data[3] = 0x00;
    HalTscrCDevWriteI2CSeq ( FW_ADDR_MSG21XX, dwiic_tx_data, 4 );	
    mdelay ( 100 );
TP_DEBUG_ERR("2222222222");
    do{
        reg_data = drvDB_ReadReg ( 0x3C, 0xE4 );
    }
    while ( reg_data != 0x5B58 );
TP_DEBUG_ERR("33333333333333");
    dwiic_tx_data[0] = 0x72;
    dwiic_tx_data[1] = 0x80;
    dwiic_tx_data[2] = 0x00;
    dwiic_tx_data[3] = 0x04;
    dwiic_tx_data[4] = 0x00;
    HalTscrCDevWriteI2CSeq ( FW_ADDR_MSG21XX_TP , dwiic_tx_data, 5 );
TP_DEBUG_ERR("4444444444444");
    mdelay ( 50 );

    // recive info data
    //HalTscrCReadI2CSeq ( FW_ADDR_MSG21XX_TP, &g_dwiic_info_data[0], 1024 );
    HalTscrCReadI2CSeq ( FW_ADDR_MSG21XX_TP, &g_dwiic_info_data[0], 8 );
    TP_DEBUG_ERR("55555555555555");
    return ( 1 );
}

static u32 Reflect ( u32 ref, char ch ) //unsigned int Reflect(unsigned int ref, char ch)
{
    u32 value = 0;
    u32 i = 0;

    for ( i = 1; i < ( ch + 1 ); i++ )
    {
        if ( ref & 1 )
        {
            value |= 1 << ( ch - i );
        }
        ref >>= 1;
    }
    return value;
}

u32 Get_CRC ( u32 text, u32 prevCRC, u32 *crc32_table )
{
    u32  ulCRC = prevCRC;
	ulCRC = ( ulCRC >> 8 ) ^ crc32_table[ ( ulCRC & 0xFF ) ^ text];
    return ulCRC ;
}
static void Init_CRC32_Table ( u32 *crc32_table )
{
    u32 magicnumber = 0x04c11db7;
    u32 i = 0, j;

    for ( i = 0; i <= 0xFF; i++ )
    {
        crc32_table[i] = Reflect ( i, 8 ) << 24;
        for ( j = 0; j < 8; j++ )
        {
            crc32_table[i] = ( crc32_table[i] << 1 ) ^ ( crc32_table[i] & ( 0x80000000L ) ? magicnumber : 0 );
        }
        crc32_table[i] = Reflect ( crc32_table[i], 32 );
    }
}



/* update the firmware part, used by apk*/
/*show the fw version*/

static ssize_t firmware_update_c2 ( struct device *dev,
                                    struct device_attribute *attr, const char *buf, size_t size )
{
    u8 i;
    u8 dbbus_tx_data[4];
    unsigned char dbbus_rx_data[2] = {0};

    // set FRO to 50M
    dbbus_tx_data[0] = 0x10;
    dbbus_tx_data[1] = 0x11;
    dbbus_tx_data[2] = 0xE2;
    HalTscrCDevWriteI2CSeq ( FW_ADDR_MSG21XX, dbbus_tx_data, 3 );
    dbbus_rx_data[0] = 0;
    dbbus_rx_data[1] = 0;
    HalTscrCReadI2CSeq ( FW_ADDR_MSG21XX, &dbbus_rx_data[0], 2 );
    TP_DEBUG ( "dbbus_rx_data[0]=0x%x", dbbus_rx_data[0] );
    dbbus_tx_data[3] = dbbus_rx_data[0] & 0xF7;  //Clear Bit 3
    HalTscrCDevWriteI2CSeq ( FW_ADDR_MSG21XX, dbbus_tx_data, 4 );

    // set MCU clock,SPI clock =FRO
    dbbus_tx_data[0] = 0x10;
    dbbus_tx_data[1] = 0x1E;
    dbbus_tx_data[2] = 0x22;
    dbbus_tx_data[3] = 0x00;
    HalTscrCDevWriteI2CSeq ( FW_ADDR_MSG21XX, dbbus_tx_data, 4 );

    dbbus_tx_data[0] = 0x10;
    dbbus_tx_data[1] = 0x1E;
    dbbus_tx_data[2] = 0x23;
    dbbus_tx_data[3] = 0x00;
    HalTscrCDevWriteI2CSeq ( FW_ADDR_MSG21XX, dbbus_tx_data, 4 );

    // Enable slave's ISP ECO mode
    dbbus_tx_data[0] = 0x10;
    dbbus_tx_data[1] = 0x08;
    dbbus_tx_data[2] = 0x0c;
    dbbus_tx_data[3] = 0x08;
    HalTscrCDevWriteI2CSeq ( FW_ADDR_MSG21XX, dbbus_tx_data, 4 );

    // Enable SPI Pad
    dbbus_tx_data[0] = 0x10;
    dbbus_tx_data[1] = 0x1E;
    dbbus_tx_data[2] = 0x02;
    HalTscrCDevWriteI2CSeq ( FW_ADDR_MSG21XX, dbbus_tx_data, 3 );
    HalTscrCReadI2CSeq ( FW_ADDR_MSG21XX, &dbbus_rx_data[0], 2 );
    TP_DEBUG ( "dbbus_rx_data[0]=0x%x", dbbus_rx_data[0] );
    dbbus_tx_data[3] = ( dbbus_rx_data[0] | 0x20 ); //Set Bit 5
    HalTscrCDevWriteI2CSeq ( FW_ADDR_MSG21XX, dbbus_tx_data, 4 );

    // WP overwrite
    dbbus_tx_data[0] = 0x10;
    dbbus_tx_data[1] = 0x1E;
    dbbus_tx_data[2] = 0x0E;
    dbbus_tx_data[3] = 0x02;
    HalTscrCDevWriteI2CSeq ( FW_ADDR_MSG21XX, dbbus_tx_data, 4 );

    // set pin high
    dbbus_tx_data[0] = 0x10;
    dbbus_tx_data[1] = 0x1E;
    dbbus_tx_data[2] = 0x10;
    dbbus_tx_data[3] = 0x08;
    HalTscrCDevWriteI2CSeq ( FW_ADDR_MSG21XX, dbbus_tx_data, 4 );

    dbbusDWIICIICNotUseBus();
    dbbusDWIICNotStopMCU();
    dbbusDWIICExitSerialDebugMode();

    drvISP_EntryIspMode();
    drvISP_ChipErase();
    _HalTscrHWReset();
    mdelay ( 300 );

    // Program and Verify
    dbbusDWIICEnterSerialDebugMode();
    dbbusDWIICStopMCU();
    dbbusDWIICIICUseBus();
    dbbusDWIICIICReshape();

    // Disable the Watchdog
    dbbus_tx_data[0] = 0x10;
    dbbus_tx_data[1] = 0x3C;
    dbbus_tx_data[2] = 0x60;
    dbbus_tx_data[3] = 0x55;
    HalTscrCDevWriteI2CSeq ( FW_ADDR_MSG21XX, dbbus_tx_data, 4 );
    dbbus_tx_data[0] = 0x10;
    dbbus_tx_data[1] = 0x3C;
    dbbus_tx_data[2] = 0x61;
    dbbus_tx_data[3] = 0xAA;
    HalTscrCDevWriteI2CSeq ( FW_ADDR_MSG21XX, dbbus_tx_data, 4 );

    //Stop MCU
    dbbus_tx_data[0] = 0x10;
    dbbus_tx_data[1] = 0x0F;
    dbbus_tx_data[2] = 0xE6;
    dbbus_tx_data[3] = 0x01;
    HalTscrCDevWriteI2CSeq ( FW_ADDR_MSG21XX, dbbus_tx_data, 4 );

    // set FRO to 50M
    dbbus_tx_data[0] = 0x10;
    dbbus_tx_data[1] = 0x11;
    dbbus_tx_data[2] = 0xE2;
    HalTscrCDevWriteI2CSeq ( FW_ADDR_MSG21XX, dbbus_tx_data, 3 );
    dbbus_rx_data[0] = 0;
    dbbus_rx_data[1] = 0;
    HalTscrCReadI2CSeq ( FW_ADDR_MSG21XX, &dbbus_rx_data[0], 2 );
    TP_DEBUG ( "dbbus_rx_data[0]=0x%x", dbbus_rx_data[0] );
    dbbus_tx_data[3] = dbbus_rx_data[0] & 0xF7;  //Clear Bit 3
    HalTscrCDevWriteI2CSeq ( FW_ADDR_MSG21XX, dbbus_tx_data, 4 );

    // set MCU clock,SPI clock =FRO
    dbbus_tx_data[0] = 0x10;
    dbbus_tx_data[1] = 0x1E;
    dbbus_tx_data[2] = 0x22;
    dbbus_tx_data[3] = 0x00;
    HalTscrCDevWriteI2CSeq ( FW_ADDR_MSG21XX, dbbus_tx_data, 4 );

    dbbus_tx_data[0] = 0x10;
    dbbus_tx_data[1] = 0x1E;
    dbbus_tx_data[2] = 0x23;
    dbbus_tx_data[3] = 0x00;
    HalTscrCDevWriteI2CSeq ( FW_ADDR_MSG21XX, dbbus_tx_data, 4 );

    // Enable slave's ISP ECO mode
    dbbus_tx_data[0] = 0x10;
    dbbus_tx_data[1] = 0x08;
    dbbus_tx_data[2] = 0x0c;
    dbbus_tx_data[3] = 0x08;
    HalTscrCDevWriteI2CSeq ( FW_ADDR_MSG21XX, dbbus_tx_data, 4 );

    // Enable SPI Pad
    dbbus_tx_data[0] = 0x10;
    dbbus_tx_data[1] = 0x1E;
    dbbus_tx_data[2] = 0x02;
    HalTscrCDevWriteI2CSeq ( FW_ADDR_MSG21XX, dbbus_tx_data, 3 );
    HalTscrCReadI2CSeq ( FW_ADDR_MSG21XX, &dbbus_rx_data[0], 2 );
    TP_DEBUG ( "dbbus_rx_data[0]=0x%x", dbbus_rx_data[0] );
    dbbus_tx_data[3] = ( dbbus_rx_data[0] | 0x20 ); //Set Bit 5
    HalTscrCDevWriteI2CSeq ( FW_ADDR_MSG21XX, dbbus_tx_data, 4 );

    // WP overwrite
    dbbus_tx_data[0] = 0x10;
    dbbus_tx_data[1] = 0x1E;
    dbbus_tx_data[2] = 0x0E;
    dbbus_tx_data[3] = 0x02;
    HalTscrCDevWriteI2CSeq ( FW_ADDR_MSG21XX, dbbus_tx_data, 4 );

    // set pin high
    dbbus_tx_data[0] = 0x10;
    dbbus_tx_data[1] = 0x1E;
    dbbus_tx_data[2] = 0x10;
    dbbus_tx_data[3] = 0x08;
    HalTscrCDevWriteI2CSeq ( FW_ADDR_MSG21XX, dbbus_tx_data, 4 );

    dbbusDWIICIICNotUseBus();
    dbbusDWIICNotStopMCU();
    dbbusDWIICExitSerialDebugMode();

    ///////////////////////////////////////
    // Start to load firmware
    ///////////////////////////////////////
    drvISP_EntryIspMode();

    for ( i = 0; i < 94; i++ ) // total  94 KB : 1 byte per R/W
    {
        drvISP_Program ( i, temp[i] ); // program to slave's flash
        drvISP_Verify ( i, temp[i] ); //verify data
    }
    TP_DEBUG_ERR ( "update_C2 OK\n" );
    drvISP_ExitIspMode();
    _HalTscrHWReset();
    FwDataCnt = 0;
    HalEnableIrq();	
    return size;
}

static ssize_t firmware_update_c32 ( struct device *dev, struct device_attribute *attr,
                                     const char *buf, size_t size,  EMEM_TYPE_t emem_type )
{
    u8  dbbus_tx_data[4];
    u8  dbbus_rx_data[2] = {0};
      // Buffer for slave's firmware

    u32 i, j;
    u32 crc_main, crc_main_tp;
    u32 crc_info, crc_info_tp;
    u16 reg_data = 0;

    crc_main = 0xffffffff;
    crc_info = 0xffffffff;

#if 1
    /////////////////////////
    // Erase  all
    /////////////////////////
    drvTP_erase_emem_c32();
    mdelay ( 1000 ); //MCR_CLBK_DEBUG_DELAY ( 1000, MCU_LOOP_DELAY_COUNT_MS );

    //ResetSlave();
    _HalTscrHWReset();
    //drvDB_EnterDBBUS();
    dbbusDWIICEnterSerialDebugMode();
    dbbusDWIICStopMCU();
    dbbusDWIICIICUseBus();
    dbbusDWIICIICReshape();
    mdelay ( 300 );

    // Reset Watchdog
    drvDB_WriteReg8Bit ( 0x3C, 0x60, 0x55 );
    drvDB_WriteReg8Bit ( 0x3C, 0x61, 0xAA );

    /////////////////////////
    // Program
    /////////////////////////

    //polling 0x3CE4 is 0x1C70
    do
    {
        reg_data = drvDB_ReadReg ( 0x3C, 0xE4 );
    }
    while ( reg_data != 0x1C70 );


    drvDB_WriteReg ( 0x3C, 0xE4, 0xE38F );  // for all-blocks

    //polling 0x3CE4 is 0x2F43
    do
    {
        reg_data = drvDB_ReadReg ( 0x3C, 0xE4 );
    }
    while ( reg_data != 0x2F43 );


    //calculate CRC 32
    Init_CRC32_Table ( &crc_tab[0] );

    for ( i = 0; i < 33; i++ ) // total  33 KB : 2 byte per R/W
    {
        if ( i < 32 )   //emem_main
        {
            if ( i == 31 )
            {
                temp[i][1014] = 0x5A; //Fmr_Loader[1014]=0x5A;
                temp[i][1015] = 0xA5; //Fmr_Loader[1015]=0xA5;

                for ( j = 0; j < 1016; j++ )
                {
                    //crc_main=Get_CRC(Fmr_Loader[j],crc_main,&crc_tab[0]);
                    crc_main = Get_CRC ( temp[i][j], crc_main, &crc_tab[0] );
                }
            }
            else
            {
                for ( j = 0; j < 1024; j++ )
                {
                    //crc_main=Get_CRC(Fmr_Loader[j],crc_main,&crc_tab[0]);
                    crc_main = Get_CRC ( temp[i][j], crc_main, &crc_tab[0] );
                }
            }
        }
        else  // emem_info
        {
            for ( j = 0; j < 1024; j++ )
            {
                //crc_info=Get_CRC(Fmr_Loader[j],crc_info,&crc_tab[0]);
                crc_info = Get_CRC ( temp[i][j], crc_info, &crc_tab[0] );
            }
        }

        //drvDWIIC_MasterTransmit( DWIIC_MODE_DWIIC_ID, 1024, Fmr_Loader );
        HalTscrCDevWriteI2CSeq ( FW_ADDR_MSG21XX_TP, temp[i], 1024 );

        // polling 0x3CE4 is 0xD0BC
        do
        {
            reg_data = drvDB_ReadReg ( 0x3C, 0xE4 );
        }
        while ( reg_data != 0xD0BC );

        drvDB_WriteReg ( 0x3C, 0xE4, 0x2F43 );
    }

    //write file done
    drvDB_WriteReg ( 0x3C, 0xE4, 0x1380 );

    mdelay ( 10 ); //MCR_CLBK_DEBUG_DELAY ( 10, MCU_LOOP_DELAY_COUNT_MS );
    // polling 0x3CE4 is 0x9432
    do
    {
        reg_data = drvDB_ReadReg ( 0x3C, 0xE4 );
    }
    while ( reg_data != 0x9432 );

    crc_main = crc_main ^ 0xffffffff;
    crc_info = crc_info ^ 0xffffffff;

    // CRC Main from TP
    crc_main_tp = drvDB_ReadReg ( 0x3C, 0x80 );
    crc_main_tp = ( crc_main_tp << 16 ) | drvDB_ReadReg ( 0x3C, 0x82 );
 
    //CRC Info from TP
    crc_info_tp = drvDB_ReadReg ( 0x3C, 0xA0 );
    crc_info_tp = ( crc_info_tp << 16 ) | drvDB_ReadReg ( 0x3C, 0xA2 );

    TP_DEBUG ( "crc_main=0x%x, crc_info=0x%x, crc_main_tp=0x%x, crc_info_tp=0x%x\n",
               crc_main, crc_info, crc_main_tp, crc_info_tp );

    //drvDB_ExitDBBUS();
    if ( ( crc_main_tp != crc_main ) || ( crc_info_tp != crc_info ) )
    {
        TP_DEBUG_ERR ( "update_C32 FAILED\n" );
		_HalTscrHWReset();
        FwDataCnt = 0;
    	HalEnableIrq();		
        return ( 0 );
    }

    TP_DEBUG_ERR ( "update_C32 OK\n" );
	_HalTscrHWReset();
    FwDataCnt = 0;
	HalEnableIrq();	

    return size;
#endif
}

static ssize_t firmware_update_c33 ( struct device *dev, struct device_attribute *attr,
                                     const char *buf, size_t size, EMEM_TYPE_t emem_type )
{
    u8  dbbus_tx_data[4];
    u8  dbbus_rx_data[2] = {0};
    u8  life_counter[2];
    u32 i, j;
    u32 crc_main, crc_main_tp;
    u32 crc_info, crc_info_tp;
  
    int update_pass = 1;
    u16 reg_data = 0;

    crc_main = 0xffffffff;
    crc_info = 0xffffffff;
    TP_DEBUG_ERR("111111111111");
	
    drvTP_read_info_dwiic_c33();
	
    if (1)
    {
        // updata FW Version
        //drvTP_info_updata_C33 ( 8, &temp[32][8], 5 );

		g_dwiic_info_data[8]=temp[32][8];
		g_dwiic_info_data[9]=temp[32][9];
		g_dwiic_info_data[10]=temp[32][10];
		g_dwiic_info_data[11]=temp[32][11];
        // updata life counter
        life_counter[1] = (( ( (g_dwiic_info_data[13] << 8 ) | g_dwiic_info_data[12]) + 1 ) >> 8 ) & 0xFF;
        life_counter[0] = ( ( (g_dwiic_info_data[13] << 8 ) | g_dwiic_info_data[12]) + 1 ) & 0xFF;
		g_dwiic_info_data[12]=life_counter[0];
		g_dwiic_info_data[13]=life_counter[1];
        //drvTP_info_updata_C33 ( 10, &life_counter[0], 3 );
        drvDB_WriteReg ( 0x3C, 0xE4, 0x78C5 );
		drvDB_WriteReg ( 0x1E, 0x04, 0x7d60 );
        // TP SW reset
        drvDB_WriteReg ( 0x1E, 0x04, 0x829F );

        mdelay ( 50 );
        TP_DEBUG_ERR("666666666666");
		
        //polling 0x3CE4 is 0x2F43
        do
        {
            reg_data = drvDB_ReadReg ( 0x3C, 0xE4 );
        }
        while ( reg_data != 0x2F43 );
		
        TP_DEBUG_ERR("777777777777");
        // transmit lk info data
        HalTscrCDevWriteI2CSeq ( FW_ADDR_MSG21XX_TP , &g_dwiic_info_data[0], 1024 );
        TP_DEBUG_ERR("88888888888");
        //polling 0x3CE4 is 0xD0BC
        do
        {
            reg_data = drvDB_ReadReg ( 0x3C, 0xE4 );
        }
        while ( reg_data != 0xD0BC );
		
        TP_DEBUG_ERR("9999999999999");
    }

    //erase main
    TP_DEBUG_ERR("aaaaaaaaaaa");
    drvTP_erase_emem_c33 ( EMEM_MAIN );
    mdelay ( 1000 );

    //ResetSlave();
    _HalTscrHWReset();

    //drvDB_EnterDBBUS();
    dbbusDWIICEnterSerialDebugMode();
    dbbusDWIICStopMCU();
    dbbusDWIICIICUseBus();
    dbbusDWIICIICReshape();
    mdelay ( 300 );

    /////////////////////////
    // Program
    /////////////////////////

    //polling 0x3CE4 is 0x1C70
    if ( ( emem_type == EMEM_ALL ) || ( emem_type == EMEM_MAIN ) )
    {
        do
        {
            reg_data = drvDB_ReadReg ( 0x3C, 0xE4 );
        }
        while ( reg_data != 0x1C70 );
    }

    switch ( emem_type )
    {
        case EMEM_ALL:
            drvDB_WriteReg ( 0x3C, 0xE4, 0xE38F );  // for all-blocks
            break;
        case EMEM_MAIN:
            drvDB_WriteReg ( 0x3C, 0xE4, 0x7731 );  // for main block
            break;
        case EMEM_INFO:
            drvDB_WriteReg ( 0x3C, 0xE4, 0x7731 );  // for info block

            drvDB_WriteReg8Bit ( 0x0F, 0xE6, 0x01 );

            drvDB_WriteReg8Bit ( 0x3C, 0xE4, 0xC5 ); //
            drvDB_WriteReg8Bit ( 0x3C, 0xE5, 0x78 ); //

            drvDB_WriteReg8Bit ( 0x1E, 0x04, 0x9F );
            drvDB_WriteReg8Bit ( 0x1E, 0x05, 0x82 );

            drvDB_WriteReg8Bit ( 0x0F, 0xE6, 0x00 );
            mdelay ( 100 );
            break;
    }
	
TP_DEBUG_ERR("bbbbbbbbbbbbbb");
    // polling 0x3CE4 is 0x2F43
    do
    {
        reg_data = drvDB_ReadReg ( 0x3C, 0xE4 );
    }
    while ( reg_data != 0x2F43 );
TP_DEBUG_ERR("ccccccccccccc");

    // calculate CRC 32
    Init_CRC32_Table ( &crc_tab[0] );

    for ( i = 0; i < 33; i++ ) // total  33 KB : 2 byte per R/W
    {
    
        if ( emem_type == EMEM_INFO )
			i = 32;

        if ( i < 32 )   //emem_main
        {
            if ( i == 31 )
            {
                temp[i][1014] = 0x5A; //Fmr_Loader[1014]=0x5A;
                temp[i][1015] = 0xA5; //Fmr_Loader[1015]=0xA5;

                for ( j = 0; j < 1016; j++ )
                {
                    //crc_main=Get_CRC(Fmr_Loader[j],crc_main,&crc_tab[0]);
                    crc_main = Get_CRC ( temp[i][j], crc_main, &crc_tab[0] );
                }
            }
            else
            {
                for ( j = 0; j < 1024; j++ )
                {
                    //crc_main=Get_CRC(Fmr_Loader[j],crc_main,&crc_tab[0]);
                    crc_main = Get_CRC ( temp[i][j], crc_main, &crc_tab[0] );
                }
            }
        }
        else  //emem_info
        {
            for ( j = 0; j < 1024; j++ )
            {
                //crc_info=Get_CRC(Fmr_Loader[j],crc_info,&crc_tab[0]);
                crc_info = Get_CRC ( g_dwiic_info_data[j], crc_info, &crc_tab[0] );
            }
            if ( emem_type == EMEM_MAIN ) break;
        }
        //drvDWIIC_MasterTransmit( DWIIC_MODE_DWIIC_ID, 1024, Fmr_Loader );
        TP_DEBUG_ERR("dddddddddddddd");
        #if 1
        {
            u32 n = 0;
            for(n=0;n<UPDATE_TIMES;n++)
            {
               // TP_DEBUG_ERR("i=%d,n=%d",i,n);
                HalTscrCDevWriteI2CSeq ( FW_ADDR_MSG21XX_TP, temp[i]+n*N_BYTE_PER_TIME, N_BYTE_PER_TIME );
            }
        }
        #else
        HalTscrCDevWriteI2CSeq ( FW_ADDR_MSG21XX_TP, temp[i], 1024 );
        #endif
        TP_DEBUG_ERR("eeeeeeeeeeee");
        // polling 0x3CE4 is 0xD0BC
        do
        {
            reg_data = drvDB_ReadReg ( 0x3C, 0xE4 );
        }
        while ( reg_data != 0xD0BC );
        TP_DEBUG_ERR("ffffffffffffff");
        drvDB_WriteReg ( 0x3C, 0xE4, 0x2F43 );
    }
        TP_DEBUG_ERR("ggggggggg");
    if ( ( emem_type == EMEM_ALL ) || ( emem_type == EMEM_MAIN ) )
    {
        // write file done and check crc
        drvDB_WriteReg ( 0x3C, 0xE4, 0x1380 );
        TP_DEBUG_ERR("hhhhhhhhhhhhhh");
    }
    mdelay ( 10 ); //MCR_CLBK_DEBUG_DELAY ( 10, MCU_LOOP_DELAY_COUNT_MS );

    if ( ( emem_type == EMEM_ALL ) || ( emem_type == EMEM_MAIN ) )
    {
        // polling 0x3CE4 is 0x9432
        TP_DEBUG_ERR("iiiiiiiiii");
        do
        {
            reg_data = drvDB_ReadReg ( 0x3C, 0xE4 );
        }while ( reg_data != 0x9432 );
        TP_DEBUG_ERR("jjjjjjjjjjjjj");
    }

    crc_main = crc_main ^ 0xffffffff;
    crc_info = crc_info ^ 0xffffffff;

    if ( ( emem_type == EMEM_ALL ) || ( emem_type == EMEM_MAIN ) )
    {
        TP_DEBUG_ERR("kkkkkkkkkkk");
        // CRC Main from TP
        crc_main_tp = drvDB_ReadReg ( 0x3C, 0x80 );
        crc_main_tp = ( crc_main_tp << 16 ) | drvDB_ReadReg ( 0x3C, 0x82 );

        // CRC Info from TP
        crc_info_tp = drvDB_ReadReg ( 0x3C, 0xA0 );
        crc_info_tp = ( crc_info_tp << 16 ) | drvDB_ReadReg ( 0x3C, 0xA2 );
    }
    TP_DEBUG ( "crc_main=0x%x, crc_info=0x%x, crc_main_tp=0x%x, crc_info_tp=0x%x\n",
               crc_main, crc_info, crc_main_tp, crc_info_tp );

    //drvDB_ExitDBBUS();
    TP_DEBUG_ERR("lllllllllllll");
    update_pass = 1;
	if ( ( emem_type == EMEM_ALL ) || ( emem_type == EMEM_MAIN ) )
    {
        if ( crc_main_tp != crc_main )
            update_pass = 0;

        if ( crc_info_tp != crc_info )
            update_pass = 0;
    }

    if ( !update_pass )
    {
        TP_DEBUG_ERR ( "update_C33 ok111\n" );
		_HalTscrHWReset();
        FwDataCnt = 0;
    	HalEnableIrq();	
        return size;
    }

    TP_DEBUG_ERR ( "update_C33 OK\n" );
	_HalTscrHWReset();
    FwDataCnt = 0;
    HalEnableIrq();	
    return size;
}

static ssize_t firmware_update_show(struct device *dev,
				      struct device_attribute *attr,
				      char *buf)
{
	return sprintf(buf, "%s\n", fw_version);
}

static ssize_t firmware_update_store(struct device *dev,
					 struct device_attribute *attr,
					 const char *buf, size_t size)
{
   u8 i;
   u8 dbbus_tx_data[4];
   unsigned char dbbus_rx_data[2] = {0};
   HalDisableIrq();
   _HalTscrHWReset();

   // Erase TP Flash first
   dbbusDWIICEnterSerialDebugMode();
   dbbusDWIICStopMCU();
   dbbusDWIICIICUseBus();
   dbbusDWIICIICReshape();
   mdelay ( 300 );

   // Disable the Watchdog
   dbbus_tx_data[0] = 0x10;
   dbbus_tx_data[1] = 0x3C;
   dbbus_tx_data[2] = 0x60;
   dbbus_tx_data[3] = 0x55;
   HalTscrCDevWriteI2CSeq ( FW_ADDR_MSG21XX, dbbus_tx_data, 4 );
   dbbus_tx_data[0] = 0x10;
   dbbus_tx_data[1] = 0x3C;
   dbbus_tx_data[2] = 0x61;
   dbbus_tx_data[3] = 0xAA;
   HalTscrCDevWriteI2CSeq ( FW_ADDR_MSG21XX, dbbus_tx_data, 4 );
   // Stop MCU
   dbbus_tx_data[0] = 0x10;
   dbbus_tx_data[1] = 0x0F;
   dbbus_tx_data[2] = 0xE6;
   dbbus_tx_data[3] = 0x01;
   HalTscrCDevWriteI2CSeq ( FW_ADDR_MSG21XX, dbbus_tx_data, 4 );
   
   /////////////////////////
   // Difference between C2 and C3
   /////////////////////////
   // c2:2133 c32:2133a(2) c33:2138
   //check id
   dbbus_tx_data[0] = 0x10;
   dbbus_tx_data[1] = 0x1E;
   dbbus_tx_data[2] = 0xCC;
   HalTscrCDevWriteI2CSeq ( FW_ADDR_MSG21XX, dbbus_tx_data, 3 );
   HalTscrCReadI2CSeq ( FW_ADDR_MSG21XX, &dbbus_rx_data[0], 2 );
   TP_DEBUG_ERR ( "111dbbus_rx version[0]=0x%x", dbbus_rx_data[0] );
   
   if ( dbbus_rx_data[0] == 2 )
   {
	   // check version
	   
	   dbbus_tx_data[0] = 0x10;
	   dbbus_tx_data[1] = 0x3C;
	   dbbus_tx_data[2] = 0xEA;
	   HalTscrCDevWriteI2CSeq ( FW_ADDR_MSG21XX, dbbus_tx_data, 3 );
	   HalTscrCReadI2CSeq ( FW_ADDR_MSG21XX, &dbbus_rx_data[0], 2 );
	   TP_DEBUG_ERR ( "dbbus_rx version[0]=0x%x", dbbus_rx_data[0] );

	   if ( dbbus_rx_data[0] == 3 ){
	   	
		   return firmware_update_c33 ( dev, attr, buf, size, EMEM_MAIN );
	   }
	   else{

		   return firmware_update_c32 ( dev, attr, buf, size, EMEM_ALL );
	   }
   }
   else
   {
   
	   return firmware_update_c2 ( dev, attr, buf, size );
   } 

}

static DEVICE_ATTR(update, 0777, firmware_update_show, firmware_update_store);

/*test=================*/
static ssize_t firmware_clear_show(struct device *dev,
				   struct device_attribute *attr, char *buf)
{
	u16 k = 0, i = 0, j = 0;
	u8 bWriteData[5] = {
	0x10, 0x03, 0, 0, 0 };
	u8 RX_data[256];
	u8 bWriteData1 = 0x12;
	u32 addr = 0;
	u32 timeOutCount = 0;
	for (k = 0; k < 94; i++) { /*total  94 KB : 1 byte per R/W*/
		addr = k * 1024;
		for (j = 0; j < 8; j++) {
			bWriteData[2] = (u8) ((addr + j * 128) >> 16);
			bWriteData[3] = (u8) ((addr + j * 128) >> 8);
			bWriteData[4] = (u8) (addr + j * 128);
			mdelay(1);
			timeOutCount = 0;
			while ((drvISP_ReadStatus() & 0x01) == 0x01) {
				timeOutCount++;
				if (timeOutCount >= 100000)
					break;	/* around 1 sec timeout */
			}
			HalTscrCDevWriteI2CSeq(FW_UPDATE_ADDR_MSG21XX,
						bWriteData, 5);
			mdelay(1);
			drvISP_Read(128, RX_data);
			HalTscrCDevWriteI2CSeq(FW_UPDATE_ADDR_MSG21XX,
						&bWriteData1, 1);
			for (i = 0; i < 128; i++) { /*log out if verify error*/

				if (RX_data[i] != 0xFF)	{
					TP_DEBUG
					    ("k=%d,j=%d,i=%d==erase not clean==",
						k, j, i);
				}
			}
		}
	}
	TP_DEBUG("read finish\n");
	return sprintf(buf, "%s\n", fw_version);
}

static ssize_t firmware_clear_store(struct device *dev,
				      struct device_attribute *attr,
				      const char *buf, size_t size)
{
	u8 dbbus_tx_data[4];
	unsigned char dbbus_rx_data[2] = { 0 };
	_HalTscrHWReset();
	dbbusDWIICEnterSerialDebugMode();
	dbbusDWIICStopMCU();
	dbbusDWIICIICUseBus();
	dbbusDWIICIICReshape();

	/*Disable the Watchdog*/
	dbbus_tx_data[0] = 0x10;
	dbbus_tx_data[1] = 0x3C;
	dbbus_tx_data[2] = 0x60;
	dbbus_tx_data[3] = 0x55;
	HalTscrCDevWriteI2CSeq(FW_ADDR_MSG21XX, dbbus_tx_data, 4);
	dbbus_tx_data[0] = 0x10;
	dbbus_tx_data[1] = 0x3C;
	dbbus_tx_data[2] = 0x61;
	dbbus_tx_data[3] = 0xAA;
	HalTscrCDevWriteI2CSeq(FW_ADDR_MSG21XX, dbbus_tx_data, 4);

	/*Stop MCU*/
	dbbus_tx_data[0] = 0x10;
	dbbus_tx_data[1] = 0x0F;
	dbbus_tx_data[2] = 0xE6;
	dbbus_tx_data[3] = 0x01;
	HalTscrCDevWriteI2CSeq(FW_ADDR_MSG21XX, dbbus_tx_data, 4);

	/*set FRO to 50M*/
	dbbus_tx_data[0] = 0x10;
	dbbus_tx_data[1] = 0x11;
	dbbus_tx_data[2] = 0xE2;
	HalTscrCDevWriteI2CSeq(FW_ADDR_MSG21XX, dbbus_tx_data, 3);
	dbbus_rx_data[0] = 0;
	dbbus_rx_data[1] = 0;
	HalTscrCReadI2CSeq(FW_ADDR_MSG21XX, &dbbus_rx_data[0], 2);
	TP_DEBUG("dbbus_rx_data[0]=0x%x", dbbus_rx_data[0]);
	dbbus_tx_data[3] = dbbus_rx_data[0] & 0xF7;	/*Clear Bit 3*/
	HalTscrCDevWriteI2CSeq(FW_ADDR_MSG21XX, dbbus_tx_data, 4);

	/*set MCU clock,SPI clock =FRO*/
	dbbus_tx_data[0] = 0x10;
	dbbus_tx_data[1] = 0x1E;
	dbbus_tx_data[2] = 0x22;
	dbbus_tx_data[3] = 0x00;
	HalTscrCDevWriteI2CSeq(FW_ADDR_MSG21XX, dbbus_tx_data, 4);
	dbbus_tx_data[0] = 0x10;
	dbbus_tx_data[1] = 0x1E;
	dbbus_tx_data[2] = 0x23;
	dbbus_tx_data[3] = 0x00;
	HalTscrCDevWriteI2CSeq(FW_ADDR_MSG21XX, dbbus_tx_data, 4);

	/*Enable slave's ISP ECO mode*/
	dbbus_tx_data[0] = 0x10;
	dbbus_tx_data[1] = 0x08;
	dbbus_tx_data[2] = 0x0c;
	dbbus_tx_data[3] = 0x08;
	HalTscrCDevWriteI2CSeq(FW_ADDR_MSG21XX, dbbus_tx_data, 4);

	/*Enable SPI Pad*/
	dbbus_tx_data[0] = 0x10;
	dbbus_tx_data[1] = 0x1E;
	dbbus_tx_data[2] = 0x02;
	HalTscrCDevWriteI2CSeq(FW_ADDR_MSG21XX, dbbus_tx_data, 3);
	HalTscrCReadI2CSeq(FW_ADDR_MSG21XX, &dbbus_rx_data[0], 2);
	TP_DEBUG("dbbus_rx_data[0]=0x%x", dbbus_rx_data[0]);
	dbbus_tx_data[3] = (dbbus_rx_data[0] | 0x20);	/*Set Bit 5*/
	HalTscrCDevWriteI2CSeq(FW_ADDR_MSG21XX, dbbus_tx_data, 4);

	/*WP overwrite*/
	dbbus_tx_data[0] = 0x10;
	dbbus_tx_data[1] = 0x1E;
	dbbus_tx_data[2] = 0x0E;
	dbbus_tx_data[3] = 0x02;
	HalTscrCDevWriteI2CSeq(FW_ADDR_MSG21XX, dbbus_tx_data, 4);

	/*set pin high*/
	dbbus_tx_data[0] = 0x10;
	dbbus_tx_data[1] = 0x1E;
	dbbus_tx_data[2] = 0x10;
	dbbus_tx_data[3] = 0x08;
	HalTscrCDevWriteI2CSeq(FW_ADDR_MSG21XX, dbbus_tx_data, 4);
	dbbusDWIICIICNotUseBus();
	dbbusDWIICNotStopMCU();
	dbbusDWIICExitSerialDebugMode();

	/*Start to load firmware*/
	drvISP_EntryIspMode();
	TP_DEBUG("chip erase+\n");
	drvISP_ChipErase();
	TP_DEBUG("chip erase-\n");
	drvISP_ExitIspMode();
	return size;
}

static DEVICE_ATTR(clear, 0777, firmware_clear_show, firmware_clear_store);

/*Add by Tracy.Lin for update touch panel firmware and get fw version*/

static ssize_t firmware_version_show(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	TP_DEBUG("*** firmware_version_show fw_version = %s***\n", fw_version);
	return sprintf(buf, "%s\n", fw_version);
}

static ssize_t firmware_version_store(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t size)
{
	unsigned char dbbus_tx_data[3] = {0};
	unsigned char dbbus_rx_data[4] = {0};
	unsigned short major = 0, minor = 0;
	fw_version = kzalloc(sizeof(char), GFP_KERNEL);

	/*SM-BUS GET FW VERSION*/
	dbbus_tx_data[0] = 0x53;
	dbbus_tx_data[1] = 0x00;
	dbbus_tx_data[2] = 0x2a;
	HalTscrCDevWriteI2CSeq(FW_ADDR_MSG21XX_TP, &dbbus_tx_data[0], 3);
	HalTscrCReadI2CSeq(FW_ADDR_MSG21XX_TP, &dbbus_rx_data[0], 4);
	major = 0;
	minor = 0;
	major = (dbbus_rx_data[1] << 8) + dbbus_rx_data[0];
	minor = (dbbus_rx_data[3] << 8) + dbbus_rx_data[2];
	TP_DEBUG("***major = %d ***\n", major);
	TP_DEBUG("***minor = %d ***\n", minor);
	sprintf(fw_version, "%03d%03d", major, minor);
	TP_DEBUG("***fw_version = %s ***\n", fw_version);
	return size;
}

static DEVICE_ATTR(version, 0777, firmware_version_show, firmware_version_store);

static ssize_t firmware_data_show(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	return FwDataCnt;
}

static ssize_t firmware_data_store(struct device *dev,
				     struct device_attribute *attr,
				     const char *buf, size_t size)
{
	int i;

	 TP_DEBUG_ERR("***FwDataCnt = %d ***\n", FwDataCnt);
	// for (i = 0; i < 1024; i++)
	 {
		 memcpy(temp[FwDataCnt], buf, 1024);
	 }
	 FwDataCnt++;
	 return size;

}

static DEVICE_ATTR(data, 0777, firmware_data_show, firmware_data_store);

#endif

static void msg21xx_suspend(struct early_suspend *h)
{
	disable_irq_nosync(msg21xx_irq);
	gpio_request(ts_gpio_reset_pin, "reset");
	gpio_direction_output(ts_gpio_reset_pin, 0);
	gpio_set_value(ts_gpio_reset_pin, 0);
	gpio_free(ts_gpio_reset_pin);
}
static void msg21xx_chip_init(void);

static void msg21xx_resume(struct early_suspend *h)
{
	msg21xx_chip_init();
#ifdef PACKET_FROM_DBUS
	dbbusDWIICEnterSerialDebugMode();
	dbbusDWIICIICReshape();
#endif
	enable_irq(msg21xx_irq);
}

static void msg21xx_chip_init(void)
{
	/* After the LCD is on, power on the TP controller */
	gpio_request(ts_gpio_reset_pin, "reset");
	gpio_direction_output(ts_gpio_reset_pin, 0);
	gpio_set_value(ts_gpio_reset_pin, 0);
	mdelay(20);/*min value is 10ms */
	gpio_set_value(ts_gpio_reset_pin, 1);
	mdelay(80);/*min value is 50ms*/
	gpio_free(ts_gpio_reset_pin);
}

static u8 Calculate_8BitsChecksum(u8 *msg, s32 s32Length)
{
	s32 s32Checksum = 0;
	s32 i;
	for (i = 0; i < s32Length; i++)
		s32Checksum += msg[i];
	return (u8) ((-s32Checksum) & 0xFF);
}

static int i2c_read_bytes(struct i2c_client *client, uint8_t *buf, int len)
{
	struct i2c_msg msgs[2];
	int ret = -1;
	msgs[0].flags = 0;
	msgs[0].addr = client->addr;
	msgs[0].len = 2;
	msgs[0].buf = &buf[0];
	msgs[1].flags = I2C_M_RD;
	msgs[1].addr = client->addr;
	msgs[1].len = len - 2;
	msgs[1].buf = &buf[2];
	ret = i2c_transfer(client->adapter, msgs, 2);
	return ret;
}

static int msg21xx_i2c_rxdata(char *rxdata, int length)
{
	int ret;

	struct i2c_msg msgs[] = {
		{
			.addr	= msg21xx_i2c_client->addr,
			.flags	= 0,
			.len	= 1,
			.buf	= rxdata,
		},
		{
			.addr	= msg21xx_i2c_client->addr,
			.flags	= I2C_M_RD,
			.len	= length,
			.buf	= rxdata,
		},
	};

	ret = i2c_transfer(msg21xx_i2c_client->adapter, msgs, 2);
	if (ret < 0)
		printk(KERN_ERR "msg %s i2c read error: %d\n", __func__, ret);

	return ret;
}

static void msg21xx_data_disposal(struct work_struct *work)
{
	u8 temp[REPORT_PACKET_LENGTH+1] = {0};
	u8 *val = &temp[1];
	u8 Checksum = 0;
	u8 i;
	u32 delta_x = 0, delta_y = 0;
	u32 u32X = 0;
	u32 u32Y = 0;
	int ret;
	u8 touchkeycode = 0;
	TouchScreenInfo_t touchData;
	static u32 preKeyStatus;
	int dbg_isr_numbak = dbg_isr_num;

	#define SWAP_X_Y   (1)
	/*#define REVERSE_X  (1)*/
	/*#define REVERSE_Y  (1)*/
#ifdef SWAP_X_Y
	int tempx;
	int tempy;
#endif
	if (g_debugTp == 1)
		printk(KERN_INFO "enter msg21xx_data_disposal.\n");
#ifdef PACKET_FROM_DBUS
	val[0] = 0x10;
	val[1] = 0x12;
	val[2] = 0xE4;
	ret = HalTscrCDevWriteI2CSeq(FW_ADDR_MSG21XX, val, 3 );
	ret = HalTscrCReadI2CSeq(FW_ADDR_MSG21XX, temp, REPORT_PACKET_LENGTH+1);
	drvDB_WriteReg(0x12, 0xEC, 0 );
#else
	ret = i2c_master_recv(msg21xx_i2c_client, &val[0], REPORT_PACKET_LENGTH);
#endif

#if 1	
	if (ret != REPORT_PACKET_LENGTH) {
		printk(KERN_ERR "data read error!\n");
		enable_irq(msg21xx_irq);
		return;
	}
	if (g_debugTp == 1) {
		printk(KERN_INFO "going to print the data.\r\n");
		for (i = 0; i < REPORT_PACKET_LENGTH; i++)
		{
			printk(KERN_INFO "msg21xx_data_disposal--val[%d]=0x%x\r\n",i,val[i]);
		}
		printk(KERN_INFO "end to print the data.\r\n");
		i = 0;
	}
#else
	if (ret < 0) {
		printk(KERN_ERR "[msg21xx_data_disposal] recv err!\n");
		return;
	}
#endif	

	/*enable_irq(msg21xx_irq);*/
	dbg_isr_disp++;
	if (g_debugTp == 2)
		return;

	Checksum = Calculate_8BitsChecksum(&val[0], 7);	/*calculate checksum*/
	if ((Checksum == val[7]) && (val[0] == 0x52)) {
		/*check the checksum  of packet*/

		u32X = (((val[1] & 0xF0) << 4) | val[2]);
		/*parse the packet to coordinates*/
		u32Y = (((val[1] & 0x0F) << 8) | val[3]);
		delta_x = (((val[4] & 0xF0) << 4) | val[5]);
		delta_y = (((val[4] & 0x0F) << 8) | val[6]);

#ifdef SWAP_X_Y
		tempy = u32X;
		tempx = u32Y;
		u32X = tempx;
		u32Y = tempy;
		tempy = delta_x;
		tempx = delta_y;
		delta_x = tempx;
		delta_y = tempy;
#endif
#ifdef REVERSE_X
		u32X = 2047 - u32X;
		delta_x = 4095 - delta_x;
#endif
#ifdef REVERSE_Y
		u32Y = 2047 - u32Y;
		delta_y = 4095 - delta_y;

#endif
		if ((val[1] == 0xFF) && (val[2] == 0xFF)
			 && (val[3] == 0xFF) && (val[4] == 0xFF)
			 && (val[6] == 0xFF)) {
			touchData.Point[0].X = 0;	/* final X coordinate*/
			touchData.Point[0].Y = 0;	/* final Y coordinate*/
			if ((val[5] == 0x0) || (val[5] == 0xFF)) {
				touchData.nFingerNum = 0;
				touchData.nTouchKeyCode = 0;
				touchData.nTouchKeyMode = 0;
			}

			else {
				touchData.nTouchKeyMode = 1;
				touchData.nTouchKeyCode = val[5];
				touchData.nFingerNum = 1;
			}
		} else {
			touchData.nTouchKeyMode = 0;
			if (
#ifdef REVERSE_X
				(delta_x == 4095)
#else
				(delta_x == 0)
#endif
				&&
#ifdef REVERSE_Y
				(delta_y == 4095)
#else
				(delta_y == 0)
#endif
			)
			{
				touchData.nFingerNum = 1;	/*one touch*/
				touchData.Point[0].X = (u32X * ts_x_max_value) / 2048;
				touchData.Point[0].Y = (u32Y * ts_y_max_value) / 2048;
			} else {
				u32 x2, y2;
				touchData.nFingerNum = 2;	/*two touch*/
				/* Finger 1 */
				touchData.Point[0].X = (u32X * ts_x_max_value) / 2048;
				touchData.Point[0].Y = (u32Y * ts_y_max_value) / 2048;

				/* Finger 2 */
				if (delta_x > 2048) {	/*transform the unsigh value to sign value*/

					delta_x -= 4096;
				}
				if (delta_y > 2048)
					delta_y -= 4096;
				x2 = (u32) (u32X + delta_x);
				y2 = (u32) (u32Y + delta_y);
				touchData.Point[1].X = (x2 * ts_x_max_value) / 2048;
				touchData.Point[1].Y = (y2 * ts_y_max_value) / 2048;
				}
			}

		    /*report...*/
			if (touchData.nTouchKeyMode) {
			#ifdef VKEY_SYS
				if (touchData.nTouchKeyCode == 1) {
					touchData.Point[0].X = 80;
					touchData.Point[0].Y = 900;
				}
				if (touchData.nTouchKeyCode == 2) {
					touchData.Point[0].X = 240;
					touchData.Point[0].Y = 900;
				}
				if (touchData.nTouchKeyCode == 4) {
					touchData.Point[0].X = 400;
					touchData.Point[0].Y = 900;
				}

				input_report_abs(input,
							     ABS_MT_POSITION_X,
							     touchData.Point[0].
							     X);
				input_report_abs(input,
							  ABS_MT_POSITION_Y,
							  touchData.Point[0].Y);
				
				input_report_abs(input, ABS_MT_TOUCH_MAJOR, 8);
				input_report_key(input, BTN_TOUCH, 1);

				if (touchData.Point[0].Y > ts_y_max_value)
				blocking_notifier_call_chain(
						&touch_key_notifier, 0, NULL);
				input_mt_sync(input);
				input_sync(input);
			#else
				if (touchData.nTouchKeyCode == 1)
					touchkeycode = KEY_BACK;
				if (touchData.nTouchKeyCode == 2)
					touchkeycode = KEY_HOME;
				if (touchData.nTouchKeyCode == 4)
					touchkeycode = KEY_MENU;
				if (preKeyStatus != touchkeycode) {

					preKeyStatus = touchkeycode;
					input_report_key(input, touchkeycode, 1);
				}
				input_sync(input);
			#endif

			} else {
				preKeyStatus = 0;	/*clear key status..*/
				if ((touchData.nFingerNum) == 0) {	/*touch end*/

					/*preFingerNum=0;*/
					//input_report_key(input, KEY_BACK, 0);
					//input_report_key(input, KEY_HOME, 0);
					//input_report_key(input, KEY_MENU, 0);
					input_report_key(input, BTN_TOUCH, 0);
					input_mt_sync(input);
					input_sync(input);
				} else {	/*touch on screen*/
					/*
				       if(preFingerNum!=touchData.nFingerNum) {
						printk("langwenlong number has changed\n");
						preFingerNum = touchData.nFingerNum;
						input_report_abs(input, ABS_MT_TOUCH_MAJOR, 0);
						input_mt_sync(input);
						input_sync(input);
					} */
					for (i = 0; i < (touchData.nFingerNum); i++) {
						input_report_abs(input,
							     ABS_MT_POSITION_X,
							     touchData.Point[i].
							     X);
						input_report_abs(input,
							  ABS_MT_POSITION_Y,
							  touchData.Point[i].Y);
						
						input_report_abs(input, ABS_MT_TOUCH_MAJOR, 8);
						input_report_key(input, BTN_TOUCH, 1);
						if (g_debugTp == 1)
							printk(KERN_INFO "x=%d,y=%d \r\n",
							touchData.Point[i].X,
							touchData.Point[i].Y);
						input_mt_sync(input);
					}
					input_sync(input);
				}
			}
	} else
		printk(KERN_ERR "checksum error!\n");
	if (g_debugTp == 1)
		printk(KERN_INFO "msg21xx_data_disposal dbg_isr_disp=%d,dbg_isr_num=%d,diff=%d\n",
		     dbg_isr_disp, dbg_isr_num, dbg_isr_num - dbg_isr_disp);
	enable_irq(msg21xx_irq);
}
static u8 getchipType(void)
{
    u8 curr_ic_type = 0;
    u8 dbbus_tx_data[4];
    unsigned char dbbus_rx_data[2] = {0};
	
	//_HalTscrHWReset();
    HalDisableIrq();
    //mdelay ( 300 );
    
    dbbusDWIICEnterSerialDebugMode();
    dbbusDWIICStopMCU();
    dbbusDWIICIICUseBus();
    dbbusDWIICIICReshape();
    mdelay ( 200 );

    // Disable the Watchdog
    dbbus_tx_data[0] = 0x10;
    dbbus_tx_data[1] = 0x3C;
    dbbus_tx_data[2] = 0x60;
    dbbus_tx_data[3] = 0x55;
    HalTscrCDevWriteI2CSeq ( FW_ADDR_MSG21XX, dbbus_tx_data, 4 );
    dbbus_tx_data[0] = 0x10;
    dbbus_tx_data[1] = 0x3C;
    dbbus_tx_data[2] = 0x61;
    dbbus_tx_data[3] = 0xAA;
    HalTscrCDevWriteI2CSeq ( FW_ADDR_MSG21XX, dbbus_tx_data, 4 );
    // Stop MCU
    dbbus_tx_data[0] = 0x10;
    dbbus_tx_data[1] = 0x0F;
    dbbus_tx_data[2] = 0xE6;
    dbbus_tx_data[3] = 0x01;
    HalTscrCDevWriteI2CSeq ( FW_ADDR_MSG21XX, dbbus_tx_data, 4 );
    /////////////////////////
    // Difference between C2 and C3
    /////////////////////////
    // c2:2133 c32:2133a(2) c33:2138
    //check id
    dbbus_tx_data[0] = 0x10;
    dbbus_tx_data[1] = 0x1E;
    dbbus_tx_data[2] = 0xCC;
    HalTscrCDevWriteI2CSeq ( FW_ADDR_MSG21XX, dbbus_tx_data, 3 );
	mdelay(300);
    HalTscrCReadI2CSeq ( FW_ADDR_MSG21XX, &dbbus_rx_data[0], 2 );
    
    if ( dbbus_rx_data[0] == 2 )
    {
    	curr_ic_type = CTP_ID_MSG21XXA;
    }
    else
    {
    	curr_ic_type = CTP_ID_MSG21XX;
    }
    TP_DEBUG_ERR("CURR_IC_TYPE = %d \n",curr_ic_type);
    //dbbusDWIICIICNotUseBus();
    //dbbusDWIICNotStopMCU();
    //dbbusDWIICExitSerialDebugMode();
    HalEnableIrq();
	_HalTscrHWReset();
     
    return curr_ic_type;
    
}
static void getMSG21XXFWVersion(u8 curr_ic_type)
{
    unsigned char dbbus_tx_data[3]={0};
    unsigned char dbbus_rx_data[4]={0} ;
    unsigned short major=0, minor=0;
	int ret_write=0;
	int ret_read=0;

    _HalTscrHWReset();
    HalDisableIrq();
    mdelay(300);
    
    dbbusDWIICEnterSerialDebugMode();
    dbbusDWIICStopMCU();
    dbbusDWIICIICUseBus();
    dbbusDWIICIICReshape();
    mdelay (400 );
    
    dbbus_tx_data[0] = 0x53;
    dbbus_tx_data[1] = 0x00;
     if(curr_ic_type==CTP_ID_MSG21XXA)
    {
    dbbus_tx_data[2] = 0x2A;
    }
    else if(curr_ic_type==CTP_ID_MSG21XX)
    {
        dbbus_tx_data[2] = 0x74;
    }
    else
    {
        TP_DEBUG_ERR("***ic_type = %d ***\n", dbbus_tx_data[2]);
        dbbus_tx_data[2] = 0x2A;
    }
    ret_write=HalTscrCDevWriteI2CSeq(FW_ADDR_MSG21XX_TP, &dbbus_tx_data[0], 3);
	mdelay(500);
    ret_read=HalTscrCReadI2CSeq(FW_ADDR_MSG21XX_TP, &dbbus_rx_data[0], 4);

    curr_ic_major = (dbbus_rx_data[1]<<8)+dbbus_rx_data[0];
    curr_ic_minor = (dbbus_rx_data[3]<<8)+dbbus_rx_data[2];
	if ((ret_write!=3)||(ret_read!=4)){
		curr_ic_major = 0xff;
		curr_ic_minor = 0xff;
	}

    TP_DEBUG("***FW Version major = %d--ret_write=%d,ret_read=%d***\n", curr_ic_major, ret_write, ret_read);
    TP_DEBUG("***FW Version minor = %d ***\n", curr_ic_minor);
    
    _HalTscrHWReset();
    HalEnableIrq();
    mdelay ( 100 );
}

static void tp_firmware_update_work(struct work_struct *work)
{
#if 0
	printk("lht--going to update the tp firmware.\r\n");
	wake_lock(&update_wake_lock);
	firmware_update_store(NULL, NULL, NULL, 0);
	wake_unlock(&update_wake_lock);
	printk("lht--after update the tp firmware.\r\n");
	return;

	firmware_version_store(NULL, NULL, NULL, 0);
	wanted_major = (MSG_FIRMWARE[0x3076] << 8) + MSG_FIRMWARE[0x3077];
	wanted_minor = (MSG_FIRMWARE[0x3074] << 8) + MSG_FIRMWARE[0x3075];
	if (((0 < major) && (major < wanted_major))
		|| ((major == wanted_major)
		&& (minor < wanted_minor))) {
		firmware_update_store(NULL, NULL, NULL, 0);
	}
#else
	printk(KERN_INFO "going to update the tp firmware.\r\n");
	wake_lock(&update_wake_lock);

	//curr_ic_type = getchipType();
	curr_ic_type = 2;
    getMSG21XXFWVersion(curr_ic_type);
	TP_DEBUG_ERR("[TP] check auto updata\n");
	if(curr_ic_type == CTP_ID_MSG21XXA) {

		update_bin_major = MSG21XX_update_bin[0x7f4f]<<8|MSG21XX_update_bin[0x7f4e];
        update_bin_minor = MSG21XX_update_bin[0x7f51]<<8|MSG21XX_update_bin[0x7f50];
        TP_DEBUG_ERR("bin_major = %d\n", update_bin_major);
        TP_DEBUG_ERR("bin_minor = %d\n", update_bin_minor);
        
		if((update_bin_major==curr_ic_major \
            &&update_bin_minor>curr_ic_minor) \
            ||((curr_ic_minor==0xff)&&(curr_ic_major==0xff))) {
    	    int i = 0;
    		for (i = 0; i < 33; i++) {
    		    firmware_data_store(NULL, NULL, &(MSG21XX_update_bin[i*1024]), 0);
    		}
            //kthread_run(fwAutoUpdate, 0, "MSG21XXA_fw_auto_update");            
			firmware_update_store(NULL, NULL, NULL, 0); 
    	}
	}
    else if(curr_ic_type == CTP_ID_MSG21XX) {

	    update_bin_major = MSG21XX_update_bin[0x3076]<<8|MSG21XX_update_bin[0x3077];
        update_bin_minor = MSG21XX_update_bin[0x3074]<<8|MSG21XX_update_bin[0x3075];
        TP_DEBUG_ERR("bin_major = %d \n",update_bin_major);
        TP_DEBUG_ERR("bin_minor = %d \n",update_bin_minor);
        
		if(update_bin_major==curr_ic_major
            &&update_bin_minor>curr_ic_minor)
    	{
    	    int i = 0;
    		for (i = 0; i < 94; i++)
    		{
    		    firmware_data_store(NULL, NULL, &(MSG21XX_update_bin[i*1024]), 0);
    		}
           // kthread_run(fwAutoUpdate, 0, "MSG21XX_fw_auto_update");
		   firmware_update_store(NULL, NULL, NULL, 0); 
    	}
	}
	
	wake_unlock(&update_wake_lock);
	printk("after update the tp firmware.\r\n");
   

#endif


}

static int msg21xx_ts_open(struct input_dev *dev)
{
	return 0;
}

static void msg21xx_ts_close(struct input_dev *dev)
{
	printk(KERN_INFO "msg21xx_ts_close\n");
}

static int msg21xx_init_input(void)
{
	int err;
	input = input_allocate_device();
	input->name = msg21xx_i2c_client->name;
	input->phys = "I2C";
	if (have_vkey) {
		input->id.bustype = BUS_I2C;
		input->id.vendor = 0x8888;
		input->id.product = 0x6666;
		input->id.version = 1000;
	}
	input->dev.parent = &msg21xx_i2c_client->dev;
	input->open = msg21xx_ts_open;
	input->close = msg21xx_ts_close;
	set_bit(EV_ABS, input->evbit);
	set_bit(EV_SYN, input->evbit);
	set_bit(EV_KEY, input->evbit);
	set_bit(BTN_TOUCH, input->keybit);
	if (have_vkey) {
		set_bit(KEY_BACK, input->keybit);
		set_bit(KEY_MENU, input->keybit);
		set_bit(KEY_HOME, input->keybit);
	}
	set_bit(INPUT_PROP_DIRECT, input->propbit);
	input_set_abs_params(input, ABS_MT_POSITION_X, 0,
				 ts_x_max_value, 0, 0);
	input_set_abs_params(input, ABS_MT_POSITION_Y, 0, ts_y_max_value,
			      0, 0);
	input_set_abs_params(input, ABS_MT_TOUCH_MAJOR, 0, 8, 0, 0);
	err = input_register_device(input);
	if (err)
		goto fail_alloc_input;
fail_alloc_input:
	return 0;
}

static irqreturn_t msg21xx_interrupt(int irq, void *dev_id)
{
	int ret;
	dbg_isr_num++;
	disable_irq_nosync(msg21xx_irq);
	//schedule_work(&msg21xx_wq);
	#if 1
	if (synaptics_wq) {
		//ret = queue_work(mstar_wq, &msg21xx_wq);
		ret = queue_work(synaptics_wq, &msg21xx_wq);
		if (!ret) {
			printk(KERN_INFO " last tp in queue=%d,%d,!!!\n",
				dbg_isr_num, dbg_isr_disp);
		}
	}
	#endif
	return IRQ_HANDLED;
}
/*
*get firmware size

@firmware_name:firmware name
*note:the firmware default path is sdcard.
	if you want to change the dir, please modify by yourself.
*/
static int msg21xx_get_firmware_size(char *firmware_name)
{
	struct file *pfile = NULL;
	struct inode *inode;
	unsigned long magic;
	off_t fsize = 0;
	char filepath[256];
	memset(filepath, 0, sizeof(filepath));

	sprintf(filepath, "%s", firmware_name);

	if (NULL == pfile)
		pfile = filp_open(filepath, O_RDONLY, 0);

	if (IS_ERR(pfile)) {
		pr_err("error occured while opening file %s.\n", filepath);
		return -EIO;
	}

	inode = pfile->f_dentry->d_inode;
	magic = inode->i_sb->s_magic;
	fsize = inode->i_size;
	filp_close(pfile, NULL);
	return fsize;
}



/*
*read firmware buf for .bin file.

@firmware_name: fireware name of full path
@firmware_buf: data buf of fireware

note:the firmware default path is sdcard.
	if you want to change the dir, please modify by yourself.
*/
static int msg21xx_read_firmware(char *firmware_name,
			       unsigned char *firmware_buf)
{
	struct file *pfile = NULL;
	struct inode *inode;
	unsigned long magic;
	off_t fsize;
	char filepath[256];
	loff_t pos;
	mm_segment_t old_fs;

	if (!firmware_buf) {
		pr_err("%s--firmware buffer NULL!\n", __func__);
		return -EIO;
	}

	memset(filepath, 0, sizeof(filepath));
	sprintf(filepath, "%s", firmware_name);
	if (NULL == pfile)
		pfile = filp_open(filepath, O_RDONLY, 0);
	if (IS_ERR(pfile)) {
		pr_err("error occured while opening file %s.\n", filepath);
		return -EIO;
	}

	inode = pfile->f_dentry->d_inode;
	magic = inode->i_sb->s_magic;
	fsize = inode->i_size;
	old_fs = get_fs();
	set_fs(KERNEL_DS);
	pos = 0;
	vfs_read(pfile, firmware_buf, fsize, &pos);
	filp_close(pfile, NULL);
	set_fs(old_fs);

	return 0;
}

/*
upgrade with *.bin file with full path firmware_name
*/

int msg_fw_upgrade_with_app_file(struct i2c_client *client,
				       char *firmware_name)
{
	u8 *pbt_buf = NULL;
	int i_ret;
	int i;
	int fwsize = msg21xx_get_firmware_size(firmware_name);
	printk(KERN_INFO "adcdefghigklmn--fw name:%s\n", firmware_name);
	if (fwsize <= 0) {
		dev_err(&client->dev, "%s ERROR:Get firmware size failed\n",
					__func__);
		return -EIO;
	}

	if (fwsize < 8 || fwsize > 50 * 1024) {
		dev_dbg(&client->dev, "%s:FW length error\n", __func__);
		return -EIO;
	}

	/*=========FW upgrade========================*/
	pbt_buf = vmalloc(fwsize + 1);
	if (!pbt_buf) {
		dev_err(&client->dev, "%s() - ERROR: memory allocate failed\n",
					__func__);
		return -EIO;
	}
	if (msg21xx_read_firmware(firmware_name, pbt_buf)) {
		dev_err(&client->dev, "%s() - ERROR: request_firmware failed\n",
					__func__);
		vfree(pbt_buf);
		return -EIO;
	}
	/*call the upgrade function */
	for (i = 0; i < 33; i++)
		firmware_data_store(NULL, NULL, pbt_buf+i*1024, 0);
	firmware_update_store(NULL, NULL, NULL, 0);
	printk(KERN_INFO "[FTS] upgrade successfully.\n");
	vfree(pbt_buf);
	return i_ret;
}

#if defined(VKEY_SYS)
static ssize_t msg21xx_virtual_keys_show(struct kobject *kobj,
				       struct kobj_attribute *attr, char *buf)
{
	return sprintf(buf, vkey_scope);
}
static struct kobj_attribute msg21xx_virtual_keys_attr = {
	.attr = {
		 .name = "virtualkeys.ms-msg21xx",
		 .mode = S_IRUGO,
		 },
	.show = &msg21xx_virtual_keys_show,
};

static ssize_t msg21xx_firmware_version_show(struct kobject *kobj,
				   struct kobj_attribute *attr, char *buf)
{
	firmware_version_store(NULL, attr, buf, 0);
	return firmware_version_show(NULL, attr, buf);

}
ssize_t msg21xx_firmware_upgrade_store(struct kobject *kobj,
		struct kobj_attribute *attr, const char *buf, size_t count)
{
	int i;
	char fwname[256];

	wake_lock(&update_wake_lock);
	memset(fwname, 0, sizeof(fwname));
	sprintf(fwname, "%s", buf);
	fwname[count - 1] = '\0';
	msg_fw_upgrade_with_app_file(msg21xx_i2c_client, fwname);
	wake_unlock(&update_wake_lock);
	return count;
}

static struct kobj_attribute msg21xx_firmware_version_attr = {
	.attr = {
		.name = "upgradefw",
		.mode = S_IRUGO,
	},
	.show = &msg21xx_firmware_version_show,
};

static struct kobj_attribute msg21xx_firmware_upgrade_attr = {
	.attr = {
		.name = "upgradeapp",
		.mode = S_IRUGO|S_IWUGO,
	},
	.show = &msg21xx_firmware_version_show,
	.store = &msg21xx_firmware_upgrade_store,
};


static struct attribute *msg21xx_properties_attrs[] = {
	&msg21xx_virtual_keys_attr.attr,
	&msg21xx_firmware_version_attr.attr,
	&msg21xx_firmware_upgrade_attr.attr,
	NULL
};
static struct attribute_group msg21xx_properties_attr_group = {
	.attrs = msg21xx_properties_attrs,
};
#endif

static int __devinit msg21xx_probe(struct i2c_client *client,
				      const struct i2c_device_id *id)
{
	int err = 0;
	u32 val;
	u8  chip_type = 0;
	msg21xx_ts_data *pdata = NULL;
	struct device_node *np = client->dev.of_node;

	if (!client) {
		err = -1;
		goto err_i2c_client_check;
	}
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		printk(KERN_ERR "%s: need I2C_FUNC_I2C\n", __func__);
		err = -ENODEV;
		goto err_i2c_client_check;
	}
	pdata = kzalloc(sizeof(msg21xx_ts_data), GFP_KERNEL);
	if (!pdata) {
		printk(KERN_ERR "Unable to allocate memory.\n");
		return -ENOMEM;
	}
	msg21xx_i2c_client = client;
	if (1 == g_debugTp)
		printk(KERN_INFO "enter msg21xx_probe.\r\n");

	if (client->dev.platform_data) {
		ts_gpio_irq_pin = pdata->gpio_irq_pin;
		ts_gpio_reset_pin = pdata->gpio_reset_pin;
		ts_x_max_value = pdata->x_max_value;
		ts_y_max_value = pdata->y_max_value;

	} else if (client->dev.of_node) {
		if (!of_property_read_u32(np, "gpio-irq-pin", &val))
			ts_gpio_irq_pin = val;
		if (!of_property_read_u32(np, "gpio-reset-pin", &val))
			ts_gpio_reset_pin = val;
		if (!of_property_read_u32(np, "x-max-value", &val))
			ts_x_max_value = val;
		if (!of_property_read_u32(np, "y-max-value", &val))
			ts_y_max_value = val;
		if (of_property_read_string(np, "vkey_scope", &vkey_scope)) {
			have_vkey = false;
			vkey_scope = "\n";
		} else {
			have_vkey = true;
		}
		if (!of_property_read_u32(np, "client_func_magic", &val))
			pdata->i2c_pdata.client_func_magic = val;

		if (!of_property_read_u32(np, "client_func_map", &val))
			pdata->i2c_pdata.client_func_map = val;

	}
	i2c_set_clientdata(client, pdata);
	client->dev.platform_data = &pdata->i2c_pdata;

	mstar_wq = create_workqueue("mstar_wq");
	if (!mstar_wq) {
		printk(KERN_ERR "%s--Failed to create workqueue.\r\n",
				 __func__);
		goto exit;
	}
	INIT_WORK(&msg21xx_wq, msg21xx_data_disposal);

	gpio_request(ts_gpio_irq_pin, "interrupt");
	gpio_direction_input(ts_gpio_irq_pin);
	
	msg21xx_irq = GPIO_TO_IRQ(ts_gpio_irq_pin);
	err = request_irq(msg21xx_irq, msg21xx_interrupt, IRQF_TRIGGER_FALLING,
			"msg21xx", NULL);
	if (err != 0) {
		printk(KERN_ERR "%s: cannot register irq\n", __func__);
		goto exit;
	}
	disable_irq(msg21xx_irq);
	msg21xx_chip_init();
	chip_type = getchipType();
	printk(KERN_INFO "chip_type=%d \n",chip_type);
	if (chip_type != 2)
		goto chip_not_exist;

#if defined(VKEY_SYS)
		struct kobject *properties_kobj;
		err = -1;
		properties_kobj = kobject_create_and_add("board_properties", NULL);
		if (properties_kobj)
			err = sysfs_create_group(properties_kobj,
						 &msg21xx_properties_attr_group);
		if (!properties_kobj || err)
			pr_err("failed to create board_properties\n");
#endif

	wake_lock_init(&update_wake_lock, WAKE_LOCK_SUSPEND, "touch");
	INIT_DELAYED_WORK(&msg21xx_firmware_update, tp_firmware_update_work);
	msg21xx_init_input();
	early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN;
	early_suspend.suspend = msg21xx_suspend;
	early_suspend.resume = msg21xx_resume;
	register_early_suspend(&early_suspend);

	 /*frameware upgrade */
#ifdef __FIRMWARE_UPDATE__
	    firmware_class =
	    class_create(THIS_MODULE, "ms-touchscreen-msg20xx");
	if (IS_ERR(firmware_class))
		pr_err("Failed to create class(firmware)!\n");
	firmware_cmd_dev =
	    device_create(firmware_class, NULL, 0, NULL, "device");
	if (IS_ERR(firmware_cmd_dev))
		pr_err("Failed to create device(firmware_cmd_dev)!\n");

	if (device_create_file(firmware_cmd_dev, &dev_attr_version) < 0)
		pr_err("Failed to create device file(%s)!\n",
			dev_attr_version.attr.name);

	/*update*/
	if (device_create_file(firmware_cmd_dev, &dev_attr_update) < 0)
		pr_err("Failed to create device file(%s)!\n",
			dev_attr_update.attr.name);

	/* data*/
	if (device_create_file(firmware_cmd_dev, &dev_attr_data) < 0)
		pr_err("Failed to create device file(%s)!\n",
			dev_attr_data.attr.name);

	/* clear*/
	    if (device_create_file(firmware_cmd_dev, &dev_attr_clear) < 0)
		pr_err("Failed to create device file(%s)!\n",
			dev_attr_clear.attr.name);

	/* debug*/
	if (device_create_file(firmware_cmd_dev, &dev_attr_debug) < 0)
		pr_err("Failed to create device file(%s)!\n",
			dev_attr_debug.attr.name);
	dev_set_drvdata(firmware_cmd_dev, NULL);
#endif	/* */
#ifdef __FIRMWARE_UPDATE_AT_BOOT__
	queue_delayed_work(mstar_wq, &msg21xx_firmware_update, 5*HZ);
#endif
#ifdef PACKET_FROM_DBUS
	mdelay ( 500 );
	dbbusDWIICEnterSerialDebugMode();
	dbbusDWIICIICReshape();
#endif
	#ifdef CONFIG_LEDS_TRIGGER_KPBL
	led_kpbl_register(&register_touch_key_notifier);
	led_kpbl_unregister(&unregister_touch_key_notifier);
	#endif
	if (1 == g_debugTp)
		printk(KERN_INFO "exit msg21xx_probe.\r\n");
	enable_irq(msg21xx_irq);
	return 0;
chip_not_exist:
	if (mstar_wq != NULL) {
		destroy_workqueue(mstar_wq);
		mstar_wq = NULL;
	}
exit:
	if(pdata) {
		kfree(pdata);
		pdata = NULL;
	}
	gpio_free(ts_gpio_irq_pin);
err_i2c_client_check:
	return err;
}

static int __devexit msg21xx_remove(struct i2c_client *client)
{
	return 0;
}

static const struct i2c_device_id msg21xx_id[] = { {"ms-msg21xx", 0x26}, {}
};

MODULE_DEVICE_TABLE(i2c, msg21xx_id);
static struct i2c_driver msg21xx_driver = {
			.driver = {
				.name = "ms-msg21xx",
				.owner = THIS_MODULE, },
			.probe = msg21xx_probe,
			.remove = __devexit_p(msg21xx_remove),
			.id_table = msg21xx_id,
};

static int __init msg21xx_init(void)
{
	int err;
	
	synaptics_wq = create_singlethread_workqueue("synaptics_wq");
	if (!synaptics_wq) {
		printk(KERN_ERR "Could not create work queue synaptics_wq: no memory");
		return -ENOMEM;
	}
	err = i2c_add_driver(&msg21xx_driver);
	if (err)
		printk(KERN_WARNING "msg21xx driver failed,(errno = %d)\n", err);
	else
		printk(KERN_INFO "Successfully added driver %s\n", msg21xx_driver.driver.name);

	return err;
}

static void __exit msg21xx_cleanup(void)
{
	i2c_del_driver(&msg21xx_driver);
}

module_init(msg21xx_init);

module_exit(msg21xx_cleanup);
MODULE_AUTHOR("Mstar semiconductor");
MODULE_DESCRIPTION("Driver for msg21xx Touchscreen Controller");
MODULE_LICENSE("GPL");

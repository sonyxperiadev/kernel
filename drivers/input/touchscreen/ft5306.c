/*****************************************************************************
* Copyright 2012 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a
* license other than the GPL, without Broadcom's express prior written
* consent.
*****************************************************************************/

/* ---- Include Files ---------------------------------------------------- */


#include <linux/module.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/hrtimer.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/i2c/ft5306.h>
#include <linux/io.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/regulator/consumer.h>
#include <linux/regulator/driver.h>
#include <linux/notifier.h>
#include <linux/i2c/ft6x06_ex_fun.h>
#include <linux/wakelock.h>
#include <linux/vmalloc.h>
#include <linux/leds.h>
#include <linux/of_gpio.h>

#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif

#define GPIO_TO_IRQ gpio_to_irq

static int ts_gpio_irq_pin=0;
#if (TP_CNTRL_PIN_TYPE == TP_CNTRL_PIN_RESET)
static int ts_gpio_reset_pin=0;
#endif
static int ts_gpio_wakeup_pin=0;
static int ts_x_max_value=0;
static int ts_y_max_value=0;
static const char *tp_power;
static const char *vkey_scope;
static const char *fw_header;
static bool have_vkey = false;
static char firmware_str[50];

/*******************************/
typedef unsigned char	FTS_BYTE;
typedef unsigned short	FTS_WORD;
typedef unsigned int	FTS_DWRD;
typedef signed int		FTS_BOOL;

#define FTS_MAX_FINGER	5

#define FTS_NULL	0x0
#define FTS_TRUE	0x01
#define FTS_FALSE	0x0

#define DEVIDE_MODE_REG		0x00
#define READ_ROW_ADDR		0x01  //rawdata row
#define RAWDATA_BEGIN_ADDR	0x10 //rawdata addr

#define DEVICE_PM_REG		0xA5
#define DEVICE_PM_ACTIVE	0x00
#define DEVICE_PM_MONITOR	0x01
#define DEVICE_PM_HIBERNATE	0x03

#define HAWAII_GARNET_FT5X06_VENDOR_ID 0x87
#define G5_A18_FT5X06_VENDOR_ID 0x79
#define G5_A21_FT5316_VENDOR_ID 0x98
#define KTOUCH_W68_FT6X06_VENDOR_ID 0x5a
#define KTOUCH_5606_FT6X06_VENDOR_ID 0x5a
#define KTOUCH_W81_FT6X06_VENDOR_ID 0x51
#define TCL_UP823_FT6206_VENDOR_ID 0x11

#define PROTOCOL_LEN 33

#define POINTER_CHECK(p)	if((p)==FTS_NULL){return FTS_FALSE;}

 /*
 Error status codes:
 */
#define CTPM_NOERROR			(0x01 << 0)
#define CTPM_ERR_PARAMETER		(0x01 << 1)
#define CTPM_ERR_PROTOCOL		(0x01 << 2)
#define CTPM_ERR_ECC			(0x01 << 3)
#define CTPM_ERR_MODE			(0x01 << 4)
#define CTPM_ERR_I2C			(0x01 << 5)
#define CTPM_ERR_SCAN			(0x01 << 6)

/*support 5 points*/
#define CTPM_STD_POINTS_MAX 0x05

/*register for checking ----------->*/
/*
Work mode: 0x01
Factory mode: 0x03
*/
#define REG_RUN_MODE (0x27+0x80)

/*
the Firmware version number should be up from 0x10
*/
#define REG_FW_VER (0x26+0x80)

/*
the frequence should be from 0x04 to 0x08
*/
#define REG_SCAN_FREQUENCE (0x08+0x80)
/*<-------------register for checking*/


/*event difinition*/
#define TC_PutDown 0x00
#define TC_PutUp   0x01
#define TC_Contact 0x02
#define TC_NoEvent 0x03

/*the information of one touch point */
typedef struct
{
	/*x coordinate*/
	FTS_WORD	w_tp_x, last_tp_x;
	/*y coordinate*/
	FTS_WORD	w_tp_y, last_tp_y;
	/*point id: start from 0*/
	FTS_BYTE	bt_tp_id;
	/*ref. event difinition */
	FTS_BYTE	bt_tp_property;
	/*the strength of the press*/
	FTS_WORD	w_tp_strenth;
	FTS_WORD	w_tp_area;
}ST_TOUCH_POINT, *PST_TOUCH_POINT;

typedef enum
{
	TYPE_Z00M_IN,
	TYPE_Z00M_OUT,
	TYPE_INVALIDE
}E_GESTURE_TYPE;

/*the information of one touch */
typedef struct
{
	/*the number of touch points*/
	FTS_BYTE			bt_tp_num;
	/*touch gesture*/
	E_GESTURE_TYPE		bt_gesture;
	/*point to a list which stored 1 to 5 touch points information*/
	ST_TOUCH_POINT		*pst_point_info;
}ST_TOUCH_INFO, *PST_TOUCH_INFO;

#define TP_MIN_GAP 3
static int pressure_support;
static int auto_update_fw;
static int fw_header_needed;
static BLOCKING_NOTIFIER_HEAD(touch_key_notifier);
static int focaltec_fts_ctpm_fw_upgrade(struct i2c_client *client, u8 *pbt_buf,
			  u32 dw_lenth);

static struct workqueue_struct *synaptics_wq;
static struct delayed_work ft6x06_firmware_update;
static struct wake_lock update_wake_lock;


static struct i2c_client *ft5306_i2c_client;
static ST_TOUCH_INFO ft5306_touch_info;
static ST_TOUCH_POINT ft5306_touch_point[FTS_MAX_FINGER];
static int min_gap = TP_MIN_GAP;
enum ft520x_ts_regs {
	FT520X_REG_THGROUP					= 0x80,
	FT520X_REG_THPEAK						= 0x81,
	FT520X_REG_THCAL						= 0x82,
	FT520X_REG_THWATER					= 0x83,
	FT520X_REG_THTEMP					= 0x84,
	FT520X_REG_THDIFF						= 0x85,
	FT520X_REG_CTRL						= 0x86,
	FT520X_REG_TIMEENTERMONITOR			= 0x87,
	FT520X_REG_PERIODACTIVE				= 0x88,
	FT520X_REG_PERIODMONITOR			= 0x89,
	FT520X_REG_HEIGHT_B					= 0x8a,
	FT520X_REG_MAX_FRAME					= 0x8b,
	FT520X_REG_DIST_MOVE					= 0x8c,
	FT520X_REG_DIST_POINT				= 0x8d,
	FT520X_REG_FEG_FRAME					= 0x8e,
	FT520X_REG_SINGLE_CLICK_OFFSET		= 0x8f,
	FT520X_REG_DOUBLE_CLICK_TIME_MIN	= 0x90,
	FT520X_REG_SINGLE_CLICK_TIME			= 0x91,
	FT520X_REG_LEFT_RIGHT_OFFSET		= 0x92,
	FT520X_REG_UP_DOWN_OFFSET			= 0x93,
	FT520X_REG_DISTANCE_LEFT_RIGHT		= 0x94,
	FT520X_REG_DISTANCE_UP_DOWN		= 0x95,
	FT520X_REG_ZOOM_DIS_SQR				= 0x96,
	FT520X_REG_RADIAN_VALUE				=0x97,
	FT520X_REG_MAX_X_HIGH				= 0x98,
	FT5X0X_REG_MAX_X_LOW					= 0x99,
	FT520X_REG_MAX_Y_HIGH					= 0x9a,
	FT520X_REG_MAX_Y_LOW					= 0x9b,
	FT520X_REG_K_X_HIGH				= 0x9c,
	FT520X_REG_K_X_LOW				= 0x9d,
	FT520X_REG_K_Y_HIGH				= 0x9e,
	FT520X_REG_K_Y_LOW				= 0x9f,
	FT520X_REG_AUTO_CLB_MODE			= 0xa0,
	FT520X_REG_LIB_VERSION_H				= 0xa1,
	FT520X_REG_LIB_VERSION_L				= 0xa2,
	FT520X_REG_CIPHER						= 0xa3,
	FT520X_REG_MODE						= 0xa4,
	FT520X_REG_PMODE	= 0xa5,	/* Power Consume Mode		*/
	FT520X_REG_FIRMID						= 0xa6,
	FT520X_REG_STATE						= 0xa7,
	FT520X_REG_FT5201ID					= 0xa8,
	FT520X_REG_ERR						= 0xa9,
	FT520X_REG_CLB						= 0xaa,
};

/*FT5X0X_REG_PMODE*/
#define PMODE_ACTIVE        0x00
#define PMODE_MONITOR       0x01
#define PMODE_STANDBY       0x02
#define PMODE_HIBERNATE     0x03

static int register_touch_key_notifier(struct notifier_block *n)
{
	return blocking_notifier_chain_register(&touch_key_notifier, n);
}

static int unregister_touch_key_notifier(struct notifier_block *n)
{
	return blocking_notifier_chain_unregister(&touch_key_notifier, n);
}

static int ft520x_i2c_rxdata(char *rxdata, int length)
{
	int ret;

	struct i2c_msg msgs[] = {
		{
			.addr	= ft5306_i2c_client->addr,
			.flags	= 0,
			.len	= 1,
			.buf	= rxdata,
		},
		{
			.addr	= ft5306_i2c_client->addr,
			.flags	= I2C_M_RD,
			.len	= length,
			.buf	= rxdata,
		},
	};

	ret = i2c_transfer(ft5306_i2c_client->adapter, msgs, 2);
	if (ret < 0)
		printk(KERN_ERR "msg %s i2c read error: %d\n", __func__, ret);

	return ret;
}

static int ft520x_read_reg(u8 addr, u8 *pdata)
{
	int ret;
	u8 buf[2] = {0};

	buf[0] = addr;
	struct i2c_msg msgs[] = {
		{
			.addr	= ft5306_i2c_client->addr,
			.flags	= 0,
			.len	= 1,
			.buf	= buf,
		},
		{
			.addr	= ft5306_i2c_client->addr,
			.flags	= I2C_M_RD,
			.len	= 1,
			.buf	= buf,
		},
	};

	ret = i2c_transfer(ft5306_i2c_client->adapter, msgs, 2);
	if (ret < 0)
		printk(KERN_ERR "msg %s i2c read error: %d\n", __func__, ret);

	*pdata = buf[0];
	return ret;

}

static int ft520x_i2c_txdata(char *txdata, int length)
{
	int ret;

	struct i2c_msg msg[] = {
		{
			.addr	= ft5306_i2c_client->addr,
			.flags	= 0,
			.len	= length,
			.buf	= txdata,
		},
	};

	ret = i2c_transfer(ft5306_i2c_client->adapter, msg, 1);
	if (ret < 0)
		printk(KERN_ERR "%s i2c write error: %d\n", __func__, ret);

	return ret;
}

static int ft520x_write_reg(u8 addr, u8 para)
{
	u8 buf[2];
	int ret = -1;

	buf[0] = addr;
	buf[1] = para;
	ret = ft520x_i2c_txdata(buf, 2);
	if (ret < 0) {
		printk(KERN_ERR "write reg failed! %#x ret:%d", buf[0], ret);
		return -1;
	}

	return 0;
}

static unsigned char ft520x_read_fw_ver(void)
{
	unsigned char ver=0xff;
	ft520x_read_reg(FT520X_REG_FIRMID, &ver);
	return ver;
}

static int ft520x_shutdown(void)
{
	int ret;
	ret = ft520x_write_reg(FT520X_REG_PMODE, PMODE_HIBERNATE);
	if (ret != 0)
		printk(" --- ft520x_shutdown -- error!\r\n");
	return ret;
}

#if 0
static int ft520x_turnon()
{
	int ret;
	ret = ft520x_write_reg(FT520X_REG_PMODE, PMODE_MONITOR);
	if (ret != 0)
		printk(KERN_INFO" --- ft520x_turnon -- error!\r\n");
	return ret;
}
#endif

#define CONFIG_SUPPORT_FTS_CTP_UPG

#ifdef CONFIG_SUPPORT_FTS_CTP_UPG

#define FT520X_FIREWARE_VERSION	0x12

typedef enum
{
	ERR_OK,
	ERR_MODE,
	ERR_READID,
	ERR_ERASE,
	ERR_STATUS,
	ERR_ECC,
	ERR_DL_ERASE_FAIL,
	ERR_DL_PROGRAM_FAIL,
	ERR_DL_VERIFY_FAIL
}E_UPGRADE_ERR_TYPE;

#define I2C_CTPM_ADDRESS       0x7E


void delay_qt_ms(unsigned long  w_ms)
{
	/*unsigned long i;
	unsigned long j;

	for (i = 0; i < w_ms; i++)
	{
		for (j = 0; j < 1000; j++)
		{
			udelay(1);
		}
	}*/
	msleep(w_ms);
}

FTS_BOOL i2c_read_interface(FTS_BYTE bt_ctpm_addr, FTS_BYTE* pbt_buf, FTS_DWRD dw_lenth)
{
	int ret;

	ret=i2c_master_recv(ft5306_i2c_client, pbt_buf, dw_lenth);

	if (ret <= 0) {
		printk("[FT520X]i2c_read_interface error\n");
		return FTS_FALSE;
	}

	return FTS_TRUE;
}

FTS_BOOL i2c_write_interface(FTS_BYTE bt_ctpm_addr, FTS_BYTE* pbt_buf, FTS_DWRD dw_lenth)
{
	int ret;
	ret=i2c_master_send(ft5306_i2c_client, pbt_buf, dw_lenth);
	if (ret <= 0) {
		printk("[FT520X]i2c_write_interface error line = %d, ret = %d\n", __LINE__, ret);
		return FTS_FALSE;
	}

	return FTS_TRUE;
}

FTS_BOOL cmd_write(FTS_BYTE btcmd,FTS_BYTE btPara1,FTS_BYTE btPara2,FTS_BYTE btPara3,FTS_BYTE num)
{
	FTS_BYTE write_cmd[4] = {0};

	write_cmd[0] = btcmd;
	write_cmd[1] = btPara1;
	write_cmd[2] = btPara2;
	write_cmd[3] = btPara3;
	return i2c_write_interface(I2C_CTPM_ADDRESS, write_cmd, num);
}

FTS_BOOL byte_write(FTS_BYTE* pbt_buf, FTS_DWRD dw_len)
{
	return i2c_write_interface(I2C_CTPM_ADDRESS, pbt_buf, dw_len);
}

FTS_BOOL byte_read(FTS_BYTE* pbt_buf, FTS_BYTE bt_len)
{
	return i2c_read_interface(I2C_CTPM_ADDRESS, pbt_buf, bt_len);
}

#define    FTS_PACKET_LENGTH        128

static unsigned char CTPM_FW[]=
{
#if defined(CONFIG_TOUCHSCREEN_HAWAII_GARNET_FT5306_FW)
	#include "45HD_FT5306_U82_LCID0x87_ver0x10_20130320_app.i"
#elif defined(CONFIG_TOUCHSCREEN_JAVA_AMETHYST_FT5336_FW)
	#include "Java_amethyst_050914_FT5336_20130910_V10_app.i"
#else
0
#endif
};

FTS_DWRD fts_ctpm_auto_clb(void)
{
	FTS_BYTE uc_temp;
	FTS_BYTE i ;
	printk(KERN_INFO"[FTS] start auto CLB.\n");
	delay_qt_ms(200);
	ft520x_write_reg(0, 0x40);
	/*make sure already enter factory mode*/
	delay_qt_ms(100);
	/*write command to start calibration */
	ft520x_write_reg(2, 0x4);
	delay_qt_ms(300);
	for (i = 0; i < 100; i++) {
		ft520x_read_reg(0, &uc_temp);
		if (((uc_temp&0x70)>>4) == 0x0)
			break;
		delay_qt_ms(200);
		printk(KERN_INFO"[FTS] waiting calibration %d\n", i);
	}
	printk(KERN_INFO"[FTS] calibration OK.\n");
	delay_qt_ms(300);
	/*goto factory mode*/
	ft520x_write_reg(0, 0x40);
	/*make sure already enter factory mode*/
	delay_qt_ms(100);
	/*store CLB result*/
	ft520x_write_reg(2, 0x5);
	delay_qt_ms(300);
	/*return to normal mode*/
	ft520x_write_reg(0, 0x0);
	delay_qt_ms(300);
	printk(KERN_INFO"[FTS] store CLB result OK.\n");
	return 0;
}


E_UPGRADE_ERR_TYPE  fts_ctpm_fw_upgrade(FTS_BYTE* pbt_buf, FTS_DWRD dw_lenth)
{
	FTS_BYTE reg_val[2] = {0};
	FTS_DWRD i = 0;

	FTS_DWRD  packet_number;
	FTS_DWRD  j;
	FTS_DWRD  temp;
	FTS_DWRD  lenght;
	FTS_BYTE  packet_buf[FTS_PACKET_LENGTH + 6];
	FTS_BYTE  auc_i2c_write_buf[10];
	FTS_BYTE bt_ecc;
	int      i_ret;

	/*write 0xaa to register 0xfc*/
	ft520x_write_reg(0xfc, 0xaa);
	delay_qt_ms(50);
	ft520x_write_reg(0xfc, 0x55);
	printk("[FT520X] Step 1: Reset CTPM test\n");

	delay_qt_ms(30);

	auc_i2c_write_buf[0] = 0x55;
	auc_i2c_write_buf[1] = 0xaa;
	do {
		i ++;
		i_ret = ft520x_i2c_txdata(auc_i2c_write_buf, 2);
		delay_qt_ms(5);
	} while (i_ret <= 0 && i < 5);

	cmd_write(0x90,0x00,0x00,0x00,4);
	byte_read(reg_val,2);
	if (reg_val[0] == 0x79 && reg_val[1] == 0x3)
		printk(KERN_INFO"[FT520X]  CTPM ID,ID1 = 0x%x,ID2 = 0x%x\n",
			reg_val[0], reg_val[1]);
	else {
		printk(KERN_INFO"[FT520X] Step 3:  readid error\n");
		return ERR_READID;
	}
	cmd_write(0x61, 0x00, 0x00, 0x00, 1);
	delay_qt_ms(1500);
	bt_ecc = 0;
	dw_lenth = dw_lenth - 8;
	packet_number = (dw_lenth) / FTS_PACKET_LENGTH;
	packet_buf[0] = 0xbf;
	packet_buf[1] = 0x00;
	for (j = 0; j < packet_number; j++) {
		temp = j * FTS_PACKET_LENGTH;
		packet_buf[2] = (FTS_BYTE)(temp>>8);
		packet_buf[3] = (FTS_BYTE)temp;
		lenght = FTS_PACKET_LENGTH;
		packet_buf[4] = (FTS_BYTE)(lenght>>8);
		packet_buf[5] = (FTS_BYTE)lenght;
		for (i = 0; i < FTS_PACKET_LENGTH; i++) {
			packet_buf[6+i] = pbt_buf[j*FTS_PACKET_LENGTH + i];
			bt_ecc ^= packet_buf[6+i];
		}

		byte_write(&packet_buf[0],FTS_PACKET_LENGTH + 6);
		delay_qt_ms(FTS_PACKET_LENGTH/6 + 1);
		if ((j * FTS_PACKET_LENGTH % 1024) == 0)
			printk(KERN_INFO"[FT520X] upgrade the 0x%x th byte.\n",
					((unsigned int)j) * FTS_PACKET_LENGTH);
	}
	if ((dw_lenth) % FTS_PACKET_LENGTH > 0) {
		temp = packet_number * FTS_PACKET_LENGTH;
		packet_buf[2] = (FTS_BYTE)(temp>>8);
		packet_buf[3] = (FTS_BYTE)temp;

		temp = (dw_lenth) % FTS_PACKET_LENGTH;
		packet_buf[4] = (FTS_BYTE)(temp>>8);
		packet_buf[5] = (FTS_BYTE)temp;

		for (i = 0; i < temp; i++) {
			packet_buf[6+i] = pbt_buf[packet_number*FTS_PACKET_LENGTH + i];
			bt_ecc ^= packet_buf[6+i];
		}
		byte_write(&packet_buf[0], temp+6);
		delay_qt_ms(20);
	}

	for (i = 0; i < 6; i++) {
		temp = 0x6ffa + i;
		packet_buf[2] = (FTS_BYTE)(temp>>8);
		packet_buf[3] = (FTS_BYTE)temp;
		temp =1;
		packet_buf[4] = (FTS_BYTE)(temp>>8);
		packet_buf[5] = (FTS_BYTE)temp;
		packet_buf[6] = pbt_buf[dw_lenth + i];
		bt_ecc ^= packet_buf[6];

		byte_write(&packet_buf[0], 7);
		delay_qt_ms(20);
	}
	/*send the opration head*/
	cmd_write(0xcc, 0x00, 0x00, 0x00, 1);
	byte_read(reg_val,1);
	if (reg_val[0] != bt_ecc) {
		printk(KERN_INFO"[FT520X]  ecc error\n");
		return ERR_ECC;
	}
	cmd_write(0x07, 0x00, 0x00, 0x00, 1);
	delay_qt_ms(300);
	return ERR_OK;
}

int focaltec_fts_ctpm_fw_upgrade_with_i_file(void)
{
	FTS_BYTE*     pbt_buf = FTS_NULL;
	int i_ret;

	//=========FW upgrade========================*/
	pbt_buf = CTPM_FW;
	/*call the upgrade function*/
	i_ret =  focaltec_fts_ctpm_fw_upgrade(ft5306_i2c_client, pbt_buf, sizeof(CTPM_FW));
	if (i_ret != 0) {
		printk(KERN_ERR "FT6x06 upgrade firmware failed! \r\n");
		return i_ret;
	} else {
		if (NEED_CALIBRATION)
			fts_ctpm_auto_clb();
		printk(KERN_INFO "[FT6x06] upgrade successfully.\n");
	}
	return i_ret;
}

unsigned char fts_ctpm_get_upg_ver(void)
{
	unsigned int ui_sz;
	ui_sz = sizeof(CTPM_FW);
	if (ui_sz > 2)
		return CTPM_FW[ui_sz - 2];
	else
		return 0xff;
}

unsigned char fts_ctpm_get_vendor_id(void)
{
	u8 vendor_id = 0;
	ft520x_read_reg(0xA8, &vendor_id);
	printk("tp vendor id: %x", vendor_id);
	return vendor_id;
}
unsigned char fts_ctpm_get_chip_id(void)
{
	u8 chip_id = 0;
	ft520x_read_reg(0xA3, &chip_id);
	printk("Focaltech tp chip id: %x", chip_id);
	return chip_id;
}

#endif

int focaltec_fts_upgrade_firmware(void)
{
	unsigned char ver = 0;
	if (ft520x_read_fw_ver() < fts_ctpm_get_upg_ver()) {
		ver = ft520x_read_fw_ver();
		printk(KERN_INFO "ft6x06 upgrading fw... before upgrd ver: %x\n", ver);
		focaltec_fts_ctpm_fw_upgrade_with_i_file();
		ver = ft520x_read_fw_ver();
		printk(KERN_INFO "ft6x06 upgrading fw... after upgrd ver: %x\n", ver);
	} else
		printk(KERN_INFO "No need to upgrade firmware\n");

	return 1;
}


FTS_BYTE bt_parser_std(FTS_BYTE* pbt_buf, FTS_BYTE bt_len, ST_TOUCH_INFO* pst_touch_info)
{
	FTS_WORD low_byte	= 0;
	FTS_WORD high_byte	= 0;
	FTS_BYTE point_num	= 0;
	FTS_BYTE i			= 0;

	/*check the pointer*/

	/*check the length of the protocol data*/
	if(bt_len < PROTOCOL_LEN) {
		return CTPM_ERR_PARAMETER;
	}
	pst_touch_info->bt_tp_num= 0;

	/* Device Mode[2:0] == 0 :Normal operating Mode*/
	if ((pbt_buf[0] & 0x70) != 0) {
		printk(KERN_ERR "[tp] mode: %x", pbt_buf[0]);
		return CTPM_ERR_PROTOCOL;
	}

	/*get the Gesture ID*/
	pst_touch_info->bt_gesture = pbt_buf[1];

	/*get the number of the touch points*/
	point_num = pbt_buf[2] & 0x0f;
	if(point_num == 0 || point_num > CTPM_STD_POINTS_MAX) {
		return CTPM_ERR_PROTOCOL;
	}

	/*remove the touch point information into pst_touch_info.*/
	for(i = 0; i < point_num; i++) {
		/*get the X coordinate, 2 bytes*/
		pst_touch_info->pst_point_info[i].bt_tp_property = pbt_buf[3+6*i] >> 6;

		high_byte = pbt_buf[3+6*i];
		high_byte <<= 8;
		high_byte &= 0x0f00;
		low_byte = pbt_buf[3+6*i + 1];
		pst_touch_info->pst_point_info[i].w_tp_x = high_byte |low_byte;

		/*get the Y coordinate, 2 bytes*/
		pst_touch_info->pst_point_info[i].bt_tp_id = pbt_buf[3+6*i+2] >> 4;

		high_byte = pbt_buf[3+6*i+2];
		high_byte <<= 8;
		high_byte &= 0x0f00;
		low_byte = pbt_buf[3+6*i+3];
		pst_touch_info->pst_point_info[i].w_tp_y = high_byte |low_byte;
		if (pressure_support) {
			pst_touch_info->pst_point_info[i].w_tp_strenth=pbt_buf[7+6*i];
			pst_touch_info->pst_point_info[i].w_tp_area=pbt_buf[8+6*i];
		}

		pst_touch_info->bt_tp_num++;
	}

	return CTPM_NOERROR;
}

/*
[function]:
	get all the information of one touch.
[parameters]:
	pst_touch_info[out] :stored all the information of one touch;
[return]:
	CTPM_NOERROR		:success;
	CTPM_ERR_I2C		:io fail;
	CTPM_ERR_PROTOCOL	:protocol data error;
	CTPM_ERR_ECC		:ecc error.
*/
void DumpFtsRegContext(FTS_BYTE *buf, int size)
{
	int i;

	for (i = 0; i < size; i += 3)
		printk(KERN_INFO "Addr: %x: %x, %x, %x\n", i, *(buf+i),
						*(buf+i+1), *(buf+i+2));
}

FTS_BYTE fts_ctpm_get_touch_info(struct synaptics_rmi4 *ts, ST_TOUCH_INFO* pst_touch_info)
{
	FTS_BYTE *p_data_buf;
	FTS_BYTE data_buf[33] = {0};

	POINTER_CHECK(pst_touch_info);
	POINTER_CHECK(pst_touch_info->pst_point_info);

	p_data_buf = &data_buf[0];

	/*Sent device to active*/
	ft5306_i2c_client = ts->client;

	memset(data_buf, 0x00, PROTOCOL_LEN);
	pst_touch_info->bt_tp_num = 0;

	/*get the touch point information*/
	/*if(!i2c_read_interface(0x00, p_data_buf, PROTOCOL_LEN)) {
		return CTPM_ERR_I2C;
	}*/
	if (ft520x_i2c_rxdata(p_data_buf, PROTOCOL_LEN) < 0) {
		printk(KERN_ERR "[tp] read axis err...\n");
		return CTPM_ERR_I2C;
	}

	/*parse the data read out from ctpm and put the touch point information into pst_touch_info*/
	return bt_parser_std(p_data_buf, PROTOCOL_LEN, pst_touch_info);
}

static void Ft5306_Enter_Sleep(void)
{
#if (TP_CNTRL_PIN_TYPE == TP_CNTRL_PIN_RESET)
	ft520x_shutdown();
#elif (TP_CNTRL_PIN_TYPE == TP_CNTRL_PIN_WAKEUP)
	ft520x_shutdown();
#else
#error NO TP CNTRL TYPE SPECIFIED!!!
#endif
}

static void Ft5306_Exit_Sleep(void)
{
#if (TP_CNTRL_PIN_TYPE == TP_CNTRL_PIN_WAKEUP)
	gpio_request(ts_gpio_wakeup_pin, "tp_wakeup");
	gpio_direction_output(ts_gpio_wakeup_pin, 0);
	gpio_set_value(ts_gpio_wakeup_pin, 0);
	mdelay(5);
	gpio_set_value(ts_gpio_wakeup_pin, 1);
	mdelay(20);
	gpio_free(ts_gpio_wakeup_pin);
#endif
}

#if (TP_CNTRL_PIN_TYPE == TP_CNTRL_PIN_RESET)
void Ft5306_Hw_Reset(void)
{
	gpio_request(ts_gpio_reset_pin, "tp_reset");
	gpio_direction_output(ts_gpio_reset_pin, 1);
	gpio_set_value(ts_gpio_reset_pin, 1);
	mdelay(5);
	gpio_set_value(ts_gpio_reset_pin, 0);
	mdelay(5);
	gpio_set_value(ts_gpio_reset_pin, 1);
	gpio_free(ts_gpio_reset_pin);
	mdelay(5);
}
#endif

/* define in platform/board file(s) */
static const struct i2c_device_id focaltech_ft5306_id[]=
{
		{"FocalTech-Ft5306", 0},
		{},

};

static int focaltech_ft5306_turnOn(struct i2c_client *client);
static int focaltech_ft5306_turnOff(struct i2c_client *client);

#ifdef CONFIG_HAS_EARLYSUSPEND
static void focaltech_ft5306_early_suspend(struct early_suspend *h);
static void focaltech_ft5306_late_resume(struct early_suspend *h);
#endif
#define FINGER_DOWN 1
#define FINGER_HOLD 2
#define FINGER_STALE 3
#define FINGER_UP 4
#define FINGER_IDLE 5

#define POINT_HISTORY_DEPTH 2
struct Pointer{
	uint8_t state;
	int x,y;
	int pressure;
	int area;
} ;

struct FingersQueue{
	int points_num;
	struct Pointer points[FTS_MAX_FINGER];
} ;

static struct FingersQueue g_CurFingers[POINT_HISTORY_DEPTH];
static int g_CurFingerInfoSlotId = 0;

#define Switch_Finger_Slot() (g_CurFingerInfoSlotId = (g_CurFingerInfoSlotId+1 < POINT_HISTORY_DEPTH) ? (g_CurFingerInfoSlotId+1) : (0))
#define Finger_Last_Slot() ((g_CurFingerInfoSlotId < 1) ? (POINT_HISTORY_DEPTH -1) : (g_CurFingerInfoSlotId - 1))
#define Finger_Cur_Slot() (g_CurFingerInfoSlotId)
void InitFingersQueue(void)
{
	int i,j;

	for (i = 0; i < FTS_MAX_FINGER; i++) {
		for (j = 0; j < POINT_HISTORY_DEPTH; j++) {
			g_CurFingers[j].points_num = 0;
			g_CurFingers[j].points[i].x = 0;
			g_CurFingers[j].points[i].y = 0;
			g_CurFingers[j].points[i].state = FINGER_IDLE;
		}
	}
	g_CurFingerInfoSlotId = 0;
}

void InvalidateFingerSlot(struct FingersQueue *slot)
{
	int i;
	slot->points_num = 0;
	for (i = 0; i < FTS_MAX_FINGER; i++) {
		slot->points[i].state = FINGER_UP;
		/*slot->points[i].x = 0;
		slot->points[i].y = 0;*/
	}
}

void UpdateFingerQueue(ST_TOUCH_INFO *touch_info)
{
	int i;
	struct FingersQueue *pFinger;

	if (touch_info->bt_tp_num > FTS_MAX_FINGER){
		printk(KERN_INFO "[tp err]: pt num %d exceed %d", touch_info->bt_tp_num, FTS_MAX_FINGER);
	}
	Switch_Finger_Slot();
	pFinger = &g_CurFingers[Finger_Cur_Slot()];
	InvalidateFingerSlot(pFinger);
	pFinger->points_num = touch_info->bt_tp_num;
	//printk(KERN_INFO "[tp] pnum=%d slot=%d", pFinger->points_num, Finger_Cur_Slot());
	for (i = 0; i < pFinger->points_num; i++) {
		if (touch_info->pst_point_info[i].bt_tp_id >= FTS_MAX_FINGER) {
			printk(KERN_INFO "[tp err]: invalid pt id %d", touch_info->pst_point_info[i].bt_tp_id);
			pFinger->points_num --;//Invalid point remove  it
			continue;
		}
		//printk(KERN_INFO "[tp] p%d detected x=%d y=%d\n", touch_info->pst_point_info[i].bt_tp_id, touch_info->pst_point_info[i].w_tp_x, touch_info->pst_point_info[i].w_tp_y);
		switch (touch_info->pst_point_info[i].bt_tp_property) {
			case TC_PutDown:
				pFinger->points[touch_info->pst_point_info[i].bt_tp_id].state = FINGER_DOWN;
				break;
			case TC_Contact:
				pFinger->points[touch_info->pst_point_info[i].bt_tp_id].state = FINGER_HOLD;
				break;
			case TC_PutUp:
				pFinger->points[touch_info->pst_point_info[i].bt_tp_id].state = FINGER_UP;
				break;
			default:
				pFinger->points[touch_info->pst_point_info[i].bt_tp_id].state = FINGER_IDLE;
				break;
		}
		pFinger->points[touch_info->pst_point_info[i].bt_tp_id].x = touch_info->pst_point_info[i].w_tp_x;
		pFinger->points[touch_info->pst_point_info[i].bt_tp_id].y = touch_info->pst_point_info[i].w_tp_y;
		if (pressure_support) {
			pFinger->points[touch_info->pst_point_info[i].bt_tp_id].pressure = touch_info->pst_point_info[i].w_tp_strenth;
			pFinger->points[touch_info->pst_point_info[i].bt_tp_id].area = touch_info->pst_point_info[i].w_tp_area;
		}
	}
}

void ReportFingers(struct synaptics_rmi4 *ts, ST_TOUCH_INFO *touch_info)
{
	struct FingersQueue * cur_info, *last_info;
	int i, j, reported=0, active_finers = 0;

	if (!touch_info) {
		printk(KERN_INFO "[tp err]: input touch data null");
		return;
	}

	UpdateFingerQueue(touch_info);
	cur_info = &g_CurFingers[Finger_Cur_Slot()];
	last_info = &g_CurFingers[Finger_Last_Slot()];
	for (i = 0; i < FTS_MAX_FINGER; i++) {
		if (cur_info->points[i].state < FINGER_STALE) {
			if (cur_info->points[i].y > ts_y_max_value)
				blocking_notifier_call_chain(
						&touch_key_notifier, 0, NULL);

			input_report_abs(ts->input_dev, ABS_MT_POSITION_X, cur_info->points[i].x);
			input_report_abs(ts->input_dev, ABS_MT_POSITION_Y, cur_info->points[i].y);
			if (pressure_support) {
				input_report_abs(ts->input_dev, ABS_MT_TOUCH_MAJOR, cur_info->points[i].area);
				input_report_abs(ts->input_dev, ABS_MT_PRESSURE, cur_info->points[i].pressure);
			}else
				input_report_abs(ts->input_dev, ABS_MT_TOUCH_MAJOR, 8);
			input_report_key(ts->input_dev, BTN_TOUCH, 1);
			//input_report_abs(ts->input_dev, ABS_MT_WIDTH_MAJOR, 1);
			input_mt_sync(ts->input_dev);
			reported = 1;
			active_finers ++;
			//printk(KERN_INFO "[tp]: p(%d) x=%d y=%d\n", i, cur_info->points[i].x, cur_info->points[i].y);
		} else if (cur_info->points[i].state == FINGER_UP && last_info->points[i].state < FINGER_UP) {
			cur_info->points[i].state = FINGER_STALE;//stale points
			/*if ((last_info->points_num == 1) && (cur_info->points_num == 1)){
				cur_info->points[i].state = FINGER_UP;
				input_report_abs(ts->input_dev, ABS_MT_TOUCH_MAJOR, 0);
				input_report_abs(ts->input_dev, ABS_MT_WIDTH_MAJOR, 0);
				input_mt_sync(ts->input_dev);
				reported = 1;
				active_finers ++;
				printk(KERN_INFO "[tp]: single p(%d) up\n", i);
			}else*/
			//printk(KERN_INFO "[tp]: stale p(%d)\n", i);
		}
	}
	if (active_finers){
		if (reported)
			input_sync(ts->input_dev);
		//queue_work(synaptics_wq, &ts->work);
		/*if (ts->use_irq) //Using int trigger can make the tp points more smooth
			enable_irq(ts->client->irq);*/
	} else {
		//printk(KERN_INFO "[tp]: no finger down\n");
		for (i = 0; i < FTS_MAX_FINGER; i++) {
			if ((cur_info->points[i].state >= FINGER_STALE) && (last_info->points[i].state < FINGER_UP)) {
				//input_report_abs(ts->input_dev, ABS_MT_TOUCH_MAJOR, 0);
				input_report_key(ts->input_dev, BTN_TOUCH, 0);
				input_mt_sync(ts->input_dev);
				reported = 1;
				//printk(KERN_INFO "[tp]: p(%d) up\n", i);
			}

			for (j = 0; j < POINT_HISTORY_DEPTH; j++) {
				//InvalidateFingerSlot(g_CurFingers[j].points[i].state == FINGER_IDLE);
				g_CurFingers[j].points[i].state = FINGER_IDLE;
			}
		}
		if (reported)
			input_sync(ts->input_dev);

		/*if (ts->use_irq)
				enable_irq(ts->client->irq);*/
	}
}

#if 0
static void DebugTpStatus(void)
{
	int i;

	printk(KERN_INFO "point Num: %d, gesture: %d", ft5306_touch_info.bt_tp_num, ft5306_touch_info.bt_gesture);
	for (i = 0; i < ft5306_touch_info.bt_tp_num; i++) {
		printk(KERN_INFO "P[%d] property: %d, x: %d, y:%d", ft5306_touch_info.pst_point_info[i].bt_tp_id,
			                                                 ft5306_touch_info.pst_point_info[i].bt_tp_property,
			                                                 ft5306_touch_info.pst_point_info[i].w_tp_x,
			                                                 ft5306_touch_info.pst_point_info[i].w_tp_y);
	}
}
#endif

#define UNEXPECT_POINT_MAX_TEST 5
static void focaltech_ft5306_work_func(struct work_struct *work)
{
	int ret = 0;
	struct synaptics_rmi4 *ts = container_of(work,
					struct synaptics_rmi4, work);

	ft5306_touch_info.pst_point_info = ft5306_touch_point;
	ret = fts_ctpm_get_touch_info(ts, &ft5306_touch_info);
	//printk(KERN_INFO "get_touch_info ret: %x", ret);
	if (ret == CTPM_ERR_I2C) {
		ft5306_i2c_client = ts->client;
		focaltech_ft5306_turnOff(ft5306_i2c_client);
		mdelay(5);
		focaltech_ft5306_turnOn(ft5306_i2c_client);
	} else {
		ReportFingers(ts, &ft5306_touch_info);
	}
	//ReportFingers(ts, &ft5306_touch_info);
}

irqreturn_t focaltech_ft5306_irq_handler(int irq, void *dev_id)
{
	struct synaptics_rmi4 *ts = dev_id;
	queue_work(synaptics_wq, &ts->work);

	return IRQ_HANDLED;
}

static void focaltech_ft5306_enable(struct synaptics_rmi4 *ts)
{
	printk("%s: %s()\n", ts->client->name, __func__);

	if (ts->use_irq)
		enable_irq(ts->client->irq);

	ts->enable = 1;
}

static void focaltech_ft5306_disable(struct synaptics_rmi4 *ts)
{
	printk("%s: %s()\n", ts->client->name, __func__);

	if (ts->use_irq)
		disable_irq_nosync(ts->client->irq);

	ts->enable = 0;
}

static int focaltech_ft5306_turnOff(struct i2c_client *client)
{
	struct synaptics_rmi4 *ts;

	if (!client) return -1;

	ts = i2c_get_clientdata(client);
	focaltech_ft5306_disable(ts);
	Ft5306_Enter_Sleep();
	mdelay(10);
	if (ts->power) {
		 ts->power(TS_OFF);
	}
	return 0;
}

static int focaltech_ft5306_turnOn(struct i2c_client *client)
{
	struct synaptics_rmi4 *ts;

	if (!client) return -1;

	ts = i2c_get_clientdata(client);

	Ft5306_Exit_Sleep();
	mdelay(20);
	if (ts->power) {
		 ts->power(TS_ON);
	}
#if (TP_CNTRL_PIN_TYPE == TP_CNTRL_PIN_RESET)
	Ft5306_Hw_Reset();
#endif
	mdelay(50);
	focaltech_ft5306_enable(ts);

	return 0;
}

static ssize_t focaltech_ft5306_enable_show(struct device *dev,
                                         struct device_attribute *attr, char *buf)
{
	struct synaptics_rmi4 *ts = dev_get_drvdata(dev);

	return sprintf(buf, "%u\n", ts->enable);
}

static ssize_t focaltech_ft5306_enable_store(struct device *dev,
                                          struct device_attribute *attr,
                                          const char *buf, size_t count)
{
	struct synaptics_rmi4 *ts = dev_get_drvdata(dev);
	unsigned long val;
	int error;

	error = strict_strtoul(buf, 10, &val);

	if (error)
		return error;

	val = !!val;

	if (val != ts->enable) {
		if (val)
			focaltech_ft5306_enable(ts);
		else
			focaltech_ft5306_disable(ts);
	}

	return count;
}

//DEV_ATTR(synaptics_rmi4, enable, 0664);
static DEVICE_ATTR(enable, 0664, focaltech_ft5306_enable_show, focaltech_ft5306_enable_store);
static ssize_t ft5306_min_gap_show(struct kobject *kobj,
				   struct kobj_attribute *attr, char *buf)
{
	return sprintf(buf, "min_gap: %d\n", min_gap);
}

ssize_t ft5306_min_gap_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	long set = 0;
	if ((strict_strtol(buf, 0, &set)) < 0)
		printk(KERN_WARNING "gap_store:error strict_strtol\n");

	min_gap = set ;
	return count;
}

static struct kobj_attribute ft5306_min_gap_attr = {
	.attr = {
		.name = "min_gap",
		.mode = 0644,
	},
	.show = &ft5306_min_gap_show,
	.store = &ft5306_min_gap_store,
};

static ssize_t ft5306_sensitivity_show(struct kobject *kobj,
				   struct kobj_attribute *attr, char *buf)
{
	uint8_t sensitivity = 0;
	ft520x_read_reg(0x88, &sensitivity);
	return sprintf(buf, "sensitivity: %d\n", sensitivity);
}

ssize_t ft5306_sensitivity_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	uint8_t sensitivity = 8;
	long set = 0;

	if ((strict_strtol(buf, 0, &set)) < 0)
		printk(KERN_WARNING "senstivity:error strict_strtol\n");

	if ( (set >= 4) && (set <= 8) ) {
		sensitivity = set ;
	}
	printk(KERN_INFO "ft5306 change sensitivity to: %d", sensitivity);
	ft520x_write_reg(0x88, sensitivity);

	return count;
}

static struct kobj_attribute ft5306_sensitivity_attr = {
	.attr = {
		.name = "sensitivity",
		.mode = 0644,
	},
	.show = &ft5306_sensitivity_show,
	.store = &ft5306_sensitivity_store,
};

int ft6x06_i2c_Read(struct i2c_client *client, char *writebuf,
		    int writelen, char *readbuf, int readlen)
{
	int ret;
	client = ft5306_i2c_client;

	if (writelen > 0) {
		struct i2c_msg msgs[] = {
			{
			 .addr = client->addr,
			 .flags = 0,
			 .len = writelen,
			 .buf = writebuf,
			 },
			{
			 .addr = client->addr,
			 .flags = I2C_M_RD,
			 .len = readlen,
			 .buf = readbuf,
			 },
		};
		ret = i2c_transfer(client->adapter, msgs, 2);
		if (ret < 0)
			dev_err(&client->dev, "f%s: i2c read error.\n",
				__func__);
	} else {
		struct i2c_msg msgs[] = {
			{
			 .addr = client->addr,
			 .flags = I2C_M_RD,
			 .len = readlen,
			 .buf = readbuf,
			 },
		};
		ret = i2c_transfer(client->adapter, msgs, 1);
		if (ret < 0)
			dev_err(&client->dev, "%s:i2c read error.\n", __func__);
	}
	return ret;
}
/*write data by i2c*/
int ft6x06_i2c_Write(struct i2c_client *client, char *writebuf, int writelen)
{
	int  ret;
	ret = ft520x_i2c_txdata(writebuf, writelen);
	return ret;
}

/*
*get firmware size

@firmware_name:firmware name
*note:the firmware default path is sdcard.
	if you want to change the dir, please modify by yourself.
*/
static int ft6x06_GetFirmwareSize(char *firmware_name)
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
static int ft6x06_ReadFirmware(char *firmware_name,
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

static int focaltec_fts_ctpm_fw_upgrade(struct i2c_client *client, u8 *pbt_buf,
			  u32 dw_lenth)
{
	u8 reg_val[2] = {0};
	u32 i = 0;
	u32 packet_number;
	u32 j = 0;
	u32 temp;
	u32 lenght;
	u8 packet_buf[FTS_PACKET_LENGTH + 6];
	u8 auc_i2c_write_buf[10];
	u8 bt_ecc;
	int i_ret;

	if (dw_lenth < 5) {
		printk(KERN_ERR "firmware size is too short, invalid.\n");
		return -1;
	}
	if (!pbt_buf) {
		printk(KERN_ERR "firmware buffer is null!\n");
		return -1;
	}

	for (i = 0; i < FTS_UPGRADE_LOOP; i++) {
		/*********Step 1:Reset  CTPM *****/
		/*write 0xaa to register 0xbc */
		ft520x_write_reg(FTS_UPGRADE_REG, FT_UPGRADE_AA);
		msleep(FT6X06_UPGRADE_AA_DELAY);

		/*write 0x55 to register 0xbc */
		ft520x_write_reg(FTS_UPGRADE_REG, FT_UPGRADE_55);

		msleep(FT6X06_UPGRADE_55_DELAY + i*5);

		/*********Step 2:Enter upgrade mode *****/
		auc_i2c_write_buf[0] = FT_UPGRADE_55;
		auc_i2c_write_buf[1] = FT_UPGRADE_AA;
		do {
			j++;
			auc_i2c_write_buf[0] = FT_UPGRADE_55;
			i_ret = ft6x06_i2c_Write(client, auc_i2c_write_buf, 1);
			msleep(5);
			auc_i2c_write_buf[0] = FT_UPGRADE_AA;
			i_ret = ft6x06_i2c_Write(client, auc_i2c_write_buf, 1);
			msleep(5);
		} while (i_ret <= 0 && j < 5);


		/*********Step 3:check READ-ID***********************/
		msleep(FT6X06_UPGRADE_READID_DELAY);
		auc_i2c_write_buf[0] = 0x90;
		auc_i2c_write_buf[1] = 0x00;
		auc_i2c_write_buf[2] = 0x00;
		auc_i2c_write_buf[3] = 0x00;
		ft6x06_i2c_Read(client, auc_i2c_write_buf, 4, reg_val, 2);


		if (reg_val[0] == FTS_UPGRADE_ID_1
			&& reg_val[1] == FTS_UPGRADE_ID_2) {
			DBG("[FTS] Step 3: CTPM ID,ID1 = 0x%x,ID2 = 0x%x\n",
				reg_val[0], reg_val[1]);
			break;
		} else {
			dev_err(&client->dev, "[FTS] Step 3: Retry---CTPM ID,ID1 = 0x%x,ID2 = 0x%x\n",
				reg_val[0], reg_val[1]);
		}
	}
	if (i > FTS_UPGRADE_LOOP)
		return -EIO;
	auc_i2c_write_buf[0] = 0xcd;

	ft6x06_i2c_Read(client, auc_i2c_write_buf, 1, reg_val, 1);


	/*Step 4:erase app and panel paramenter area*/
	DBG("Step 4:erase app and panel paramenter area\n");
	auc_i2c_write_buf[0] = 0x61;
	ft6x06_i2c_Write(client, auc_i2c_write_buf, 1);	/*erase app area */
	msleep(FT6X06_UPGRADE_EARSE_DELAY);
	/*erase panel parameter area */
	auc_i2c_write_buf[0] = 0x63;
	ft6x06_i2c_Write(client, auc_i2c_write_buf, 1);
	msleep(100);

	/*********Step 5:write firmware(FW) to ctpm flash*********/
	bt_ecc = 0;
	DBG("Step 5:write firmware(FW) to ctpm flash\n");

	dw_lenth = dw_lenth - 8;
	packet_number = (dw_lenth) / FTS_PACKET_LENGTH;
	packet_buf[0] = 0xbf;
	packet_buf[1] = 0x00;

	for (j = 0; j < packet_number; j++) {
		temp = j * FTS_PACKET_LENGTH;
		packet_buf[2] = (u8) (temp >> 8);
		packet_buf[3] = (u8) temp;
		lenght = FTS_PACKET_LENGTH;
		packet_buf[4] = (u8) (lenght >> 8);
		packet_buf[5] = (u8) lenght;

		for (i = 0; i < FTS_PACKET_LENGTH; i++) {
			packet_buf[6 + i] = pbt_buf[j *	FTS_PACKET_LENGTH + i];
			bt_ecc ^= packet_buf[6 + i];
		}
		ft6x06_i2c_Write(client, packet_buf, FTS_PACKET_LENGTH + 6);
		msleep(FTS_PACKET_LENGTH / 6 + 1);
	}

	if ((dw_lenth) % FTS_PACKET_LENGTH > 0) {
		temp = packet_number * FTS_PACKET_LENGTH;
		packet_buf[2] = (u8) (temp >> 8);
		packet_buf[3] = (u8) temp;
		temp = (dw_lenth) % FTS_PACKET_LENGTH;
		packet_buf[4] = (u8) (temp >> 8);
		packet_buf[5] = (u8) temp;

		for (i = 0; i < temp; i++) {
			packet_buf[6 + i] = pbt_buf[packet_number * FTS_PACKET_LENGTH + i];
			bt_ecc ^= packet_buf[6 + i];
		}

		ft6x06_i2c_Write(client, packet_buf, temp + 6);
		msleep(20);
	}


	/*send the last six byte */
	for (i = 0; i < 6; i++) {
		temp = 0x6ffa + i;
		packet_buf[2] = (u8) (temp >> 8);
		packet_buf[3] = (u8) temp;
		temp = 1;
		packet_buf[4] = (u8) (temp >> 8);
		packet_buf[5] = (u8) temp;
		packet_buf[6] = pbt_buf[dw_lenth + i];
		bt_ecc ^= packet_buf[6];
		ft6x06_i2c_Write(client, packet_buf, 7);
		msleep(20);
	}


	/*********Step 6: read out checksum***********************/
	/*send the opration head */
	DBG("Step 6: read out checksum\n");
	auc_i2c_write_buf[0] = 0xcc;
	ft6x06_i2c_Read(client, auc_i2c_write_buf, 1, reg_val, 1);
	if (reg_val[0] != bt_ecc) {
		dev_err(&client->dev, "[FTS]--ecc error! FW=%02x bt_ecc=%02x\n",
					reg_val[0],
					bt_ecc);
		return -EIO;
	}

	/*********Step 7: reset the new FW***********************/
	DBG("Step 7: reset the new FW\n");
	auc_i2c_write_buf[0] = 0x07;
	ft6x06_i2c_Write(client, auc_i2c_write_buf, 1);
	msleep(300);	/*make sure CTP startup normally */

	return 0;
}

static int update_complete = 1;
static void focaltech_fts_firmware_update_func(void)
{
	update_complete = 0;
	focaltec_fts_upgrade_firmware();
	wake_unlock(&update_wake_lock);
	update_complete = 1;
}

/*
upgrade with *.bin file with full path firmware_name
*/

int fts_ctpm_fw_upgrade_with_app_file(struct i2c_client *client,
				       char *firmware_name)
{
	u8 *pbt_buf = NULL;
	int i_ret;
	int fw_ver;
	int length;
	u8  pshare[100] = {0};
	char *fw_str = NULL;
	int fwsize = ft6x06_GetFirmwareSize(firmware_name);

	if (fwsize <= 0) {
		dev_err(&client->dev, "%s ERROR:Get firmware size failed\n",
					__func__);
		return -EIO;
	}

	if (fwsize < 8 || fwsize > 32 * 1024) {
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
	if (ft6x06_ReadFirmware(firmware_name, pbt_buf)) {
		dev_err(&client->dev, "%s() - ERROR: request_firmware failed\n",
					__func__);
		vfree(pbt_buf);
		return -EIO;
	}
	if (fw_header_needed) {
		/*parse the firmware file to ensure right firmware for special project*/
		memcpy(pshare, pbt_buf, 5);/*fetch the first 5 bytes*/
		pshare[5] = '\0';
		if (strcmp(pshare, "touch") != 0) {
			printk(KERN_ERR "Invalid firmware head, please pack it first.\n");
			vfree(pbt_buf);
			return -90;
		}
		memset(pshare, 0, 100);
		memcpy(pshare, pbt_buf+5, 4);/*fetch the key string length*/
		length = *((unsigned *)pshare);
		memset(pshare, 0, 100);
		memcpy(pshare, pbt_buf+5+4, length);/*fetch the key string*/
		pshare[length] = '\0';
		if (strcmp(pshare, firmware_str) != 0) {
			printk(KERN_ERR "wrong firmware, stop update.\n");
			vfree(pbt_buf);
			return -100;
		}
		printk(KERN_INFO "Valid firmare, going to update.\n");
		/*call the upgrade function */
		i_ret = focaltec_fts_ctpm_fw_upgrade(client, pbt_buf+5+4+length, fwsize-5-4-length);
	} else/*call the upgrade function */
		i_ret = focaltec_fts_ctpm_fw_upgrade(client, pbt_buf, fwsize);

	if (i_ret != 0)
		dev_err(&client->dev, "%s() - ERROR:[FTS] upgrade failed..\n",
					__func__);
	else {
		if (NEED_CALIBRATION)
			fts_ctpm_auto_clb();
		printk(KERN_INFO "[FTS] upgrade successfully.\n");
	}
	vfree(pbt_buf);
	return i_ret;
}

static ssize_t ft6x06_fwupgradeapp_show(struct kobject *kobj,
				   struct kobj_attribute *attr, char *buf)
{
	unsigned char ver = 0;
	ver = ft520x_read_fw_ver();
	printk(KERN_INFO "ft6x06 fw ver show: %x\n", ver);
	return sprintf(buf, "fw version: %x\n", ver);
}


/*upgrade from app.bin*/
ssize_t ft6x06_fwupgradeapp_store(struct kobject *kobj,
		struct kobj_attribute *attr, const char *buf, size_t count)
{
	char fwname[256];

	struct i2c_client *client = ft5306_i2c_client;

	wake_lock(&update_wake_lock);
	memset(fwname, 0, sizeof(fwname));
	sprintf(fwname, "%s", buf);
	fwname[count - 1] = '\0';
	disable_irq(client->irq);
	fts_ctpm_fw_upgrade_with_app_file(client, fwname);
	enable_irq(client->irq);
	wake_unlock(&update_wake_lock);
	return count;
}

static struct kobj_attribute ft6x06_upgrade_app_attr = {
	.attr = {
		.name = "upgradeapp",
		.mode = 0644,
	},
	.show = &ft6x06_fwupgradeapp_show,
	.store = &ft6x06_fwupgradeapp_store,
};

static ssize_t ft5306_upgrade_fw_show(struct kobject *kobj,
				   struct kobj_attribute *attr, char *buf)
{
	unsigned char ver = 0;
	ver = ft520x_read_fw_ver();
	printk(KERN_INFO "ft5306 fw ver show: %x\n", ver);
	return sprintf(buf, "fw version: %x\n", ver);
}

ssize_t ft5306_upgrade_fw_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	unsigned char ver = 0;
	ver = ft520x_read_fw_ver();
	printk(KERN_INFO "ft5306 upgrading fw... before upgrd ver: %x\n", ver);
	focaltec_fts_ctpm_fw_upgrade_with_i_file();
	ver = ft520x_read_fw_ver();
	printk(KERN_INFO "ft5306 upgrading fw... after upgrd ver: %x\n", ver);
	return count;
}

static struct kobj_attribute ft5306_upgrade_fw_attr = {
	.attr = {
		.name = "upgradefw",
		.mode = 0644,
	},
	.show = &ft5306_upgrade_fw_show,
	.store = &ft5306_upgrade_fw_store,
};

static ssize_t ft5306_vendor_show(struct kobject *kobj,
				   struct kobj_attribute *attr, char *buf)
{
	unsigned char vendor_id = 0;
	vendor_id = fts_ctpm_get_vendor_id();
	printk(KERN_INFO "ft5306 vendor_id is: %x\n", vendor_id);
	if (vendor_id == 0x53)
		return   sprintf(buf, "%s\n",  "Mutto Optronics") ;
	else if (vendor_id == 0x5D)
		return   sprintf(buf, "%s\n", "BAOMING OPTRONICS CO,.LTD") ;
	else if  (vendor_id == HAWAII_GARNET_FT5X06_VENDOR_ID)
		return sprintf(buf, "%s\n", "FocalTech FT5X06 Hawaii Garnet");
	else if  (vendor_id == G5_A18_FT5X06_VENDOR_ID)
		return   sprintf(buf, "%s\n", "FocalTech FT5X06 G5 A18");
	else if  (vendor_id == G5_A21_FT5316_VENDOR_ID)
		return   sprintf(buf, "%s\n", "FocalTech FT5316 G5 A21");
	else if  (vendor_id == KTOUCH_W68_FT6X06_VENDOR_ID)
		return   sprintf(buf, "%s\n", "FocalTech FT6X06 Ktouch W68");
	else
		return   sprintf(buf, "%s\n",  "UNKNOWN");

}

static struct kobj_attribute ft5306_vendor_attr = {
	.attr = {
		.name = "vendor",
		.mode = S_IRUGO,
	},
	.show = &ft5306_vendor_show,
	.store = NULL,
};

static ssize_t ft5306_virtual_keys_show(struct kobject *kobj,
				   struct kobj_attribute *attr, char *buf)
{
	return sprintf(buf, vkey_scope);
}

static struct kobj_attribute ft5306_virtual_keys_attr = {
	.attr = {
		.name = "virtualkeys.FocalTech-Ft5306",//"virtualkeys.Synaptics-RMI4",
		.mode = S_IRUGO,
	},
	.show = &ft5306_virtual_keys_show,
};

static struct attribute *ft5306_properties_attrs[] = {
	&ft5306_virtual_keys_attr.attr,
	&ft5306_sensitivity_attr.attr,
	&ft5306_min_gap_attr.attr,
	&ft5306_vendor_attr.attr,
	&ft6x06_upgrade_app_attr.attr,
	&ft5306_upgrade_fw_attr.attr,
	NULL
};

static struct attribute_group ft5306_properties_attr_group = {
	.attrs = ft5306_properties_attrs,
};

extern int set_irq_type(unsigned int irq, unsigned int type);
extern int bcm_gpio_pull_up_down_enable(unsigned int gpio, bool enable);
extern int bcm_gpio_pull_up(unsigned int gpio, bool up);
static int ft5306_focaltech_init_platform_hw(void)
{
    printk(KERN_INFO "ft5306_focaltech_init_platform_hw\n");
    /*bcm_gpio_pull_up_down_enable(26, false);
    bcm_gpio_pull_up_down_enable(27, false);
    gpio_request(TP_HW_INT_PIN, "ft5306_focaltech_ts");
    gpio_direction_input(TP_HW_INT_PIN);
    set_irq_type(GPIO_TO_IRQ(TP_HW_INT_PIN), IRQF_TRIGGER_FALLING);
    gpio_free(TP_HW_INT_PIN);*/
    return 0;
}
static int ts_power(ts_power_status vreg_en)
{
	struct regulator *reg = NULL;
	if (!reg) {
		reg = regulator_get(NULL, tp_power);
		if (!reg || IS_ERR(reg)) {
			pr_err("No Regulator available for %s\n", tp_power);
			return -1;
		}
	}
	if (reg) {
		if (vreg_en) {
			regulator_set_voltage(reg, 3000000, 3000000);
			pr_err("Turn on TP (%s) to 2.8V\n", tp_power);
			regulator_enable(reg);
		} else {
			pr_err("Turn off TP (%s)\n", tp_power);
			regulator_disable(reg);
		}
		regulator_put(reg);
	} else {
		pr_err("TP Regulator Alloc Failed");
		return -1;
	}
	return 0;
}

static int i2c_probe_failed;

static int focaltech_ft5306_probe(
	struct i2c_client *client, const struct i2c_device_id *id)
{
	int ret = 0;
	u32 val;
	int detect_fail = 0;
	unsigned char vendor_id = 0;
	unsigned char chip_id = 0;
	unsigned char chip_id2 = 0;
	struct synaptics_rmi4 *ts;
	struct Synaptics_ts_platform_data *pdata;
	struct device_node *np;
	struct kobject *properties_kobj;
	ret = -1;

	if(client==NULL)
	{
	    printk(KERN_ERR "ft5306 client null.\n");
	    goto err_i2c_client_check;
	}
	np = client->dev.of_node;
	ft5306_i2c_client = client;
	ts = kzalloc(sizeof(struct synaptics_rmi4), GFP_KERNEL);
	ts->client = client;
	i2c_set_clientdata(client, ts);
	printk(KERN_ERR "probing for Synaptics RMI4 device %s at $%02X...\n", client->name, client->addr);
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		printk(KERN_ERR "%s: need I2C_FUNC_I2C\n", __func__);
		ret = -ENODEV;
		goto err_check_functionality_failed;
	}
	pdata = client->dev.platform_data;
	if (pdata) {
		ts->power = pdata->power;
		ts_gpio_irq_pin = pdata->gpio_irq_pin;
	#if (TP_CNTRL_PIN_TYPE == TP_CNTRL_PIN_RESET)
		ts_gpio_reset_pin = pdata->gpio_reset_pin;
	#else
		ts_gpio_wakeup_pin = pdata->gpio_wakeup_pin;
	#endif
		ts_x_max_value = pdata->x_max_value;
		ts_y_max_value = pdata->y_max_value;

	} else if (client->dev.of_node) {
		ts_gpio_irq_pin = of_get_named_gpio(np,
			"gpio-irq-pin", 0);
		if (!gpio_is_valid(ts_gpio_irq_pin)) {
			pr_err(
			"%s: ERROR Invalid gpio-irq-pin\n",
			__func__);
		}

#if (TP_CNTRL_PIN_TYPE == TP_CNTRL_PIN_RESET)
		ts_gpio_reset_pin = of_get_named_gpio(np,
			"gpio-reset-pin", 0);
		if (!gpio_is_valid(ts_gpio_reset_pin)) {
			pr_err(
			"%s: ERROR Invalid gpio-reset-pin\n",
			__func__);
		}
#else
		ts_gpio_wakeup_pin = of_get_named_gpio(np,
			"gpio-wakeup-pin", 0);
		if (!gpio_is_valid(ts_gpio_wakeup_pin)) {
			pr_err(
			"%s: ERROR Invalid gpio-wakeup-pin\n",
			__func__);
		}
#endif
		if (!of_property_read_u32(np, "x-max-value", &val))
			ts_x_max_value = val;
		if (!of_property_read_u32(np, "y-max-value", &val))
			ts_y_max_value = val;
		if (!of_property_read_u32(np, "use-irq", &val))
			client->irq = val;
		if (!of_property_read_u32(np, "auto_update_fw", &val))
			auto_update_fw = val;
		if (!of_property_read_u32(np, "pressure_support", &val))
			pressure_support = val;
		val = 0;
		if (!of_property_read_u32(np, "chip_id", &val))
			chip_id2 = val;
		of_property_read_string(np, "power", &tp_power);
		if (!of_property_read_u32(np, "firmware_header_needed", &val))
			fw_header_needed = val;
		if (fw_header_needed) {
			if (of_property_read_string(np, "firmware_header", &fw_header))
				sprintf(firmware_str, "%s", "no_header");
			else
				sprintf(firmware_str, "%s", fw_header);
		}
		if (of_property_read_string(np, "vkey_scope", &vkey_scope)) {
			have_vkey = false;
			vkey_scope = "\n";
		}
		else
			have_vkey = true;

		if (tp_power)
			ts->power = ts_power;
	}

	ft5306_focaltech_init_platform_hw();

#if (TP_CNTRL_PIN_TYPE == TP_CNTRL_PIN_WAKEUP)
		gpio_request(ts_gpio_wakeup_pin, "tp_wakeup");
		gpio_direction_output(ts_gpio_wakeup_pin, 1);
		gpio_set_value(ts_gpio_wakeup_pin, 0);
		mdelay(5);
		gpio_set_value(ts_gpio_wakeup_pin, 1);
		gpio_free(ts_gpio_wakeup_pin);
		mdelay(200);
#endif
		if (ts->power) {
			printk(KERN_ERR "Repower Tp Now...\n");
			ts->power(TS_ON);
			mdelay(10);
		}
		/*read chip ID to detect if the chip exists*/
		if (chip_id2 > 0) {
			chip_id = fts_ctpm_get_chip_id();
			if (chip_id != chip_id2)
				detect_fail = 1;
		} else {
			vendor_id = fts_ctpm_get_vendor_id();

			if ((vendor_id != HAWAII_GARNET_FT5X06_VENDOR_ID)
				&& (vendor_id != G5_A18_FT5X06_VENDOR_ID)
				&& (vendor_id != G5_A21_FT5316_VENDOR_ID)
				&& (vendor_id != KTOUCH_W81_FT6X06_VENDOR_ID)
				&& (vendor_id != TCL_UP823_FT6206_VENDOR_ID)
				&& (vendor_id != KTOUCH_5606_FT6X06_VENDOR_ID)
				&& (vendor_id != KTOUCH_W68_FT6X06_VENDOR_ID))
				detect_fail = 1;
		}

		if (detect_fail) {
			if (ts->power) {
				ts->power(TS_OFF);
				mdelay(10);
			}

			kfree(ts);
			ret = -ENODEV;
			i2c_probe_failed = 1;
			printk(KERN_ERR "Focaltech tp not exist!\r\n");
			goto  err_chip_not_exist ;
		}


#if (TP_CNTRL_PIN_TYPE == TP_CNTRL_PIN_RESET)
		Ft5306_Hw_Reset();
#endif
		INIT_WORK(&ts->work, focaltech_ft5306_work_func);
		wake_lock_init(&update_wake_lock, WAKE_LOCK_SUSPEND, "touch");
		if (auto_update_fw) {
			update_complete = 0;
			INIT_DELAYED_WORK(&ft6x06_firmware_update, focaltech_fts_firmware_update_func);
			wake_lock(&update_wake_lock);
			queue_delayed_work(synaptics_wq, &ft6x06_firmware_update, 5*HZ);
		}


#if ENABLE_TP_DIAG
		INIT_WORK(&ts->diag_work, focaltech_ft5306_diag_work_func);
#endif

	properties_kobj = kobject_create_and_add("board_properties", NULL);
	if (properties_kobj)
		ret = sysfs_create_group(properties_kobj,
					&ft5306_properties_attr_group);
	if (!properties_kobj || ret)
		pr_err("failed to create board_properties\n");

	ts->input_dev = input_allocate_device();
	if (!ts->input_dev) {
		printk(KERN_ERR "failed to allocate input device.\n");
		ret = -EBUSY;
		goto err_alloc_dev_failed;
	}

#if (TP_CNTRL_PIN_TYPE == TP_CNTRL_PIN_RESET)
	if (ts->power) {
		ts->power(TS_ON);
		ts->power(TS_OFF);
		mdelay(500);
	}
#endif

	ft5306_touch_info.pst_point_info = ft5306_touch_point;

	ts->input_dev->name = "FocalTech-Ft5306";
	ts->input_dev->phys = client->name;
	if (have_vkey) {
		ts->input_dev->id.bustype = BUS_I2C;
		ts->input_dev->id.vendor = 0x8888;
		ts->input_dev->id.product = 0x6666;
		ts->input_dev->id.version = 1000;
	}
	set_bit(EV_ABS, ts->input_dev->evbit);
	set_bit(EV_SYN, ts->input_dev->evbit);
	set_bit(BTN_TOUCH, ts->input_dev->keybit);
	if (have_vkey) {
		set_bit(KEY_MENU, ts->input_dev->keybit);
		set_bit(KEY_HOME, ts->input_dev->keybit);
		set_bit(KEY_BACK, ts->input_dev->keybit);
		set_bit(KEY_SEARCH, ts->input_dev->keybit);
	}

    set_bit(INPUT_PROP_DIRECT, ts->input_dev->propbit);

	input_set_abs_params(ts->input_dev, ABS_MT_POSITION_X, 0, ts_x_max_value, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_MT_POSITION_Y, 0, ts_y_max_value, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_MT_TOUCH_MAJOR, 0, 8, 0, 0);
	if (pressure_support)
		input_set_abs_params(ts->input_dev, ABS_MT_PRESSURE, 0, 255, 0, 0);

	InitFingersQueue();
	if (client->irq) {
		printk("%s IRQ %d", __func__, client->irq);
		ret = gpio_request(ts_gpio_irq_pin, client->name);
		printk(KERN_INFO "%s: request tp int ret: %d\n", __func__, ret);

		gpio_direction_input(ts_gpio_irq_pin);
		printk("Requesting IRQ...\n");

		if (request_irq(GPIO_TO_IRQ(ts_gpio_irq_pin), focaltech_ft5306_irq_handler,
				IRQF_TRIGGER_FALLING, client->name, ts) >= 0) {
			printk("Requested IRQ\n");
			ts->use_irq = 1;
			printk(KERN_INFO "GPIO_%d INT: %d", ts_gpio_irq_pin,
						GPIO_TO_IRQ(ts_gpio_irq_pin));
			/*if ((ret = set_irq_wake(client->irq, 1)) < 0) {
				printk(KERN_ERR "failed to set IRQ wake: %d\n", ret);
			}*/
		} else {
			printk("Failed to request IRQ!\n");
		}
	}

	if (!ts->use_irq) {
		printk(KERN_ERR "Synaptics RMI4 device %s in polling mode\n", client->name);
	}

	/*
	 * Device will be /dev/input/event#
	 * For named device files, use udev
	 */
	ret = input_register_device(ts->input_dev);
	if (ret) {
		printk(KERN_ERR "focaltech_ft5306_probe: Unable to register %s input device\n", ts->input_dev->name);
		goto err_input_register_device_failed;
	} else {
		printk("tp input device registered\n");
	}

	ts->enable = 1;

	dev_set_drvdata(&ts->input_dev->dev, ts);

	if (sysfs_create_file(&ts->input_dev->dev.kobj, &dev_attr_enable.attr) < 0)
		printk("failed to create sysfs file for input device\n");

	#ifdef CONFIG_HAS_EARLYSUSPEND
	ts->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN;
	ts->early_suspend.suspend = focaltech_ft5306_early_suspend;
	ts->early_suspend.resume = focaltech_ft5306_late_resume;
	register_early_suspend(&ts->early_suspend);
	#endif
#ifdef CONFIG_LEDS_TRIGGER_KPBL
	led_kpbl_register(&register_touch_key_notifier);
	led_kpbl_unregister(&unregister_touch_key_notifier);
#endif

	printk("Tp Probe Done!!!");

	return 0;

err_input_register_device_failed:
	input_free_device(ts->input_dev);
err_chip_not_exist:
err_alloc_dev_failed:
err_check_functionality_failed:
err_i2c_client_check:

	return ret;
}


static int focaltech_ft5306_remove(struct i2c_client *client)
{
struct synaptics_rmi4 *ts = i2c_get_clientdata(client);

#ifdef CONFIG_HAS_EARLYSUSPEND
	unregister_early_suspend(&ts->early_suspend);
#endif
	if (ts->use_irq)
		free_irq(client->irq, ts);

	input_unregister_device(ts->input_dev);
	kfree(ts);
	return 0;
}

static int focaltech_ft5306_suspend(struct i2c_client *client, pm_message_t mesg)
{
	struct synaptics_rmi4 *ts = i2c_get_clientdata(client);

	if (!update_complete)
		return 0;
	focaltech_ft5306_disable(ts);
	Ft5306_Enter_Sleep();
	mdelay(10);
	if (ts->power) {
		 ts->power(TS_OFF);
	}
	return 0;
}

static int focaltech_ft5306_resume(struct i2c_client *client)
{
	struct synaptics_rmi4 *ts = i2c_get_clientdata(client);

	if (!update_complete)
		return 0;
	Ft5306_Exit_Sleep();
	mdelay(2);
	if (ts->power)
		ts->power(TS_ON);
#if (TP_CNTRL_PIN_TYPE == TP_CNTRL_PIN_RESET)
	Ft5306_Hw_Reset();
#endif
	mdelay(5);
	focaltech_ft5306_enable(ts);

	return 0;
}

#ifdef CONFIG_HAS_EARLYSUSPEND
static void focaltech_ft5306_early_suspend(struct early_suspend *h)
{
	struct synaptics_rmi4 *ts;
	printk(KERN_INFO "call %s\n", __func__);
	ts = container_of(h, struct synaptics_rmi4, early_suspend);
	focaltech_ft5306_suspend(ts->client, PMSG_SUSPEND);
}

static void focaltech_ft5306_late_resume(struct early_suspend *h)
{
	struct synaptics_rmi4 *ts;
	printk(KERN_INFO "call %s\n", __func__);
	ts = container_of(h, struct synaptics_rmi4, early_suspend);
	focaltech_ft5306_resume(ts->client);
}
#endif

static struct i2c_driver focaltech_ft5306_driver = {
	.probe		= focaltech_ft5306_probe,
	.remove		= focaltech_ft5306_remove,
#ifndef CONFIG_HAS_EARLYSUSPEND
	.suspend	= focaltech_ft5306_suspend,
	.resume		= focaltech_ft5306_resume,
#endif
	.id_table	= focaltech_ft5306_id,
	.driver = {
		.name	= "FocalTech-Ft5306",
	},
};

static int focaltech_ft5306_init(void)
{
	int retval;

	printk(KERN_INFO "Synaptics I2C RMI4 driver init");

	synaptics_wq = create_singlethread_workqueue("synaptics_wq");
	if (!synaptics_wq) {
		printk(KERN_ERR "Could not create work queue synaptics_wq: no memory");
		return -ENOMEM;
	}

	retval =  i2c_add_driver(&focaltech_ft5306_driver);
	if (i2c_probe_failed) {
		i2c_del_driver(&focaltech_ft5306_driver);
		if (synaptics_wq)
			destroy_workqueue(synaptics_wq);
	}
	return retval;
}

static void __exit focaltech_ft5306_exit(void)
{
	i2c_del_driver(&focaltech_ft5306_driver);
	if (synaptics_wq)
		destroy_workqueue(synaptics_wq);
}

module_init(focaltech_ft5306_init);
module_exit(focaltech_ft5306_exit);

MODULE_DESCRIPTION("FocalTech FT5306 Driver");
MODULE_LICENSE("GPL");

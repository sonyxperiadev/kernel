#ifndef _CM36XX_H_
#define _CM36XX_H_

#define VENDOR_NAME     "Capella"
#define SENSOR_NAME	"cm36xx"
#define DRIVER_VERSION	"1.0"
#define CM36XX_MAX_DELAY 200/* register definitation */

#define ALS_CONF 0x00
#define ALS_THDL  0x01
#define ALS_THDH  0x02
#define PS_CONF1_CONF2 0x03
#define PS_CONF3_MS  0x04
#define PS_CANC  0x05
#define PS_THD  0x06
#define PS_DATA 0x08
#define ALS_DATA 0x09
#define INT_FLAG 0x0B
#define DEV_ID 0x0C

#define PS_MAX  255
#define ALS_MAX 3277

#define ALS_CONF_VAL 0x0040
#define ALS_DISABLE 0x0001

#define PS_CONF12_VAL 0x00A2
#define PS_DISABLE 0x0001

#define PS_CONF3_VAL 0x0000
/*ALS_CONFIG*/
#define CM36XX_ALS_IT_MASK     0xC0
#define CM36XX_ALS_PERS_MASK     0x60

/*PS_CONFIG1*/
#define CM36XX_PS_DUTY_MASK	0xC0
#define CM36XX_PS_IT_MASK	0x30
#define CM36XX_PS_PERS_MASK	0x0C
#define CM36XX_PS_CONFIG1_RESERVED	0x02
#define CM36XX_PS_SD_MASK	0x01
/*PS_CONFIG2*/
#define CM36XX_PS_ITB_MASK	0xC0
#define CM36XX_PS_INT_MASK	0X03

/*INT_FLAG*/
#define CM36XX_ALS_IF_L_BIT	0x20
#define CM36XX_ALS_IF_H_BIT	0x10
#define CM36XX_PS_IF_CLOSE_BIT	0x02
#define CM36XX_PS_IF_AWAY_BIT	0x01

struct cm36xx_platform_data {
	u32 gpio_no;
	u8 als_poll_mode;
	u8 ps_poll_mode;
	u8 als_it; /*ALS_CONF[7:6]*/
	u8 als_pers;/*ALS_CONF[3:2]*/
	u16 als_thdl;/*ALS_THDL*/
	u16 als_thdh;/*ALS_THDH*/
	u8 ps_duty;/*PS_CONF1[7:6]*/
	u8 ps_it;/*PS_CONF1[5:4]*/
	u8 ps_pers;/*PS_CONF1[3:2]*/
	u8 ps_itb;/*PS_CONF2[7:6]*/
	u16 ps_thdl;/*PS_THDL*/
	u16 ps_thdh;/*PS_THDH*/
};

#endif

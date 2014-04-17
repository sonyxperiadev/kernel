/**
 * @file	ami306_def.h
 * @brief	AMI306 Device Driver definition
 */
#ifndef AMI306_DEF_H
#define AMI306_DEF_H

/******************************************************************************
 Constant
 ******************************************************************************/
#ifndef __KERNEL__
typedef signed char s8;
typedef unsigned char u8;
typedef signed short s16;
typedef unsigned short u16;
typedef signed long s32;
typedef unsigned long u32;
#endif

/* #define AMI_6AXIS */
#define	AMI_OK		0		/**< Normal */
#define	AMI_PARAM_ERR	-1		/**< Parameter Error  */
#define	AMI_SEQ_ERR	-2		/**< Sequence Error  */
#define	AMI_COMM_ERR	-3		/**< Communication Error  */
#define	AMI_SYSTEM_ERR	-10		/**< System Error  */
#define AMI_OVERFLOW	-20		/**< mag overflow */
#define	AMI_ERROR	-99		/**< other Error  */

#define AMI_ST_OK			0	/* OK */
#define AMI_ST_ERR_COMMUNICATION	1	/* Communication Error */
#define AMI_ST_ERR_OTP			2	/* OTP Error */
#define AMI_ST_ERR_TEMP_SENSOR		3	/* Temp sensor Error */
#define AMI_ST_ERR_MI_ELEMENT		4	/* MI element Error */
#define AMI_ST_ERR_DIGITAL_CIRCUIT	5	/* Digital circuit Error */
/******************************************************************************
 Struct definition
 ******************************************************************************/
/** AMI chip information */
struct ami_chipinfo {
	u16 info;			/**< INFO reg. */
	u16 ver;			/**< VER  reg. */
	u16 sn;				/**< SN   reg. */
	u8 wia;				/**< WIA  reg. */
};

/** magnetic sensor parameter */
struct ami_mag_parameter {
	u16 sensitivity[3];		/**< sensitivity */
	u8 interference[6];		/**< axis interference */
};

/** magnetic sensor window parameter */
struct ami_win_parameter {
	u16 fine[3];			/**< magnetic sensor fine */
	u16 fine_output[3];		/**< change per 1fine */
	u16 zero_gauss_fine[3];		/**< fine value at zero gauss */
};

/** sensor calibration Parameter information  */
struct ami_sensor_parameter {
	struct ami_chipinfo chip;	/**< sensor chip information */
	struct ami_mag_parameter mag;	/**< magnetic sensor parameter */
	struct ami_win_parameter win;	/**< sensor window parameter */
};

/** G2-Sensor measurement value */
struct ami_sensor_value {
	s16 mag[3];			/**< magnetic sensor value (mGauss) */
};

/** AMI Driver Information  */
struct ami_driverinfo {
	u8 remarks[40];			/**< Some Information   */
	u8 datetime[30];		/**< compiled date&time */
	u8 ver_major;			/**< major version */
	u8 ver_middle;			/**< middle.. */
	u8 ver_minor;			/**< minor .. */
};

/** Debug */
struct ami_register {
	u16 adr;			/**< register address */
	union {
		u8 byte;		/**< byte data */
		u16 word;		/**< word data */
		u32 dword;		/**< double word data */
	} dat;
};

struct ami306_platform_data {
	int gpio_intr;
	int gpio_drdy;
	int dir;
	int polarity;
	const char *supply_name;
};
#endif

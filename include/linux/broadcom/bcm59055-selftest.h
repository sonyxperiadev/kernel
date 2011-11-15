/************************************************************************************************/
/*                                                                                              */
/*  Copyright 2011  Broadcom Corporation                                                        */
/*                                                                                              */
/*     Unless you and Broadcom execute a separate written software license agreement governing  */
/*     use of this software, this software is licensed to you under the terms of the GNU        */
/*     General Public License version 2 (the GPL), available at                                 */
/*                                                                                              */
/*          http://www.broadcom.com/licenses/GPLv2.php                                          */
/*                                                                                              */
/*     with the following added to such license:                                                */
/*                                                                                              */
/*     As a special exception, the copyright holders of this software give you permission to    */
/*     link this software with independent modules, and to copy and distribute the resulting    */
/*     executable under terms of your choice, provided that you also meet, for each linked      */
/*     independent module, the terms and conditions of the license of that module.              */
/*     An independent module is a module which is not derived from this software.  The special  */
/*     exception does not apply to any modifications of the software.                           */
/*                                                                                              */
/*     Notwithstanding the above, under no circumstances may you combine this software in any   */
/*     way with any other Broadcom software provided under a license other than the GPL,        */
/*     without Broadcom's express prior written consent.                                        */
/*                                                                                              */
/************************************************************************************************/

/* Selftest User Space Commands */
typedef enum SelftestUSCmds_e {
	ST_SUSC_HOOKINT_TEST,		/*a*/
	ST_SUSC_SIM_CLK_LOOP,		/*b*/
	ST_SUSC_1,
	ST_SUSC_DIGIMIC,		/*c*/
	ST_SUSC_BT_LOOPBACK,		/*d*/
	ST_SUSC_GPS_TEST,		/*e*/
	ST_SUSC_SLEEPCLOCK,		/*f*/
	ST_SUSC_PMU_ST,			/*g*/
	ST_SUSC_USB_ST,			/*h*/
	ST_SUSC_ADC_ST,			/*i*/
	ST_SUSC_HEADSET_ST,		/*j*/
	ST_SUSC_IHF_ST			/*k*/
} SelftestUSCmds_e;

typedef enum Selftest_Results_t {
	ST_SELFTEST_OK,			/* Test succeded */
	ST_SELFTEST_FAILED,		/* Test Failed */
	ST_SELFTEST_NOT_SUPPORTED	/* Test Not supported */
} Selftest_Results_t;

/* Subtest Detailed Return Codes */
/* Accesories */
#define ST_SELFTEST_SHORTED_GROUND_ACI  0x10
#define ST_SELFTEST_SHORTED_GROUND_BIAS 0x11
#define ST_SELFTEST_SHORTED_POWER_ACI   0x12
#define ST_SELFTEST_SHORTED_POWER_BIAS  0x13
/* SIM */
#define ST_SELFTEST_SIM_SHORT_GROUND		0x20
#define ST_SELFTEST_SIM_SHORT_POWER		0x21
#define ST_SELFTEST_SIM_SHORT_INTERCONNECT	0x22
/* Audio */
#define ST_SELFTEST_SHORTED_GROUND		0x10
#define ST_SELFTEST_SHORTED_POWER		0x11
#define ST_SELFTEST_BAD_CONNECTION		0x12
#define ST_SELFTEST_BAD_CONNECTION_OR_GROUND	0x13
#define ST_SELFTEST_BAD_CONNECTION_OR_POWER	0x14
#define ST_SELFTEST_NOT_TESTED			0x15

/*  Digmic selftest defines */
typedef enum Selftest_Digimic_Setup_e {
	ST_SELFTEST_DIGIMIC_NONE,
	ST_SELFTEST_DIGIMIC_1MIC,
	ST_SELFTEST_DIGIMIC_2MICS
} Selftest_Digimic_Setup_e;

/* GPS selftest defines */
typedef enum Selftest_GPS_Pin_e {
	ST_SELFTEST_GPS_TXP,
	ST_SELFTEST_GPS_TIMESTAMP
} Selftest_GPS_Pin_e;

typedef enum Selftest_GPS_Pin_Action_e {
	ST_SELFTEST_GPS_READ,
	ST_SELFTEST_GPS_WRITE
} Selftest_GPS_Pin_Action_e;

typedef enum Selftest_GPS_Pin_State_e {
	ST_SELFTEST_GPS_HIGH,
	ST_SELFTEST_GPS_LOW,
	ST_SELFTEST_GPS_RELEASE
} Selftest_GPS_Pin_State_e;

/********************************************************/
/** Usermode space communication **/
/********************************************************/
typedef struct SelftestUserCmdData_t {
	/* Input */
	__u32 testId; /* SelftestCmds_e */
	__u32 parm1;
	__u32 parm2;
	__u32 parm3;
	/* Output */
	__u32 subtestCount;
	__u32 subtestStatus[10]; /* Max 10 subtests supported */
	__u32 testStatus;
} SelftestUserCmdData_t;

typedef struct SelftestDevData_t {
	struct bcm590xx *bcm_5900xx_pmu_dev;
} SelftestDevData_t;

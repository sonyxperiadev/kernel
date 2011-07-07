/****************************************************************************
*
*     Copyright (c) 2009 Broadcom Corporation
*           All Rights Reserved
*
*     No portions of this material may be reproduced in any form without the
*     written permission of:
*
*           Broadcom Corporation
*           5300 California Avenue
*           Irvine, California 92617
*
*     All information contained in this document is Broadcom Corporation
*     company private, proprietary, and trade secret.
*
****************************************************************************/
/**
*
*   @file   
*
*   @brief  
*
****************************************************************************/

//******************************** History *************************************
//
// $Log:  $
// Initial Ver. 
//*****************************************************************************

//******************************************************************************
//                          definition block
//******************************************************************************
#ifndef _PMU_MAX8986_H_
#define _PMU_MAX8986_H_

#ifndef FEATURE_PMIC_MAX8986
#define FEATURE_PMIC_MAX8986

#define PMIC_I2C_BUS            0x01

//=======================================
// Slave address for Max8986
//=======================================
#define PMIC_SLAVE_ADDR_PM      0x10
#define PMIC_SLAVE_ADDR_MUIC    0x4A
#define PMIC_SLAVE_ADDR_RTC     0xD0
#define PMIC_SLAVE_ADDR_AUDIO   0x9A



//=======================================
// Register Map for Max8986
//=======================================
// PM part (Slave Address : 0x10)
#define REG_PM_PMIC_ID         0x00 // 0x00
#define REG_PM_INT1            0x01 // 0x01
#define REG_PM_INT2            0x02 // 0x02
#define REG_PM_INT3            0x03 // 0x03
#define REG_PM_INT1MSK         0x04 // 0x0B
#define REG_PM_INT2MSK         0x05 // 0x0C
#define REG_PM_INT3MSK         0x06 // 0x0D
#define REG_PM_A1OPMODCTRL     0x07 // 0x16
#define REG_PM_D1OPMODCTRL     0x08 // 0x17
#define REG_PM_A8OPMODCTRL     0x09 // 0x18
#define REG_PM_A2OPMODCTRL     0x0A // 0x19
#define REG_PM_H1OPMODCTRL     0x0B // 0x1A
#define REG_PM_H2OPMODCTRL     0x0C // 0x1B
#define REG_PM_D2OPMODCTRL     0x0D // 0x1C
#define REG_PM_A5OPMODCTRL     0x0E // 0x1D
#define REG_PM_A4OPMODCTRL     0x0F // 0x1E
#define REG_PM_LVOPMODCTRL     0x10 // 0x1F
#define REG_PM_SIMOPMODCTRL    0x11 // 0x20
#define REG_PM_CSRCTRL1        0x12 // 0x21
#define REG_PM_CSRCTRL2        0x13 // 0x22
#define REG_PM_CSRCTRL3        0x14 // 0x23
#define REG_PM_CSROPMODCTRL    0x15 // 0x2A
#define REG_PM_IOSRCTRL2       0x16 // 0x2C
#define REG_PM_IOSROPMODCTRL   0x17 // 0x34
#define REG_PM_MBCCTRL1        0x18 // 0x36
#define REG_PM_MBCCTRL2        0x19 // 0x37
#define REG_PM_MBCCTRL3        0x1A // 0x38
#define REG_PM_MBCCTRL4        0x1B // 0x39
#define REG_PM_MBCCTRL7        0x1C // 0x3C
#define REG_PM_MBCCTRL8        0x1D // 0x3D
#define REG_PM_ENV1            0x1E // 0x4C
#define REG_PM_ENV2            0x1F // 0x4D
#define REG_PM_HOSTACT         0x20 // 0x51
#define REG_PM_PWRONBDB        0x21 // 0x5D
#define REG_PM_JIGONBDB        0x22 // 0x5E
#define REG_PM_BBCCTRL         0x23 // 0x6F
#define REG_PM_OTPCGHCVS       0x24 // 0x7A
#define REG_PM_SMPL            0x25 // 0x88
#define REG_PM_A5_A3PWRGRP     0x26 // 0x9E
#define REG_PM_AX1_A7PWRGRP    0x27 // 0x9F
#define REG_PM_LV_A6PWRGRP     0x28 // 0xA0
#define REG_PM_D2PWRGRP        0x29 // 0xA1
#define REG_PM_C_IOSRPWRGRP    0x2A // 0xA2
#define REG_PM_A6OPMODCTRL     0x2B // 0xA4
#define REG_PM_A3OPMODCTRL     0x2C // 0xA5
#define REG_PM_AX1OPMODCTRL    0x2D // 0xA6
#define REG_PM_A7OPMODCTRL     0x2E // 0xA7
#define REG_PM_AX1LDOCTRL      0x2F // 0xA8
#define REG_PM_ALDO7CTRL       0x30 // 0xA9
#define REG_PM_A1_D1PWRGRP     0x31 // 0xAA
#define REG_PM_A8_A2PWRGRP     0x32 // 0xAB
#define REG_PM_A4_SIMPWRGRP    0x33 // 0xAC
#define REG_PM_HCPWRGRP        0x34 // 0xAD
#define REG_PM_CSRCTRL10       0x35 // 0xAE
#define REG_PM_A1_D1CTRL       0x36 // 0xB3
#define REG_PM_A8_A2LDOCTRL    0x37 // 0xB4
#define REG_PM_HCLDOCTRL       0x38 // 0xB5
#define REG_PM_D2LDOCTRL       0x39 // 0xB6
#define REG_PM_A5_A3LDOCTRL    0x3A // 0xB7
#define REG_PM_A4_SIMLDOCTRL   0x3B // 0xB8
#define REG_PM_LVLDO_ALDO6CTRL 0x3C // 0xB9
#define REG_PM_PWRONBDB1       0x3D // 0xBA
#define REG_PM_ADISCHARG1      0x3E // 0xE0
#define REG_PM_ADISCHARG2      0x3F // 0xE1
#define REG_PM_ADISCHARG3      0x40 // 0xE2
#define REG_PM_D3OPMODCTRL     0x41 // 0xE3
#define REG_PM_D3LDOCTRL       0x42 // 0xE4
#define REG_PM_D4OPMODCTRL     0x43 // 0xE5
#define REG_PM_D4LDOCTRL       0x44 // 0xE6
#define REG_PM_A9OPMODCTRL     0x45 // 0xE7
#define REG_PM_A9LDOCTRL       0x46 // 0xE8
#define REG_PM_TSROPMODCTRL    0x47 // 0xE9
#define REG_PM_TSRLDOCTRL      0x48 // 0xEA

// MUIC part (Slave Address : 0x4A)
#define REG_MUIC_DEVICEID      0x49 // 0x00
#define REG_MUIC_INT1          0x4A // 0x01
#define REG_MUIC_INT2          0x4B // 0x02
#define REG_MUIC_INT3          0x4C // 0x03
#define REG_MUIC_STATUS1       0x4D // 0x04
#define REG_MUIC_STATUS2       0x4E // 0x05
#define REG_MUIC_STATUS3       0x4F // 0x06
#define REG_MUIC_INTMASK1      0x50 // 0x07
#define REG_MUIC_INTMASK2      0x51 // 0x08
#define REG_MUIC_INTMASK3      0x52 // 0x09
#define REG_MUIC_CDETCTRL0     0x53 // 0x0A
#define REG_MUIC_CONTROL1      0x54 // 0x0C
#define REG_MUIC_CONTROL2      0x55 // 0x0D
#define REG_MUIC_CONTROL3      0x56 // 0x0E


// RTC part (Slave Address : 0xD0)
//#define REG_RTC_INT                 0x57 // 0x00
//#define REG_RTC_INTMASK             0x58 // 0x01
#define REG_RTC_CTRLMASK            0x59 // 0x02
#define REG_RTC_CTRL                0x5A // 0x03
#define REG_RTC_UPDATE1             0x5B // 0x04    // RTC Update Register1
#define REG_RTC_UPDATE2             0x5C // 0x05    // RTC Update Register2
#define REG_RTC_WTSR_SMPL           0x5D // 0x06    // WTSR and SMPL Register
#define REG_RTC_SEC                 0x5E // 0x10
#define REG_RTC_MIN                 0x5F // 0x11
#define REG_RTC_HOUR                0x60 // 0x12
#define REG_RTC_DAYOFWEEK           0x61 // 0x13
#define REG_RTC_MONTH               0x62 // 0x14
#define REG_RTC_YEAR                0x63 // 0x15
#define REG_RTC_DAYOFMONTH          0x64 // 0x16
#define REG_RTC_SEC_ALARM1          0x65 // 0x17
#define REG_RTC_MIN_ALARM1          0x66 // 0x18
#define REG_RTC_HOUR_ALARM1         0x67 // 0x19
#define REG_RTC_DAYOFWEEK_ALARM1    0x68 // 0x1A
#define REG_RTC_MONTH_ALARM1        0x69 // 0x1B
#define REG_RTC_YEAR_ALARM1         0x6A // 0x1C
#define REG_RTC_DAYOFMONTH_ALARM1   0x6B // 0x1D
#define REG_RTC_SEC_ALARM2          0x6C // 0x1E
#define REG_RTC_MIN_ALARM2          0x6D // 0x1F
#define REG_RTC_HOUR_ALARM2         0x6E // 0x20
#define REG_RTC_DAYOFWEEK_ALARM2    0x6F // 0x21
#define REG_RTC_MONTH_ALARM2        0x70 // 0x22
#define REG_RTC_YEAR_ALARM2         0x71 // 0x23
#define REG_RTC_DAYOFMONTH_ALARM2   0x72 // 0x24


// AUDIO part (Slave Address : 0x9A)
#define REG_AUDIO_INPUT             0x73 // 0x00
#define REG_AUDIO_MIXER1            0x74 // 0x01
#define REG_AUDIO_MIXER2            0x75 // 0x02
#define REG_AUDIO_VOL_CTRL1         0x76 // 0x03    // Volume Control Register1
#define REG_AUDIO_VOL_CTRL2         0x77 // 0x04    // Volume Control Register2
#define REG_AUDIO_VOL_CTRL3         0x78 // 0x05    // Volume Control Register3
#define REG_AUDIO_DISTORT_LIMIT     0x79 // 0x07    // Distortion Limiter Register
#define REG_AUDIO_PM                0x7A // 0x08    // Audio Power Management Register
#define REG_AUDIO_CHARG_PUMP        0x7B // 0x09    // Charge Pump Control Register

#define PMU_REG_TOTAL               0x7C

/* MAX8986 each register info */
typedef const struct {
    UInt8  slave_addr;
    UInt8  addr;
} max8986_register_type;


typedef enum {
    CSR_LPM_VALUE,
    CSR_NM_VALUE,
    CSR_NM2_VALUE,
    LDO_ALDO1_VALUE,
    LDO_ALDO2_VALUE,
    LDO_ALDO3_VALUE,
    LDO_ALDO4_VALUE,
    LDO_ALDO5_VALUE,
    LDO_ALDO6_VALUE,
    LDO_ALDO7_VALUE,
    LDO_ALDO8_VALUE,
    LDO_ALDO9_VALUE,
    LDO_DLDO1_VALUE,
    LDO_DLDO2_VALUE,
    LDO_DLDO3_VALUE,
    LDO_DLDO4_VALUE,
    LDO_LVLDO_VALUE,
    LDO_SIMLDO_VALUE,
    LDO_HCLDO1_VALUE,
    LDO_HCLDO2_VALUE,
    LDO_AUXLDO1_VALUE, 
    ENDOFLDO
} PMU_LDO_ID_t;

typedef enum {
    CSR_DVS_VOLTAGTE_0P76V = 0,
    CSR_DVS_VOLTAGTE_0P78V,
    CSR_DVS_VOLTAGTE_0P80V,
    CSR_DVS_VOLTAGTE_0P82V,
    CSR_DVS_VOLTAGTE_0P84V,
    CSR_DVS_VOLTAGTE_0P86V,
    CSR_DVS_VOLTAGTE_1P38V,
    CSR_DVS_VOLTAGTE_1P36V,
    CSR_DVS_VOLTAGTE_1P34V,
    CSR_DVS_VOLTAGTE_1P32V,
    CSR_DVS_VOLTAGTE_1P30V,
    CSR_DVS_VOLTAGTE_1P28V,
    CSR_DVS_VOLTAGTE_1P26V,
    CSR_DVS_VOLTAGTE_1P24V,
    CSR_DVS_VOLTAGTE_1P22V,
    CSR_DVS_VOLTAGTE_1P20V,
    CSR_DVS_VOLTAGTE_1P18V,
    CSR_DVS_VOLTAGTE_1P16V,
    CSR_DVS_VOLTAGTE_1P14V,
    CSR_DVS_VOLTAGTE_1P12V,
    CSR_DVS_VOLTAGTE_1P10V,
    CSR_DVS_VOLTAGTE_1P08V,
    CSR_DVS_VOLTAGTE_1P06V,
    CSR_DVS_VOLTAGTE_1P04V,
    CSR_DVS_VOLTAGTE_1P02V,
    CSR_DVS_VOLTAGTE_1P00V,
    CSR_DVS_VOLTAGTE_0P98V,
    CSR_DVS_VOLTAGTE_0P96V,
    CSR_DVS_VOLTAGTE_0P94V,
    CSR_DVS_VOLTAGTE_0P92V,
    CSR_DVS_VOLTAGTE_0P90V,
    CSR_DVS_VOLTAGTE_0P88V
} PMU_CSR_voltage_t;

typedef struct{
    UInt8   onoff_reg;
    UInt8   volt_reg;
} max8986_ldo_type;


typedef enum
{
	PMU_MUIC_NOTHING=0,           // nothing attached
	PMU_MUIC_CABLE,               // USB Cable attached
	PMU_MUIC_CDP,                 // Charging Downstream port: current depends on USB operating speed
	PMU_MUIC_DC1800MA,            // dedicated charger: current up to 1.8A
	PMU_MUIC_SC500MA,             // Special 500mA charger: Current 500mA max
	PMU_MUIC_SC1000MA,            // Special 1A charger: Current up to 1A
	PMU_MUIC_DBC100MA             // Dead Battery Charging - 100mA max
} PMU_DRV_MUICType_en_t;


typedef enum {
    PMU_IRQID_INT1_PONKEYBR,      // R&C  on key rising
    PMU_IRQID_INT1_PONKEYBF,      // R&C  on key falling
    PMU_IRQID_INT1_PONKEYBH,      // R&C  on key pressed > hold debounce time
    PMU_IRQID_INT1_RTC60S,        // R&C  real-time clock needs adjustment
    PMU_IRQID_INT1_RTCA,          // R&C  alarm
    PMU_IRQID_INT1_SMPL,          // R&C  one minute elapsed
	PMU_IRQID_INT1_RTC1S,         // R&C  SMPL

    PMU_IRQID_INT2_CHGINS,        // R&C  wall charger inserted (high clears CHGRM)
    PMU_IRQID_INT2_CHGRM,         // R&C  wall charger removed (high clears CHGINS)
    PMU_IRQID_INT2_CHGERR,        // R&C  wall charger error (V too high)
    PMU_IRQID_INT2_EOC,           // R&C  wall/usb charging done
    PMU_IRQID_INT2_MBCCHGERR,     // R&C  main battery charge error (over time)

    PMU_IRQID_INT3_JIGONBINS,     // R&C  JIGONB insertion : accessory inserted (high clears ACDRM)
    PMU_IRQID_INT3_JIGONBRM,      // R&C  JIGONB removal : accessory removed (high clears ACDINS)
    PMU_IRQID_INT3_MUIC,          // R&C  MUIC interrupt
    PMU_IRQID_INT3_VERYLOWBAT,    // R&C  The main battery voltage falling below 3.1V

    PMU_MUICID_INT1_ADC,           // R&C ADC Change Interrupt
    PMU_MUICID_INT1_ADCLOW,   	   // R&C ADC Low bit change Interrupt
    PMU_MUICID_INT1_ADCERROR,	   // R&C ADC Error Interrupt

    PMU_MUICID_INT2_CHGTYP,        // R&C Charge Type Interrupt
    PMU_MUICID_INT2_CHGDETRUN,     // R&C Charger Detection Running Status Interrupt
    PMU_MUICID_INT2_DCDTMR,        // R&C DCD Timer Interrupt
    PMU_MUICID_INT2_DBCHG,         // R&C Dead Battery Charging Interrupt
    PMU_MUICID_INT2_VBVOLT,        // R&C VB Voltage Interrupt

    PMU_MUICID_INT3_OVP,           // R&C VB Over Voltage Protection Interrupt

    PMU_TOTAL_IRQ
} PMU_InterruptId_t;


typedef enum{
    PMU_EOCS_50MA,
    PMU_EOCS_60MA,
    PMU_EOCS_70MA,
    PMU_EOCS_80MA,
    PMU_EOCS_90MA,
    PMU_EOCS_100MA,
    PMU_EOCS_110MA,
    PMU_EOCS_120MA,
    PMU_EOCS_130MA,
    PMU_EOCS_140MA,
    PMU_EOCS_150MA,
    PMU_EOCS_160MA,
    PMU_EOCS_170MA,
    PMU_EOCS_180MA,
    PMU_EOCS_190MA,
    PMU_EOCS_200MA
} PMU_EOCS_t;

typedef enum {
	PMU_NO_CHARGER_IS_INUSE = 0,		// No charger is in use
	PMU_USB_IS_INUSE,					// USB charger/cable is in use
    PMU_CDP_IS_INUSE,                   // Charging Downstream port is in use
	PMU_DCP_IS_INUSE,					// dedicated charger: current up to 1.8A is in use
    PMU_SDP_500mA_IS_INUSE,             // Special 500mA charger: current 500mA max is in use
    PMU_SDP_1A_IS_INUSE,                // Special 1A charger: current 1A max is in use
    PMU_DBCP_IS_INUSE                    // Dead Battery Charging - 100mA max is in use
} PMU_ChargerInUse_t;


typedef struct {
  UInt8   intAdc;
  UInt8   intAdcLow;
  UInt8   intAdcError;
  UInt8   intChargerType;
  UInt8   intChargerDetectRun;
  UInt8   intDataContactDetectTimeout;
  UInt8   intDeadBatteryChargeMode;
  UInt8   intVbVoltage;
  UInt8   intEndOfCharge;
  UInt8   intCgMbc;
  UInt8   intVbOverVoltageProtection;
  UInt8   intMbcChgErr;

} PMU_MUIC_INT_T;


/**-----------------------------------------------------------------------------
 *
 * MUIC ADC setting values with 200kOhm pullup
 *
 *------------------------------------------------------------------------------
 */
typedef enum {
  PMU_MUIC_ADC_GND_ADCLOW   = 0x00,                  
  PMU_MUIC_ADC_2K           = 0x01,                  
  PMU_MUIC_ADC_2P6K         = 0x02,                  
  PMU_MUIC_ADC_3P2K         = 0x03,                  
  PMU_MUIC_ADC_4K           = 0x04,                  
  PMU_MUIC_ADC_4P8K         = 0x05,                  
  PMU_MUIC_ADC_6K           = 0x06,                  
  PMU_MUIC_ADC_8K           = 0x07,                  
  PMU_MUIC_ADC_10K          = 0x08,                  
  PMU_MUIC_ADC_12K          = 0x09,                  
  PMU_MUIC_ADC_14K          = 0x0A,
  PMU_MUIC_ADC_17K          = 0x0B,                  
  PMU_MUIC_ADC_20K          = 0x0C,                  
  PMU_MUIC_ADC_24K          = 0x0D,                  
  PMU_MUIC_ADC_29K          = 0x0E,                  
  PMU_MUIC_ADC_34K          = 0x0F,
  PMU_MUIC_ADC_40K          = 0x10,                  
  PMU_MUIC_ADC_50K          = 0x11,                  
  PMU_MUIC_ADC_65K          = 0x12,                  
  PMU_MUIC_ADC_80K          = 0x13,                  
  PMU_MUIC_ADC_102K         = 0x14,                  
  PMU_MUIC_ADC_121K         = 0x15,                  
  PMU_MUIC_ADC_150K         = 0x16,                  
  PMU_MUIC_ADC_200K         = 0x17,                  
  PMU_MUIC_ADC_255K         = 0x18,                  
  PMU_MUIC_ADC_301K         = 0x19,                  
  PMU_MUIC_ADC_365K         = 0x1A,
  PMU_MUIC_ADC_442K         = 0x1B,                  
  PMU_MUIC_ADC_523K         = 0x1C,                  
  PMU_MUIC_ADC_619K         = 0x1D,                  
  PMU_MUIC_ADC_1000K        = 0x1E,                  
  PMU_MUIC_ADC_OPEN         = 0x1F,
  PMU_MUIC_ADC_TABLE_MAX    = 0x20,

  PMU_MUIC_ADC_INIT,
  PMU_MUIC_ADC_GND,

  PMU_MUIC_ADC_TABLE_MIN   = PMU_MUIC_ADC_GND

} PMU_MUIC_ADC_T;


/**-----------------------------------------------------------------------------
 *
 * AL25 CHG_TYP setting values
 *
 *------------------------------------------------------------------------------
 */
typedef enum {
    PMU_MUIC_CHGTYP_NO_VOLTAGE        = 0x00,   /**< No Valid voltage at VB (Vvb < Vvbdet)                          */
    PMU_MUIC_CHGTYP_USB               = 0x01,   /**< Unknown (D+/D- does not present a valid USB charger signature) */
    PMU_MUIC_CHGTYP_DOWNSTREAM_PORT   = 0x02,   /**< Charging Downstream Port                                       */
    PMU_MUIC_CHGTYP_DEDICATED_CHGR    = 0x03,   /**< Dedicated Charger (D+/D- shorted)                              */
    PMU_MUIC_CHGTYP_500MA             = 0x04,   /**< Special 500mA charger, max current 500mA                       */
    PMU_MUIC_CHGTYP_1A                = 0x05,   /**< Special 1A charger, max current 1A                             */
    PMU_MUIC_CHGTYP_RFU               = 0x06,   /**< Reserved                                                       */
    PMU_MUIC_CHGTYP_DB_100MA          = 0x07,   /**< Dead Battery Charging, max current 100mA                       */
    PMU_MUIC_CHGTYP_MAX,

    PMU_MUIC_CHGTYP_INIT,
    PMU_MUIC_CHGTYP_MIN = PMU_MUIC_CHGTYP_NO_VOLTAGE

} PMU_MUIC_CHGTYP_T;

/**-----------------------------------------------------------------------------
 *
 * Interrupt Status Struct
 *
 *------------------------------------------------------------------------------
 */
typedef struct {
  PMU_MUIC_ADC_T     statAdc;
  UInt8               statAdcLow;
  UInt8               statAdcError;
  PMU_MUIC_CHGTYP_T  statChargerType;
  UInt8               statChargerDetectRun;
  UInt8               statDataContactDetectTimeout;
  UInt8               statDeadBatteryChargeMode;
  UInt8               statVbVoltage;
  UInt8               statEndOfCharge;
  UInt8               statCgMbc;
  UInt8               statVbOverVoltageProtection;
  UInt8               statMbcChgErr;
} PMU_MUIC_INTSTAT_T;
#endif // FEATURE_PMIC_MAX8986

// PMU CSR Vout 

typedef enum
{
	//CSRVOUT_0P70	= 0x00,	// not available in max8986.
	CSRVOUT_0P80	= 0x02,
	CSRVOUT_0P86 	= 0x05,
	CSRVOUT_0P88 	= 0x1F,
	CSRVOUT_0P90	= 0x1E,
	CSRVOUT_0P92 	= 0x1D,
	CSRVOUT_0P94	= 0x1C,
	CSRVOUT_0P96	= 0x1B,
	CSRVOUT_0P98	= 0x1A,
	CSRVOUT_1P00 	= 0x19,
	CSRVOUT_1P02 	= 0x18,
	CSRVOUT_1P04 	= 0x17,
	CSRVOUT_1P06	= 0x16,
	CSRVOUT_1P08 	= 0x15,
	CSRVOUT_1P10 	= 0x14,
	CSRVOUT_1P12 	= 0x13,
	CSRVOUT_1P14 	= 0x12,
	CSRVOUT_1P16 	= 0x11,
	CSRVOUT_1P18 	= 0x10,
	CSRVOUT_1P20 	= 0x0F,
	CSRVOUT_1P22	= 0x0E,
	CSRVOUT_1P24	= 0x0D,
	CSRVOUT_1P26	= 0x0C,
	CSRVOUT_1P28	= 0x0B,
	CSRVOUT_1P30	= 0x0A,
	CSRVOUT_1P32	= 0x09,
	CSRVOUT_1P34	= 0x08
} PMU_CSRVOut_en_t;


//*****************************************************************************
/**
*	@param enable (in) 	
*
*******************************************************************************/
void PMU_DRV_USBChargerDetectionEnable (Boolean enable);


//*****************************************************************************
/**
*	@param enable (in) 	
*
*******************************************************************************/
void PMU_DRV_USBDCDEnable(Boolean enable);

//*****************************************************************************
/**
*PMU_DRV_MUICDetectionStatus	
* @return	
*******************************************************************************/

PMU_DRV_MUICType_en_t  PMU_DRV_MUICDetectionStatus(void);

//*****************************************************************************
/**
*	@param enable (in) 	
*
*******************************************************************************/

void PMU_DRV_USBHSEnable(Boolean enable);

//*****************************************************************************
/**
*	@param regNum (in) 
*	@param volNum (in) 
*
*******************************************************************************/

void PMU_DRV_ActivatePowerSupply( UInt8 regNum, UInt8 volNum );


//*****************************************************************************
/**
*	@param regNum (in) 
*
*******************************************************************************/

void PMU_DRV_DeactivatePowerSupply( UInt8 regNum );

//*******************************************************************************
/**
*PMU_StartCharging
*******************************************************************************/

void PMU_StartCharging( void );
//*****************************************************************************
/**
*PMU_DRV_GetChargerInUse	
* @return	
*******************************************************************************/

PMU_ChargerInUse_t PMU_DRV_GetChargerInUse ( void );

//*****************************************************************************
/**
*	@param enable (in) 	
*
*******************************************************************************/

void PMU_EnableCharging(UInt8 enable);


//*****************************************************************************
/**
*	@param isUSBCharger (in) 
*	@param current (in) 
*
*******************************************************************************/

void PMU_SetChargingCurrent(UInt8 isUSBCharger, UInt8 current);

//*****************************************************************************
/**
*PMU_IsUsbPlugIn	
* @return	
*******************************************************************************/
Boolean PMU_IsUsbPlugIn(void);

//*****************************************************************************
/**
*	@param setToOn (in) 	
*
*******************************************************************************/
void PMU_UserPONKEYLock(UInt8 setToOn);
#endif


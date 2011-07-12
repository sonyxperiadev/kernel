/*
Nokia/D/CT/SASW/BBC
CONFIDENTIAL
Deliverable software



            adc_chipset_api.h
            ----------------------------

            SW Include Document - ANSI C



Document identification and location in Synergy/CM

Database:            co1core

Object (this information is substituted by Synergy/CM):
    %name:           adc_chipset_api.h %
    %cvtype:         incl %
    %instance:       co1core_1 %
    %version:        co1core2#7.0 %
    %date_modified:  Thu Dec 16 12:00:00 2010 %

Copyright (c) Nokia. This material, including documentation and any related
computer programs, is protected by copyright controlled by Nokia.
All rights are reserved. Copying, including reproducing, storing, adapting or
translating, any or all of this material requires the prior written consent
of Nokia. This material also contains confidential information, which may not
be disclosed to others without the prior written consent of Nokia.

------------------------------------------------------------------------------
*/


#ifndef ADC_CHIPSET_API_H
#define ADC_CHIPSET_API_H


/* ---------------------------------------------------------------------------
 *
 * CONSTANTS
 *
 */

#define HAL_ADC_CHIPSET_VBAT           0x01    /* Battery Voltage (Scaled)       */
#define HAL_ADC_CHIPSET_VCHAR          0x02    /* Charger Voltage                */
#define HAL_ADC_CHIPSET_ICHAR          0x03    /* Charger Current                */
#define HAL_ADC_CHIPSET_BSI            0x04    /* Battery Size Identification    */
#define HAL_ADC_CHIPSET_BTEMP          0x05    /* Battery Temperature            */
#define HAL_ADC_CHIPSET_PATEMP         0x09    /* Power Amplifier Temperature    */
#define HAL_ADC_CHIPSET_VCXOTEMP       0x0a    /* Crystal Amplifier Temperature    */
#define HAL_ADC_CHIPSET_IBAT           0x0E    /* Battery current (Average)      */
#define HAL_ADC_CHIPSET_IBAT_FAST      0x0F    /* Battery current (One periode)  */
#define HAL_ADC_CHIPSET_VBACK          0x13    /* Backup Battery Voltage         */
#define HAL_ADC_CHIPSET_VBUS           0x19    /* VBUS Voltage                   */
#define HAL_ADC_CHIPSET_IDDET          0x1a    /* ID detection ADC 		 */
#define HAL_ADC_CHIPSET_VENDOR_CH_0    0xC0    /* Vendor specific ADC channel 0  */
#define HAL_ADC_CHIPSET_VENDOR_CH_1    0xC1    /* Vendor specific ADC channel 1  */
#define HAL_ADC_CHIPSET_VENDOR_CH_2    0xC2    /* Vendor specific ADC channel 2  */
#define HAL_ADC_CHIPSET_VENDOR_CH_3    0xC3    /* Vendor specific ADC channel 3  */
#define HAL_ADC_CHIPSET_VENDOR_CH_4    0xC4    /* Vendor specific ADC channel 4  */
#define HAL_ADC_CHIPSET_VENDOR_CH_5    0xC5    /* Vendor specific ADC channel 5  */
#define HAL_ADC_CHIPSET_VENDOR_CH_6    0xC6    /* Vendor specific ADC channel 6  */
#define HAL_ADC_CHIPSET_VENDOR_CH_7    0xC7    /* Vendor specific ADC channel 7  */
#define HAL_ADC_CHIPSET_VENDOR_CH_8    0xC8    /* Vendor specific ADC channel 8  */
#define HAL_ADC_CHIPSET_VENDOR_CH_9    0xC9    /* Vendor specific ADC channel 9  */
#define HAL_ADC_CHIPSET_VENDOR_CH_10   0xCA    /* Vendor specific ADC channel 10 */
#define HAL_ADC_CHIPSET_VENDOR_CH_11   0xCB    /* Vendor specific ADC channel 11 */
#define HAL_ADC_CHIPSET_VENDOR_CH_12   0xCC    /* Vendor specific ADC channel 12 */
#define HAL_ADC_CHIPSET_VENDOR_CH_13   0xCD    /* Vendor specific ADC channel 13 */
#define HAL_ADC_CHIPSET_VENDOR_CH_14   0xCE    /* Vendor specific ADC channel 14 */
#define HAL_ADC_CHIPSET_VENDOR_CH_15   0xCF    /* Vendor specific ADC channel 15 */
#define HAL_ADC_CHIPSET_MAIN_CAL       0xFE    /* Main ADC channel               */
#define HAL_ADC_MAIN_CAL               0xFE    /* Main ADC channel (Obsolete)    */

typedef enum
    {
	HAL_ADC_ERR_SUCCESS = 0,
	HAL_ADC_ERR_NOTSUP = -1,
	HAL_ADC_ERR_NOTREADY = -2,
	HAL_ADC_ERR_OVERFLOW = -3,
	HAL_ADC_ERR_FAIL = -4
    } HAL_ADC_ERR_TYPE;


typedef void (*HAL_ADC_CALLBACK) (u32 event,
                                  int  p1,
                                  void*  p2);


/* Read/convert raw values */
int hal_adc_raw_read (u8 channel, u32* value, HAL_ADC_CALLBACK adc_handler, void* context);
int hal_adc_unit_convert (u8 channel, u32 value);


#endif /* ADC_CHIPSET_API_H */
/* End of Include File */



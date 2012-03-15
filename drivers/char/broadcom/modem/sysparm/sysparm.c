/*****************************************************************************
*  Copyright 2003 - 2011 Broadcom Corporation.  All rights reserved.
*
*  Unless you and Broadcom execute a separate written software license
*  agreement governing use of this software, this software is licensed to you
*  under the terms of the GNU General Public License version 2, available at
*  http://www.gnu.org/copyleft/gpl.html (the "GPL").
*
*  Notwithstanding the above, under no circumstances may you combine this
*  software in any way with any other Broadcom software provided under a
*  license other than the GPL, without Broadcom's express prior written
*  consent.
*
*****************************************************************************/

//
//      SKELETON_DRIVER==1 will build a skeleton driver for debugging
//      the sysparms interface.  The skeleton driver maps to COMMS sysparm 
//      shared memory but does not implement specific sysparm accessor APIs.  
//
#define SKELETON_DRIVER 0

#include <linux/io.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/module.h>
#include <linux/interrupt.h>

#include <linux/broadcom/mobcom_types.h>
#include <linux/broadcom/bcmtypes.h>

#include <mach/comms/platform_mconfig.h>
#include <linux/broadcom/bcm_fuse_sysparm_CIB.h>

static int sysparm_init(void);

static UInt8 *sys_data_dep;
static SysparmIndex_t *sysparm_index;
volatile static UInt32 *sysparm_ready_ind_ptr;
volatile static UInt32 *total_index_ptr;

#if !SKELETON_DRIVER
  // need to match sysparm.h from RTOS
  // SysAudioParm_t     audio_parm[AUDIO_MODE_NUMBER_VOICE];  //(number of audio devices) X (modes per device)
//#define       AUDIO_MODE_NUMBER               9       ///< Up to 9 Audio Profiles (modes) after 213x1
//#define AUDIO_APP_NUMBER        2   // must be consistent with parm_audio.txt
//#define AUDIO_MODE_NUMBER_VOICE       (AUDIO_MODE_NUMBER*AUDIO_APP_NUMBER)
static SysAudioParm_t audio_parm_table[AUDIO_MODE_NUMBER_VOICE];
static SysIndMultimediaAudioParm_t mmaudio_parm_table[AUDIO_MODE_NUMBER];

static UInt8 gpioInit_table[GPIO_INIT_REC_NUM][GPIO_INIT_FIELD_NUM];
#endif // !SKELETON_DRIVER

static int fuse_sysparm_initialised = 0;

#if !SKELETON_DRIVER
// utility function used during IMEI retrieval
static UInt8 CalculateCheckDigit(UInt8 *inImeiStrPtr);
#define SYS_IMEI_LEN 8
#endif // !SKELETON_DRIVER

// ** uncomment to support debugging sysparm mismatches during CP updates
#define SYSPARM_DEBUG 1

//******************************************************************************
//
// Function Name: SYSPARM_GetNameFromPA
//
// Description:  
//
//
// Notes:
//
//******************************************************************************
static int SYSPARM_GetNameFromPA(char *nameaddr, char *str)
{
	char *name_ptr;
	int name_len;

	//printk(KERN_INFO"[sysparm]: SYSPARM_GetNameFromPA: nameaddr:0x%08lX\n",(UInt32)nameaddr);
	name_ptr =
	    (char *)ioremap_nocache((UInt32)nameaddr, MAX_SYSPARM_NAME_SIZE);
	if (!name_ptr) {
		pr_err
		    ("[sysparm]: SYSPARM_GetNameFromPA: nameaddr ioremap failed\n");
		return 0;
	}

	for (name_len = 0; name_len < MAX_SYSPARM_NAME_SIZE; name_len++) {
		if (name_ptr[name_len] == '\0')
			break;
	}

	if (name_len == 0 || name_len == MAX_SYSPARM_NAME_SIZE) {
		pr_err
		    ("[sysparm]: SYSPARM_GetNameFromPA: name_len is invalid:%d\n",
		     name_len);
		iounmap(name_ptr);
		return 0;
	}

	strcpy(str, name_ptr);

	iounmap(name_ptr);
	return 1;
}

//******************************************************************************
//
// Function Name: SYSPARM_GePAtByIndex
// Params:  name: sysparm name (case sensitive, must match SysCalData_t fields)
//          size: specifies the size of the sysparm in number of bytes
//          flag: specifies required flag value
// Return:  ptr  = pointer to the sysparm physical adress if it is found
//          NULL = sysparm not found, or size/flag mismatch
//
//******************************************************************************
static UInt32 SYSPARM_GePAtByIndex(char *name, unsigned int size,
				   unsigned int flag)
{
	int i;
	char sysparm_name[MAX_SYSPARM_NAME_SIZE];

	for (i = 0; i < *total_index_ptr; i++) {
		memset(sysparm_name, 0, MAX_SYSPARM_NAME_SIZE);

		if (!SYSPARM_GetNameFromPA(sysparm_index[i].name, sysparm_name)) {
			pr_err("[sysparm]: SYSPARM_GePAtByIndex: i:%d\n", i);
			return 0;
		}

		if (!strcmp(sysparm_name, name)) {
			if (size != sysparm_index[i].size) {
				pr_err
				    ("[sysparm]: SYSPARM_GePAtByIndex: sysparm: \"%s\" size:%u mismatch input size:%u\n",
				     name, sysparm_index[i].size, size);
				return 0;
			}

			if (flag != sysparm_index[i].flag) {
				pr_err
				    ("[sysparm]: SYSPARM_GePAtByIndex: sysparm: \"%s\" flag:%u mismatch input flag:%u\n",
				     name, sysparm_index[i].flag, flag);
				return 0;
			}

			return (UInt32)sysparm_index[i].ptr;
		}
	}

	return 0;
}

//******************************************************************************
//
// Function Name: SYSPARM_SetByIndex
// Params:  name: sysparm name (case sensitive, must match SysCalData_t fields)
//      ptr:  pointer to the sysparm to be set
//      size: specifies the size of the sysparm in number of bytes
//      flag: specifies required flag value
// Return:  1 = sysparm is set successfully
//      0 = sysparm not found, or size/flag mismatch
//
//******************************************************************************
#if 0
static int SYSPARM_SetByIndex(void *ptr, char *name, unsigned int size,
			      unsigned int flag)
{
	UInt32 i, mask;

	for (i = 0; i < *total_index_ptr; i++) {
		if (!strcmp(sysparm_index[i].name, name)) {
			if (size != sysparm_index[i].size)
				return 0;
			if (flag != sysparm_index[i].flag)
				return 0;
			local_irq_save(mask);
			memcpy(sysparm_index[i].ptr, ptr, size);
			local_irq_restore(mask);
			return 1;
		}
	}
	return 0;
}
#endif

/*****************************************************************************
 * Function Name: SYSPARM_GetParamU32ByName
 *
 * Description:  Generic function to read value of 32-bit parameters from
 *               sysparm. Returns 0 on success, negative on error. The
 *               arguments are:
 *
 *               name - name of the parameter.
 *               parm - 32-bit value of the parameter is returned via this
 *                      address (memory must be allocated by caller).
 *
 * Notes:
 *****************************************************************************/
int SYSPARM_GetParmU32ByName(char *name, unsigned int *parm)
{
	UInt32 parm_addr;
	UInt32 *parm_ptr;

	if (name == NULL || parm == NULL) {
		pr_err("%s: invalid arguments\n", __func__);
		return -EINVAL;
	}

	if (!fuse_sysparm_initialised) {
		if (sysparm_init()) {
			pr_err("%s: Fuse sysparm is not yet initialised\n",
			       __func__);
			return -EIO;
		}
	}

	parm_addr = SYSPARM_GePAtByIndex(name, sizeof(*parm_ptr), 1);
	if (!parm_addr)
		return -ENOENT;

	parm_ptr = (UInt32 *)ioremap_nocache(parm_addr, sizeof(UInt32));
	if (!parm_ptr) {
		pr_err("%s: ioremap failed\n", __func__);
		return -EIO;
	}

	*parm = ioread32(parm_ptr);

	iounmap(parm_ptr);
	return 0;
}

#if !SKELETON_DRIVER

/*****************************************************************************
 * Function Name: SYSPARM_GetPMURegSettings
 *
 * Description:  Read pmu register settings from sysparm file. The arguments
 *               are:
 *
 *               index - index of the pmu reg to be read.
 *               parm - 32-bit value of the parameter is returned via this
 *                      address (memory must be allocated by caller).
 *
 * Notes:
 *****************************************************************************/
#define PARM_PMU_REG_NAME		"pmu_reg_settings"

int SYSPARM_GetPMURegSettings(int index, unsigned int *parm)
{
/******
	UInt32  parm_addr;
	UInt8   *parm_ptr;

	if (index < 0 || index >= PARM_PMU_REG_TOTAL || parm == NULL) {
		pr_err("%s: invalid arguments\n", __func__);
		return -EINVAL;
	}

	if (!fuse_sysparm_initialised) {
		if (sysparm_init()) {
			pr_err("%s: Fuse sysparm is not yet initialised\n",
				__func__);
			return -EIO;
		}
	}

	parm_addr = SYSPARM_GePAtByIndex(PARM_PMU_REG_NAME, PARM_PMU_REG_TOTAL,
					 1);
	if (!parm_addr)
		return -ENOENT;

	parm_ptr = (UInt8 *)ioremap_nocache(parm_addr, PARM_PMU_REG_TOTAL);
	if (!parm_ptr) {
		pr_err("%s: ioremap failed\n", __func__);
		return -EIO;
	}

	*parm = ioread8(parm_ptr + index);

	iounmap(parm_ptr);
*****/
	return 0;
}

//******************************************************************************
//
// Function Name: SYSPARM_GetLogFormat
//
// Description:   Return default log output format
//                0: ASCII, 1: binary(Mobile Analyzer), 2: MTT
//
// Notes:
//
//******************************************************************************
UInt16 SYSPARM_GetLogFormat(void)
{
	UInt32 log_format_addr;
	UInt16 *log_format_ptr;
	UInt16 log_format;

	if (!fuse_sysparm_initialised) {
		if (sysparm_init()) {
			pr_err
			    ("[sysparm]: Fuse sysparm is not yet initialised\n");
			return 0;
		}
	}

	log_format_addr =
	    SYSPARM_GePAtByIndex("log_format", sizeof(*log_format_ptr), 1);
	if (!log_format_addr) {
		pr_err("[sysparm]: Get log_format PA failed\n");
		return 0;
	}

	log_format_ptr =
	    (UInt16 *)ioremap_nocache(log_format_addr, sizeof(UInt16));
	if (!log_format_ptr) {
		pr_err("[sysparm]: log_format_addr ioremap failed\n");
		return 0;
	}

	log_format = *log_format_ptr;

	iounmap(log_format_ptr);
	return log_format;
}

//******************************************************************************
//
// Function Name: APSYSPARM_GetAudioParmAccessPtr
//
// Description:   Get access pointer to root of audio sysparm structure 
//
// Notes:     This is only applicable to audio tuning parameters.
//
//******************************************************************************
SysAudioParm_t *APSYSPARM_GetAudioParmAccessPtr(void)
{
	UInt32 audio_parm_addr;
	SysAudioParm_t *audio_parm_ptr;
	static int loaded_audio_parm_table = 0;

	if (loaded_audio_parm_table)
		return &audio_parm_table[0];

	if (!fuse_sysparm_initialised) {
		if (sysparm_init()) {
			pr_err
			    ("[sysparm]: Fuse sysparm is not yet initialised\n");
			return 0;
		}
	}

	audio_parm_addr =
	    SYSPARM_GePAtByIndex("audio_parm", sizeof(audio_parm_table), 1);
	if (!audio_parm_addr) {
		pr_err("[sysparm]: Get audio_parm PA failed\n");
		return 0;
	}

	audio_parm_ptr =
	    (SysAudioParm_t *) ioremap_nocache(audio_parm_addr,
					       sizeof(audio_parm_table));

	if (!audio_parm_ptr) {
		pr_err("[sysparm]: audio_parm_addr ioremap failed\n");
		return 0;
	}

	memcpy(&audio_parm_table[0], audio_parm_ptr, sizeof(audio_parm_table));
	iounmap(audio_parm_ptr);
	loaded_audio_parm_table = 1;
	return &audio_parm_table[0];
}

//******************************************************************************
//
// Function Name: APSYSPARM_GetMultimediaAudioParmAccessPtr
//
// Description:   Get access pointer to root of MM audio sysparm structure 
//
// Notes:     This is only applicable to audio tuning parameters.
//
//******************************************************************************
SysIndMultimediaAudioParm_t *APSYSPARM_GetMultimediaAudioParmAccessPtr(void)
{
	UInt32 mmaudio_parm_addr;
	SysIndMultimediaAudioParm_t *mmaudio_parm_ptr;
	static int loaded_mmaudio_parm_table = 0;

	if (loaded_mmaudio_parm_table)
		return &mmaudio_parm_table[0];

	if (!fuse_sysparm_initialised) {
		if (sysparm_init()) {
			pr_err
			    ("[sysparm]: Fuse sysparm is not yet initialised\n");
			return 0;
		}
	}

	mmaudio_parm_addr =
	    SYSPARM_GePAtByIndex("mmaudio_parm", sizeof(mmaudio_parm_table), 1);
	if (!mmaudio_parm_addr) {
		pr_err("[sysparm]: Get mmaudio_parm PA failed\n");
		return 0;
	}

	mmaudio_parm_ptr =
	    (SysIndMultimediaAudioParm_t *) ioremap_nocache(mmaudio_parm_addr,
							    sizeof
							    (mmaudio_parm_table));

	if (!mmaudio_parm_ptr) {
		pr_err("[sysparm]: audio_parm_addr ioremap failed\n");
		return 0;
	}

	memcpy(&mmaudio_parm_table[0], mmaudio_parm_ptr,
	       sizeof(mmaudio_parm_table));
	iounmap(mmaudio_parm_ptr);
	loaded_mmaudio_parm_table = 1;

	return &mmaudio_parm_table[0];
}

//******************************************************************************
//
// Function Name: SYSPARM_GetDefault4p2VoltReading( void )
//
// Description:   Return batt_Default4p2VoltReading
//
// Notes:
//
//******************************************************************************
UInt16 SYSPARM_GetDefault4p2VoltReading(void)
{
	UInt32 batt_Default4p2VoltReading_addr;
	UInt16 *batt_Default4p2VoltReading_ptr;
	UInt16 batt_Default4p2VoltReading_value;

	if (!fuse_sysparm_initialised) {
		if (sysparm_init()) {
			pr_err
			    ("[sysparm]: Fuse sysparm is not yet initialised\n");
			return 0;
		}
	}

	batt_Default4p2VoltReading_addr =
	    SYSPARM_GePAtByIndex("batt_Default4p2VoltReading",
				 sizeof(*batt_Default4p2VoltReading_ptr), 1);
	if (!batt_Default4p2VoltReading_addr) {
		pr_err("[sysparm]: Get batt_Default4p2VoltReading PA failed\n");
		return 0;
	}

	batt_Default4p2VoltReading_ptr =
	    (UInt16 *)ioremap_nocache(batt_Default4p2VoltReading_addr,
				      sizeof(*batt_Default4p2VoltReading_ptr));
	if (!batt_Default4p2VoltReading_ptr) {
		pr_err
		    ("[sysparm]: batt_Default4p2VoltReading_addr ioremap failed\n");
		return 0;
	}
	batt_Default4p2VoltReading_value = *batt_Default4p2VoltReading_ptr;

	iounmap(batt_Default4p2VoltReading_ptr);
	return batt_Default4p2VoltReading_value;
}

//******************************************************************************
//
// Function Name: SYSPARM_GetActual4p2VoltReading( void )
//
// Description:   Return batt_Actual4p2VoltReading
//
// Notes:
//
//******************************************************************************
UInt16 SYSPARM_GetActual4p2VoltReading(void)
{
	UInt32 batt_Actual4p2VoltReading_addr;
	UInt16 *batt_Actual4p2VoltReading_ptr;
	UInt16 batt_Actual4p2VoltReading_value;

	if (!fuse_sysparm_initialised) {
		if (sysparm_init()) {
			pr_err
			    ("[sysparm]: Fuse sysparm is not yet initialised\n");
			return 0;
		}
	}

	batt_Actual4p2VoltReading_addr =
	    SYSPARM_GePAtByIndex("batt_Actual4p2VoltReading",
				 sizeof(*batt_Actual4p2VoltReading_ptr), 1);
	if (!batt_Actual4p2VoltReading_addr) {
		pr_err("[sysparm]: Get batt_Default4p2VoltReading PA failed\n");
		return 0;
	}

	batt_Actual4p2VoltReading_ptr =
	    (UInt16 *)ioremap_nocache(batt_Actual4p2VoltReading_addr,
				      sizeof(*batt_Actual4p2VoltReading_ptr));
	if (!batt_Actual4p2VoltReading_ptr) {
		pr_err
		    ("[sysparm]: batt_Actual4p2VoltReading_addr ioremap failed\n");
		return 0;
	}
	batt_Actual4p2VoltReading_value = *batt_Actual4p2VoltReading_ptr;

	iounmap(batt_Actual4p2VoltReading_ptr);
	return batt_Actual4p2VoltReading_value;
}

//******************************************************************************
//
// Function Name: SYSPARM_GetBattLowThresh( void )
//
// Description:   Get battery low charge threshold
//
// Notes:
//
//******************************************************************************
UInt16 SYSPARM_GetBattLowThresh(void)
{
	UInt32 default_batt_low_thresh_addr;
	UInt16 *default_batt_low_thresh_ptr;
	UInt16 default_batt_low_thresh_value;

	if (!fuse_sysparm_initialised) {
		if (sysparm_init()) {
			pr_err
			    ("[sysparm]: Fuse sysparm is not yet initialised\n");
			return 0;
		}
	}

	default_batt_low_thresh_addr =
	    SYSPARM_GePAtByIndex("batt_low_thresh",
				 sizeof(*default_batt_low_thresh_ptr), 1);
	if (!default_batt_low_thresh_addr) {
		pr_err
		    ("[sysparm]: Get default_batt_low_thresh_addr PA failed\n");
		return 0;
	}

	default_batt_low_thresh_ptr =
	    (UInt16 *)ioremap_nocache(default_batt_low_thresh_addr,
				      sizeof(*default_batt_low_thresh_ptr));
	if (!default_batt_low_thresh_ptr) {
		pr_err
		    ("[sysparm]: default_batt_low_thresh_addr ioremap failed\n");
		return 0;
	}
	default_batt_low_thresh_value = *default_batt_low_thresh_ptr;

	iounmap(default_batt_low_thresh_ptr);
	return default_batt_low_thresh_value;

}

//******************************************************************************
//
// Function Name: SYSPARM_GetActualLowVoltReading( void )
//
// Description:   Return batt_ActualLowVoltReading from sysparms
//
// Notes:
//
//******************************************************************************
UInt16 SYSPARM_GetActualLowVoltReading(void)
{
	UInt32 default_batt_ActualLowVoltReading_addr;
	UInt16 *default_batt_ActualLowVoltReading_ptr;
	UInt16 default_batt_ActualLowVoltReading_value;

	if (!fuse_sysparm_initialised) {
		if (sysparm_init()) {
			pr_err
			    ("[sysparm]: Fuse sysparm is not yet initialised\n");
			return 0;
		}
	}

	default_batt_ActualLowVoltReading_addr =
	    SYSPARM_GePAtByIndex("batt_ActualLowVoltReading",
				 sizeof(*default_batt_ActualLowVoltReading_ptr),
				 1);
	if (!default_batt_ActualLowVoltReading_addr) {
		pr_err
		    ("[sysparm]: Get default_batt_ActualLowVoltReading_addr PA failed\n");
		return 0;
	}

	default_batt_ActualLowVoltReading_ptr =
	    (UInt16 *)ioremap_nocache(default_batt_ActualLowVoltReading_addr,
				      sizeof
				      (*default_batt_ActualLowVoltReading_ptr));
	if (!default_batt_ActualLowVoltReading_ptr) {
		pr_err
		    ("[sysparm]: default_batt_ActualLowVoltReading_addr ioremap failed\n");
		return 0;
	}
	default_batt_ActualLowVoltReading_value =
	    *default_batt_ActualLowVoltReading_ptr;

	iounmap(default_batt_ActualLowVoltReading_ptr);
	return default_batt_ActualLowVoltReading_value;

}

//******************************************************************************
//
// Function Name:       SYSPARM_GetBattEmptyThresh
//
// Description:         Get battery empty charge threshold
//
// Notes:
//
//******************************************************************************
UInt16 SYSPARM_GetBattEmptyThresh(void)
{
	UInt32 default_batt_empty_thresh_addr;
	UInt16 *default_batt_empty_thresh_ptr;
	UInt16 default_batt_empty_thresh_value;

	if (!fuse_sysparm_initialised) {
		if (sysparm_init()) {
			pr_err
			    ("[sysparm]: Fuse sysparm is not yet initialised\n");
			return 0;
		}
	}

	default_batt_empty_thresh_addr =
	    SYSPARM_GePAtByIndex("batt_empty_thresh",
				 sizeof(*default_batt_empty_thresh_ptr), 1);
	if (!default_batt_empty_thresh_addr) {
		pr_err
		    ("[sysparm]: Get default_batt_empty_thresh_addr PA failed\n");
		return 0;
	}

	default_batt_empty_thresh_ptr =
	    (UInt16 *)ioremap_nocache(default_batt_empty_thresh_addr,
				      sizeof(*default_batt_empty_thresh_ptr));
	if (!default_batt_empty_thresh_ptr) {
		pr_err
		    ("[sysparm]: default_batt_empty_thresh_addr ioremap failed\n");
		return 0;
	}
	default_batt_empty_thresh_value = *default_batt_empty_thresh_ptr;

	iounmap(default_batt_empty_thresh_ptr);
	return default_batt_empty_thresh_value;
}

//******************************************************************************
//
// Function Name: SYSPARM_GetGPIO_Default_Value
//
// Description:   Obtain one record of a GPIO pin settings based on index
//
// Notes:
//
//******************************************************************************
UInt8 *SYSPARM_GetGPIO_Default_Value(UInt8 gpio_index)
{
	UInt32 gpioInit_addr;
	UInt8 *gpioInit_ptr;

	if (!fuse_sysparm_initialised) {
		if (sysparm_init()) {
			pr_err
			    ("[sysparm]: Fuse sysparm is not yet initialised\n");
			return 0;
		}
	}

	gpioInit_addr =
	    SYSPARM_GePAtByIndex("gpioInit", sizeof(gpioInit_table), 1);
	if (!gpioInit_addr) {
		pr_err("[sysparm]: Get gpioInit PA failed\n");
		return 0;
	}

	gpioInit_ptr =
	    (UInt8 *)ioremap_nocache(gpioInit_addr, sizeof(gpioInit_table));
	if (!gpioInit_ptr) {
		pr_err("[sysparm]: gpioInit_addr ioremap failed\n");
		return 0;
	}

	memcpy(&gpioInit_table[gpio_index][0], &gpioInit_ptr[gpio_index * 5],
	       GPIO_INIT_FIELD_NUM * sizeof(UInt8));

	iounmap(gpioInit_ptr);
	return &gpioInit_table[gpio_index][0];
}

//******************************************************************************
// Function Name:      SYSPARM_GetImeiStr
//
// Description:        This function gets the 14-digit IMEI saved in Sys Parm
//                                         and calculates the 15th digit (check digit) based on the
//                                         14 digits. It then returns the 15-digit IMEI in a
//                                         Null-terminated string.
//
//
// UInt8* inImeiStr - Buffer to store the returned 15-digit IMEI buffer. Must
//                                               be at least 16-byte in size.
//
//******************************************************************************
Boolean SYSPARM_GetImeiStr(UInt8 *inImeiStr)
{
	UInt8 indexA = 0;
	UInt8 indexB = 0;
	UInt32 imei_addr;
	UInt8 *pCurrImei;
	UInt8 *pSysparmImei;
	Boolean bIMEIError = FALSE;

	/* Default IMEI (allocated to Broadcom as test IMEI) to return in case:
	 * 1. The IMEI digits in Sysparm are all 0's or
	 * 2. The IMEI digits in Sysparm are invalid
	 */
	static const UInt8 default_imei[SYS_IMEI_LEN] =
	    { 0x0F, 0x10, 0x60, 0x00, 0x00, 0x10, 0x32, 0xF4 };

	// sanity check 
	if (!inImeiStr) {
		pr_err("[sysparm]: SYSPARM_GetImeiStr inImeiStr NULL\n");
		return FALSE;
	}

	if (!fuse_sysparm_initialised) {
		if (sysparm_init()) {
			pr_err
			    ("[sysparm]: Fuse sysparm is not yet initialised\n");
			return FALSE;
		}
	}
	// lookup address of IMEI sysparm data by name
	imei_addr = SYSPARM_GePAtByIndex("imei", IMEI_SIZE, 1);
	if (!imei_addr) {
		pr_err("[sysparm]: Get imei_addr PA failed\n");
		return FALSE;
	}
	// do some linux memory magic...
	pSysparmImei = (UInt8 *)ioremap_nocache(imei_addr, IMEI_SIZE);
	if (!pSysparmImei) {
		pr_err("[sysparm]: imei_addr ioremap failed\n");
		return FALSE;
	}

	/* Check the validity of sysparm IMEI */
	for (indexA = 0; indexA < SYS_IMEI_LEN; indexA++) {
		if ((indexA != (SYS_IMEI_LEN - 1)
		     && (pSysparmImei[indexA] & 0xF0) > 0x90) || (indexA != 0
								  &&
								  (pSysparmImei
								   [indexA] &
								   0x0F) >
								  0x09)) {
			pr_err
			    ("[sysparm]: invalid imei digit, using default\n");
			bIMEIError = TRUE;	// invalid IMEI digit
		}
	}

	// check for all-zero IMEI
	if (!bIMEIError &&
	    ((pSysparmImei[0] & 0xF0) == 0 && pSysparmImei[1] == 0
	     && pSysparmImei[2] == 0 && pSysparmImei[3] == 0
	     && pSysparmImei[4] == 0 && pSysparmImei[5] == 0
	     && pSysparmImei[6] == 0 && (pSysparmImei[7] & 0x0F) == 0)
	    ) {
		// all-zero IMEI considered error
		pr_err("[sysparm]: all zero sysparm imei, using default\n");
		bIMEIError = TRUE;
	}

	if (bIMEIError) {
		// invalid sysparm IMEI, so use hardcoded default testing value
		pCurrImei = (UInt8 *)default_imei;
	} else {
		pCurrImei = pSysparmImei;
	}

	// convert from BCD to NULL terminated string
	indexA = 0;
	inImeiStr[indexB] = pCurrImei[indexA] >> 4;	// First Digit
	for (indexA = 1, indexB = 1; indexA < 7; indexA++)	// 2-13th Digit
	{
		inImeiStr[indexB++] = pCurrImei[indexA] & 0x0F;
		inImeiStr[indexB++] = pCurrImei[indexA] >> 4;
	}
	inImeiStr[indexB++] = pCurrImei[indexA] & 0x0F;	// 14th Digit
	inImeiStr[indexB++] = CalculateCheckDigit(inImeiStr);	// 15th Digit
	inImeiStr[indexB] = '\0';

	indexA = 0;

	while (indexA < 15)	// 1-15th Digit
	{
		inImeiStr[indexA] = inImeiStr[indexA] | 0x30;
		indexA++;
	}

	iounmap(pSysparmImei);

	return TRUE;
}

//******************************************************************************
// Function Name:       CalculateCheckDigit
//
// Description:         Computate the Check Digit (CD) for the IMEI (2.16)
//                                      Computation of CD from the IMEI proceeds as follows:
//                                      Step 1: Double the values of the odd labelled digits
//                                                      D1, D3, D5 ... D13 of the IMEI.
//                                      Step 2: Add together the individual digits of all the
//                                                      seven numbers obtained in Step 1 as follow
//                                                      while and if digit odd
//                                                      {
//                                                              SummStep1 += (Step1[i] / 10) + (Step1[i] % 10);
//                                                      }
//                                                      , and then add this sum to the sum of all the even
//                                                      labelled digits D2, D4, D6 ... D14 of the IMEI.
//                                      Step 3: If the number obtained in Step 2 ends in 0, then
//                                                      set CD to be 0. If the number obtained in Step 2
//                                                      does not end in 0, then set CD to be that number
//                                                      subtracted from the next higher number which does
//                                                      end in 0.
//******************************************************************************
static UInt8 CalculateCheckDigit(UInt8 *inImeiStrPtr)
{
	int theSum = 0;
	UInt8 theIndex = 1;
	const UInt8 theMudTen = 10;
	UInt8 *theImeiStrPtr = inImeiStrPtr;

	while (theIndex++ < 15) {
		if (theIndex % 2) {
			theSum += (((*theImeiStrPtr * 2) / theMudTen) +
				   ((*theImeiStrPtr * 2) % theMudTen));
		} else {
			theSum += *theImeiStrPtr;
		}

		theImeiStrPtr++;
	}

	return ((theMudTen - (theSum % theMudTen)) % theMudTen);
}
#endif // !SKELETON_DRIVER

//******************************************************************************
//
// Function Name: sysparm_init
//
// Description:   Start system parameter driver initialise
//
//
// Notes:
//
//******************************************************************************
static int sysparm_init(void)
{
	int sysparm_ready_count = 0;

	pr_info
	    ("[sysparm]: sysparm driver start (PARM_DEP_RAM_ADDR=%x PARM_DEP_SIZE=%x)\n",
	     (int)PARM_DEP_RAM_ADDR, (int)PARM_DEP_SIZE);

	if (fuse_sysparm_initialised) {
		pr_info("[sysparm]: init already done\n");
		return 0;
	}

	sys_data_dep = ioremap_nocache(PARM_DEP_RAM_ADDR, PARM_DEP_SIZE);
	if (!sys_data_dep) {
		pr_err("[sysparm]: PARM_DEP_RAM_ADDR ioremap failed\n");
		BUG();
	}

	sysparm_ready_ind_ptr = (UInt32 *)(sys_data_dep + 0x4000);

	while (SYSPARM_INDEX_READY_INDICATOR != *sysparm_ready_ind_ptr) {
		if (sysparm_ready_count >= 5) {
			pr_err("[sysparm]: sys_data_dep+0x0: %x %x %x %x\n",
			       (int)(*(unsigned long *)(sys_data_dep + 0x0)),
			       (int)(*(unsigned long *)(sys_data_dep + 0x4)),
			       (int)(*(unsigned long *)(sys_data_dep + 0x8)),
			       (int)(*(unsigned long *)(sys_data_dep + 0xc)));

			pr_err("[sysparm]: sysparm_ind_ready_ind: %x\n",
			       (int)(*sysparm_ready_ind_ptr));

			pr_err
			    ("[sysparm]: timeout waiting for ready indicator\n");

			BUG();
		}

		msleep(25);
		sysparm_ready_count++;
	}

	total_index_ptr =
	    (UInt32 *)((UInt32)sysparm_ready_ind_ptr +
		       sizeof(sysparm_ready_ind_ptr));
	sysparm_index =
	    (SysparmIndex_t *) ((UInt32)total_index_ptr +
				sizeof(total_index_ptr));

	pr_info("[sysparm]: sysparm_init ok\n");
	fuse_sysparm_initialised = 1;

#if !SKELETON_DRIVER
#ifdef SYSPARM_DEBUG
	{
		UInt16 tmp;

		SysAudioParm_t *pAudioTmp;
		pr_info("[sysparm] audio parm table size: 0x%x bytes\n",
			sizeof(audio_parm_table));
		pAudioTmp = APSYSPARM_GetAudioParmAccessPtr();
		if (!pAudioTmp) {
			pr_err
			    ("[sysparm]: APSYSPARM_GetAudioParmAccessPtr failed\n");
			BUG();
		}

		pr_info("ext_speaker_pga_l 0x%x\n",
			pAudioTmp->ext_speaker_pga_l);
		pr_info("ext_speaker_pga_l 0x%x\n",
			audio_parm_table[0].ext_speaker_pga_l);
		pr_info("mode 1, ext_speaker_pga_l 0x%x\n",
			audio_parm_table[1].ext_speaker_pga_l);

		tmp = SYSPARM_GetBattEmptyThresh();
		pr_info("SYSPARM_GetBattEmptyThresh: %d\n", tmp);
		tmp = SYSPARM_GetActual4p2VoltReading();
		pr_info("SYSPARM_GetActual4p2VoltReading: %d\n", tmp);
		tmp = SYSPARM_GetDefault4p2VoltReading();
		pr_info("SYSPARM_GetDefault4p2VoltReading: %d\n", tmp);
		tmp = SYSPARM_GetActualLowVoltReading();
		pr_info("SYSPARM_GetActualLowVoltReading: %d\n", tmp);
	}
#endif
#endif // !SKELETON_DRIVER

	return 0;
}

//******************************************************************************
//
// Function Name: sysparm_module_init
//
// Description:  Function to satisfy the condition of module_init
//
//
// Notes:
//
//******************************************************************************
static int __init sysparm_module_init(void)
{
#if SKELETON_DRIVER
	sysparm_init();
#endif // SKELETON_DRIVER
	return 0;
}

//******************************************************************************
//
// Function Name: sysparm_module_exit
//
// Description:   Unload system parameter driver
//
//
// Notes:
//
//******************************************************************************
static void __exit sysparm_module_exit(void)
{
	iounmap(sys_data_dep);
	return;
}

module_init(sysparm_module_init);
module_exit(sysparm_module_exit);

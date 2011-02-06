/****************************************************************************
*
*     Copyright (c) 2009 Broadcom Corporation
*
*   Unless you and Broadcom execute a separate written software license 
*   agreement governing use of this software, this software is licensed to you 
*   under the terms of the GNU General Public License version 2, available 
*    at http://www.gnu.org/licenses/old-licenses/gpl-2.0.html (the "GPL"). 
*
*   Notwithstanding the above, under no circumstances may you combine this 
*   software in any way with any other Broadcom software provided under a license 
*   other than the GPL, without Broadcom's express prior written consent.
*
****************************************************************************/

/*
*
*   @file   bcm_kril_simlockfun.c
*
*   @brief  This file contains the SIMLOCK implementations defined in GSM 02.22.
*
****************************************************************************/
#include <linux/io.h>
#include "bcm_kril_common.h"
#include "bcm_kril_simlockfun.h"
#include "aes.h"
#include "crypto_api.h"
#ifdef CONFIG_BRCM_CKBLOCK_READER
#include <linux/broadcom/ckblock_reader.h>
#endif

//-------------------------------------------------
// Global variables
//-------------------------------------------------

SIMLOCK_STATE_t g_CurrentSimLockState;

extern SIMLOCK_SIM_DATA_t* GetSIMData(void);
//-------------------------------------------------
// Local defines
//-------------------------------------------------
/* Default PH-SIM simlock password */
#define DEFAULT_PHSIM_LOCK_PWD "0000"


/* Just a magic number to indicate non-volatile SIMLOCK data has been initialized in file system */
#define SIMLOCK_IND_INITIALIZED 0x5AF749BD

/* Non-volatile data for SIMLOCK feature saved in file system in encrypted format */
typedef struct
{
  Boolean network_lock_ind;     /* TRUE if feature is on */
  Boolean network_subset_lock_ind;  /* TRUE if feature is on */
  Boolean service_provider_lock_ind;  /* TRUE if feature is on */
  Boolean corporate_lock_ind;     /* TRUE if feature is on */

  /* Number of times we have tried to unlock a locked feature */
  UInt8 network_unlock_attempt;   
  UInt8 network_subset_unlock_attempt;
  UInt8 provider_unlock_attempt;
  UInt8 corporate_unlock_attempt;

  /* Indicates whether the non-volatile SIMLOCK data has been initialized: 
   * If it is initialized, set to SIMLOCK_IND_INITIALIZED, otherwise not initialized.
   */
  UInt32 simlock_init_flag;

  /* 16-bit check sum to protect the all the elements above. If the check_sum verification fails, 
   * we will re-initialize the above SIMLOCK data from the Secure Data Block. This means that
   * an unlocked phone becomes locked again if the non-volatile SIMLOCK data is ever corrupted. 
   */
  UInt16  check_sum;

  /* Phone Lock: lock the use of the phone to a particular SIM */
  PH_SIM_Lock_Status_t  phone_lock_setting;
  char  phone_lock_pwd[8 + 1];  /* Add one for Null character to terminate string */
  IMSI_t  phone_lock_imsi;    /* IMSI the phone is locked to */

} SIMLOCK_NVDATA_t;


/* Two long words to identify the existance of BRCM Secure Bootloader (see secboot\src\sblhead.s) */
#define BRCM_SBL_FLAG_ADDRESS 0x34
#define BRCM_SBL_FLAG_WORD1   0x6f356b93        
#define BRCM_SBL_FLAG_WORD2   0x21a454c3
  
typedef enum
{
  SIMLOCK_NOT_INIT,   /* SIMLOCK status not initialized yet */
  SIMLOCK_OPENED,     /* SIMLOCK opened (SIMLOCK check passed) */
  SIMLOCK_CLOSED,     /* SIMLOCK closed (SIMLOCK check not passed, password needed) */
  SIMLOCK_VERIFIED    /* SIMLOCK password has been verified */
} SIMLOCK_RESULT_t;

// SIM Lock state mask
#define  SIMLOCK_NETWORK_LOCK_OPEND   (1 << SIMLOCK_NETWORK_LOCK)
#define  SIMLOCK_NET_SUBSET_LOCK_OPEND  (1 << SIMLOCK_NET_SUBSET_LOCK)
#define  SIMLOCK_PROVIDER_LOCK_OPEND  (1 << SIMLOCK_PROVIDER_LOCK)
#define  SIMLOCK_CORP_LOCK_OPEND    (1 << SIMLOCK_CORP_LOCK)
#define  SIMLOCK_PHONE_LOCK_OPEND   (1 << SIMLOCK_PHONE_LOCK)

#define SIMLOCK_NV_DATA  "/data/simlocknvdata.bin"
#define SIMLOCK_BASE    0x3f0000    //  SIMLOCK info

#define MIN(x, y) ((x) > (y) ? (y) : (x))
#define MAX(x, y) ((x) > (y) ? (x) : (y))

// The SIM lock data size is fixed to 2400 bytes
#define SIM_LOCK_DATA_SIZE   2400

//-------------------------------------------------
// Local Function Prototypes
//-------------------------------------------------
static Boolean GetSimlockNvdata(SIMLOCK_NVDATA_t *simlock_nvdata);
static Boolean WriteSimlockNvdata(const SIMLOCK_NVDATA_t *simlock_nvdata);
static SIMLock_CodeFile_t* GetSimlockData(void);
static Boolean  SIMLockCheckNetworkLock(UInt8* imsi);
static Boolean  SIMLockCheckNetSubsetLock(UInt8* imsi);
static Boolean  SIMLockCheckProviderLock(UInt8* imsi, UInt8* gid1);
static Boolean  SIMLockCheckCorpLock(UInt8* imsi, UInt8* gid1, UInt8* gid2);
static SIMLock_Status_t  SIMLockUnlockOneType(SIMLockType_t lockType, UInt8* key);
static SIMLock_Status_t  SIMLockSetLockOneType(SIMLockType_t lockType, Boolean action, UInt8* key);

static Boolean SIMLockCheckPHSIMLock(UInt8* imsi);
static SIMLock_Status_t SIMLockSetPHSIMLock(PH_SIM_Lock_Status_t lock_setting, UInt8* imsi, UInt8* key);
static Boolean  IsSIMSecureEnable(void);

static Boolean KRIL_simlock_Readfile(UInt8* pSIMLockInfo);
static void KRIL_simlock_Writefile(UInt8* pSIMLockInfo);
static void KRIL_simlock_NVdata_Init(SIMLOCK_NVDATA_t *simlock_nvdata);
void SIMLockUpdateSIMLockState(void);

//-------------------------------------------------
// Local variables
//-------------------------------------------------
static SIMLOCK_RESULT_t Network_Lock_Status = SIMLOCK_NOT_INIT;
static SIMLOCK_RESULT_t Network_Subset_Lock_Status = SIMLOCK_NOT_INIT;
static SIMLOCK_RESULT_t Service_Provider_Lock_Status = SIMLOCK_NOT_INIT;
static SIMLOCK_RESULT_t Corporate_Lock_Status = SIMLOCK_NOT_INIT;
static SIMLOCK_RESULT_t Phone_Lock_Status = SIMLOCK_NOT_INIT;

/* Number of attempts to unlock after powerup: these variables are used only when 
 * the "ResetUnlockCounter" element in "SIMLock_CodeFile_t" structure
 * is TRUE. 
 */
static UInt8 Network_Unlock_Attempt = 0;
static UInt8 Network_Subset_Unlock_Attempt = 0;
static UInt8 Service_Provider_Unlock_Attempt = 0;
static UInt8 Corporate_Unlock_Attempt = 0;

static const UInt16 crc_table[] =
{
  0x0000, 0xc0c1, 0xc181, 0x0140, 0xc301, 0x03c0, 0x0280, 0xc241,
  0xc601, 0x06c0, 0x0780, 0xc741, 0x0500, 0xc5c1, 0xc481, 0x0440,
  0xcc01, 0x0cc0, 0x0d80, 0xcd41, 0x0f00, 0xcfc1, 0xce81, 0x0e40,
  0x0a00, 0xcac1, 0xcb81, 0x0b40, 0xc901, 0x09c0, 0x0880, 0xc841,
  0xd801, 0x18c0, 0x1980, 0xd941, 0x1b00, 0xdbc1, 0xda81, 0x1a40,
  0x1e00, 0xdec1, 0xdf81, 0x1f40, 0xdd01, 0x1dc0, 0x1c80, 0xdc41,
  0x1400, 0xd4c1, 0xd581, 0x1540, 0xd701, 0x17c0, 0x1680, 0xd641,
  0xd201, 0x12c0, 0x1380, 0xd341, 0x1100, 0xd1c1, 0xd081, 0x1040,
  0xf001, 0x30c0, 0x3180, 0xf141, 0x3300, 0xf3c1, 0xf281, 0x3240,
  0x3600, 0xf6c1, 0xf781, 0x3740, 0xf501, 0x35c0, 0x3480, 0xf441,
  0x3c00, 0xfcc1, 0xfd81, 0x3d40, 0xff01, 0x3fc0, 0x3e80, 0xfe41,
  0xfa01, 0x3ac0, 0x3b80, 0xfb41, 0x3900, 0xf9c1, 0xf881, 0x3840,
  0x2800, 0xe8c1, 0xe981, 0x2940, 0xeb01, 0x2bc0, 0x2a80, 0xea41,
  0xee01, 0x2ec0, 0x2f80, 0xef41, 0x2d00, 0xedc1, 0xec81, 0x2c40,
  0xe401, 0x24c0, 0x2580, 0xe541, 0x2700, 0xe7c1, 0xe681, 0x2640,
  0x2200, 0xe2c1, 0xe381, 0x2340, 0xe101, 0x21c0, 0x2080, 0xe041,
  0xa001, 0x60c0, 0x6180, 0xa141, 0x6300, 0xa3c1, 0xa281, 0x6240,
  0x6600, 0xa6c1, 0xa781, 0x6740, 0xa501, 0x65c0, 0x6480, 0xa441,
  0x6c00, 0xacc1, 0xad81, 0x6d40, 0xaf01, 0x6fc0, 0x6e80, 0xae41,
  0xaa01, 0x6ac0, 0x6b80, 0xab41, 0x6900, 0xa9c1, 0xa881, 0x6840,
  0x7800, 0xb8c1, 0xb981, 0x7940, 0xbb01, 0x7bc0, 0x7a80, 0xba41,
  0xbe01, 0x7ec0, 0x7f80, 0xbf41, 0x7d00, 0xbdc1, 0xbc81, 0x7c40,
  0xb401, 0x74c0, 0x7580, 0xb541, 0x7700, 0xb7c1, 0xb681, 0x7640,
  0x7200, 0xb2c1, 0xb381, 0x7340, 0xb101, 0x71c0, 0x7080, 0xb041,
  0x5000, 0x90c1, 0x9181, 0x5140, 0x9301, 0x53c0, 0x5280, 0x9241,
  0x9601, 0x56c0, 0x5780, 0x9741, 0x5500, 0x95c1, 0x9481, 0x5440,
  0x9c01, 0x5cc0, 0x5d80, 0x9d41, 0x5f00, 0x9fc1, 0x9e81, 0x5e40,
  0x5a00, 0x9ac1, 0x9b81, 0x5b40, 0x9901, 0x59c0, 0x5880, 0x9841,
  0x8801, 0x48c0, 0x4980, 0x8941, 0x4b00, 0x8bc1, 0x8a81, 0x4a40,
  0x4e00, 0x8ec1, 0x8f81, 0x4f40, 0x8d01, 0x4dc0, 0x4c80, 0x8c41,
  0x4400, 0x84c1, 0x8581, 0x4540, 0x8701, 0x47c0, 0x4680, 0x8641,
  0x8201, 0x42c0, 0x4380, 0x8341, 0x4100, 0x81c1, 0x8081, 0x4040
};


//******************************************************************************
//
// Function Name: Calculate16BitCrc
//
// Description: Calculate the 16 bit crc for the passed data block.
//
//******************************************************************************
UInt16 Calculate16BitCrc(UInt16 crc, UInt8 *data, UInt32 length)
{
  while (length-- > 0)
  {
    crc = (crc >> 8) ^ crc_table[(crc ^ data[0]) & 0xFF];
    data++;
  }

  return crc;
}


//******************************************************************************
//
// Function Name: SIM_IsTestSIM
//
// Description: This function returns TRUE if the SIM card, if present in the mobile, is a test SIM.
//
// Return: TRUE if the SIM card is present and is a test SIM. FALSE otherwise.
//
//******************************************************************************
Boolean SIM_IsTestSIM(void)
{
		SIMLOCK_SIM_DATA_t *sim_data;
    
		// Check if SIM data is available
    sim_data = GetSIMData();
    if (!sim_data)
    {
        return FALSE;
    }
        
	  if (memcmp((UInt8*) sim_data->imsi_string, "00101", 5) == 0)
	  {
	  	  return TRUE;
	  }
	  else
	  {
        return FALSE;
	  }
}


//******************************************************************************
//
// Function Name: SIM_NOT_PRESENT
//
// Description: This function returns TRUE if the SIM card is not present
//
// Return: TRUE if the SIM card is not present. FALSE otherwise.
//
//******************************************************************************
Boolean SIM_NOT_PRESENT(void)
{
    return FALSE;
}

//******************************************************************************
// Function Name: MS_GetSimlockBase
//
// Description: This function returns the base address of SIMLOCK data block in flash. 
//        It is N/A if MS_IsPlatformSimlockSupported() returns FALSE. 
//
// Notes:
//******************************************************************************
UInt32 MS_GetSimlockBase(void)
{
    return SIMLOCK_BASE ;
}

//******************************************************************************
//
// Function Name: KRIL_simlock_Readfile
//
// Description: Read SIM lock data from file system.
//
//******************************************************************************
static Boolean KRIL_simlock_Readfile(UInt8* pSIMLockInfo)
{
    Boolean result = FALSE;
    struct file *hFileTmp = NULL;
    struct inode *inode   = NULL;
    UInt32 fsize;
    int retval;
    mm_segment_t orgfs = get_fs();
    set_fs(KERNEL_DS);
    
    hFileTmp = filp_open(SIMLOCK_NV_DATA, O_RDONLY, 0);
    
    if (IS_ERR(hFileTmp))
    {
        KRIL_DEBUG(DBG_INFO,"Open simlocknvdata.bin Failed!!! IS_ERR(hFileTmp):%ld\n",IS_ERR(hFileTmp));
        set_fs(orgfs);
        return FALSE;
    }

    if (hFileTmp->f_path.dentry)
    {
        inode = hFileTmp->f_path.dentry->d_inode;
        fsize = (UInt32)inode->i_size;
        
        if (fsize != sizeof(SIMLOCK_NVDATA_t))
        {
            KRIL_DEBUG(DBG_ERROR,"fsize:%lu does not match SIMLOCK_NVDATA_t size:%d Error!!!\n",
                fsize,sizeof(SIMLOCK_NVDATA_t));
        }
        else
        {
            if (hFileTmp->f_op && hFileTmp->f_op->read)
            {
                retval = hFileTmp->f_op->read(hFileTmp, pSIMLockInfo, sizeof(SIMLOCK_NVDATA_t), &hFileTmp->f_pos);
                
                if (retval > 0)
                {
                    result = TRUE;
                }
                else
                {
                    KRIL_DEBUG(DBG_ERROR,"Read SIM Lock NV data Failed!!! retval:%d sizeof(SIMLOCK_NVDATA_t):%d\n",
                        retval, sizeof(SIMLOCK_NVDATA_t));
                }
            }
            else
            {
                KRIL_DEBUG(DBG_ERROR,"hFileTmp does not have a read method Error!!!\n");
            }                
        }
    }
    else
    {
        KRIL_DEBUG(DBG_ERROR,"hFileTmp->f_path.dentry is NULL Error!!!\n");
    }
    
    filp_close(hFileTmp, NULL);
    set_fs(orgfs);
    return result;
}


//******************************************************************************
//
// Function Name: KRIL_simlock_Writefile
//
// Description: Write SIM lock data to file system.
//
//******************************************************************************
static void KRIL_simlock_Writefile(UInt8* pSIMLockInfo)
{
    struct file *hFileTmp = NULL;
    int retval;
    mm_segment_t orgfs = get_fs();
    set_fs(KERNEL_DS);   
    
    hFileTmp = filp_open(SIMLOCK_NV_DATA, O_CREAT|O_TRUNC|O_WRONLY, 0);
    if (IS_ERR(hFileTmp))
    {
        KRIL_DEBUG(DBG_INFO,"Create simlocknvdata.bin Failed!!! IS_ERR(hFileTmp):%ld\n",IS_ERR(hFileTmp));
        set_fs(orgfs);
        return;
    }
    
    if (hFileTmp->f_op && hFileTmp->f_op->write)
    {
        retval = hFileTmp->f_op->write(hFileTmp, pSIMLockInfo, sizeof(SIMLOCK_NVDATA_t), &hFileTmp->f_pos);
    
        if (retval != sizeof(SIMLOCK_NVDATA_t) || retval <= 0)
        {
            KRIL_DEBUG(DBG_ERROR,"Write SIM Lock NV data Failed!!! retval:%d sizeof(SIMLOCK_NVDATA_t):%d\n",
                retval, sizeof(SIMLOCK_NVDATA_t));
        }
    }
    else
    {
        KRIL_DEBUG(DBG_ERROR,"hFileTmp does not have a write method Error!!!\n");
    }
    
    filp_close(hFileTmp, NULL);
    set_fs(orgfs);
}


//******************************************************************************
//
// Function Name: freeSimLockdata
//
// Description: Free the memory allocated for SIM Lock data.
//
//******************************************************************************
static void freeSimLockdata(SIMLock_CodeFile_t* simlock_data)
{
    UInt8* psimlock_data = (UInt8*)simlock_data;
    
    // The original 4 head bytes of SIM lock data image is the size value.
    // In order to free the allocated memory for SIM lock data, shift SIM 
    // lock data point address 4 bytes forward.
    psimlock_data -= 4;
    kfree(psimlock_data);
}


//******************************************************************************
//
// Function Name: KRIL_simlock_NVdata_initial
//
// Description: Initialise the SIM lock NV data.
//
//******************************************************************************
static void KRIL_simlock_NVdata_Init(SIMLOCK_NVDATA_t *simlock_nvdata)
{
    SIMLock_CodeFile_t *simlock_data = GetSimlockData();    
    
    KRIL_DEBUG(DBG_INFO,"SIM lock NV data initialize\n");
    
    if (simlock_data != NULL)
    {
        memset(simlock_nvdata, 0, sizeof(SIMLOCK_NVDATA_t));
        
        simlock_nvdata->network_lock_ind = (simlock_data->indicator.networkLockIndicator == 1);
        simlock_nvdata->network_subset_lock_ind = (simlock_data->indicator.netSubsetLockIndicator == 1);
        simlock_nvdata->service_provider_lock_ind = (simlock_data->indicator.providerLockIndicator == 1);
        simlock_nvdata->corporate_lock_ind = (simlock_data->indicator.corpLockIndicator == 1);

        /* Memory of simlock_data is dynamically allocated in GetSimlockData(), must
         * deallocate the memory after we finish using it. 
         */
        freeSimLockdata(simlock_data);
    }
    else
    {
        simlock_nvdata->network_lock_ind = 0;
        simlock_nvdata->network_subset_lock_ind = 0;
        simlock_nvdata->service_provider_lock_ind = 0;
        simlock_nvdata->corporate_lock_ind = 0;        
    }  

    simlock_nvdata->network_unlock_attempt = 0;
    simlock_nvdata->network_subset_unlock_attempt = 0;
    simlock_nvdata->provider_unlock_attempt = 0;
    simlock_nvdata->corporate_unlock_attempt = 0;

    simlock_nvdata->simlock_init_flag = SIMLOCK_IND_INITIALIZED;

    //simlock_nvdata->check_sum = 
    //  Calculate16BitCrc(0, (UInt8 *) simlock_nvdata, (UInt32) &simlock_nvdata->check_sum - (UInt32) simlock_nvdata);

    //simlock_nvdata->check_sum = Calculate16BitCrc(simlock_nvdata->check_sum, GET_IMEI_ADDR(), BCD_IMEI_LEN);

    simlock_nvdata->phone_lock_setting = PH_SIM_LOCK_OFF;
    strcpy(simlock_nvdata->phone_lock_pwd, DEFAULT_PHSIM_LOCK_PWD);        
}


//--------------------------------------------------------------------------
//
// Function Name: GetSimlockNvdata()
//
// Description:   This function gets the non-volatile SIMLOCK data saved in 
//          file system. If SIMLOCK indicator information has not been
//          initialized, this function will do the initialization.  
//
// Return:      TRUE if SIMLOCK data is correctly obtained; FALSE otherwise.
//
//----------------------------------------------------------------------------
static Boolean GetSimlockNvdata(SIMLOCK_NVDATA_t *simlock_nvdata)
{
    SIMLOCK_NVDATA_t temp_nvdata;
    //UInt16  check_sum;

    if (!IsSIMSecureEnable())
    {
        return FALSE;
    }

    if (FALSE == KRIL_simlock_Readfile((UInt8*)&temp_nvdata))
    {
        KRIL_simlock_NVdata_Init(simlock_nvdata);

        /* Encrypt the SIMLOCK data */
        if (!EncDec((UInt8*)&temp_nvdata, (UInt8*)simlock_nvdata, sizeof(SIMLOCK_NVDATA_t), AES_OPERATION_ENCRYPT))
        {
            KRIL_DEBUG(DBG_ERROR,"simlock_nvdata Encrypt Failed!!!\n");
            return FALSE;
        }
        
        KRIL_simlock_Writefile((UInt8*)&temp_nvdata);
    }
    else
    {
        /* Decrypt the SIMLOCK data */
        if (!EncDec((UInt8*)simlock_nvdata, (UInt8*)&temp_nvdata, sizeof(SIMLOCK_NVDATA_t), AES_OPERATION_DECRYPT))
        {
            KRIL_DEBUG(DBG_ERROR,"temp_nvdata Decrypt Failed!!!\n");
            return FALSE;
        }
        
        //check_sum = Calculate16BitCrc(0, (UInt8 *) simlock_nvdata, (UInt32) &simlock_nvdata->check_sum - (UInt32) simlock_nvdata);
        //check_sum = Calculate16BitCrc(check_sum, GET_IMEI_ADDR(), BCD_IMEI_LEN);      
        
        if((simlock_nvdata->simlock_init_flag != SIMLOCK_IND_INITIALIZED) /*||
          (simlock_nvdata->check_sum != check_sum)*/)
        {
            /* Either data has not been initialized or data is corrupted, re-initialize the non-volatile 
             * SIMLOCK data in file system. 
             */
            KRIL_simlock_NVdata_Init(simlock_nvdata);
            
            memset((UInt8*)&temp_nvdata, 0x00, sizeof(SIMLOCK_NVDATA_t));

            /* Encrypt the SIMLOCK data */
            if (!EncDec((UInt8*)&temp_nvdata, (UInt8*)simlock_nvdata, sizeof(SIMLOCK_NVDATA_t), AES_OPERATION_ENCRYPT))
            {
                KRIL_DEBUG(DBG_ERROR,"simlock_nvdata Encrypt Failed!!!\n");
                return FALSE;
            }
            
            KRIL_simlock_Writefile((UInt8*)&temp_nvdata);
        }
    }

  return TRUE;
}


//--------------------------------------------------------------------------
//
// Function Name: WriteSimlockNvdata()
//
// Description:   This function re-calculates the check sum in the passed
//          SIMLOCK data, encrypts the data and then saves the data 
//          in file system.    
//
// Return:      TRUE if SIMLOCK data is correctly written to file
//          system; FALSE otherwise.
//
//----------------------------------------------------------------------------
static Boolean WriteSimlockNvdata(const SIMLOCK_NVDATA_t *simlock_nvdata)
{
    SIMLOCK_NVDATA_t temp_nvdata;
    
    if (!IsSIMSecureEnable())
    {
        return FALSE;
    }

    //simlock_nvdata->simlock_init_flag = SIMLOCK_IND_INITIALIZED;
    
    //temp_nvdata->check_sum = 
    //  Calculate16BitCrc(0, (UInt8 *) simlock_nvdata, (UInt32) simlock_nvdata->check_sum - (UInt32) simlock_nvdata);
    
    //temp_nvdata->check_sum = Calculate16BitCrc(simlock_nvdata->check_sum, GET_IMEI_ADDR(), BCD_IMEI_LEN);
    
    /* Encrypt the SIMLOCK data */
    if (!EncDec((UInt8*)&temp_nvdata, (UInt8*)simlock_nvdata, sizeof(SIMLOCK_NVDATA_t), AES_OPERATION_ENCRYPT))
    {
        KRIL_DEBUG(DBG_ERROR,"simlock_nvdata Encrypt Failed!!!\n");
        return FALSE;
    }
    
    KRIL_simlock_Writefile((UInt8*)&temp_nvdata);
    
    return TRUE;
}


//--------------------------------------------------------------------------
//
// Function Name: GetSimlockData()
//
// Description:   Get Simlock Data Block pointer.  
//
// Return:      a dynamically allocated buffer that stores the Simlock
//          data which is already decrypted. The caller must 
//          de-allocate the memory returned in this function! 
//
//----------------------------------------------------------------------------
static SIMLock_CodeFile_t* GetSimlockData(void)
{
    if (IsSIMSecureEnable())
    {
#ifdef CONFIG_BRCM_CKBLOCK_READER
        UInt8 *simlock_temp_ptr = NULL;
        UInt8 *simlock_data_ptr = NULL;
        UInt16 ckdatasize;
        
        simlock_temp_ptr = kmalloc(SIM_LOCK_DATA_SIZE, GFP_KERNEL);
        if (NULL == simlock_temp_ptr)
        {
            KRIL_DEBUG(DBG_ERROR,"Allocate memory for simlock_temp_ptr Failed!!!\n");
            return NULL;
        }

        simlock_data_ptr = kmalloc(SIM_LOCK_DATA_SIZE, GFP_KERNEL);
        if (NULL == simlock_data_ptr)
        {
            KRIL_DEBUG(DBG_ERROR,"Allocate memory for simlock_data_ptr Failed!!!\n");
            return NULL;
        }
                
        // Get Simlock Data Block data
        if ((ckdatasize = readCKDataBlock((char*)simlock_temp_ptr, SIM_LOCK_DATA_SIZE)) == -1)
        {
            KRIL_DEBUG(DBG_ERROR,"readCKDataBlock Failed!!!\n");
            return NULL;
        }
        
        if (SIM_LOCK_DATA_SIZE != ckdatasize)
        {
            KRIL_DEBUG(DBG_ERROR,"ckdatasize:%d != SIM_LOCK_DATA_SIZE. Error!!!\n",ckdatasize);
            return NULL;
        }
        
        // Decrypt SIM lock data
        if (!EncDec(simlock_data_ptr, simlock_temp_ptr, SIM_LOCK_DATA_SIZE, AES_OPERATION_DECRYPT))
        {
            KRIL_DEBUG(DBG_ERROR,"simlock_temp_ptr Decrypt Failed!!!\n");
            return FALSE;
        }
        
        kfree(simlock_temp_ptr);
        
        // The ahead 4 bytes of SIM lock data image is size value. 
        simlock_data_ptr += 4;
        return ((SIMLock_CodeFile_t *) simlock_data_ptr);
        
#else  //CONFIG_BRCM_CKBLOCK_READER
        
        // The readCKDataBlock function is disable, just return NULL.
        KRIL_DEBUG(DBG_ERROR,"readCKDataBlock() not support . Error!!!\n");
        return NULL;
#endif //CONFIG_BRCM_CKBLOCK_READER
    }
    else
    {
        return NULL;
    }
}


//--------------------------------------------------------------------------
//
// Function Name: SIMLockCheckNetworkLock()
// Input:       imsi - IMSI (MCC and MNC)
// Output:        none
// Return:        TRUE - SIM valid for the network; FALSE - invalid SIM.
//  
// Description:   Check if SIM is valid for the network lock.
//
// Note:            The passed IMSI data must have the following format:
//                  MCC (3 Ascii digits) + MNC (2 or 3 Ascii digits). 
//
//----------------------------------------------------------------------------
static Boolean  SIMLockCheckNetworkLock(UInt8* imsi)
{
    Boolean result = FALSE;
    
    if (SIM_IsTestSIM())
    {
        result = TRUE;
    }
    else
    {
        UInt8 i;
        SIMLock_CodeFile_t* simlock_data = GetSimlockData();
        
        if (simlock_data != NULL)
        {
            SIMLock_NetworkCode_t *network_code = 
                (SIMLock_NetworkCode_t *) ( ((UInt8 *) simlock_data) + sizeof(SIMLock_CodeFile_t) );
            
            for (i = 0; i < simlock_data->numNetworkLocks; i++, network_code++)
            {
                // Compare MCC and MNC. 
                // According to the Annex A of GSM 03.22, MNC contains 2 or 3
                // digits depending on the PLMN implementation (For PCS1900, 
                // MNC contains three digits). 
                KRIL_DEBUG(DBG_INFO,"imsi:%s network_code->mcc:%s mnc:%s\n", imsi, network_code->mcc, network_code->mnc);
                if ( (network_code->mcc[0] == imsi[0]) && (network_code->mcc[1] == imsi[1]) && 
                   (network_code->mcc[2] == imsi[2]) && (network_code->mnc[0] == imsi[3]) &&
                   (network_code->mnc[1] == imsi[4]) && 
                   (network_code->mnc[2] == '\0' ? TRUE : network_code->mnc[2] == imsi[5]) )
                {
                    result = TRUE;
                    break;
                }
            }
            
            /* Memory of simlock_data is dynamically allocated in GetSimlockData(), must
             * deallocate the memory after we finish using it. 
             */
            freeSimLockdata(simlock_data);
        }
        else
        {
            result = TRUE;
        }
    }
    
    return result;
}


//--------------------------------------------------------------------------
//
// Function Name: SIMLockCheckNetSubsetLock()
// Input:       imsi - IMSI (MCC, MNC and digits 6 & 7)
// Output:        none
// Return:        TRUE - SIM valid for network subset lock; 
//          FALSE - invalid SIM.
//  
// Description:   Check if SIM is valid for the network subset lock.
//
// Note:            The IMSI data must have the following format:
//                  MCC (3 Ascii digits) + MNC (2 or 3 Ascii digits).
// 
//----------------------------------------------------------------------------
static Boolean SIMLockCheckNetSubsetLock(UInt8* imsi)
{
    Boolean result = FALSE;
    
    if (SIM_IsTestSIM())
    {
        result = TRUE;
    }
    else
    {
        UInt8 i;
        SIMLock_CodeFile_t* simlock_data = GetSimlockData();
        
        if (simlock_data != NULL)
        {
            SIMLock_NetSubSetCode_t *network_subset_code = (SIMLock_NetSubSetCode_t *) 
              ( ((UInt8 *) simlock_data) + sizeof(SIMLock_CodeFile_t) + 
                (sizeof(SIMLock_NetworkCode_t) * simlock_data->numNetworkLocks) + 
                (sizeof(SIMLock_ProviderCode_t) * simlock_data->numProviderLocks) );
            
            for (i = 0; i < simlock_data->numNetSubsetLocks; i++, network_subset_code++)
            {
              if ( (network_subset_code->mcc[0] == imsi[0]) && (network_subset_code->mcc[1] == imsi[1]) &&
                 (network_subset_code->mcc[2] == imsi[2]) && (network_subset_code->mnc[0] == imsi[3]) &&
                 (network_subset_code->mnc[1] == imsi[4]) && 
                 (network_subset_code->mnc[2] == '\0' ? TRUE : network_subset_code->mnc[2] == imsi[5]) &&
                 ((network_subset_code->mnc[2] == '\0' ? imsi[5] : imsi[6]) == network_subset_code->imsi_6_and_7[0]) &&
                 ((network_subset_code->mnc[2] == '\0' ? imsi[6] : imsi[7]) == network_subset_code->imsi_6_and_7[1]) )
              {
                  result = TRUE;
                  break;
              }
            }
            
            /* Memory of simlock_data is dynamically allocated in GetSimlockData(), must
             * deallocate the memory after we finish using it. 
             */
            freeSimLockdata(simlock_data);
        }
        else
        {
            result = TRUE;
        }
    }
    
    return result;
}


//--------------------------------------------------------------------------
// Function Name: SIMLockCheckProviderLock()
// Input:     imsi - IMSI (MCC and MNC)
//          gid1 - GID1 file.
// Output:      none
// Return:      TRUE - SIM valid for provider lock; FALSE - invalid SIM.
//  
// Description:   Check if SIM is valid for the provider lock.
//
// Note:            The passed IMSI data must have the following format:
//                  MCC (3 Ascii digits) + MNC (2 or 3 Ascii digits). 
//
//----------------------------------------------------------------------------
static Boolean SIMLockCheckProviderLock(UInt8* imsi, UInt8* gid1)
{
    Boolean result = FALSE;
    
    if (SIM_IsTestSIM())
    {
        result = TRUE;
    }
    else
    {
        UInt8 i;
        SIMLock_CodeFile_t* simlock_data = GetSimlockData();
        
        if (simlock_data != NULL)
        {
            SIMLock_ProviderCode_t *provider_code = (SIMLock_ProviderCode_t *) 
              ( ((UInt8 *) simlock_data) + sizeof(SIMLock_CodeFile_t) + 
                (sizeof(SIMLock_NetworkCode_t) * simlock_data->numNetworkLocks) );
            
            for (i = 0; i < simlock_data->numProviderLocks; i++, provider_code++)
            {
                // Compare MCC, MNC and first byte of GID1. 
                // According to the Annex A of GSM 03.22, MNC contains 2 or 3
                // digits depending on the PLMN implementation (For PCS1900, 
                // MNC contains three digits).
                if ( (provider_code->mcc[0] == imsi[0]) && (provider_code->mcc[1] == imsi[1]) &&
                   (provider_code->mcc[2] == imsi[2]) && (provider_code->mnc[0] == imsi[3]) &&
                   (provider_code->mnc[1] == imsi[4]) && 
                   (provider_code->mnc[2] == '\0' ? TRUE : provider_code->mnc[2] == imsi[5]) &&
                   (gid1[0] == '\0' ? FALSE : gid1[0] == provider_code->gid1[0]) )
                {
                  result = TRUE;
                  break;
                }
            }
            
            /* Memory of simlock_data is dynamically allocated in GetSimlockData(), must
             * deallocate the memory after we finish using it. 
             */
            freeSimLockdata(simlock_data);
        }
        else
        {
            result = TRUE;
        }
    }
    
    return result;
}


//--------------------------------------------------------------------------
// Function Name: SIMLockCheckCorpLock()
// Input:     imsi - IMSI (MCC and MNC)
//          gid1 - GID1 file.
//          gid2 - GID2 file.
//
// Output:      none
//
// Return:      TRUE - SIM valid for the corp lock; 
//          FALSE - invalid SIM.
//  
// Description:   Check if SIM is valid for the network lock.
//
// Note:            The passed IMSI data must have the following format:
//                  MCC (3 Ascii digits) + MNC (2 or 3 Ascii digits). 
//
//----------------------------------------------------------------------------
static Boolean  SIMLockCheckCorpLock(UInt8* imsi, UInt8* gid1, UInt8* gid2)
{
    Boolean result = FALSE;
    
    if (SIM_IsTestSIM())
    {
        result = TRUE;
    }
    else
    {
        UInt8 i;
        SIMLock_CodeFile_t* simlock_data = GetSimlockData();
        
        if (simlock_data != NULL)
        {
            SIMLock_CorporateCode_t *corp_code = (SIMLock_CorporateCode_t *) 
              ( ((UInt8 *) simlock_data) + sizeof(SIMLock_CodeFile_t) + 
                (sizeof(SIMLock_NetworkCode_t) * simlock_data->numNetworkLocks) + 
                (sizeof(SIMLock_ProviderCode_t) * simlock_data->numProviderLocks) + 
                (sizeof(SIMLock_NetSubSetCode_t) * simlock_data->numNetSubsetLocks) );
            
            for (i = 0; i < simlock_data->numCorpLocks; i++, corp_code++)
            {
                if ( (corp_code->mcc[0] == imsi[0]) && (corp_code->mcc[1] == imsi[1]) &&
                   (corp_code->mcc[2] == imsi[2]) && (corp_code->mnc[0] == imsi[3]) &&
                   (corp_code->mnc[1] == imsi[4]) && 
                   (corp_code->mnc[2] == '\0' ? TRUE : corp_code->mnc[2] == imsi[5]) &&
                   (gid1[0] == '\0' ? FALSE : gid1[0] == corp_code->gid1[0]) &&
                   (gid2[0] == '\0' ? FALSE : gid2[0] == corp_code->gid2[0]) )
                {
                    result = TRUE;
                    break;
                }
            }
            
            /* Memory of simlock_data is dynamically allocated in GetSimlockData(), must
             * deallocate the memory after we finish using it. 
             */
            freeSimLockdata(simlock_data);
        }
        else
        {
            result = TRUE;
        }
    }
    
    return result;
}


//--------------------------------------------------------------------------
// Function Name: SIMLockUnlockOneType()
//
// Input:     lockType - Simlock type except SIMLOCK_PHONE_LOCK. 
//          key - Control key of the simlock type.
//
// Return:      SIMLOCK_SUCCESS - if the simlock type is unlocked.
//          SIMLOCK_FAILURE - if the simlock type is not unlocked, 
//                    but user can try to unlock again.
//          SIMLOCK_PERMANENTLY_LOCKED - if the simlock type is not unlocked,
//            and the simlock type can not be unlocked again because
//            the user has tried the maxinum times.  
//  
// Description:   Unlock a simlock type.
//
//----------------------------------------------------------------------------
static SIMLock_Status_t  SIMLockUnlockOneType(SIMLockType_t lockType, UInt8* key)
{
    SIMLock_Status_t result = SIMLOCK_SUCCESS;
    UInt8 control_key[MAX_CONTROL_KEY_LENGTH + 1];
    UInt8 *unlock_attempt;
    Boolean *lock_ind;
    SIMLock_CodeFile_t *simlockFile = GetSimlockData(); 
    SIMLOCK_NVDATA_t simlock_nvdata;
    
    if (GetSimlockNvdata(&simlock_nvdata) && (simlockFile != NULL))
    {
        switch (lockType)
        {
            case SIMLOCK_NETWORK_LOCK:
                memcpy(control_key, simlockFile->nck, sizeof(control_key));
                unlock_attempt = simlockFile->resetUnlockCounter ? &Network_Unlock_Attempt : &simlock_nvdata.network_unlock_attempt;
                lock_ind = &simlock_nvdata.network_lock_ind;
                break;
            
            case SIMLOCK_NET_SUBSET_LOCK:
                memcpy(control_key, simlockFile->nsck, sizeof(control_key));
                unlock_attempt = simlockFile->resetUnlockCounter ? &Network_Subset_Unlock_Attempt : &simlock_nvdata.network_subset_unlock_attempt;
                lock_ind = &simlock_nvdata.network_subset_lock_ind;
                break;
              
            case SIMLOCK_PROVIDER_LOCK:
                memcpy(control_key, simlockFile->spck, sizeof(control_key));
                unlock_attempt = simlockFile->resetUnlockCounter ? &Service_Provider_Unlock_Attempt : &simlock_nvdata.provider_unlock_attempt;
                lock_ind = &simlock_nvdata.service_provider_lock_ind;
                break;
            
            case SIMLOCK_CORP_LOCK:
                memcpy(control_key, simlockFile->cck, sizeof(control_key));
                unlock_attempt = simlockFile->resetUnlockCounter ? &Corporate_Unlock_Attempt : &simlock_nvdata.corporate_unlock_attempt;
                lock_ind = &simlock_nvdata.corporate_lock_ind;
                break;
            
            case SIMLOCK_PHONE_LOCK: /* To unlock Phone Lock, another function is used */
            default:
                /* Should not be here: return FAILURE */
                KRIL_DEBUG(DBG_ERROR,"Error lockType:%d Failed!!!\n",lockType);
                result = SIMLOCK_FAILURE;
                break;
        }
    }
    else
    {
        result = SIMLOCK_FAILURE;
    }
    
    if ((result == SIMLOCK_SUCCESS) && *lock_ind)
    {
        /* If we have attempted maximum times, we can not unlock again */
        if (*unlock_attempt >= simlockFile->maxUnlockAttempt)
        {
            result = SIMLOCK_PERMANENTLY_LOCKED;
        }
        else
        {
            KRIL_DEBUG(DBG_INFO,"unlock_attempt:%d control_key:%s key:%s\n",*unlock_attempt, (char*)control_key, (char*)key);
            
            if (strcmp((char *) control_key, (char *) key) == 0 )
            {
                *unlock_attempt = 0;
                *lock_ind = FALSE;
            }
            else
            {
                (*unlock_attempt)++;
                result = (*unlock_attempt >= simlockFile->maxUnlockAttempt ? SIMLOCK_PERMANENTLY_LOCKED : SIMLOCK_WRONG_KEY);
            }
            
            /* Update non-volatile SIMLOCK data */
            WriteSimlockNvdata(&simlock_nvdata);
        }
    }
    
    /* Memory of simlockFile is dynamically allocated in GetSimlockData(), must
     * deallocate the memory after we finish using it. 
     */
    if (simlockFile != NULL)
    {
        kfree(simlockFile);
    }
    
    return result;
}


//--------------------------------------------------------------------------
// Function Name: SIMLockSetLockOneType()
//
// Input:     lockType - Simlock type except SIMLOCK_PHONE_LOCK
//          action - TRUE to set lock on, FALSE to set lock off.
//          key - Control key of the simlock type.
//
// Return:      
//          SIMLOCK_FAILURE - if the simlock type is invalid
//          SIMLOCK_WRONG_KEY - if the simlock control key is invalid
//  
// Description:   To set a SIMLOCK type on/off. 
//
// Note:    
//
//----------------------------------------------------------------------------
static SIMLock_Status_t  SIMLockSetLockOneType(SIMLockType_t lockType, Boolean action, UInt8* key)
{
    SIMLock_Status_t result = SIMLOCK_SUCCESS;
    UInt8 control_key[MAX_CONTROL_KEY_LENGTH + 1];
    SIMLock_CodeFile_t *simlockFile = GetSimlockData(); 
    UInt8 *unlock_attempt;
    Boolean *lock_ind;
    SIMLOCK_NVDATA_t simlock_nvdata;
    
    if (GetSimlockNvdata(&simlock_nvdata) && (simlockFile != NULL))
    { 
        switch (lockType)
        {
            case SIMLOCK_NETWORK_LOCK:
                memcpy(control_key, simlockFile->nck, sizeof(control_key));
                unlock_attempt = simlockFile->resetUnlockCounter ? &Network_Unlock_Attempt : &simlock_nvdata.network_unlock_attempt;
                lock_ind = &simlock_nvdata.network_lock_ind;
                break;
            
            case SIMLOCK_NET_SUBSET_LOCK:
                memcpy(control_key, simlockFile->nsck, sizeof(control_key));
                unlock_attempt = simlockFile->resetUnlockCounter ? &Network_Subset_Unlock_Attempt : &simlock_nvdata.network_subset_unlock_attempt;
                lock_ind = &simlock_nvdata.network_subset_lock_ind;
                break;
              
            case SIMLOCK_PROVIDER_LOCK:
                memcpy(control_key, simlockFile->spck, sizeof(control_key));
                unlock_attempt = simlockFile->resetUnlockCounter ? &Service_Provider_Unlock_Attempt : &simlock_nvdata.provider_unlock_attempt;
                lock_ind = &simlock_nvdata.service_provider_lock_ind;
                break;
            
            case SIMLOCK_CORP_LOCK:
                memcpy(control_key, simlockFile->cck, sizeof(control_key));
                unlock_attempt = simlockFile->resetUnlockCounter ? &Corporate_Unlock_Attempt : &simlock_nvdata.corporate_unlock_attempt;
                lock_ind = &simlock_nvdata.corporate_lock_ind;
                break;
            
            case SIMLOCK_PHONE_LOCK: /* To unlock Phone Lock, another function is used */
            default:
                /* Should not be here: return FAILURE */
                result = SIMLOCK_FAILURE;
                break;
        }
    }
    else
    {
        result = SIMLOCK_FAILURE;
    }
    
    KRIL_DEBUG(DBG_INFO,"lockType:%d action:%d lock_ind:%d unlock_attempt:%d result:%d\n", 
        lockType, action, *lock_ind, *unlock_attempt, result);
    KRIL_DEBUG(DBG_INFO,"control_key:%s key:%s\n", control_key, key);
    
    if ((result == SIMLOCK_SUCCESS) && (*lock_ind != action))
    {
        Boolean set_lock_ok = FALSE;
        
        if (*unlock_attempt < simlockFile->maxUnlockAttempt)
        {
            if (strcmp((char *) control_key, (char *) key) == 0)
            {
                set_lock_ok = TRUE;
                
                *lock_ind = action;
                *unlock_attempt = 0;
                
                /* Update non-volatile SIMLOCK data */
                WriteSimlockNvdata(&simlock_nvdata);
            }
            else
            {
                (*unlock_attempt)++;
                
                /* If we try to set SIMLOCK off with wrong password, 
                 * we need to increment the attempt counter in non-volatile SIMLOCK data. 
                 */
                if( (!simlockFile->resetUnlockCounter) && (!action) )
                {
                    WriteSimlockNvdata(&simlock_nvdata);
                }
            }
        }
        
        if (!set_lock_ok)
        {
            if (*unlock_attempt >= simlockFile->maxUnlockAttempt)
            {
                switch (lockType)
                {
                    case SIMLOCK_NETWORK_LOCK:
                        Network_Lock_Status = SIMLOCK_CLOSED;
                        break;
                    
                    case SIMLOCK_NET_SUBSET_LOCK:
                        Network_Subset_Lock_Status = SIMLOCK_CLOSED;
                        break;
                    
                    case SIMLOCK_PROVIDER_LOCK:
                        Service_Provider_Lock_Status = SIMLOCK_CLOSED;
                        break;
                    
                    case SIMLOCK_CORP_LOCK:
                        Corporate_Lock_Status = SIMLOCK_CLOSED;
                        break;
                    
                    // These two case shouldn't happen, add for fixing compile warning.
                    case SIMLOCK_PHONE_LOCK:
                    case SIMLOCK_INVALID_LOCK:
                    default:
                       break;
                } 
                
                if (!SIM_NOT_PRESENT())
                {
                    SIMLockUpdateSIMLockState();
                }
                
                result = SIMLOCK_PERMANENTLY_LOCKED;
            }
            else
            {
                result = SIMLOCK_WRONG_KEY;
            }
        }
    }
    
    /* Memory of simlockFile is dynamically allocated in GetSimlockData(), must
     * deallocate the memory after we finish using it. 
     */
    if (simlockFile != NULL)
    {
        kfree(simlockFile);
    }
    
    return result;
}


//--------------------------------------------------------------------------
// Function Name: SIMLockGetSinguture()
//
// Output:      SimLock Signature
//
// Description:   Return the SimLock signature.
//
// simLockSignSize - size of the "simLockSign" buffer. 
//
//----------------------------------------------------------------------------
void SIMLockGetSignature(UInt8* simLockSign, UInt8 simLockSignSize)
{
  if (IsSIMSecureEnable())
  {
      if ((simLockSign != NULL) && (simLockSignSize != 0))
      {
          SIMLock_CodeFile_t *simlockFile = GetSimlockData(); 
          
          if (simlockFile != NULL)
          {
              strncpy( (char *) simLockSign, (char *) simlockFile->simLockSign, 
                   MIN(simLockSignSize - 1, sizeof(simlockFile->simLockSign)) );
              
              simLockSign[simLockSignSize - 1] = '\0';
              
              /* Memory of simlockFile is dynamically allocated in GetSimlockData(), must
               * deallocate the memory after we finish using it. 
               */
              kfree(simlockFile);
          }
          else
          {
              simLockSign[0] = '\0';
          }
      }
  }
  else
  {
      simLockSign[0] = '\0';
  }
}


//--------------------------------------------------------------------------
// Function Name: SIMLockCheckPHSIMLock()
//
// Input:       imsi - ASCII coded IMSI (null terminated)
//                                   
// Output:        none
//
// Return:      TRUE - if the IMSI passes the PH-SIM lock check.  
//                  FALSE - otherwise
//                    
//                    
// Description:   Check if the passed IMSI (which is unique to a SIM card) 
//          is good for the PH-SIM lock settings.
// 
//----------------------------------------------------------------------------
static Boolean SIMLockCheckPHSIMLock(UInt8* imsi)
{
    Boolean result;
    SIMLOCK_NVDATA_t simlock_nvdata;
    
    if (SIM_IsTestSIM())
    {
        return TRUE;
    }
    
    if (GetSimlockNvdata(&simlock_nvdata))
    {
        if (simlock_nvdata.phone_lock_setting == PH_SIM_FULL_LOCK_ON)
        {
            /* Always need user to enter password for full lock */ 
            result = FALSE;
        }
        else if (simlock_nvdata.phone_lock_setting == PH_SIM_LOCK_ON)
        {
            /* Phone lock on, check if IMSI is correct */
            KRIL_DEBUG(DBG_INFO,"simlock_nvdata.phone_lock_imsi:%s imsi:%s\n", 
                (char*) &simlock_nvdata.phone_lock_imsi, (char*)imsi);
            result = ((imsi != NULL) && (strcmp((char *) imsi, (char *) &simlock_nvdata.phone_lock_imsi) == 0));
        }
        else
        {
            /* Phone lock off */
            result = TRUE;
        }
    }
    else
    {
        result = TRUE;
    }
    
    return result;
}


//--------------------------------------------------------------------------
// Function Name: SIMLockSetPHSIMLock()
//
// Input:       lock_setting - PH-SIM lock setting: one of 
//            PH_SIM_LOCK_OFF, PH_SIM_LOCK_ON & PH_SIM_FULL_LOCK_ON.
//
//          imsi - ASCII coded IMSI (null terminated). Pass NULL if SIM not inserted.
//
//          key - ASCII coded password for PH-SIM lock (null terminated)
//            
// Output:        none
//
// Return:      SIMLOCK_SUCCESS - if PH-SIM lock is successfully set.  
//                  SIMLOCK_WRONG_KEY - otherwise.
//                    
//                    
// Description:   Enable/disable the PH-SIM lock feature.
//           
//----------------------------------------------------------------------------
static SIMLock_Status_t SIMLockSetPHSIMLock(PH_SIM_Lock_Status_t lock_setting, UInt8* imsi, UInt8* key)
{
    SIMLock_Status_t result;
    SIMLOCK_NVDATA_t simlock_nvdata;

	  if (((imsi == NULL) || (imsi[0] == '\0')) && (lock_setting != PH_SIM_LOCK_OFF))
	  {
		    KRIL_DEBUG(DBG_ERROR,"IMSI is NULL\n");
		    result = SIMLOCK_FAILURE;
	  }
    else if (GetSimlockNvdata(&simlock_nvdata))
    {
        KRIL_DEBUG(DBG_INFO,"lock_setting:%d simlock_nvdata.phone_lock_pwd:%s key:%s\n", 
            lock_setting, simlock_nvdata.phone_lock_pwd, (char*)key);
        
        if (strcmp((char *) key, simlock_nvdata.phone_lock_pwd) == 0) 
        {
            simlock_nvdata.phone_lock_setting = lock_setting;
            
            if ((imsi != NULL) && (imsi[0] != '\0'))
            {
                strncpy((char *) &simlock_nvdata.phone_lock_imsi, (char *) imsi, sizeof(simlock_nvdata.phone_lock_imsi) - 1);
                ((char *) &simlock_nvdata.phone_lock_imsi)[sizeof(simlock_nvdata.phone_lock_imsi) - 1] = '\0';
            }
            
            /* Update non-volatile SIMLOCK data */
            WriteSimlockNvdata(&simlock_nvdata);
            
            /* Make sure the PH-SIM lock is unlocked */
            Phone_Lock_Status = SIMLOCK_VERIFIED;
            
            result = SIMLOCK_SUCCESS;
        }
        else
        {
            result = SIMLOCK_WRONG_KEY;
        }
    }
    else
    {
        result = SIMLOCK_FAILURE;
    }
    
    return result;
}


//--------------------------------------------------------------------------
// Function Name: IsSIMSecureEnable()
//
// Description:   Return SIM Secure function enable. 
//
// Return:  TRUE if enable; FALSE disable 
//
//----------------------------------------------------------------------------
static Boolean IsSIMSecureEnable(void)
{
#ifdef CONFIG_BRCM_SIM_SECURE_ENABLE 
    return TRUE;
#else
    return FALSE;
#endif
}


//--------------------------------------------------------------------------
//
// Function Name: SIMLockIsLockOn()
//
// Input:     lockType - network lock, provider lock, and etc.
//          ps_full_lock_on - pointer to PH-SIM full lock status. 
//                (set to NULL if not checking SIMLOCK_PHONE_LOCK). 
//
// Output:      ps_full_lock_on - TRUE if PH-SIM full lock is on.
//
// Return:      TRUE - the lock is on; FALSE - lock is off.
//  
// Description:   Check if a lock personalization (indicator) is turned on.
//----------------------------------------------------------------------------
Boolean SIMLockIsLockOn(SIMLockType_t lockType, Boolean *ps_full_lock_on)
{
    Boolean result;
    SIMLOCK_NVDATA_t  simlock_nvdata;
    
    if (GetSimlockNvdata(&simlock_nvdata))
    {
        // Mobilink implementation per GSM 02.22
        switch (lockType)
        {
            case SIMLOCK_NETWORK_LOCK:
                result = (simlock_nvdata.network_lock_ind == 1 ? TRUE : FALSE);
                break;
            
            case SIMLOCK_NET_SUBSET_LOCK: 
                result = (simlock_nvdata.network_subset_lock_ind == 1 ? TRUE : FALSE);
                break;
            
            case SIMLOCK_PROVIDER_LOCK:
                result = (simlock_nvdata.service_provider_lock_ind == 1 ? TRUE : FALSE);
                break;
            
            case SIMLOCK_CORP_LOCK: 
                result = (simlock_nvdata.corporate_lock_ind == 1 ? TRUE : FALSE);
                break;
            
            case SIMLOCK_PHONE_LOCK:
                result = (simlock_nvdata.phone_lock_setting != PH_SIM_LOCK_OFF ? TRUE : FALSE);
                
                if (ps_full_lock_on != NULL)
                {
                    *ps_full_lock_on = (simlock_nvdata.phone_lock_setting == PH_SIM_FULL_LOCK_ON ? TRUE : FALSE);
                }
                break;
            
            default:
                KRIL_DEBUG(DBG_ERROR,"Unsupport lockType:%d Error!!\n", lockType);
                result = FALSE;
                break;
        }
        
        KRIL_DEBUG(DBG_INFO,"lockType:%d\n result:%d", lockType, result); 
    }
    else
    {
        result = FALSE;
    }
    
    return result;
}


//--------------------------------------------------------------------------
// Function Name: SIMLockCheckAllLocks()
// 
// Input:     imsi - IMSI (MCC and MNC), pass NULL if SIM not inserted
//          gid1 - GID1 file, pass NULL if SIM not inserted
//          gid2 - GID2 file. pass NULL if SIM not inserted
//
// Return:      TRUE - SIM valid for all active locks; FALSE - invalid SIM.
//  
// Description:   Check if SIM is valid for all the SIMLOCK types.
//
// Note:            The passed IMSI data must have the following format:
//                  MCC (3 Ascii digits) + MNC (2 or 3 Ascii digits). 
//
//----------------------------------------------------------------------------
Boolean  SIMLockCheckAllLocks(UInt8* imsi, UInt8* gid1, UInt8* gid2)
{
    Boolean result;
    SIMLOCK_NVDATA_t  simlock_nvdata;
    
    result = TRUE;
    
    // SimlockTestImage();
        
    if (GetSimlockNvdata(&simlock_nvdata))
    {
        /* Find out if there is any SIMLOCK type locked. The lock status may have been set to "SIMLOCK_VERIFIED". 
         * This happens when the ME is powered up without SIM and the SIMLOCK type is unlocked, then a SIM is 
         * inserted. In this case we do not need to unlock again. Just keep the original "SIMLOCK_VERIFIED" status.
         */
        if (Network_Lock_Status != SIMLOCK_VERIFIED)
        {
            if( !simlock_nvdata.network_lock_ind || 
              ((imsi != NULL) && SIMLockCheckNetworkLock(imsi)) )
            {
                Network_Lock_Status = SIMLOCK_OPENED; 
            }
            else
            {
                Network_Lock_Status = SIMLOCK_CLOSED;
                result = FALSE; 
            }
        }
        
        if (Network_Subset_Lock_Status != SIMLOCK_VERIFIED)
        {
            if( !simlock_nvdata.network_subset_lock_ind || 
              ((imsi != NULL) && SIMLockCheckNetSubsetLock(imsi)) ) 
            {
                Network_Subset_Lock_Status = SIMLOCK_OPENED; 
            }
            else
            {
                Network_Subset_Lock_Status = SIMLOCK_CLOSED; 
                result = FALSE; 
            }
        }
        
        if (Service_Provider_Lock_Status != SIMLOCK_VERIFIED)
        {
            if( !simlock_nvdata.service_provider_lock_ind || 
              ((imsi != NULL) && (gid1 != NULL) && SIMLockCheckProviderLock(imsi, gid1)) )
            {
                Service_Provider_Lock_Status = SIMLOCK_OPENED;
            }
            else
            {
                Service_Provider_Lock_Status = SIMLOCK_CLOSED;
                result = FALSE; 
            }
        }
        
        if (Corporate_Lock_Status != SIMLOCK_VERIFIED)
        {
            if( !simlock_nvdata.corporate_lock_ind || 
              ((imsi != NULL) && (gid1 != NULL) && (gid2 != NULL) && SIMLockCheckCorpLock(imsi, gid1, gid2)) )
            {
                Corporate_Lock_Status = SIMLOCK_OPENED;
            }
            else
            {
                Corporate_Lock_Status = SIMLOCK_CLOSED;
                result = FALSE;
            } 
        }
    }
    
    /* Check PH-SIM lock status */
    if (Phone_Lock_Status != SIMLOCK_VERIFIED)
    {
        if (SIMLockCheckPHSIMLock(imsi))
        {
            Phone_Lock_Status = SIMLOCK_OPENED;
        }
        else
        {
            Phone_Lock_Status = SIMLOCK_CLOSED;
            result = FALSE;
        }
    }
    
    return result;
}


//--------------------------------------------------------------------------
// Function Name: SIMLockGetCurrentClosedLock()
// Input:     none
// Output:      none
// Return:      return one of the closed lock type.
//          
//  
// Description:   return one of the closed lock type. If no lock type is
//                  closed, SIMLOCK_INVALID_LOCK is returned.
//
// Note:            Function SIMLockCheckAllLocks must have been called
//                  before this function is called!!!!!!     
//----------------------------------------------------------------------------
SIMLockType_t SIMLockGetCurrentClosedLock(Boolean *lock_blocked)
{
    SIMLockType_t lock_type;
    SIMLOCK_NVDATA_t simlock_nvdata;
    SIMLock_CodeFile_t *simlockFile = GetSimlockData(); 
    
    if (Network_Lock_Status == SIMLOCK_CLOSED)
    {
        lock_type = SIMLOCK_NETWORK_LOCK;
    }
    else if (Network_Subset_Lock_Status == SIMLOCK_CLOSED)
    {
        lock_type = SIMLOCK_NET_SUBSET_LOCK;
    }
    else if (Service_Provider_Lock_Status == SIMLOCK_CLOSED)
    {
        lock_type = SIMLOCK_PROVIDER_LOCK;
    }
    else if (Corporate_Lock_Status == SIMLOCK_CLOSED)
    {
        lock_type = SIMLOCK_CORP_LOCK;
    }
    else if (Phone_Lock_Status == SIMLOCK_CLOSED)
    {
        lock_type = SIMLOCK_PHONE_LOCK;
    }
    else
    {
        lock_type = SIMLOCK_INVALID_LOCK;
    }
    
    if (GetSimlockNvdata(&simlock_nvdata) && (simlockFile != NULL))
    {
        switch (lock_type)
        {
            case SIMLOCK_NETWORK_LOCK:
                *lock_blocked = 
                    ( (simlockFile->resetUnlockCounter ? Network_Unlock_Attempt : simlock_nvdata.network_unlock_attempt) 
                      >= simlockFile->maxUnlockAttempt );
                break;
            
            case SIMLOCK_NET_SUBSET_LOCK:
                *lock_blocked = 
                  ( (simlockFile->resetUnlockCounter ? Network_Subset_Unlock_Attempt : simlock_nvdata.network_subset_unlock_attempt) 
                    >= simlockFile->maxUnlockAttempt );
                break;
            
            case SIMLOCK_PROVIDER_LOCK:
                *lock_blocked = 
                  ( (simlockFile->resetUnlockCounter ? Service_Provider_Unlock_Attempt : simlock_nvdata.provider_unlock_attempt) 
                    >= simlockFile->maxUnlockAttempt );
                break;
            
            case SIMLOCK_CORP_LOCK:
                *lock_blocked = 
                  ( (simlockFile->resetUnlockCounter ? Corporate_Unlock_Attempt : simlock_nvdata.corporate_unlock_attempt) 
                    >= simlockFile->maxUnlockAttempt );
                break;
            
            case SIMLOCK_PHONE_LOCK:
            case SIMLOCK_INVALID_LOCK:
            default:
                *lock_blocked = FALSE;
                break;  
        }
    }
      
    /* Memory of simlockFile is dynamically allocated in GetSimlockData(), must
     * deallocate the memory after we finish using it. 
     */
    if (simlockFile != NULL)
    {
        kfree(simlockFile);
    }
    
    return lock_type;
}


//--------------------------------------------------------------------------
// Function Name: SIMLockChangePasswordPHSIM()
//
// Input:     old_pwd - the old ASCII coded PH-SIM password (null terminated)
//          new_pwd - the new ASCII coded PH-SIM password (null terminated)
//      
// Output:      none
//    
// Return:      TRUE - if password is successfully changed
//          FALSE - otherwise
//          
// Description:   This function changes the PH-SIM lock password
//                       
//----------------------------------------------------------------------------
SIMLock_Status_t  SIMLockChangePasswordPHSIM(UInt8 *old_pwd, UInt8 *new_pwd)
{
    SIMLock_Status_t result;  
    SIMLOCK_NVDATA_t simlock_nvdata;

    if (GetSimlockNvdata(&simlock_nvdata))
    {
        if (strcmp((char *) old_pwd, simlock_nvdata.phone_lock_pwd) == 0)
        {
            strncpy(simlock_nvdata.phone_lock_pwd, (char *) new_pwd, sizeof(simlock_nvdata.phone_lock_pwd) - 1);
            simlock_nvdata.phone_lock_pwd[sizeof(simlock_nvdata.phone_lock_pwd) - 1] = '\0';
            
            WriteSimlockNvdata(&simlock_nvdata);
            
            result = SIMLOCK_SUCCESS;
        }
        else
        {
            result = SIMLOCK_WRONG_KEY;
        }
    }
    else
    {
        result = SIMLOCK_FAILURE;
    }

    return result;
}


//--------------------------------------------------------------------------
// Function Name: Boolean SIMLockCheckPasswordPHSIM()
//
// Input:     pwd - the ASCII coded PH-SIM password (null terminated)
//      
// Output:      none
//    
// Return:      TRUE - if password is correct
//          FALSE - otherwise
//          
// Description:   This function checks the PH-SIM lock password
//                       
//----------------------------------------------------------------------------
Boolean SIMLockCheckPasswordPHSIM(UInt8 *pwd)
{
    SIMLOCK_NVDATA_t simlock_nvdata;
    
    if (GetSimlockNvdata(&simlock_nvdata))
    {
        return (strcmp((char *) pwd, simlock_nvdata.phone_lock_pwd) == 0);
    }
    else
    {
        return FALSE;
    }
}


//--------------------------------------------------------------------------
// Function Name: SIMLockUnlockSIM()
// Input:           key - control key
//                  lockType - SIM Lock type except SIMLOCK_PHONE_LOCK
//
// Output:          none
// Return:          lock/unlock status
//
// Description:     lock/unlock desired SIM lock.
//
//  Note:     This function is only allowed to try 3 time after power up.
//----------------------------------------------------------------------------
SIMLock_Status_t  SIMLockUnlockSIM(SIMLockType_t lockType, UInt8* key)
{
    SIMLock_Status_t  simlockStatus = SIMLockUnlockOneType(lockType, key);
    
    /* Update the SIMLOCK status in static variable */
    if (simlockStatus == SIMLOCK_SUCCESS)
    {
        switch (lockType)
        {
            case SIMLOCK_NETWORK_LOCK:
                Network_Lock_Status = SIMLOCK_VERIFIED;
                break;
            
            case SIMLOCK_NET_SUBSET_LOCK:
                Network_Subset_Lock_Status = SIMLOCK_VERIFIED;
                break;
            
            case SIMLOCK_PROVIDER_LOCK:
                Service_Provider_Lock_Status = SIMLOCK_VERIFIED;
                break;
            
            case SIMLOCK_CORP_LOCK:
                Corporate_Lock_Status = SIMLOCK_VERIFIED;
                break;
            
            case SIMLOCK_PHONE_LOCK:
                Phone_Lock_Status = SIMLOCK_VERIFIED;
                break;
            
            default:
               break;
        } 
        
        if (!SIM_NOT_PRESENT())
        {
            SIMLockUpdateSIMLockState();
        }
    }
    else if ((simlockStatus == SIMLOCK_PERMANENTLY_LOCKED) && !SIM_NOT_PRESENT())
    {
        if (!SIM_NOT_PRESENT())
        {
            SIMLockUpdateSIMLockState();
        }
    }
    
    return simlockStatus;

}


//--------------------------------------------------------------------------
// Function Name: SIMLockSetLock()
// Input:           action - 1: lock SIM; 0 = unlock SIM
//          ph_sim_full_lock_on - whether PH-SIM full lock should be set on.
//          lockType - SIM Lock type: network, network subset, ...
//          key - control key
//          imsi - IMSI (MCC and MNC)
//          gid1 - GID1 in SIM
//          gid2 - GID2 in SIM
//
// Output:          none
//
// Return:          lock/unlock status
//
// Description:     Permanently lock/unlock desired SIM lock (indicator on/off).
//
//  Note:     This function is only allowed to try 3 time after power up.
//----------------------------------------------------------------------------

SIMLock_Status_t  SIMLockSetLock(UInt8 action, Boolean ph_sim_full_lock_on, SIMLockType_t lockType,  
                 UInt8* key, UInt8* imsi, UInt8* gid1, UInt8* gid2)
{
    if (lockType == SIMLOCK_PHONE_LOCK)
    {
        SIMLock_Status_t result = SIMLockSetPHSIMLock( ((action == 0) ? PH_SIM_LOCK_OFF :
                (ph_sim_full_lock_on ? PH_SIM_FULL_LOCK_ON : PH_SIM_LOCK_ON)), imsi, key);
        
        /* If unlocking PH-SIM lock, we need to update the SIM PIN status */
        if ((result == SIMLOCK_SUCCESS) && !SIM_NOT_PRESENT())
        {
            SIMLockUpdateSIMLockState();
        }
        
        return result;
    }
    else
    {
        return SIMLockSetLockOneType(lockType, action == 1, key);
    }
}


//--------------------------------------------------------------------------
// Function Name: SIMLockUpdateSIMLockState()
//
// Description:  Update one of locked SIM lock type. If no SIM lock type 
//               is locked, set all SIM lock type as SIM_SECURITY_OPEN.
//           
//
// Return:  NULL 
//      
//
//----------------------------------------------------------------------------
void SIMLockUpdateSIMLockState(void)
{
    Boolean lock_blocked = FALSE;
    
    switch (SIMLockGetCurrentClosedLock(&lock_blocked))
    {
        case SIMLOCK_NETWORK_LOCK:
            g_CurrentSimLockState.network_lock = lock_blocked ? SIM_SECURITY_BLOCKED : SIM_SECURITY_LOCKED;
            break;
        
        case SIMLOCK_NET_SUBSET_LOCK:
            g_CurrentSimLockState.network_subset_lock = lock_blocked ? SIM_SECURITY_BLOCKED: SIM_SECURITY_LOCKED;
            break;
              
        case SIMLOCK_PROVIDER_LOCK:
            g_CurrentSimLockState.service_provider_lock = lock_blocked ? SIM_SECURITY_BLOCKED : SIM_SECURITY_LOCKED;
            break;
              
        case SIMLOCK_CORP_LOCK:
            g_CurrentSimLockState.corporate_lock = lock_blocked ? SIM_SECURITY_BLOCKED : SIM_SECURITY_LOCKED;
            break;
              
        case SIMLOCK_PHONE_LOCK:
            g_CurrentSimLockState.phone_lock = lock_blocked ? SIM_SECURITY_BLOCKED : SIM_SECURITY_LOCKED;
            break;
              
        case SIMLOCK_INVALID_LOCK:
        default:
            g_CurrentSimLockState.network_lock = SIM_SECURITY_OPEN;
            g_CurrentSimLockState.network_subset_lock = SIM_SECURITY_OPEN;
            g_CurrentSimLockState.service_provider_lock = SIM_SECURITY_OPEN;
            g_CurrentSimLockState.corporate_lock = SIM_SECURITY_OPEN;
            g_CurrentSimLockState.phone_lock = SIM_SECURITY_OPEN;
            break;
    }
    
    KRIL_DEBUG(DBG_INFO,"SIM security state PN(%d), PU(%d), PP(%d), PC(%d), PS(%d)\r\n", 
    g_CurrentSimLockState.network_lock, g_CurrentSimLockState.network_subset_lock, 
    g_CurrentSimLockState.service_provider_lock, g_CurrentSimLockState.corporate_lock, 
    g_CurrentSimLockState.phone_lock);
}


//--------------------------------------------------------------------------
// Function Name: SIMLockGetSIMLockState()
//
// Description:  Get the current SIM lock state. 
//
// input/output: simlock_state - this parameter will be used for 
//               CAPI2_SIMLOCK_GetStatus_RSP() to tell CP the currently SIM 
//               lock state.
//
// Return:  NULL 
//
//----------------------------------------------------------------------------
void SIMLockGetSIMLockState(SIMLOCK_STATE_t* simlock_state)
{
    simlock_state->network_lock = g_CurrentSimLockState.network_lock;
    simlock_state->network_subset_lock = g_CurrentSimLockState.network_subset_lock;
    simlock_state->service_provider_lock = g_CurrentSimLockState.service_provider_lock;
    simlock_state->corporate_lock = g_CurrentSimLockState.corporate_lock;
    simlock_state->phone_lock = g_CurrentSimLockState.phone_lock;    
}


//--------------------------------------------------------------------------
// Function Name: SIMLockGetRemainAttempt()
//
// Description:  Get the remain attempt. 
//
// Return:  NULL 
//      
//
//----------------------------------------------------------------------------
UInt8 SIMLockGetRemainAttempt(void)
{
    UInt8 remain_attempt;
    SIMLockType_t lock_type;
    UInt8 *unlock_attempt = NULL;
    SIMLOCK_NVDATA_t simlock_nvdata;
    SIMLock_CodeFile_t *simlockFile = GetSimlockData();
    
    if (Network_Lock_Status == SIMLOCK_CLOSED)
    {
        lock_type = SIMLOCK_NETWORK_LOCK;
    }
    else if (Network_Subset_Lock_Status == SIMLOCK_CLOSED)
    {
        lock_type = SIMLOCK_NET_SUBSET_LOCK;
    }
    else if (Service_Provider_Lock_Status == SIMLOCK_CLOSED)
    {
        lock_type = SIMLOCK_PROVIDER_LOCK;
    }
    else if (Corporate_Lock_Status == SIMLOCK_CLOSED)
    {
        lock_type = SIMLOCK_CORP_LOCK;
    }
    else if (Phone_Lock_Status == SIMLOCK_CLOSED)
    {
        lock_type = SIMLOCK_PHONE_LOCK;
    }
    else
    {
        lock_type = SIMLOCK_INVALID_LOCK;
    }    
    
    if (GetSimlockNvdata(&simlock_nvdata) && (simlockFile != NULL))
    {
        switch (lock_type)
        {
            case SIMLOCK_NETWORK_LOCK:
                unlock_attempt = simlockFile->resetUnlockCounter ? &Network_Unlock_Attempt : &simlock_nvdata.network_unlock_attempt;
                break;
            
            case SIMLOCK_NET_SUBSET_LOCK:
                unlock_attempt = simlockFile->resetUnlockCounter ? &Network_Subset_Unlock_Attempt : &simlock_nvdata.network_subset_unlock_attempt;
                break;
              
            case SIMLOCK_PROVIDER_LOCK:
                unlock_attempt = simlockFile->resetUnlockCounter ? &Service_Provider_Unlock_Attempt : &simlock_nvdata.provider_unlock_attempt;
                break;
            
            case SIMLOCK_CORP_LOCK:
                unlock_attempt = simlockFile->resetUnlockCounter ? &Corporate_Unlock_Attempt : &simlock_nvdata.corporate_unlock_attempt;
                break;
            
            case SIMLOCK_PHONE_LOCK:
            case SIMLOCK_INVALID_LOCK:
            default:
                break;  
        }
    }
    
    if (NULL != unlock_attempt)
    {
        remain_attempt = (*unlock_attempt < simlockFile->maxUnlockAttempt ? simlockFile->maxUnlockAttempt - (*unlock_attempt) : 0); 
    }
    else
    {
        remain_attempt = simlockFile->maxUnlockAttempt;
    }
    
    /* Memory of simlockFile is dynamically allocated in GetSimlockData(), must
     * deallocate the memory after we finish using it. 
     */
    if (simlockFile != NULL)
    {
        kfree(simlockFile);
    }
    
    return remain_attempt;
}
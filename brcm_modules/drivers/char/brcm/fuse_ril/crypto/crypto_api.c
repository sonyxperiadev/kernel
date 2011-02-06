/****************************************************************************
*
*     Copyright (c) 2010 Broadcom Corporation
*           All Rights Reserved
*
*     No portions of this material may be reproduced in any form without the
*     written permission of:
*
*           Broadcom Corporation
*           16215 Alton Parkway
*           P.O. Box 57013
*           Irvine, California 92619-7013
*
*     All information contained in this document is Broadcom Corporation
*     company private, proprietary, and trade secret.
*
****************************************************************************/

#include <linux/io.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/string.h>
#include <linux/module.h>
#include "type_def.h"
#include "aes.h"
#include "crypto_api.h"
#include "brcm_hw_otp.h"
#include "bcm_kril_debug.h"


#define MAC_INPUT_LEN_IMEI            15
#define MAC_INPUT_LEN_SIMLOCK_P_DATA   8

#define MAC_INPUT_TOTAL_LEN           64



// TBD - USE REAL HUK XOR FIXED STRINGS LATER
const uint8 HukStringImei[HUK_LEN] = {
   0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
   0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f
};
const uint8 HukStringSimLockPData[HUK_LEN] = {
   0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
   0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f
};
const uint8 HukStringSimLockCKData[HUK_LEN] = {
   0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
   0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f
};

/* AES-CBC initialization vector for EncDec function */
const uint8 EncDecIv[AES_BLOCK_SIZE] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

uint32 EncDec(uint8 *outDataPtr, const uint8 *inDataPtr, uint32 inDataSize,
              uint32 inEncDec)
{
   uint8 key[HUK_LEN];
   uint32 x;

   /* Check for invalid parameters */
   if ( (outDataPtr == NULL) ||
        (inDataSize < AES_BLOCK_SIZE) ||
        ((inDataSize % AES_BLOCK_SIZE) != 0) ||
        ( (inEncDec != AES_OPERATION_ENCRYPT) &&
          (inEncDec != AES_OPERATION_DECRYPT) ) )
   {
      KRIL_DEBUG(DBG_ERROR,"outDataPtr = NULL:%d inDataSize:%d inEncDec:%d Failed!!!\n",
          (outDataPtr == NULL), inDataSize, inEncDec);
      return(0);
   }

   /* Set the key to bet the HUK XORed with a fixed string */
   if (GetHuk(key) == 1)
   {
      for (x=0; x<HUK_LEN; x++)
      {
         key[x] ^= HukStringSimLockCKData[x];
      }
   }
   else
   {
      return(0);
   }

   /* Encrypt in AES-CBC-128 mode */
   return(Aes(outDataPtr, inDataPtr, inDataSize, (const uint8 *) key,
              AES_KEY_SIZE_128, EncDecIv, AES_MODE_CBC, inEncDec));
}

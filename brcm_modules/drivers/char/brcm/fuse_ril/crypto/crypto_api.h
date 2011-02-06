/*****************************************************************************
*  Copyright 2010 Broadcom Corporation.  All rights reserved.
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

#ifndef CRYPTO_API_H

#define CRYPTO_API_H


#include "type_def.h"


#define MAC_TYPE_IMEI            0
#define MAC_TYPE_SIMLOCK_P_DATA  1

#define MAC_OUTPUT_LEN           8


/******************************************************************************
*  Function Name:  MAC
*
*  Description:    This function generates a MAC for integrity checking of
*                  the IMEI or SIMLock at boot time.
*
*                  A data block used to generate the MAC is concatenated from:
*                     the data to be checked (IMEI or SIMLock) |
*                     HUK from OTP XORed with a fixed string |
*                     stuffing of pseudo random string to fill 512 bits
*                  The output is the first MAC_OUTPUT_LEN bytes of the SHA-1
*                  message digest of the data block.
*
*                  The caller must allocate memory and ensure that the memory
*                  allocated will be sufficient to handle the returned data.
*                  All pointers are required to be word aligned and are not
*                  checked by the API.
*
*  Returns:        uint32:     status
*                                 0=error
*                                 1=ok
*
*  Parameters:     outMacPtr:  output pointer for MAC (MAC_OUTPUT_LEN bytes)
*                  inDataPtr:  input data pointer to the location of the
*                              IMEI (15 bytes) or SIMLock (8 bytes) data
*                  inMacType:  MAC type indicating either IMEI or SIMLock
*                                 MAC_TYPE_IMEI
*                                 MAC_TYPE_SIMLOCK_P_DATA
******************************************************************************/
uint32 MAC(uint8 *outMacPtr, const uint8 *inDataPtr, uint32 inMacType);


/******************************************************************************
*  Function Name:  EncDec
*
*
*
*  Description:    This function encrypts or decrypts a single block of data
*                  using AES-CBC-128.  The key used is the HUK from OTP XORed
*                  with a fixed string.  The initialization vector for CBC
*                  mode is all zeros
*
*                  The caller must allocate memory and ensure that the memory
*                  allocated will be sufficient to handle the returned data.
*                  All pointers are required to be word aligned and are not
*                  checked by the API.
*
*  Returns:        uint32:      status
*                                  0=error
*                                  1=ok
*
*  Parameters:     outDataPtr:  output data pointer (16 bytes)
*                  inDataPtr:   input data pointer (16 bytes)
*                  inDataSize:  size in bytes
*                  inEncDec:    encrypt or decrypt
*                                  AES_OPERATION_ENCRYPT
*                                  AES_OPERATION_DECRYPT
******************************************************************************/
uint32 EncDec(uint8 *outDataPtr, const uint8 *inDataPtr, uint32 inDataSize,
              uint32 inEncDec);

#endif

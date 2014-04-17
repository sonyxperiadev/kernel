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


/*AES definition*/
#define AES_OPERATION_ENCRYPT  1
#define AES_OPERATION_DECRYPT  0
#define AES_BLOCK_SIZE        16

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
*  Returns:        unsigned char:          status
*                                          0=error
*                                          1=ok
*
*  Parameters:     outDataPtr:  output data pointer (16 bytes)
*                  inDataPtr:   input data pointer (16 bytes)
*                  inDataSize:  size in bytes
*                  inEncDec:    encrypt or decrypt
*                                  AES_OPERATION_ENCRYPT
*                                  AES_OPERATION_DECRYPT
******************************************************************************/
unsigned char EncDec(unsigned char *outDataPtr,
					const unsigned char *inDataPtr,
					unsigned int inDataSize,
					unsigned int inEncDec);

#endif

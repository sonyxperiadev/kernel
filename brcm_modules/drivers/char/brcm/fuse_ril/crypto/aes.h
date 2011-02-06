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

#ifndef AES_H

#define AES_H


#include "type_def.h"


/* AES constants */
#define AES_KEY_SIZE_128       0
#define AES_KEY_SIZE_192       1
#define AES_KEY_SIZE_256       2

#define AES_BLOCK_SIZE        16

#define AES_MODE_ECB           0
#define AES_MODE_CBC           1

#define AES_OPERATION_ENCRYPT  0
#define AES_OPERATION_DECRYPT  1


/******************************************************************************
*  Function Name:  Aes
*
*  Description:    This function encrypts or decrypts in AES-ECB or AES-CBC
*                  modes.
*
*                  The caller must allocate memory and ensure that the memory
*                  allocated will be sufficient to handle the returned data.
*                  All pointers are required to be word aligned and are not
*                  checked by the API.
*
*  Returns:        uint32:       status
*                                   0=error
*                                   1=ok
*
*  Parameters:     outDataPtr:   output data pointer, which will contain the
*                                same number of bytes as inDataSize
*                  inDataPtr:    input data pointer
*                  inDataSize:   size in bytes (must be a multiple of the AES
*                                block size)
*                  inKeyPtr:     pointer to the key
*                  inKeySize:    key size
*                                   AES_KEY_SIZE_128
*                                   AES_KEY_SIZE_192
*                                   AES_KEY_SIZE_256
*                  inIvPtr:      pointer to initialization vector (for CBC mode
*                                only)
*                  inMode:       AES mode, ECB or CBC
*                                   AES_MODE_ECB
*                                   AES_MODE_CBC
*                  inOperation:  encrypt or decrypt direction
*                                   AES_OPERATION_ENCRYPT
*                                   AES_OPERATION_DECRYPT
******************************************************************************/
uint32 Aes(uint8 *outDataPtr, const uint8 *inDataPtr, uint32 inDataSize,
           const uint8 *inKeyPtr, uint32 inKeySize, const uint8 *inIvPtr,
           uint32 inMode, uint32 inOperation);

#endif

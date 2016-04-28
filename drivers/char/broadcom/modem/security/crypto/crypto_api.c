/*********************************************************************
*
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license agreement
* governing use of this software, this software is licensed to you under the
* terms of the GNU General Public License version 2, available at
* http://www.gnu.org/copyleft/gpl.html (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a license
* other than the GPL, without Broadcom's express prior written consent.
***************************************************************************/
#include <linux/slab.h>
#include <mach/clock.h>
#include <linux/dma-mapping.h>
#include "crypto_api.h"

#ifdef CONFIG_KONA_SECURE_MONITOR_CALL
#include <mach/sec_api.h>
#endif

/******************************************************************************
// Function Name: EncDec()
//
// Input:	  *outDataPtr -  output data.
//			  *inDataPtr  -  inpit data.
//            inDataSize  -  input data length.
//            inEncDec    -  lower 16 bit : 1:Encryption 0:Decryption.
//                           upper 16 bit:  HDCP wrapping key=1; Random key=0
// Description: Do AES Encryption/Decryption.
//
******************************************************************************/
unsigned char EncDec(unsigned char *outDataPtr, const unsigned char *inDataPtr,
			unsigned int inDataSize, unsigned int inEncDec)
{
#if defined(CONFIG_KONA_SECURE_MONITOR_CALL) &&\
		defined(CONFIG_CRYPTO_DEV_BRCM_SPUM_AES)

	 void *aes_buf_vir = NULL;
	 dma_addr_t aes_buf_phy;
	 unsigned char *aes_v = NULL;
	 unsigned char *aes_p = NULL;

	 /* Check for invalid parameters */
	 if ((outDataPtr == NULL) ||
		(inDataSize > SZ_4K) ||
		(inDataSize < AES_BLOCK_SIZE) ||
	    ((inDataSize % AES_BLOCK_SIZE) != 0)) {
		pr_err
		("outDataPtr = NULL:%d inDataSize:%d inEncDec:%d Failed!!!\n",
			(outDataPtr == NULL), (int)inDataSize, (int)inEncDec);
		return 0;
	}

	aes_buf_vir = dma_alloc_coherent(NULL, SZ_4K, &aes_buf_phy, GFP_KERNEL);
	if (aes_buf_vir == NULL) {
		pr_info("%s: dma buffer alloc for aes failed\n", __func__);
		return 0;
	}
	aes_v = (unsigned char *) aes_buf_vir;
	aes_p = (unsigned char *) aes_buf_phy;

	memset(aes_v, 0x00, SZ_4K);
	memcpy(&aes_v[0], inDataPtr, inDataSize);

	/*secure_api_call is the secure service entry point.Caller calls
	   the API with proper parameters and gets AES results returned.
	   argument 1 : Application Id.
	   argument 2 : Pointer to the data to be encrypted or decrypted.
			It should be physical address.
	   argument 3 : Length of data
	   argument 4 : Pointer to output buffer.It should be physical address.
	   argument 5 : Encrypt or decrypt and HDCP wrapping key or Random key
	 */
	secure_api_call(SSAPI_ROW_AES,
			(int)&aes_p[0],
			inDataSize, (int)&aes_p[inDataSize], inEncDec);

	memcpy(outDataPtr, &aes_v[inDataSize], inDataSize);
	if (aes_buf_vir)
		dma_free_coherent(NULL, SZ_4K, aes_buf_vir, aes_buf_phy);

	return 1;
#else
	/*memcpy(outDataPtr, inDataPtr, inDataSize);*/
	return 0;
#endif
}

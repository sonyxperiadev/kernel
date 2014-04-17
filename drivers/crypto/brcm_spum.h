/*******************************************************************************
* Copyright 2011 Broadcom Corporation.  All rights reserved.
*
*       @file   drivers/crypto/brcm_spum.h
*
* Unless you and Broadcom execute a separate written software license agreement
* governing use of this software, this software is licensed to you under the
* terms of the GNU General Public License version 2, available at
* http://www.gnu.org/copyleft/gpl.html (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a license
* other than the GPL, without Broadcom's express prior written consent.
*******************************************************************************/
#ifndef _BRCM_SPUM_H_
#define _BRCM_SPUM_H_

#include <crypto/algapi.h>

#define SPUM_CMD_CRYPTO_ENCRYPTION        0x00000000
#define SPUM_CMD_CRYPTO_DECRYPTION        0x80000000

/* Misc. key entry defines */
#define SPUM_CMD_KEY_SIZE_SHIFT           12
#define SPUM_CMD_KEY_SIZE_MASK            (0x000001FF << SPUM_CMD_KEY_SIZE_SHIFT)
#define SPUM_CMD_KEY_VALID                0x80000000
#define SPUM_CMD_KEY_OFFSET_MASK          0x00000FFF

/* Misc. key type defines (cache key) */
#define SPUM_CMD_KEY_SHIFT                21
#define SPUM_CMD_KEY_MASK                 (0x000000FF << SPUM_CMD_KEY_SHIFT)
#define SPUM_CMD_KEY_DES_CBC              (0x00000000 << SPUM_CMD_KEY_SHIFT)
#define SPUM_CMD_KEY_3DES_CBC             (0x00000001 << SPUM_CMD_KEY_SHIFT)
#define SPUM_CMD_KEY_AES128_CBC           (0x00000002 << SPUM_CMD_KEY_SHIFT)
#define SPUM_CMD_KEY_AES128_ECB           (0x00000002 << SPUM_CMD_KEY_SHIFT)
#define SPUM_CMD_KEY_AES192_CBC           (0x00000003 << SPUM_CMD_KEY_SHIFT)
#define SPUM_CMD_KEY_AES192_ECB           (0x00000003 << SPUM_CMD_KEY_SHIFT)
#define SPUM_CMD_KEY_AES256_CBC           (0x00000004 << SPUM_CMD_KEY_SHIFT)
#define SPUM_CMD_KEY_AES256_ECB           (0x00000004 << SPUM_CMD_KEY_SHIFT)
#define SPUM_CMD_KEY_AES128_CTR           (0x00000005 << SPUM_CMD_KEY_SHIFT)
#define SPUM_CMD_KEY_AES192_CTR           (0x00000006 << SPUM_CMD_KEY_SHIFT)
#define SPUM_CMD_KEY_AES256_CTR           (0x00000007 << SPUM_CMD_KEY_SHIFT)
#define SPUM_CMD_KEY_HMAC_SHA1            (0x00000040 << SPUM_CMD_KEY_SHIFT)
#define SPUM_CMD_KEY_HMAC_MD5             (0x00000041 << SPUM_CMD_KEY_SHIFT)
#define SPUM_CMD_KEY_HMAC_SHA224          (0x00000042 << SPUM_CMD_KEY_SHIFT)
#define SPUM_CMD_KEY_HMAC_SHA256          (0x00000043 << SPUM_CMD_KEY_SHIFT)
#define SPUM_CMD_KEY_FHMAC_SHA1           (0x00000044 << SPUM_CMD_KEY_SHIFT)
#define SPUM_CMD_KEY_FHMAC_MD5            (0x00000045 << SPUM_CMD_KEY_SHIFT)
#define SPUM_CMD_KEY_CTXT_SHA1            (0x00000048 << SPUM_CMD_KEY_SHIFT)
#define SPUM_CMD_KEY_CTXT_MD5             (0x00000049 << SPUM_CMD_KEY_SHIFT)
#define SPUM_CMD_KEY_RC4                  (0x00000050 << SPUM_CMD_KEY_SHIFT)
#define SPUM_CMD_KEY_HASH_UPDT_SHA1       (0x00000052 << SPUM_CMD_KEY_SHIFT)
#define SPUM_CMD_KEY_HASH_UPDT_MD5        (0x00000053 << SPUM_CMD_KEY_SHIFT)
#define SPUM_CMD_KEY_HASH_UPDT_SHA224     (0x00000054 << SPUM_CMD_KEY_SHIFT)
#define SPUM_CMD_KEY_HASH_UPDT_SHA256     (0x00000055 << SPUM_CMD_KEY_SHIFT)
#define SPUM_CMD_KEY_HASH_FIN_SHA1        (0x00000056 << SPUM_CMD_KEY_SHIFT)
#define SPUM_CMD_KEY_HASH_FIN_MD5         (0x00000057 << SPUM_CMD_KEY_SHIFT)
#define SPUM_CMD_KEY_HASH_FIN_SHA224      (0x00000058 << SPUM_CMD_KEY_SHIFT)
#define SPUM_CMD_KEY_HASH_FIN_SHA256      (0x00000059 << SPUM_CMD_KEY_SHIFT)

/* Misc. key mode defines (cache key) */
#define SPUM_CMD_KEY_SECURE               0x40000000
#define SPUM_CMD_KEY_OPEN                 0x00000000
#define SPUM_CMD_KEY_VALLID               0x80000000

/* Misc. crypto algorithms */
#define SPUM_CMD_CRYPTO_SHIFT             21
#define SPUM_CMD_CRYPTO_MASK              (0x00000007 << SPUM_CMD_CRYPTO_SHIFT)
#define SPUM_CMD_CRYPTO_NULL              (0x00000000 << SPUM_CMD_CRYPTO_SHIFT)
#define SPUM_CMD_CRYPTO_RC4               (0x00000001 << SPUM_CMD_CRYPTO_SHIFT)
#define SPUM_CMD_CRYPTO_DES               (0x00000002 << SPUM_CMD_CRYPTO_SHIFT)
#define SPUM_CMD_CRYPTO_3DES              (0x00000003 << SPUM_CMD_CRYPTO_SHIFT)
#define SPUM_CMD_CRYPTO_AES               (0x00000004 << SPUM_CMD_CRYPTO_SHIFT)

/* Misc. crypto modes */
#define SPUM_CMD_CMODE_SHIFT              18
#define SPUM_CMD_CMODE_MASK               (0x00000007 << SPUM_CMD_CMODE_SHIFT)
#define SPUM_CMD_CMODE_ECB                (0x00000000 << SPUM_CMD_CMODE_SHIFT)
#define SPUM_CMD_CMODE_CBC                (0x00000001 << SPUM_CMD_CMODE_SHIFT)
#define SPUM_CMD_CMODE_CTR                (0x00000004 << SPUM_CMD_CMODE_SHIFT)
#define SPUM_CMD_CMODE_GCM                (0x00000006 << SPUM_CMD_CMODE_SHIFT)
#define SPUM_CMD_CMODE_XTS                (0x00000007 << SPUM_CMD_CMODE_SHIFT)

/* Misc. crypto operation types */
#define SPUM_CMD_COPTYPE_SHIFT            16
#define SPUM_CMD_COPTYPE_MASK             (0x00000003 << SPUM_CMD_COPTYPE_SHIFT)
#define SPUM_CMD_COPTYPE_INIT             (0x00000000 << SPUM_CMD_COPTYPE_SHIFT)
#define SPUM_CMD_COPTYPE_K56              (0x00000000 << SPUM_CMD_COPTYPE_SHIFT)
#define SPUM_CMD_COPTYPE_K168             (0x00000000 << SPUM_CMD_COPTYPE_SHIFT)
#define SPUM_CMD_COPTYPE_K128             (0x00000000 << SPUM_CMD_COPTYPE_SHIFT)
#define SPUM_CMD_COPTYPE_UPDATE           (0x00000001 << SPUM_CMD_COPTYPE_SHIFT)
#define SPUM_CMD_COPTYPE_K192             (0x00000001 << SPUM_CMD_COPTYPE_SHIFT)
#define SPUM_CMD_COPTYPE_K256             (0x00000002 << SPUM_CMD_COPTYPE_SHIFT)

#define SPUM_CMD_AUTH_FIRST               0x40000000
#define SPUM_CMD_AUTH_LAST                0x00000000

/* Misc. authentication algorithms */
#define SPUM_CMD_AUTH_SHIFT               13
#define SPUM_CMD_AUTH_MASK                (0x00000007 << SPUM_CMD_AUTH_SHIFT)
#define SPUM_CMD_AUTH_NULL                (0x00000000 << SPUM_CMD_AUTH_SHIFT)
#define SPUM_CMD_AUTH_MD5                 (0x00000001 << SPUM_CMD_AUTH_SHIFT)
#define SPUM_CMD_AUTH_SHA1                (0x00000002 << SPUM_CMD_AUTH_SHIFT)
#define SPUM_CMD_AUTH_SHA224              (0x00000003 << SPUM_CMD_AUTH_SHIFT)
#define SPUM_CMD_AUTH_SHA256              (0x00000004 << SPUM_CMD_AUTH_SHIFT)
#define SPUM_CMD_AUTH_AES                 (0x00000005 << SPUM_CMD_AUTH_SHIFT)

/* Misc. authetication modes */
#define SPUM_CMD_AMODE_SHIFT              10
#define SPUM_CMD_AMODE_MASK               (0x00000007 << SPUM_CMD_AMODE_SHIFT)
#define SPUM_CMD_AMODE_HASH               (0x00000000 << SPUM_CMD_AMODE_SHIFT)
#define SPUM_CMD_AMODE_CTXT               (0x00000001 << SPUM_CMD_AMODE_SHIFT)
#define SPUM_CMD_AMODE_HMAC               (0x00000002 << SPUM_CMD_AMODE_SHIFT)
#define SPUM_CMD_AMODE_FHMAC              (0x00000006 << SPUM_CMD_AMODE_SHIFT)
#define SPUM_CMD_AMODE_GCM                (0x00000006 << SPUM_CMD_AMODE_SHIFT)

/* Misc. authetication operation types */
#define SPUM_CMD_AOPTYPE_SHIFT            8
#define SPUM_CMD_AOPTYPE_MASK             (0x00000003 << SPUM_CMD_AOPTYPE_SHIFT)
#define SPUM_CMD_AOPTYPE_NONE             (0x00000000 << SPUM_CMD_AOPTYPE_SHIFT)
#define SPUM_CMD_AOPTYPE_FULL             (0x00000000 << SPUM_CMD_AOPTYPE_SHIFT)
#define SPUM_CMD_AOPTYPE_INIT             (0x00000001 << SPUM_CMD_AOPTYPE_SHIFT)
#define SPUM_CMD_AOPTYPE_UPDATE           (0x00000002 << SPUM_CMD_AOPTYPE_SHIFT)
#define SPUM_CMD_AOPTYPE_FIN              (0x00000003 << SPUM_CMD_AOPTYPE_SHIFT)
#define SPUM_CMD_AOPTYPE_AES_K128         (0x00000000 << SPUM_CMD_AOPTYPE_SHIFT)

#define SPUM_CMD_SCTX_KEY_PROTECT         0x80000000

#define SPUM_CMD_SCTX_KEY_HANDLE_SHIFT    20
#define SPUM_CMD_SCTX_KEY_HANDLE_MASK     (0x000001FF << SPUM_CMD_SCTX_KEY_HANDLE_SHIFT)

#define SPUM_CMD_SCTX_ICV_INSERT          0x00002000
#define SPUM_CMD_SCTX_ICV_CHECK           0x00001000
#define SPUM_CMD_SCTX_ICV_SIZE_SHIFT      8
#define SPUM_CMD_SCTX_ICV_SIZE_MASK       (0x0000000F << SPUM_CMD_SCTX_ICV_SIZE_SHIFT)

#define SPUM_CMD_SCTX_IV_CONTEXT          0x00000080
#define SPUM_CMD_SCTX_IV_EXPLICIT         0x00000040
#define SPUM_CMD_SCTX_IV_GENERATE         0x00000020

#define SPUM_CMD_SCTX_IV_OFFSET_SHIFT     3
#define SPUM_CMD_SCTX_IV_OFFSET_MASK      (0x00000003 << SPUM_CMD_SCTX_IV_SHIFT)

#define SPUM_CMD_SCTX_IV_EXPLICIT_SIZE    0x00000007

#define SPUM_CMD_AUTH_RESULT_ENABLE       0x04000000	/* Generate authentication result  */
#define SPUM_CMD_AUTH_RESULT_DISABLE      0x00000000	/* Do not generate authentication result  */

#define SPUM_CMD_CRYPTO_STATUS_MASK                  0x0000FF00
#define SPUM_CMD_CRYPTO_STATUS_SUCCESS               0x00000000
#define SPUM_CMD_CRYPTO_STATUS_INVALID_KEY_HANDLE    0x00000300
#define SPUM_CMD_CRYPTO_STATUS_INVALID_ICV           0x00000400
#define SPUM_CMD_CRYPTO_STATUS_INVALID_KEY           0x00000200
#define SPUM_CMD_CRYPTO_STATUS_UNKNOWN               0x0000FF00
#define SPUM_CMD_CRYPTO_STATUS_ERROR                 0x00020000

#define SPUM_OUTPUT_HEADER_LEN            12	/* Total SPU output header in bytes */
#define SPUM_INPUT_STATUS_LEN		  4	/* Out status length in bytes */
#define SPUM_OUTPUT_STATUS_LEN            4	/* Out status length in bytes */

/* Misc. defines for command buffer */
#define INVALID_KEY_OFFSET	0x00000FFF	/* Invalid offset to the cache key */

#define SCTX_PRESENT		0x80000000	/* Security context field is present in the command */
#define BDESC_PRESENT		0x20000000	/* Buffer data descriptor is present in the command */
#define BD_PRESENT		0x08000000	/* Buffer data is present in the command */
#define OUT_ALL_DATA		0x00010000	/* Output all data */
#define HASH_PRESENT		0x04000000	/* Hash data is present */

#define SCTX_TYPE_GENERIC	0x00000000
#define SCTX_SIZE_MASK		0x000000FF
#define SCTX_SIZE_MIN		3
#define SCTX_AUTH_FIRST		0x40000000	/* Authentication is performed first. Encryption is done afterwards */

typedef enum {
	SPUM_CRYPTO_ALGO_NULL = SPUM_CMD_CRYPTO_NULL,	/* Perform NULL encryption algorithm */
	SPUM_CRYPTO_ALGO_RC4 = SPUM_CMD_CRYPTO_RC4,	/* Perform RC4 encryption algorithm */
	SPUM_CRYPTO_ALGO_DES = SPUM_CMD_CRYPTO_DES,	/* Perform DES encryption algorithm */
	SPUM_CRYPTO_ALGO_3DES = SPUM_CMD_CRYPTO_3DES,	/* Perform 3DES encryption algorithm */
	SPUM_CRYPTO_ALGO_AES = SPUM_CMD_CRYPTO_AES	/* Perform AES encryption algorithm */
} spum_crypto_algo;

typedef enum {
	SPUM_CRYPTO_ENCRYPTION = SPUM_CMD_CRYPTO_ENCRYPTION,
	SPUM_CRYPTO_DECRYPTION = SPUM_CMD_CRYPTO_DECRYPTION
} spum_crypto_op;

typedef enum {
	SPUM_CRYPTO_MODE_ECB = SPUM_CMD_CMODE_ECB,
	SPUM_CRYPTO_MODE_CBC = SPUM_CMD_CMODE_CBC,
	SPUM_CRYPTO_MODE_CTR = SPUM_CMD_CMODE_CTR,
	SPUM_CRYPTO_MODE_GCM = SPUM_CMD_CMODE_GCM,
	SPUM_CRYPTO_MODE_XTS = SPUM_CMD_CMODE_XTS
} spum_crypto_mode;

typedef enum {
	SPUM_CRYPTO_TYPE_NULL_NONE = SPUM_CMD_COPTYPE_INIT,
	SPUM_CRYPTO_TYPE_RC4_INIT = SPUM_CMD_COPTYPE_INIT,
	SPUM_CRYPTO_TYPE_RC4_UPDATE = SPUM_CMD_COPTYPE_UPDATE,
	SPUM_CRYPTO_TYPE_DES_K56 = SPUM_CMD_COPTYPE_K56,
	SPUM_CRYPTO_TYPE_3DES_K168 = SPUM_CMD_COPTYPE_K168,
	SPUM_CRYPTO_TYPE_AES_K128 = SPUM_CMD_COPTYPE_K128,
	SPUM_CRYPTO_TYPE_AES_K192 = SPUM_CMD_COPTYPE_K192,
	SPUM_CRYPTO_TYPE_AES_K256 = SPUM_CMD_COPTYPE_K256
} spum_crypto_type;

typedef enum {
	SPUM_AUTH_ALGO_NULL = SPUM_CMD_AUTH_NULL,	/* Perform NULL authentication/hash algorithm */
	SPUM_AUTH_ALGO_MD5 = SPUM_CMD_AUTH_MD5,	/* Perform MD5 authentication/hash algorithm */
	SPUM_AUTH_ALGO_SHA1 = SPUM_CMD_AUTH_SHA1,	/* Perform SHA1 authentication/hash algorithm */
	SPUM_AUTH_ALGO_SHA224 = SPUM_CMD_AUTH_SHA224,	/* Perform SHA224 authentication/hash algorithm */
	SPUM_AUTH_ALGO_SHA256 = SPUM_CMD_AUTH_SHA256,	/* Perform SHA256 authentication/hash algorithm */
	SPUM_AUTH_ALGO_AES = SPUM_CMD_AUTH_AES	/* Perform AES authentication/hash algorithm for AES-GCM */
} spum_auth_algo;

typedef enum {
	SPUM_AUTH_MODE_HASH = SPUM_CMD_AMODE_HASH,
	SPUM_AUTH_MODE_CTXT = SPUM_CMD_AMODE_CTXT,
	SPUM_AUTH_MODE_HMAC = SPUM_CMD_AMODE_HMAC,
	SPUM_AUTH_MODE_FHMAC = SPUM_CMD_AMODE_FHMAC,
	SPUM_AUTH_MODE_GCM = SPUM_CMD_AMODE_GCM
} spum_auth_mode;

typedef enum {
	SPUM_AUTH_TYPE_NULL_NONE = SPUM_CMD_AOPTYPE_NONE,
	SPUM_AUTH_TYPE_MD5_FULL = SPUM_CMD_AOPTYPE_FULL,
	SPUM_AUTH_TYPE_MD5_INIT = SPUM_CMD_AOPTYPE_INIT,
	SPUM_AUTH_TYPE_MD5_UPDATE = SPUM_CMD_AOPTYPE_UPDATE,
	SPUM_AUTH_TYPE_MD5_FINAL = SPUM_CMD_AOPTYPE_FIN,
	SPUM_AUTH_TYPE_SHA1_FULL = SPUM_CMD_AOPTYPE_FULL,
	SPUM_AUTH_TYPE_SHA1_INIT = SPUM_CMD_AOPTYPE_INIT,
	SPUM_AUTH_TYPE_SHA1_UPDATE = SPUM_CMD_AOPTYPE_UPDATE,
	SPUM_AUTH_TYPE_SHA1_FINAL = SPUM_CMD_AOPTYPE_FIN,
	SPUM_AUTH_TYPE_SHA224_FULL = SPUM_CMD_AOPTYPE_FULL,
	SPUM_AUTH_TYPE_SHA256_FULL = SPUM_CMD_AOPTYPE_FULL,
	SPUM_AUTH_TYPE_AES_K128 = SPUM_CMD_AOPTYPE_AES_K128
} spum_auth_type;

typedef enum {
	SPUM_AUTH_ORDER_FIRST = SPUM_CMD_AUTH_FIRST,	/* Authentication is performed first. Encryption is done afterwards  */
	SPUM_AUTH_ORDER_LAST = SPUM_CMD_AUTH_LAST	/* Encryption is performed first. Authentication is done over the encrypted data */
} spum_auth_order;

typedef enum {
	SPUM_KEY_OPEN = 0,
	SPUM_KEY_PROTECTED = SPUM_CMD_SCTX_KEY_PROTECT,
} spum_key;

/* Definitions for Input/Output message format. */

typedef struct {
	uint32_t firstWord;
	uint32_t secondWord;
	uint32_t thirdWord;
	uint32_t extended;
} spum_sctx_hdr;

typedef struct {
	u16 data_length;	/* Total data length in bytes */
	u16 prev_length;	/* Previous length on which hashing is performed till now. In number of blocks. */
	u16 mac_offset;		/* Authentication operation to be performed at */
	u16 mac_length;		/* Length of the authentication data in bytes */
	u16 crypto_offset;	/* Crypto operation to be performed at */
	u16 crypto_length;	/* Length of the crypto data in bytes */
	u16 aes_gcm_auth_length;	/* AES-GCM Authentication only data length in bytes */
} spum_data_attrib;

/* Defining chip specific crypto context */
struct spum_hw_context {
	u32 emh;		/* Extended Message Header Word */
	spum_crypto_op operation;	/* Cryptographic operation */
	spum_crypto_algo crypto_algo;	/* Cryptographic algorithm */
	spum_crypto_mode crypto_mode;	/* Cryptographic mode */
	spum_crypto_type crypto_type;	/* Cryptographic operation type */
	spum_auth_algo auth_algo;	/* Authentication/Hash algorithm */
	spum_auth_mode auth_mode;	/* Authentication/Hash mode */
	spum_auth_type auth_type;	/* Authentication/Hash type */
	spum_auth_order auth_order;	/* Authentication order */

	spum_key key_type;	/* Protected or open key */
	u32 key_handle;		/* Key handle (Key entry), when "key_type" is SPUM_KEY_PROTECTED */
	void *auth_key;		/* Authentication key (MAC key), when "key_type" is SPUM_KEY_OPEN */
	u32 auth_key_len;	/* Authentication key length in words, when "key_type" is SPUM_KEY_OPEN */
	void *crypto_key;	/* Crypto key (Cihper key), when "key_type" is SPUM_KEY_OPEN */
	u32 crypto_key_len;	/* Crypto key length in words, when "key_type" is SPUM_KEY_OPEN */
	u32 key_update;		/* Offset within the key to update the state */
	void *init_vector;	/* Initialize vector */
	u32 init_vector_len;	/* Initialize vector length in words */
	u32 icv_len;		/* ICV (Integrity Check Value ~ HASH) length in word */
	spum_data_attrib data_attribute;	/* Data attribute */
};

#define FLAGS_BUSY		1
#define FLAGS_RBUSY		2
#define FLAGS_TBUSY		4
#define SPUM_QUEUE_LENGTH       300

struct spum_hash_device {
	void __iomem *io_apb_base;
	void __iomem *io_axi_base;
	struct list_head list;
	struct device *dev;
	struct clk *spum_open_clk;
	u32 dma_len;
	struct scatterlist *sg;
	struct ahash_request *req;
};

struct spum_aes_device {
	void __iomem *io_apb_base;
	void __iomem *io_axi_base;
	struct list_head list;
	struct device *dev;
	struct clk *spum_open_clk;
	struct tasklet_struct done_task;
	struct ablkcipher_request *req;
};

struct brcm_spum_device {
	ulong flags;
	spinlock_t lock;
	u32 rx_dma_chan;
	u32 tx_dma_chan;
	struct spum_hash_device *hash_dev;
	struct spum_aes_device *aes_dev;
	struct crypto_queue spum_queue;
};

extern struct brcm_spum_device *spum_dev;

extern void spum_queue_task(unsigned long data);

extern void spum_dma_init(void __iomem *io_axi_base);
extern int spum_format_command(struct spum_hw_context *spum_ctx,
				void *spum_cmd);
extern void spum_set_pkt_length(void __iomem *io_axi_base,
				u32 rx_len, u32 tx_len);
extern int spum_processing_done(void __iomem *io_axi_base);
extern void spum_init_device(void __iomem *io_apb_base,
				void __iomem *io_axi_base);
extern int spum_hash_process_request(struct spum_hash_device *dd);
extern int spum_aes_process_request(struct spum_aes_device *dd);
extern int spum_enqueue_request(struct crypto_async_request *req);

#endif /* _BRCM_SPUM_H_ */

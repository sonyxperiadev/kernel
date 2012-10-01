/****************************************************************************
*
* Copyright (c) 2009 Broadcom Corporation
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available
* at http://www.gnu.org/licenses/old-licenses/gpl-2.0.html (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a
* license other than the GPL, without Broadcom's express prior written
* consent.
*
* The below HMAC-SHA256 computation code was taken from-
* http://www.ouah.org/ogay/hmac/
* I've removed unnecesary code from the original source and fixed checkpatch
* errors and warnings.
* I'm also retaining the original license header from the
* original source which is below.
*
****************************************************************************/

/*
 * HMAC-SHA-224/256/384/512 implementation
 * Last update: 06/15/2005
 * Issue date:  06/15/2005
 *
 * Copyright (C) 2005 Olivier Gay <olivier.gay@a3.epfl.ch>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the project nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef HMAC_SHA2_H
#define HMAC_SHA2_H

#include "sha2.h"

#ifdef __cplusplus
extern "C" {
#endif

struct hmac_sha256_ctx {
	struct sha256_ctx ctx_inside;
	struct sha256_ctx ctx_outside;

	/* for hmac_reinit */
	struct sha256_ctx ctx_inside_reinit;
	struct sha256_ctx ctx_outside_reinit;

	unsigned char block_ipad[SHA256_BLOCK_SIZE];
	unsigned char block_opad[SHA256_BLOCK_SIZE];
};

void hmac_sha256_init(struct hmac_sha256_ctx *ctx,
			const unsigned char *key, unsigned int key_size);
void hmac_sha256_reinit(struct hmac_sha256_ctx *ctx);
void hmac_sha256_update(struct hmac_sha256_ctx *ctx,
			const unsigned char *message, unsigned int message_len);
void hmac_sha256_final(struct hmac_sha256_ctx *ctx, unsigned char *mac,
						unsigned int mac_size);
void hmac_sha256(const unsigned char *key, unsigned int key_size,
			const unsigned char *message, unsigned int message_len,
			unsigned char *mac, unsigned mac_size);

#ifdef __cplusplus
}
#endif

#endif /* !HMAC_SHA2_H */

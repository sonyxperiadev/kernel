/*
 * Copyright (c) 2013, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials provided
 *      with the distribution.
 *    * Neither the name of The Linux Foundation nor the names of its
 *      contributors may be used to endorse or promote products derived
 *      from this software without specific prior written permission.
 *
 *THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 *WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 *MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 *ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 *BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 *BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 *WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 *OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 *IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#if !defined _WLAN_NV_TEMPLATE_API_H
#define  _WLAN_NV_TEMPLATE_API_H

#include "wlan_nv_types.h"

/*
* API Prototypes
* These are meant to be exposed to outside applications
*/
//extern void writeNvData(void);
//extern VOS_STATUS nvParser(tANI_U8 *pnvEncodedBuf, tANI_U32 nvReadBufSize, sHalNv *);

/*
* Parsing control bitmap
*/
#define _ABORT_WHEN_MISMATCH_MASK  0x00000001     /*set: abort when mismatch, clear: continue taking matched entries*/
#define _IGNORE_THE_APPENDED_MASK  0x00000002     /*set: ignore, clear: take*/

#define _FLAG_AND_ABORT(b)    (((b) & _ABORT_WHEN_MISMATCH_MASK) ? 1 : 0)

#endif /*#if !defined(_WLAN_NV_TEMPLATE_API_H) */

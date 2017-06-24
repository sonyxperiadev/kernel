/*
 * Copyright (c) 2012-2013 The Linux Foundation. All rights reserved.
 *
 * Previously licensed under the ISC license by Qualcomm Atheros, Inc.
 *
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all
 * copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * This file was originally distributed by Qualcomm Atheros, Inc.
 * under proprietary terms before Copyright ownership was assigned
 * to the Linux Foundation.
 */

/*===========================================================================

                       wlan_qct_wda_debug.c

  OVERVIEW:

  This software unit holds the implementation of the WLAN Device Adaptation
  Layer for debugging APIs.

  The functions externalized by this module are to be called ONLY by other
  WLAN modules that properly register with the Transport Layer initially.

  DEPENDENCIES:

  Are listed for each API below.


===========================================================================*/


#include "palTypes.h"
#include "wlan_qct_wda_debug.h"

void wdaLog(tpAniSirGlobal pMac, tANI_U32 loglevel, const char *pString,...) {
    va_list marker;
    
    if(loglevel > pMac->utils.gLogDbgLevel[WDA_DEBUG_LOGIDX])
        return;
   
    va_start( marker, pString );     /* Initialize variable arguments. */
    
    logDebug(pMac, SIR_WDA_MODULE_ID, loglevel, pString, marker);
    
    va_end( marker );              /* Reset variable arguments.      */
}


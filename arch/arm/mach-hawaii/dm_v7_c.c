/*
  Copyright (c) 2009-11, ARM Limited. All rights reserved.
  
  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

   * Redistributions of source code must retain the above copyright notice,
     this list of conditions and the following disclaimer.
   * Redistributions in binary form must reproduce the above copyright notice,
     this list of conditions and the following disclaimer in the documentation
     and/or other materials provided with the distribution.
   * Neither the name of ARM nor the names of its contributors may be used to
     endorse or promote products derived from this software without specific
     prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE.
*/
/****************************************************************************
* The contents of the original source code is minimally modified to suit
* the needs of the environment in which this code is being used.
*****************************************************************************/
 
/*
 * The code to save and restore debug context uses the memory-mapped interface.
 * The registers that are saved are enough to support a self-hosted debugger,
 * but a different approach should be taken with an external debugger (cp14).
 */

#include <mach/appf_types.h>
#include <mach/appf_internals.h>
#include <mach/appf_helpers.h>

#define DIDR_VERSION_SHIFT 16
#define DIDR_VERSION_MASK  0xF
#define DIDR_VERSION_7_1   5
#define DIDR_BP_SHIFT      24
#define DIDR_BP_MASK       0xF
#define DIDR_WP_SHIFT      28
#define DIDR_WP_MASK       0xF
#define CLAIMCLR_CLEAR_ALL 0xff

#define DRAR_VALID_MASK   0x00000003
#define DSAR_VALID_MASK   0x00000003
#define DRAR_ADDRESS_MASK 0xFFFFF000
#define DSAR_ADDRESS_MASK 0xFFFFF000
#define OSLSR_OSLM_MASK   0x00000009
#define OSLAR_UNLOCKED    0x00000000
#define OSLAR_LOCKED      0xC5ACCE55
#define LAR_UNLOCKED      0xC5ACCE55
#define LAR_LOCKED        0x00000000
#define OSDLR_UNLOCKED    0x00000000
#define OSDLR_LOCKED      0x00000001

typedef volatile struct
{                             /* Registers  Save?                                          */
    appf_u32 const didr;        /*         0  Read only                                    */
    appf_u32 dscr_i;            /*         1  ignore - use dscr_e instead                  */
    appf_u32 const dummy1[3];   /*       2-4  ignore                                       */
    appf_u32 dtrrx_dtrtx_i;     /*         5  ignore                                       */
    appf_u32 wfar;              /*         6  ignore - transient information               */
    appf_u32 vcr;               /*         7  Save                                         */
    appf_u32 const dummy2;      /*         8  ignore                                       */
    appf_u32 ecr;               /*         9  ignore                                       */
    appf_u32 dsccr;             /*        10  ignore                                       */
    appf_u32 dsmcr;             /*        11  ignore                                       */
    appf_u32 const dummy3[20];  /*     12-31  ignore                                       */
    appf_u32 dtrrx_e;           /*        32  ignore                                       */
    appf_u32 itr_pcsr;          /*        33  ignore                                       */
    appf_u32 dscr_e;            /*        34  Save                                         */
    appf_u32 dtrtx_e;           /*        35  ignore                                       */
    appf_u32 drcr;              /*        36  ignore                                       */
    appf_u32 eacr;              /*        37  Save - V7.1 only                             */
    appf_u32 const dummy4[2];   /*     38-39  ignore                                       */
    appf_u32 pcsr;              /*        40  ignore                                       */
    appf_u32 cidsr;             /*        41  ignore                                       */
    appf_u32 vidsr;             /*        42  ignore                                       */
    appf_u32 const dummy5[21];  /*     43-63  ignore                                       */
    appf_u32 bvr[16];           /*     64-79  Save                                         */
    appf_u32 bcr[16];           /*     80-95  Save                                         */
    appf_u32 wvr[16];           /*    96-111  Save                                         */
    appf_u32 wcr[16];           /*   112-127  Save                                         */
    appf_u32 const dummy6[16];  /*   128-143  ignore                                       */
    appf_u32 bxvr[16];          /*   144-159  Save if have Virtualization extensions       */
    appf_u32 const dummy7[32];  /*   160-191  ignore                                       */
    appf_u32 oslar;             /*       192  If oslsr[0] is 1, unlock before save/restore */
    appf_u32 const oslsr;       /*       193  ignore                                       */
    appf_u32 ossrr;             /*       194  ignore                                       */
    appf_u32 const dummy8;      /*       195  ignore                                       */
    appf_u32 prcr;              /*       196  ignore                                       */
    appf_u32 prsr;              /*       197  clear SPD on restore                         */
    appf_u32 const dummy9[762]; /*   198-959  ignore                                       */
    appf_u32 itctrl;            /*       960  ignore                                       */
    appf_u32 const dummy10[39]; /*   961-999  ignore                                       */
    appf_u32 claimset;          /*      1000  Restore claim bits to here                   */
    appf_u32 claimclr;          /*      1001  Save claim bits from here                    */
    appf_u32 const dummy11[2];  /* 1002-1003  ignore                                       */
    appf_u32 lar;               /*      1004  Unlock before restore                        */
    appf_u32 const lsr;         /*      1005  ignore                                       */
    appf_u32 const authstatus;  /*      1006  Read only                                    */
    appf_u32 const dummy12;     /*      1007  ignore                                       */
    appf_u32 const devid2;      /*      1008  Read only                                    */
    appf_u32 const devid1;      /*      1009  Read only                                    */
    appf_u32 const devid;       /*      1010  Read only                                    */
    appf_u32 const devtype;     /*      1011  Read only                                    */
    appf_u32 const pid[8];      /* 1012-1019  Read only                                    */
    appf_u32 const cid[4];      /* 1020-1023  Read only                                    */
} debug_registers_t;

typedef struct
{
    appf_u32 vcr;
    appf_u32 dscr_e;
    appf_u32 eacr;
    appf_u32 bvr[16];
    appf_u32 bcr[16];
    appf_u32 wvr[16];
    appf_u32 wcr[16];
    appf_u32 bxvr[16];
    appf_u32 claim;
} debug_context_t;    /* total size 86 * 4 = 344 bytes */

debug_registers_t *read_debug_address(void)
{
    unsigned drar, dsar;

    drar = read_drar();
    dsar = read_dsar();

    if (!(drar & DRAR_VALID_MASK)
     || !(dsar & DSAR_VALID_MASK))
    {
        return 0;  /* No memory-mapped debug on this processor */
    }

    return (debug_registers_t *)((drar & DRAR_ADDRESS_MASK)
                               + (dsar & DSAR_ADDRESS_MASK));
}

/*
 * We assume that before save (and after restore):
 *   - OSLAR is NOT locked, or the debugger would not work properly
 *   - LAR is locked, because the ARM ARM says it must be
 *   - OSDLR is NOT locked, or the debugger would not work properly
 */

void save_v7_debug(appf_u32 *context)
{
    debug_registers_t *dbg = (void*)read_debug_address();
    debug_context_t *ctx = (void*)context;
    unsigned v71, num_bps, num_wps, i;
    appf_u32 didr;

    if (!dbg)
    {
        return;
    }

    didr = dbg->didr;
    /*
     * Work out what version of debug we have
     */
    v71 = (((didr >> DIDR_VERSION_SHIFT) & DIDR_VERSION_MASK) == DIDR_VERSION_7_1);

    /*
     * Save all context to memory
     */
    ctx->vcr    = dbg->vcr;
    ctx->dscr_e = dbg->dscr_e;
    ctx->claim  = dbg->claimclr;

    if (v71)
    {
        ctx->eacr = dbg->eacr;
    }

    num_bps = 1 + ((didr >> DIDR_BP_SHIFT) & DIDR_BP_MASK);
    for (i=0; i<num_bps; ++i)
    {
        ctx->bvr[i]  = dbg->bvr[i];
        ctx->bcr[i]  = dbg->bcr[i];
#ifdef VIRTUALIZATION
        ctx->bxvr[i] = dbg->bxvr[i]; /* TODO: don't save the ones that don't exist */
#endif
    }

    num_wps = 1 + ((didr >> DIDR_WP_SHIFT) & DIDR_WP_MASK);
    for (i=0; i<num_wps; ++i)
    {
        ctx->wvr[i] = dbg->wvr[i];
        ctx->wcr[i] = dbg->wcr[i];
    }

    /*
     * If Debug V7.1, we must set osdlr (by cp14 interface) before power down.
     * Once we have done this, debug becomes inaccessible.
     */
    if (v71)
    {
        write_osdlr(OSDLR_LOCKED);
    }
}

void restore_v7_debug(appf_u32 *context)
{
    debug_registers_t *dbg = (void*)read_debug_address();
    debug_context_t *ctx = (void*)context;
    unsigned v71, num_bps, num_wps, i;
    appf_u32 didr;

    if (!dbg)
    {
        return;
    }

    didr = dbg->didr;
    /*
     * Work out what version of debug we have
     */
    v71 = (((didr >> DIDR_VERSION_SHIFT) & DIDR_VERSION_MASK) == DIDR_VERSION_7_1);

    /* Enable write access to registers */
    dbg->lar = LAR_UNLOCKED;
    /*
     * If Debug V7.1, we must unset osdlr (by cp14 interface) before restoring.
     * (If the CPU has not actually power-cycled, osdlr may not be reset).
     */
    if (v71)
    {
        write_osdlr(OSDLR_UNLOCKED);
    }

    /*
     * Restore all context from memory
     */
    dbg->vcr      = ctx->vcr;
    dbg->claimclr = CLAIMCLR_CLEAR_ALL;
    dbg->claimset = ctx->claim;

    if (v71)
    {
        dbg->eacr = ctx->eacr;
    }

    num_bps = 1 + ((didr >> DIDR_BP_SHIFT) & DIDR_BP_MASK);
    for (i=0; i<num_bps; ++i)
    {
        dbg->bvr[i]  = ctx->bvr[i];
        dbg->bcr[i]  = ctx->bcr[i];
#ifdef VIRTUALIZATION
        dbg->bxvr[i] = ctx->bxvr[i];  /* TODO: don't restore the ones that don't exist */
#endif
    }

    num_wps = 1 + ((didr >> DIDR_WP_SHIFT) & DIDR_WP_MASK);
    for (i=0; i<num_wps; ++i)
    {
        dbg->wvr[i] = ctx->wvr[i];
        dbg->wcr[i] = ctx->wcr[i];
    }

    /* Clear PRSR.SPD by reading PRSR */
    if (!v71)
    {
        (dbg->prsr);
    }

    /* Re-enable debug */
    dbg->dscr_e   = ctx->dscr_e;

    /* Disable write access to registers */
    dbg->lar = LAR_LOCKED;
}


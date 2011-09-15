/*****************************************************************************
* Copyright 2006 - 2010 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a
* license other than the GPL, without Broadcom's express prior written
* consent.
*****************************************************************************/
/**
*
*  @file    vcp_test.h
*
*  @brief   Test definitions
*
****************************************************************************/
#if !defined( VCPTEST_H )
#define VCPTEST_H

#include <linux/broadcom/vcp.h>

/* ---- Include Files ---------------------------------------------------- */
/* ---- Constants and Types ---------------------------------------------- */
typedef void * VCP_TEST_HDL;

#define VCPTEST_DEFAULT_ENCODER_PARAMETERS \
{ \
   VCP_CODEC_H264,                /* codec: default video encoder */ \
   VCP_PROFILE_H264_BASELINE,     /* profile */ \
   VCP_LEVEL_H264_3_1,            /* level */ \
   30,                            /* targetFrameRate: enc default frame rate (per second) */ \
   VCP_BIT_RATE_1_MBPS,           /* targetBitRate: enc default bit rate */ \
   0,                             /* rateControlMode */ \
   VCP_VGA_WIDTH,                 /* targetResolutionWidth: enc default width */ \
   VCP_VGA_HEIGHT,                /* targetResolutionHeight: enc default height */ \
   VCP_GOP_SIZE_0_5_SEC,          /* maxGOPsize */ \
   0,                             /* CustomMaxMBPS */ \
   0,                             /* MaxStaticMBPS */ \
   0,                             /* CustomMaxFS */ \
   0,                             /* CustomMaxDPB */ \
   0,                             /* CustomMaxBRandCPB */ \
   VCP_NAL_UNIT_SIZE_1200,        /* max_nal_unit_size */ \
   VCP_NAL_UNIT_SIZE_OFF,         /* max_rcmd_nal_unit_size */ \
   1,                             /* sampleAspectRatioX: enc default aspect ratio value X */ \
   1,                             /* sampleAspectRatioY: enc default aspect ratio value Y */ \
   0,                             /* Min quant */ \
   0,                             /* Max quant */ \
   0,                             /* enable/disable estimator */  \
   0,                             /* enable/disable estimator */  \
}

#define VCPTEST_DEFAULT_DECODER_PARAMETERS \
{ \
   VCP_CODEC_H264,                /* codec: default video decoder */ \
   0,                             /* maxFrameSize */ \
   0,                             /* maxFrameWidth */ \
   0,                             /* plcOption: default to no PLC */ \
   0,                             /* errConcealEnable: */ \
   0,                             /* sdpParmEnable: */ \
   "\0"                           /* spropParm */ \
}

#define MAXOPS 300 // Maximum number of command arguments

typedef struct
{
   char str[128];
}
OPSTR;

/* ---- Variable Externs ------------------------------------------------- */
/* ---- Function Prototypes ---------------------------------------------- */

int vcptest_init( VCP_TEST_HDL *hdlp );
int vcptest_exit( VCP_TEST_HDL hdl );
int vcptest_reinit( VCP_TEST_HDL hdl, int opargs );
int vcptest_start_enc( VCP_TEST_HDL hdl, int opargs );
int vcptest_set_encdisp( VCP_TEST_HDL hdl, int opargs );
int vcptest_stop_enc( VCP_TEST_HDL hdl );
int vcptest_start_dec( VCP_TEST_HDL hdl, int opargs );
int vcptest_set_decdisp( VCP_TEST_HDL hdl, int opargs );
int vcptest_set_decdisp_mask( VCP_TEST_HDL hdl, int opargs );
int vcptest_set_encdisp_mask( VCP_TEST_HDL hdl, int opargs );
int vcptest_stop_dec( VCP_TEST_HDL hdl );
int vcptest_pkt_loss_cfg( VCP_TEST_HDL hdl, int opargs );
int vcptest_encfreeze( VCP_TEST_HDL hdl, int opargs );
int vcptest_get_encstats( VCP_TEST_HDL hdl, int opargs );
int vcptest_get_decstats( VCP_TEST_HDL hdl, int opargs );
int vcptest_get_deccaps( VCP_TEST_HDL hdl );
int vcptest_set_encfir( VCP_TEST_HDL hdl );
int vcptest_set_encpriv( VCP_TEST_HDL hdl, int opargs );
int vcptest_set_encpanzoom( VCP_TEST_HDL hdl, int opargs );
int vcptest_cfg_decparms( VCP_TEST_HDL hdl, int opargs );
int vcptest_cfg_encparms( VCP_TEST_HDL hdl, int opargs );

#endif /* VCPTEST_H */


/*****************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
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

#ifndef _CHAL_MEMC_DDR3_H_
#define _CHAL_MEMC_DDR3_H_

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************/
/*include file declarations*/
#include <plat/chal/chal_types.h>
#include <plat/chal/chal_common.h>
#include <chal/bcmerr.h>
#include <mach/rdb/brcm_rdb_csr.h>


/*****************************************************************************/
/* macro declarations */
#define MEMC_EXPORT

/******************************************************************************/
/* typedef declarations */

#define DDR3_MODE_REG_0         0
#define DDR3_MODE_REG_1         1
#define DDR3_MODE_REG_2         2
#define DDR3_MODE_REG_3         3

/* DDR3- MR0 - Defintions from JEDEC STD 79-3D */
#define DDR3_MR0_BL_SHIFT                   0           /* Burst Length */
#define DDR3_MR0_BL_MASK                    0x0003
#define DDR3_MR0_CL_SHIFT                   2           /* CAS Latency */
#define DDR3_MR0_CL_MASK                    0x0074
#define DDR3_MR0_RBT_SHIFT                  3           /* Read Burst Type */
#define DDR3_MR0_RBT_MASK                   0x0008
#define DDR3_MR0_TM_SHIFT                   7           /* Test Mode */
#define DDR3_MR0_TM_MASK                    0x0080
#define DDR3_MR0_DLL_SHIFT                  8           /* DLL Reset */
#define DDR3_MR0_DLL_MASK                   0x0100
#define DDR3_MR0_WR_SHIFT                   9           /* Write Recovery */
#define DDR3_MR0_WR_MASK                    0x0e00
#define DDR3_MR0_PPD_SHIFT                  12          /* Precharge PD DLL */
#define DDR3_MR0_PPD_MASK                   0x1000

#define DDR3_MR0_BL_BL8_FIXED               (0 << DDR3_MR0_BL_SHIFT)
#define DDR3_MR0_BL_BC4_BL8_ONFLY           (1 << DDR3_MR0_BL_SHIFT)
#define DDR3_MR0_BL_BC4_FIXED               (2 << DDR3_MR0_BL_SHIFT)

#define DDR3_MR0_CL_5                       (0x04 << DDR3_MR0_CL_SHIFT)   /* A6=0, A5=0, A4=1, A3=X, A2=0 */
#define DDR3_MR0_CL_6                       (0x08 << DDR3_MR0_CL_SHIFT)   /* A6=0, A5=1, A4=0, A3=X, A2=0 */
#define DDR3_MR0_CL_7                       (0x0c << DDR3_MR0_CL_SHIFT)   /* A6=0, A5=1, A4=1, A3=X, A2=0 */
#define DDR3_MR0_CL_8                       (0x10 << DDR3_MR0_CL_SHIFT)   /* A6=1, A5=0, A4=0, A3=X, A2=0 */
#define DDR3_MR0_CL_9                       (0x14 << DDR3_MR0_CL_SHIFT)   /* A6=1, A5=0, A4=1, A3=X, A2=0 */
#define DDR3_MR0_CL_10                      (0x18 << DDR3_MR0_CL_SHIFT)   /* A6=1, A5=1, A4=0, A3=X, A2=0 */
#define DDR3_MR0_CL_11                      (0x1c << DDR3_MR0_CL_SHIFT)   /* A6=1, A5=1, A4=1, A3=X, A2=0 */
#define DDR3_MR0_CL_12                      (0x01 << DDR3_MR0_CL_SHIFT)   /* A6=0, A5=0, A4=0, A3=X, A2=1 */
#define DDR3_MR0_CL_13                      (0x05 << DDR3_MR0_CL_SHIFT)   /* A6=0, A5=0, A4=1, A3=X, A2=1 */
#define DDR3_MR0_CL_14                      (0x09 << DDR3_MR0_CL_SHIFT)   /* A6=0, A5=1, A4=0, A3=X, A2=1 */
#define DDR3_MR0_CL_15                      (0x0D << DDR3_MR0_CL_SHIFT)   /* A6=0, A5=1, A4=1, A3=X, A2=1 */
#define DDR3_MR0_CL_16                      (0x11 << DDR3_MR0_CL_SHIFT)   /* A6=1, A5=0, A4=0, A3=X, A2=1 */

#define DDR3_MR0_RBT_NIBBLE_SEQ             (0 << DDR3_MR0_RBT_SHIFT)
#define DDR3_MR0_RBT_INTERLEAVE             (1 << DDR3_MR0_RBT_SHIFT)

#define DDR3_MR0_TM_NORMAL                  (0 << DDR3_MR0_TM_SHIFT)
#define DDR3_MR0_TM_TEST                    (1 << DDR3_MR0_TM_SHIFT)

#define DDR3_MR0_DLL_NO_RESET               (0 << DDR3_MR0_DLL_SHIFT)
#define DDR3_MR0_DLL_RESET                  (1 << DDR3_MR0_DLL_SHIFT)

#define DDR3_MR0_WR_5                       (1 << DDR3_MR0_WR_SHIFT)
#define DDR3_MR0_WR_6                       (2 << DDR3_MR0_WR_SHIFT)
#define DDR3_MR0_WR_7                       (3 << DDR3_MR0_WR_SHIFT)
#define DDR3_MR0_WR_8                       (4 << DDR3_MR0_WR_SHIFT)
#define DDR3_MR0_WR_10                      (5 << DDR3_MR0_WR_SHIFT)
#define DDR3_MR0_WR_12                      (6 << DDR3_MR0_WR_SHIFT)
#define DDR3_MR0_WR_14                      (7 << DDR3_MR0_WR_SHIFT)
#define DDR3_MR0_WR_16                      (0 << DDR3_MR0_WR_SHIFT)

#define DDR3_MR0_PDD_SLOW_EXIT              (0 << DDR3_MR0_PPD_SHIFT)
#define DDR3_MR0_PDD_FAST_EXIT              (1 << DDR3_MR0_PPD_SHIFT)

/* DDR3- MR1 - Defintions from JEDEC STD 79-3D */

#define DDR3_MR1_RTT_NOM_0_SHIFT            2
#define DDR3_MR1_RTT_NOM_0_MASK             (1 << DDR3_MR1_RTT_NOM_0_SHIFT)
#define DDR3_MR1_RTT_NOM_1_SHIFT            6
#define DDR3_MR1_RTT_NOM_1_MASK             (1 << DDR3_MR1_RTT_NOM_1_SHIFT)
#define DDR3_MR1_RTT_NOM_2_SHIFT            9
#define DDR3_MR1_RTT_NOM_2_MASK             (1 << DDR3_MR1_RTT_NOM_2_SHIFT)
#define DDR3_MR1_RTT_NOM_MASK               (DDR3_MR1_RTT_NOM_0_MASK | DDR3_MR1_RTT_NOM_1_MASK | DDR3_MR1_RTT_NOM_2_MASK)
#define DDR3_MR1_RTT_NOM_VALUE_GET(r)       ( ((((r) & DDR3_MR1_RTT_NOM_0_MASK) >> DDR3_MR1_RTT_NOM_0_SHIFT) << 0) | \
                                              ((((r) & DDR3_MR1_RTT_NOM_1_MASK) >> DDR3_MR1_RTT_NOM_1_SHIFT) << 1) | \
                                              ((((r) & DDR3_MR1_RTT_NOM_2_MASK) >> DDR3_MR1_RTT_NOM_2_SHIFT) << 2) \
                                            )
#define DDR3_MR1_RTT_NOM_VALUE_SET(v)       ( ((((v) & 0x1) >> 0) << DDR3_MR1_RTT_NOM_0_SHIFT) | \
                                              ((((v) & 0x2) >> 1) << DDR3_MR1_RTT_NOM_1_SHIFT) | \
                                              ((((v) & 0x4) >> 2) << DDR3_MR1_RTT_NOM_2_SHIFT) \
                                            )
#define DDR3_MR1_RTT_NOM_DISABLED           DDR3_MR1_RTT_NOM_VALUE_SET(0)
#define DDR3_MR1_RTT_NOM_RZQ_DIV4           DDR3_MR1_RTT_NOM_VALUE_SET(1)
#define DDR3_MR1_RTT_NOM_RZQ_DIV2           DDR3_MR1_RTT_NOM_VALUE_SET(2)
#define DDR3_MR1_RTT_NOM_RZQ_DIV6           DDR3_MR1_RTT_NOM_VALUE_SET(3)
#define DDR3_MR1_RTT_NOM_RZQ_DIV12          DDR3_MR1_RTT_NOM_VALUE_SET(4)
#define DDR3_MR1_RTT_NOM_RZQ_DIV8           DDR3_MR1_RTT_NOM_VALUE_SET(5)

/* DDR3- MR2 - Defintions from JEDEC STD 79-3D */

#define DDR3_MR2_CWL_SHIFT                  3           /* CAS Write Latency */
#define DDR3_MR2_CWL_MASK                   0x0038

#define DDR3_MR2_CWL_5                       (0 << DDR3_MR2_CWL_SHIFT)
#define DDR3_MR2_CWL_6                       (1 << DDR3_MR2_CWL_SHIFT)
#define DDR3_MR2_CWL_7                       (2 << DDR3_MR2_CWL_SHIFT)
#define DDR3_MR2_CWL_8                       (3 << DDR3_MR2_CWL_SHIFT)
#define DDR3_MR2_CWL_9                       (4 << DDR3_MR2_CWL_SHIFT)
#define DDR3_MR2_CWL_10                      (5 << DDR3_MR2_CWL_SHIFT)
#define DDR3_MR2_CWL_11                      (6 << DDR3_MR2_CWL_SHIFT)
#define DDR3_MR2_CWL_12                      (7 << DDR3_MR2_CWL_SHIFT)

#define DDR3_MR2_RTT_WR_SHIFT               9
#define DDR3_MR2_RTT_WR_MASK                (0x3 << DDR3_MR2_RTT_WR_SHIFT)
#define DDR3_MR2_RTT_WR_OFF                 (0x0 << DDR3_MR2_RTT_WR_SHIFT)
#define DDR3_MR2_RTT_WR_RZQ_DIV4            (0x1 << DDR3_MR2_RTT_WR_SHIFT)
#define DDR3_MR2_RTT_WR_RZQ_DIV2            (0x2 << DDR3_MR2_RTT_WR_SHIFT)

typedef enum {
    CHAL_MEMC_DDR3_AD_WIDTH_13B     = 0,
    CHAL_MEMC_DDR3_AD_WIDTH_14B     = 1,
    CHAL_MEMC_DDR3_AD_WIDTH_15B     = 2,
    CHAL_MEMC_DDR3_AD_WIDTH_16B     = 3
} CHAL_MEMC_DDR3_AD_WIDTH;

typedef enum {
    CHAL_MEMC_DDR3_RANK_SINGLE      = 0,
    CHAL_MEMC_DDR3_RANK_DUAL        = 1
} CHAL_MEMC_DDR3_RANK;

typedef enum {
    CHAL_MEMC_DDR3_BUS_WIDTH_32     = 0,     /* BUS16 = 0, BUS8 = 0 */
    CHAL_MEMC_DDR3_BUS_WIDTH_8      = 1,     /* BUS16 = X, BUS8 = 1 */
    CHAL_MEMC_DDR3_BUS_WIDTH_16     = 2      /* BUS16 = 1, BUS8 = 0 */
} CHAL_MEMC_DDR3_BUS_WIDTH;

typedef enum {
    CHAL_MEMC_DDR3_CHIP_WIDTH_8     = 0,
    CHAL_MEMC_DDR3_CHIP_WIDTH_16    = 1
} CHAL_MEMC_DDR3_CHIP_WIDTH;

typedef enum {
    CHAL_MEMC_DDR3_VDDQ_1P35V       = 0,
    CHAL_MEMC_DDR3_VDDQ_1P50V       = 1,
    CHAL_MEMC_DDR3_VDDQ_1P80V       = 2
} CHAL_MEMC_DDR3_VDDQ;

typedef enum {
    CHAL_MEMC_DDR3_LDO_1P80V        = 0,
    CHAL_MEMC_DDR3_LDO_RESERVED     = 1,
    CHAL_MEMC_DDR3_LDO_1P50V        = 2,
    CHAL_MEMC_DDR3_LDO_1P35V        = 3
} CHAL_MEMC_DDR3_LDO;

typedef enum {
    CHAL_MEMC_DDR3_CHIP_SIZE_4Gb    = 0,
    CHAL_MEMC_DDR3_CHIP_SIZE_2Gb    = 1,
    CHAL_MEMC_DDR3_CHIP_SIZE_1Gb    = 2,
    CHAL_MEMC_DDR3_CHIP_SIZE_8Gb    = 3
} CHAL_MEMC_DDR3_CHIP_SIZE;

#define CHAL_MEMC_DDR3_CHIP_SIZE_MAX    CHAL_MEMC_DDR3_CHIP_SIZE_8Gb
#define CHAL_MEMC_DDR3_CHIP_SIZE_MIN    CHAL_MEMC_DDR3_CHIP_SIZE_4Gb
#define CHAL_MEMC_DDR3_CHIP_SIZE_CNT    (1 + CHAL_MEMC_DDR3_CHIP_SIZE_MAX - CHAL_MEMC_DDR3_CHIP_SIZE_MIN)

typedef enum {
    CHAL_MEMC_DDR3_JEDEC_TYPE_DDR2_533B   = 0,
    CHAL_MEMC_DDR3_JEDEC_TYPE_DDR2_533C   = 1,
    CHAL_MEMC_DDR3_JEDEC_TYPE_DDR2_667C   = 2,
    CHAL_MEMC_DDR3_JEDEC_TYPE_DDR2_667D   = 3,
    CHAL_MEMC_DDR3_JEDEC_TYPE_DDR2_800C   = 4,
    CHAL_MEMC_DDR3_JEDEC_TYPE_DDR2_800D   = 5,
    CHAL_MEMC_DDR3_JEDEC_TYPE_DDR2_800E   = 6,
    CHAL_MEMC_DDR3_JEDEC_TYPE_DDR2_1066E  = 7,
    CHAL_MEMC_DDR3_JEDEC_TYPE_DDR2_1066F  = 8,
    CHAL_MEMC_DDR3_JEDEC_TYPE_DDR3_800D   = 9,
    CHAL_MEMC_DDR3_JEDEC_TYPE_DDR3_800E   = 10,
    CHAL_MEMC_DDR3_JEDEC_TYPE_DDR3_1066E  = 11,
    CHAL_MEMC_DDR3_JEDEC_TYPE_DDR3_1066F  = 12,
    CHAL_MEMC_DDR3_JEDEC_TYPE_DDR3_1066G  = 13,
    CHAL_MEMC_DDR3_JEDEC_TYPE_DDR3_1333F  = 14,
    CHAL_MEMC_DDR3_JEDEC_TYPE_DDR3_1333G  = 15,
    CHAL_MEMC_DDR3_JEDEC_TYPE_DDR3_1333H  = 16,
    CHAL_MEMC_DDR3_JEDEC_TYPE_DDR3_1333J  = 17,
    CHAL_MEMC_DDR3_JEDEC_TYPE_DDR3_1600G  = 18,
    CHAL_MEMC_DDR3_JEDEC_TYPE_DDR3_1600H  = 19,
    CHAL_MEMC_DDR3_JEDEC_TYPE_DDR3_1600J  = 20,
    CHAL_MEMC_DDR3_JEDEC_TYPE_DDR3_1600K  = 21,
    CHAL_MEMC_DDR3_JEDEC_TYPE_DDR3_1866J  = 22,
    CHAL_MEMC_DDR3_JEDEC_TYPE_DDR3_1866K  = 23,
    CHAL_MEMC_DDR3_JEDEC_TYPE_DDR3_1866L  = 24,
    CHAL_MEMC_DDR3_JEDEC_TYPE_DDR3_1866M  = 25,
    CHAL_MEMC_DDR3_JEDEC_TYPE_DDR3_2133K  = 26,
    CHAL_MEMC_DDR3_JEDEC_TYPE_DDR3_2133L  = 27,
    CHAL_MEMC_DDR3_JEDEC_TYPE_DDR3_2133M  = 28,
    CHAL_MEMC_DDR3_JEDEC_TYPE_DDR3_2133N  = 29,
    CHAL_MEMC_DDR3_JEDEC_TYPE_MAX
} CHAL_MEMC_DDR3_JEDEC_TYPE;

#define CHAL_MEMC_DDR3_JEDEC_TYPE_DDR3_MIN 	CHAL_MEMC_DDR3_JEDEC_TYPE_DDR3_800D
#define CHAL_MEMC_DDR3_JEDEC_TYPE_IS_DDR3(t)    ((t) >= CHAL_MEMC_DDR3_JEDEC_TYPE_DDR3_MIN)

typedef enum
{
    CHAL_MEMC_DDR3_POWER_DOWN_MODE_ACTIVE = 0,
    CHAL_MEMC_DDR3_POWER_DOWN_MODE_PRECHARGE = CSR_DDR_SW_POWER_DOWN_CONTROL_POWER_DOWN_MODE_MASK
} CHAL_MEMC_DDR3_POWER_DOWN_MODE;

typedef enum {
    CHAL_MEMC_DDR3_STRAP_SOURCE_DDR_DQ     = 0,    /* Straps obtained from DDR_DQ pins */
    CHAL_MEMC_DDR3_STRAP_SOURCE_IPHY       = 1,    /* Straps obtained from iphy pins */
    CHAL_MEMC_DDR3_STRAP_SOURCE_OVERRIDE   = 2     /* Straps override the hardware straps */
} CHAL_MEMC_DDR3_STRAP_SOURCE;


typedef struct CHAL_MEMC_DDR3_CONFIG {
    _Bool                     strap_override;    /* TRUE - use straps in config structure;  FALSE - get hardware straps (not supported) */
    CHAL_MEMC_DDR3_JEDEC_TYPE   jedec_type;
    CHAL_MEMC_DDR3_CHIP_SIZE    chip_size;
    CHAL_MEMC_DDR3_VDDQ         vddq;
    CHAL_MEMC_DDR3_CHIP_WIDTH   chip_width;
    CHAL_MEMC_DDR3_BUS_WIDTH    bus_width;
    CHAL_MEMC_DDR3_RANK         rank;
    CHAL_MEMC_DDR3_AD_WIDTH     ad_width;
    uint32_t                    clock_hz;
    uint32_t                    odt_phy_enabled;
    uint32_t                    odt_sdram_mr1;
    uint32_t                    odt_sdram_mr2;
} CHAL_MEMC_DDR3_CONFIG_T;

typedef struct CHAL_MEMC_DDR3_DEV_INFO {
    char                         jedec_name[16];
    uint32_t                      device_speed_mhz;
    uint32_t                      ddr_speed_hz;
    uint32_t                      wr;                  /* Write recovery */
    uint32_t                      al;                  /* Additive latency */
    uint32_t                      cwl;                 /* CAS write latency */
    uint32_t                      cl;                  /* CAS latency */
    CHAL_MEMC_DDR3_STRAP_SOURCE  strap_source;        /* Strap source */
} CHAL_MEMC_DDR3_DEV_INFO_T;

typedef struct CHAL_MEMC_DDR3_DEVICE {
    CHAL_MEMC_DDR3_CONFIG_T     dev_config;    /* Input:  Configuration when strap_override == TRUE  */
    CHAL_MEMC_DDR3_DEV_INFO_T   dev_info;      /* Output: Additonal device info based on strap settings */
} CHAL_MEMC_DDR3_DEVICE_T;

/*****************************************************************************/
/* function declarations */


MEMC_EXPORT BCM_ERR_CODE chal_memc_ddr3_init (
    void* generic_hdl
    );

MEMC_EXPORT BCM_ERR_CODE chal_memc_ddr3_get_clock_speed (
    void* generic_hdl,
    uint32_t *clock_speed_hz
    );

MEMC_EXPORT BCM_ERR_CODE chal_memc_ddr3_get_dev_info (
    void* generic_hdl
    );

MEMC_EXPORT uint32_t chal_memc_ddr3_read_mode_register(
    void* generic_hdl,
    uint32_t cs,
    uint32_t reg_addr
    );

MEMC_EXPORT BCM_ERR_CODE ddr3_auto_power_down_disable (
    void* generic_hdl
    );

MEMC_EXPORT BCM_ERR_CODE ddr3_auto_power_down_enable (
    void* generic_hdl,
    CHAL_MEMC_DDR3_POWER_DOWN_MODE mode
    );

MEMC_EXPORT BCM_ERR_CODE chal_memc_ddr3_self_refresh_enter (
    void* generic_hdl
    );

MEMC_EXPORT BCM_ERR_CODE chal_memc_ddr3_self_refresh_exit (
    void* generic_hdl
    );

MEMC_EXPORT _Bool chal_memc_ddr3_write_mode_register(
    void* generic_hdl,
    uint32_t cs,
    uint32_t reg_addr,
    uint32_t value
    );


#ifdef __cplusplus
}
#endif

#endif /* _CHAL_MEMC_DDR3_H_ */


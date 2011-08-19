/*****************************************************************************
* Copyright 2008 Broadcom Corporation.  All rights reserved.
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

#ifndef _MEMC_H_
#define _MEMC_H_

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/

/* include file declarations */
#include <plat/chal/chal_types.h>
#include <plat/chal/chal_common.h>
#include <chal/chal_memc_ddr3.h>
#include <chal/bcmerr.h>


/**
*  NOTE: RECOMMENDED MEMC INITIALIZATION SEQUENCE (FROM MOBMM SDRAM_INIT.C).
* 
*        UNTIL THE CSL DRIVER IS DEFINED, THE CHAL MEMC CAN BE USED TO
*        INITIALIZE THE MEMORY AS FOLLOWS:
* 
*   BCM_ERR_CODE ret;
* 
*   CHAL_MEMC_HANDLE_T sys_memc_handle = {
*       .memc_open_reg_base     = MEMC0_OPEN_BASE_ADDR,
*       .memc_secure_reg_base   = MEMC0_SECURE_BASE_ADDR,
* 
*       .cs_max                 = 1,
* 
*       .MR1_device_feature1    = LPDDR2_BL4 | LPDDR2_SEQUENTIAL | LPDDR2_WRAP | LPDDR2_NWR_3,
*       .MR2_device_feature2    = LPDDR2_RL3_WL1,
*       .MR3_device_ioconfig    = LPDDR2_68_OHM,
* 
*       .timing0                = 0x2323323,
*       .timing1                = 0x332082,
*       .refresh_ctrl           = 0x30065,
*   };
* 
*   ret = chal_memc_initialize (&sys_memc_handle);
*****************************************************************************/


/****************************************************************************/
/* macro declarations*/
#define MEMC_EXPORT

/* MEMC0_OPEN_DRAM_INIT_CONTROL */
#define MEMCX_OPEN_DRAM_INIT_CONTROL_CS0    (2<<CSR_DRAM_INIT_CONTROL_CS_BITS_SHIFT)
#define MEMCX_OPEN_DRAM_INIT_CONTROL_CS1    (1<<CSR_DRAM_INIT_CONTROL_CS_BITS_SHIFT)

#define MEMCX_OPEN_DRAM_INIT_CONTROL_MRW    (0<<CSR_DRAM_INIT_CONTROL_INIT_CMD_SHIFT)
#define MEMCX_OPEN_DRAM_INIT_CONTROL_MRR    (1<<CSR_DRAM_INIT_CONTROL_INIT_CMD_SHIFT)
#define MEMCX_OPEN_DRAM_INIT_CONTROL_AREF   (2<<CSR_DRAM_INIT_CONTROL_INIT_CMD_SHIFT)
#define MEMCX_OPEN_DRAM_INIT_CONTROL_PALL   (3<<CSR_DRAM_INIT_CONTROL_INIT_CMD_SHIFT)

/* LPDDR2 mode registers; common */
#define LPDDR2_MODE_REG_DEVICE_INFO         0
#define LPDDR2_MODE_REG_DEVICE_FEATURE1     1
#define LPDDR2_MODE_REG_DEVICE_FEATURE2     2
#define LPDDR2_MODE_REG_IO_CONFIG1          3
#define LPDDR2_MODE_REG_SDRAM_REFR          4
#define LPDDR2_MODE_REG_BASIC_CONFIG1       5
#define LPDDR2_MODE_REG_BASIC_CONFIG2       6
#define LPDDR2_MODE_REG_BASIC_CONFIG3       7
#define LPDDR2_MODE_REG_BASIC_CONFIG4       8
#define LPDDR2_MODE_REG_TEST_MODE           9
#define LPDDR2_MODE_REG_IO_CALIBRATION      0xA

/* LPDDR2 mode registers; SDRAM only */
#define LPDDR2_MODE_REG_PASR_BANK           0x10
#define LPDDR2_MODE_REG_PASR_REG            0x11

/* LPDDR2 mode registers; NVM only */
#define LPDDR2_MODE_REG_NVM_GEOMETRY        0x14
#define LPDDR2_MODE_REG_NVM_TRCD            0x15
#define LPDDR2_MODE_REG_OVERLAY_WIN_EN      0x18
#define LPDDR2_MODE_REG_OW_BASE_ADDR1       0x19
#define LPDDR2_MODE_REG_OW_BASE_ADDR2       0x1A
#define LPDDR2_MODE_REG_OW_BASE_ADDR3       0x1B
#define LPDDR2_MODE_REG_DNV_LONG_DELAY      0x1D

#define LPDDR2_MODE_REG_DQ_CALIBRATION_PATTER_A 0x20
#define LPDDR2_MODE_REG_DQ_CALIBRATION_PATTER_B 0x28


#define LPDDR2_MODE_REG_RESET               0x3F

/* MR1 - */
#define LPDDR2_BL4                          2
#define LPDDR2_BL8                          3
#define LPDDR2_BL16                         4
#define LPDDR2_BL_MASK                      0x7

#define LPDDR2_SEQUENTIAL                   (0 << 3)
#define LPDDR2_INTERLEAVE                   (1 << 3)

#define LPDDR2_WRAP                         (0 << 4)
#define LPDDR2_NO_WRAP                      (1 << 4)

#define LPDDR2_NWR_3                        (1 << 5)
#define LPDDR2_NWR_4                        (2 << 5)
#define LPDDR2_NWR_5                        (3 << 5)
#define LPDDR2_NWR_6                        (4 << 5)
#define LPDDR2_NWR_7                        (5 << 5)
#define LPDDR2_NWR_8                        (6 << 5)



/* MR2 - DRAM Read & Write latency sepcification register */
#define LPDDR2_RL3_WL1                      1
#define LPDDR2_RL4_WL2                      2
#define LPDDR2_RL5_WL2                      3
#define LPDDR2_RL6_WL3                      4
#define LPDDR2_RL7_WL4                      5
#define LPDDR2_RL8_WL4                      6

/* MR3 - IO Config */
#define LPDDR2_30_OHM                       0
#define LPDDR2_34_OHM                       1
#define LPDDR2_40_OHM                       2
#define LPDDR2_48_OHM                       3
#define LPDDR2_60_OHM                       4
#define LPDDR2_68_OHM                       5
#define LPDDR2_80_OHM                       6
#define LPDDR2_120_OHM                      7

/* MR24 - Overlay Window */
#define NVM_OVERLAY_WINDOW                  24
#define NVM_OVERLAY_WIN_ENABLE              1
#define NVM_OVERLAY_WIN_DISABLE             2

/* MR25 - 27 - Overlay Window address */
#define NVM_OVERLAY_WINDOW_ADDR1            25
#define NVM_OVERLAY_WINDOW_ADDR2            26
#define NVM_OVERLAY_WINDOW_ADDR3            27

/*MEMC0_OPEN_DRAM_CONFIG - "DRAM Basic Configuration for Memory Controller module.
 Bits [7:0] are loaded by reading DRAM Basic Configuration Register, MR8, MA = 0x08"*/
#define DRAM_CONFIG_WIDTH_32                (0 << SEC_DRAM_CONFIG_DRAM_DEVICE_WIDTH_SHIFT)
#define DRAM_CONFIG_WIDTH_16                (1 << SEC_DRAM_CONFIG_DRAM_DEVICE_WIDTH_SHIFT)

#define DRAM_CONFIG_64_MB                   (0 << SEC_DRAM_CONFIG_DRAM_DENSITY_SHIFT)
#define DRAM_CONFIG_128_MB                  (1 << SEC_DRAM_CONFIG_DRAM_DENSITY_SHIFT)
#define DRAM_CONFIG_256_MB                  (2 << SEC_DRAM_CONFIG_DRAM_DENSITY_SHIFT)
#define DRAM_CONFIG_512_MB                  (3 << SEC_DRAM_CONFIG_DRAM_DENSITY_SHIFT)
#define DRAM_CONFIG_1_GB                    (4 << SEC_DRAM_CONFIG_DRAM_DENSITY_SHIFT)
#define DRAM_CONFIG_2_GB                    (5 << SEC_DRAM_CONFIG_DRAM_DENSITY_SHIFT)
#define DRAM_CONFIG_4_GB                    (6 << SEC_DRAM_CONFIG_DRAM_DENSITY_SHIFT)
#define DRAM_CONFIG_8_GB                    (7 << SEC_DRAM_CONFIG_DRAM_DENSITY_SHIFT)
#define DRAM_CONFIG_16_GB                   (8 << SEC_DRAM_CONFIG_DRAM_DENSITY_SHIFT)
#define DRAM_CONFIG_32_GB                   (9 << SEC_DRAM_CONFIG_DRAM_DENSITY_SHIFT)

#define DRAM_CONFIG_TYPE_S4                 (0 << SEC_DRAM_CONFIG_MEM_TYPE_SHIFT)
#define DRAM_CONFIG_TYPE_S2                 (1 << SEC_DRAM_CONFIG_MEM_TYPE_SHIFT)
#define DRAM_CONFIG_TYPE_NVM                (2 << SEC_DRAM_CONFIG_MEM_TYPE_SHIFT)


/* NVM registers definition */
#define NVM_STATUS_REG_OFFSET               0xCC
#define NVM_STATUS_DRB_SHIFT                7                 /* device ready */
#define NVM_STATUS_DRB_MASK                 0x80
#define NVM_STATUS_ESSB_SHIFT               6                 /* erase suspend */
#define NVM_STATUS_ESSB_MASK                0x40
#define NVM_STATUS_ESB_SHIFT                5                 /* erase fail */
#define NVM_STATUS_ESB_MASK                 0x20
#define NVM_STATUS_PSB_SHIFT                4                 /* program fail */
#define NVM_STATUS_PSB_MASK                 0x10
#define NVM_STATUS_VCC_SHIFT                3                 /* VCC */
#define NVM_STATUS_VCC_MASK                 0x8
#define NVM_STATUS_PSSB_SHIFT               2                 /* program suspend */
#define NVM_STATUS_PSSB_MASK                0x4
#define NVM_STATUS_BLSB_SHIFT               1                 /* lock status */
#define NVM_STATUS_BLSB_MASK                0x2

#define NVM_COMMAND_CODE_REG_OFFSET         0x80
#define NVM_COMMAND_DATA_REG_OFFSET         0x84
#define NVM_COMMAND_ADDR_REG_OFFSET         0x88
#define NVM_MULTI_PURPOSE_REG_OFFSET        0x90
#define NVM_COMMAND_EXE_REG_OFFSET          0xC0
#define NVM_SUSPEND_REG_OFFSET              0xC8
#define NVM_ABORT_REG_OFFSET                0xCA
#define NVM_PROG_BUFFER_REG_OFFSET          0x800


#define NVM_NUM_PARTITIONS                  0x10              /* 16 partitions */
#define NVM_BLOCKS_PARTITIONS               0x20              /* 32 blocks */
#define NVM_NUM_BLOCKS                      (NVM_NUM_PARTITIONS*NVM_BLOCKS_PARTITIONS)
#define NVM_BLOCK_SIZE                      0x20000           /* 1Mbit = 128 KB */

#define    SEC_RGN_2_CTL_RGN_2_GID_SANDBOX_EN_LOCK_SHIFT                  24
#define    SEC_RGN_2_CTL_RGN_2_GID_SANDBOX_EN_LOCK_MASK                   0x01000000
/* MEMC0_OPEN_AXI_PORT_CTRL - AXI port control register*/

#define NUM_SECURE_REGION                   8
#define NUM_QOS_CAM_ENTRY                   16

/* delays specified in LPDDR2 spec.  Use units of microsec.*/
#define tINIT0                              20000     /* 20 ms */
#define tINIT1                              1         /* 100 ns */
#define tINIT2                              1         /* 5 tCK */
#define tINIT3                              200       /* 200 us */
#define tINIT4                              1         /* 1 us */
#define tINIT5                              10        /* 10 us */
#define tZQINIT                             1         /* 1 us */
#define tCK                                 1         /* 50 ns */


#define MEMC_HW_FREQ_CHANGE_DEFAULT         0x3E9
#define MEMC_IO_PAD_DRV_SLEW_RATE           0x223  /* default value */

#define MEMC_CS_MAX                         2

/******************************************************************************/
/* typedef declarations */

/* a memory device can have a DRAM or combo of DRAM & NVM */
typedef enum
{
    MEMC_DEV_SINGLE_DRAM_CS0     = 1,     /* single DRAM on CS0 */
    MEMC_DEV_SINGLE_NVM_CS0      = 2,     /* single NVM on CS0 */
    MEMC_DEV_COMBO_DRAM_CS1      = 3,     /* DRAM & NVM; DRAM on CS1 */
} MEMC_CS_DEVICE;

typedef enum {
    MEMC_ID_SYS = 0,
    MEMC_ID_VC = 1,
    MEMC_ID_MAX
} MEMC_ID;

/* one memc can support up to 2 CS;
 * one CS connects to DRAM and the other CS to NVM
 * CS0 can be connected to DRAM or NVM; same for CS1
 */
typedef enum
{
    MEMC_CS_0           = 0,
    MEMC_CS_1           = 1,
    MEMC_CS_NONE        = 2,
    MEMC_CS_BOTH        = 3,
} MEMC_CS_CONNECTION;

typedef enum
{
    MEMC_TYPE_DRAM      = 0,
    MEMC_TYPE_NVM       = 1,
    MEMC_TYPE_NONE      = 2,
} MEMC_MEMORY_TYPE;

typedef enum {
    MEMC_DDR_TYPE_LPDDR2 = 0,
    MEMC_DDR_TYPE_DDR3 = 1,
    MEMC_DDR_TYPE_UNKNOWN = -1,
} MEMC_DDR_TYPE;

typedef enum {
    LPDDR2_ID_SAMSUNG   = 1,
    LPDDR2_ID_QIMONDA   = 2,
    LPDDR2_ID_ELPIDA    = 3,
    LPDDR2_ID_ETRON     = 4,
    LPDDR2_ID_NANYA     = 5,
    LPDDR2_ID_HYNIX     = 6,
    LPDDR2_ID_MOSEL     = 7,
    LPDDR2_ID_WINBOND   = 8,
    LPDDR2_ID_ESMT      = 9,
    LPDDR2_ID_SPANSION  = 0xB,
    LPDDR2_ID_SST       = 0xC,
    LPDDR2_ID_ZMOS      = 0xD,
    LPDDR2_ID_INTEL     = 0xE,
    LPDDR2_ID_NUMONYX   = 0xFE,
    LPDDR2_ID_MICRON    = 0xFF
} LPDDR2_VENDOR_ID;

typedef enum {
    LPDDR2_TYPE_S4      = 0,
    LPDDR2_TYPE_S2      = 1,
    LPDDR2_TYPE_NVM     = 2
} LPDDR2_TYPE;

typedef enum {
    LPDDR2_DENSITY_64M  = 0,
    LPDDR2_DENSITY_128M = 1,
    LPDDR2_DENSITY_256M = 2,
    LPDDR2_DENSITY_512M = 3,
    LPDDR2_DENSITY_1G   = 4,
    LPDDR2_DENSITY_2G   = 5,
    LPDDR2_DENSITY_4G   = 6,
    LPDDR2_DENSITY_8G   = 7,
    LPDDR2_DENSITY_16G  = 8,
    LPDDR2_DENSITY_32G  = 9
} LPDDR2_DENSITY;

typedef enum {
    LPDDR2_WIDTH_X32    = 0,
    LPDDR2_WIDTH_X16    = 1,
    LPDDR2_WIDTH_X8     = 2
} LPDDR2_IO_WIDTH;

typedef enum {
    QOS_LATENCY_URGENCY     = 0,
    QOS_BANDWIDTH_URGENCY   = 1
} QOS_URGENCY_TYPE;

typedef enum {
    MDIV_2              = 0,     /* MDIV = divide by 2 */
    MDIV_4              = 1      /* MDIV = divide by 4 */
} MEMC_MDIV_T;

/* Target Freqency - Hera only */
typedef enum {
    FSYS_DIV_2          = 0,     /* SYS PLL/2 */
    FSYS_DIV_4          = 1,     /* SYS PLL/4 */
    FSYS_DIV_6          = 2,     /* SYS PLL/6 */
    FDDR_DIV_1          = 4,     /* DDR PLL */
    FDDR_DIV_2          = 5,     /* DDR PLL/2 */
    FDDR_DIV_3          = 6,     /* DDR PLL/3 */
    FDDR_DIV_4          = 7,     /* DDR PLL/4 */
    FXTAL               = 8,     /* crystal clock */
    FXTOTAL
} MEMC_CLOCK_T;

/* Freq State Mapping BigIsland/Rhea only */
typedef enum {
    SYS_PLL_FREQ_DIV_2      = 0,     /* SYS PLL/2 */
    SYS_PLL_FREQ_DIV_4      = 1,     /* SYS PLL/4 */
    SYS_PLL_FREQ_DIV_6      = 2      /* SYS PLL/6 */
} MEMC_FREQ_MAPPING_SYS_PLL_T;

/* Freq State Mapping BigIsland/Rhea only */
typedef enum {
    DDR_PLL_FREQ_DIV_2      = 0,     /* DDR PLL/2 */
    DDR_PLL_FREQ_DIV_4      = 1,     /* DDR PLL/4 */
    DDR_PLL_FREQ_DIV_6      = 2,     /* DDR PLL/6 */
    DDR_PLL_FREQ_DIV_8      = 3      /* DDR PLL/8 */
} MEMC_FREQ_MAPPING_DDR_PLL_T;

/* Max Power State Mapping BigIsland/Rhea only */
typedef enum {
    MEMC_MAX_POWER_STATE_INVALID      = 0,     /* Illegal, don't use */
    MEMC_MAX_POWER_STATE_XTAL         = 1,     /* Controller can enter mode to stop SYS PLL, DDR PLL and run off XTAL */
    MEMC_MAX_POWER_STATE_SYS_PLL      = 2,     /* Controller can enter mode to stop DDR PLL and run off SYS PLL */
    MEMC_MAX_POWER_STATE_DDR_PLL      = 3      /* Controller can enter mode to run off DDR PLL */
} MEMC_MAX_POWER_STATE_T;

typedef enum {
    MEMC_MIN_POWER_STATE_SELF_REFRESH = 0,     /* Controller can enter mode to stop SYS PLL, DDR PLL, stop XTAL, enter self-refresh */
    MEMC_MIN_POWER_STATE_XTAL         = 1,     /* Controller can enter mode to stop SYS PLL, DDR PLL and run off XTAL */
    MEMC_MIN_POWER_STATE_SYS_PLL      = 2,     /* Controller can enter mode to stop DDR PLL and run off SYS PLL */
    MEMC_MIN_POWER_STATE_DDR_PLL      = 3      /* Controller can enter mode to run off DDR PLL */
} MEMC_MIN_POWER_STATE_T;

typedef enum {
    MEMC_IO_PAD_DRV_80_OHM      = 0,
    MEMC_IO_PAD_DRV_60_OHM      = 1,
    MEMC_IO_PAD_DRV_48_OHM      = 2,
    MEMC_IO_PAD_DRV_40_OHM      = 3,
    MEMC_IO_PAD_DRV_34_OHM      = 4,
} MEMC_IO_PAD_DRV_T;

typedef enum {
    VIO_READ  = 0,
    VIO_WRITE = 1,
} CHAL_MEU_ACC_VIO_TYPE_T;

typedef enum {
    REGION_0  = 0,
    REGION_1  = 1,
    REGION_2  = 2,
    REGION_3  = 3,
    REGION_4  = 4,
    REGION_5  = 5,
    REGION_6  = 6,
    REGION_7  = 7,
    REGION_MAX = 8
} CHAL_MEU_REGIONS_T;

typedef enum
{
    RGN_TZ,
    RGN_SECURE,
    RGN_SUPERVISOR,
    RGN_USER
}CHAL_MEU_RGN_TYPE_t;

typedef enum {
    GROUP_ID_MATCHED  = 1,
    REGION_MATCHED    = 2,
    SECURITY_VIO      = 3,
    RESERVED          = 4,
} CHAL_MEU_ACC_VIO_REASON_T;

typedef enum {
    DATA_KEY_WK_IN  = 0,
    DATA_KEY_WK_OUT = 1,
    ADDR_KEY_WK_IN  = 2,
    ADDR_KEY_WK_OUT = 3,
    RND_0_DATA_KEY  = 4,
    RND_1_DATA_KEY  = 5,
    RND_2_DATA_KEY  = 6,
    RND_0_ADDR_KEY  = 7,
    RND_1_ADDR_KEY  = 8,
    RND_2_ADDR_KEY  = 9
} CHAL_MEU_KEY_T;

typedef enum
{
    MEMC_REG_BLOCK_SEC  = 0,
    MEMC_REG_BLOCK_CSR  = 1,
    MEMC_REG_BLOCK_APHY = 2,
    MEMC_REG_BLOCK_DPHY = 3
} MEMC_REG_BLOCK;

typedef enum
{
    MEMC_OP_MODE_FPGA = 0,
    MEMC_OP_MODE_ASIC = 1,
} CHAL_MEMC_OP_MODE;

typedef enum
{
    MEMC_BOOT_MODE_COLD = 0,
    MEMC_BOOT_MODE_WARM = 1,
} CHAL_MEMC_BOOT_MODE;

typedef enum
{
    MEMC_ACTIVE_POWER_DOWN = 0,
    MEMC_PRECHARGE_POWER_DOWN = 1,
    MEMC_SELF_REFRESH = 2
} CHAL_MEMC_POWER_DOWN_MODE;

typedef enum
{
    MEMC_ACTIVE_STATE = 0,
    MEMC_ACTIVE_PWR_DOWN_STATE = 1,
    MEMC_PRECHARGEPWR_DOWN_STATE = 2,
    MEMC_SELF_REFRESH_STATE = 3
} CHAL_MEMC_POWER_DOWN_STATUS;

typedef enum
{
    NVM_BLOCK_UNLOCK = 0,
    NVM_BLOCK_LOCK = 1,
    NVM_BLOCK_LOCK_DOWN = 2,
} NVM_BLOCK_LOCK_MODE;

typedef enum
{
    NVM_STATUS_DRB    = 0x80,       /* device ready */
    NVM_STATUS_ESSB   = 0x40,       /* erase suspend */
    NVM_STATUS_ESB    = 0x20,       /* erase status */
    NVM_STATUS_PSB    = 0x10,       /* program status */
    NVM_STATUS_VSESB  = 0x8,        /* voltage supply error */
    NVM_STATUS_PSSB   = 0x4,        /* program suspend */
    NVM_STATUS_BLSB   = 0x2         /* block lock status */
} NVM_OPERATION_STATUS;

typedef enum
{
    NVM_COMMAND_BLOCK_ERASE         = 0x20,
    NVM_COMMAND_PROG_WORD           = 0x41,
    NVM_COMMAND_OVERWRITE_WORD      = 0x42,
    NVM_COMMAND_BUFFERED_PROG       = 0xE9,
    NVM_COMMAND_BUFFERED_OVERWRITE  = 0xEA,
    NVM_COMMAND_RESUME              = 0xD0,
    NVM_COMMAND_BLOCK_LOCK          = 0x61,
    NVM_COMMAND_BLOCK_UNLOCK        = 0x62,
    NVM_COMMAND_BLOCK_LOCK_DOWN     = 0x63,
    NVM_COMMAND_READ_LOCK_STATUS    = 0x65
} NVM_COMMAND_CODE;

typedef enum
{
    /* block erase error */
    NVM_ERROR_BLOCK_ERASE_FAIL            = 0x20,
    NVM_ERROR_BLOCK_ERASE_LOCK            = 0x22,
    NVM_ERROR_BLOCK_ERASE_VCC             = 0x28,

    /* single word program error */
    NVM_ERROR_PROGRAM_FAIL                = 0x10,
    NVM_ERROR_PROGRAM_LOCK                = 0x12,
    NVM_ERROR_PROGRAM_VCC                 = 0x18,

    NVM_ERROR_OPERATION_BAD_ADDR          = 0x30

} NVM_OPERATION_ERROR;

typedef struct CHAL_MEU_ACCESS_VIO_INFO{
    _Bool interrupted;
    _Bool vio_type;
    _Bool region_num[NUM_SECURE_REGION];
    uint32_t client_id;
    uint32_t port_id;
    uint32_t acc_vio_reason;
    uint32_t acc_vio_addr;

} CHAL_MEU_ACCESS_VIO_INFO_T;


typedef struct CHAL_LPDDR2_DEV_INFO {
    LPDDR2_VENDOR_ID    vendor_id;
    char                vendor_name[16];
    LPDDR2_TYPE         type;
    LPDDR2_DENSITY      density;
    LPDDR2_IO_WIDTH     io_width;
} CHAL_LPDDR2_DEV_INFO_T;


typedef struct CHAL_LPDDR2_CONFIG {
    uint8_t              MR1_device_feature1;
    uint8_t              MR2_device_feature2;
    uint8_t              MR3_device_ioconfig;

    uint8_t              MR4_SDRAM_refresh_rate;

    uint8_t              MR5_basic_config1;
    uint8_t              MR6_basic_config2;
    uint8_t              MR7_basic_config3;
    uint8_t              MR8_basic_config4;

    MEMC_IO_PAD_DRV_T    aphy_io_pad_drv;
    uint8_t              aphy_slew;
    _Bool                aphy_zq_calibration;
    MEMC_IO_PAD_DRV_T    dphy_io_pad_drv;
    uint8_t              dphy_slew;
    _Bool                dphy_zq_calibration;
    
    _Bool                device_zq_calibration;

    _Bool                auto_vref_management;
} CHAL_LPDDR2_CONFIG_T;

typedef struct CHAL_LPDDR2_DEVICE {
    CHAL_LPDDR2_DEV_INFO_T dev_info;
    CHAL_LPDDR2_CONFIG_T   dev_config;

    MEMC_CS_CONNECTION     dev_cs;
    MEMC_MEMORY_TYPE       dev_type;
}CHAL_LPDDR2_DEVICE_T;

typedef struct CHAL_MEMC_QOS {
    QOS_URGENCY_TYPE    type;
    uint8_t              urgent_counter;
    uint8_t              threshold;
    uint16_t             tag;
} CHAL_MEMC_QOS_T;

typedef struct CHAL_MEMC_SECURE_RNG_T {
    uint32_t             start_addr;
    uint32_t             end_addr;
    uint32_t             start_addr_lock;
    uint32_t             end_addr_lock;

    uint8_t              MRS_disable;
    uint8_t              MRS_disable_lock;
    uint8_t              bypass_scr;
    uint8_t              bypass_scr_lock;

    uint8_t              data_scramle;
    uint8_t              addr_scramle;

    uint8_t              mem_type;
    uint8_t              mem_type_lock;

    uint8_t              enable;
} CHAL_MEMC_SECURE_RNG_T;


typedef struct CHAL_MEMC_STATS {
    /* all in cycless */
    uint32_t             read_data;
    uint32_t             write_data;
    uint32_t             page_hit_data;
    uint32_t             page_miss_data;
    uint32_t             page_hit_transaction;
    uint32_t             page_miss_transaction;
    uint32_t             auto_ref;
    uint32_t             bank_active;
} CHAL_MEMC_STATS_T;

typedef struct CHAL_MEMC_MASTER_DLL_STATUS {
    uint32_t             CSR_CURRENT_FREQUENCY_STATE;
    uint32_t             APHY_CSR_ADDR_MASTER_DLL_OUTPUT;
    uint32_t             APHY_CSR_GLOBAL_ADDR_DLL_LOCK_STATUS;
    uint32_t             DPHY_CSR_BYTE0_MASTER_DLL_OUTPUT;
    uint32_t             DPHY_CSR_BYTE1_MASTER_DLL_OUTPUT;
    uint32_t             DPHY_CSR_GLOBAL_MASTER_DLL_LOCK_STATUS;
} CHAL_MEMC_MASTER_DLL_STATUS_T;

typedef struct CHAL_MEMC_NVM_WINDOW_INFO_T
{
    uint16_t             query_string_0;
    uint16_t             query_string_1;
    uint16_t             query_string_2;
    uint16_t             query_string_3;

    uint16_t             window_id;
    uint16_t             window_revision;
    uint16_t             window_size;

    uint16_t             prog_buffer_offset;
    uint16_t             prog_buffer_size;

    uint16_t             manufacturer_id;
    uint16_t             device_id;

} CHAL_MEMC_NVM_WINDOW_INFO;

typedef struct CHAL_MEMC_T
{
    MEMC_DDR_TYPE       memc_ddr_type;
    
    /* Common MEMC Base Addresses */
    uint32_t             memc_secure_reg_base;
    uint32_t             memc_open_reg_base;

    /* LPDDR2 PHY Base Addresses */
    uint32_t             memc_aphy_reg_base;
    uint32_t             memc_dphy_reg_base;

    /* DDR3 PHY (DDR40) Base Addresses */
    uint32_t             memc_open_ddr3_ctl_base;
    uint32_t             memc_open_ddr40_phy_addr_ctl_base;
    uint32_t             memc_open_ddr40_phy_wl_0_base;
    uint32_t             memc_open_ddr40_phy_wl_1_base;

    uint32_t             memc_memory_base_dram;
    uint32_t             memc_memory_base_nvm;

    /* NVM */
    uint32_t             overlay_win_base_addr;
    uint32_t             overlay_win_enable;

    CHAL_MEMC_OP_MODE   operation_mode;   /* either FPGA or ASIC */
    CHAL_MEMC_BOOT_MODE boot_mode;        /* COLD or WARM */
    uint64_t             total_bytes;    /* including both lpddr2 */

    CHAL_LPDDR2_DEVICE_T        mem_device[MEMC_CS_MAX];
    
    CHAL_MEMC_DDR3_DEVICE_T     mem_device_ddr3;

    CHAL_MEMC_NVM_WINDOW_INFO   nvm_window_info;
    uint32_t                     nvm_operation_error;

    /* power-down */
    uint32_t             auto_power_down_cycles;

    /* timing */
    uint32_t             clock_Mhz;
    uint32_t             timing0;
    uint32_t             timing1;
    uint32_t             refresh_ctrl;

    /* Secure */
    CHAL_MEMC_SECURE_RNG_T      secure_rgn[NUM_SECURE_REGION];
    uint8_t                      glb_addr_scr_en;
    uint8_t                      glb_data_scr_en;

    /* Qos */
    CHAL_MEMC_QOS_T             qos_cam_entry[NUM_QOS_CAM_ENTRY];

    uint32_t                     expireReadCnt;
    uint32_t                     expireWriteCnt;

} CHAL_MEMC_HANDLE_T, *CHAL_MEMC_HANDLE;

/****************************************************************************/
/* function declarations */

MEMC_EXPORT BCM_ERR_CODE chal_memc_init (
    CHAL_MEMC_HANDLE handle
    );

MEMC_EXPORT BCM_ERR_CODE chal_memc_soft_reset (
    CHAL_MEMC_HANDLE handle
    );

MEMC_EXPORT BCM_ERR_CODE chal_lpddr2_get_dev_info (
    CHAL_MEMC_HANDLE handle,
    MEMC_CS_CONNECTION cs,
    CHAL_LPDDR2_DEV_INFO_T *pDevInfo
    );

MEMC_EXPORT BCM_ERR_CODE chal_lpddr2_get_mem_size (
    CHAL_MEMC_HANDLE handle,
    MEMC_CS_CONNECTION cs,
    uint32_t *memSize
    );

MEMC_EXPORT BCM_ERR_CODE chal_lpddr2_get_base_addr (
    CHAL_MEMC_HANDLE handle,
    MEMC_REG_BLOCK base
    );

MEMC_EXPORT BCM_ERR_CODE chal_memc_set_clock (
    CHAL_MEMC_HANDLE handle,
    MEMC_CLOCK_T clock
    );

MEMC_EXPORT BCM_ERR_CODE chal_memc_get_clock (
    CHAL_MEMC_HANDLE handle,
    MEMC_CLOCK_T *clock
    );

MEMC_EXPORT BCM_ERR_CODE chal_memc_get_clock_speed (
    CHAL_MEMC_HANDLE handle,
    uint32_t *clock_speed_hz
    );

MEMC_EXPORT BCM_ERR_CODE chal_memc_set_auto_power_down_cycles (
    CHAL_MEMC_HANDLE handle,
    uint32_t cycles
    );

MEMC_EXPORT BCM_ERR_CODE chal_memc_get_auto_power_down_cycles(
    CHAL_MEMC_HANDLE handle,
    uint32_t *cycles
    );

MEMC_EXPORT BCM_ERR_CODE chal_memc_enable_auto_pwr(
    CHAL_MEMC_HANDLE handle,
    _Bool enable
    );

MEMC_EXPORT BCM_ERR_CODE chal_memc_enter_sw_power_down (
    CHAL_MEMC_HANDLE handle,
    CHAL_MEMC_POWER_DOWN_MODE mode
    );

MEMC_EXPORT BCM_ERR_CODE chal_memc_exit_selfrefresh (
    CHAL_MEMC_HANDLE handle
    );

MEMC_EXPORT BCM_ERR_CODE chal_memc_sw_power_fully_on (
    CHAL_MEMC_HANDLE handle
    );

MEMC_EXPORT BCM_ERR_CODE chal_memc_get_sw_power_down_status (
    CHAL_MEMC_HANDLE handle,
    uint32_t *status
    );

MEMC_EXPORT BCM_ERR_CODE chal_memc_config_burst_length(
    CHAL_MEMC_HANDLE handle,
    uint8_t blen
    );

MEMC_EXPORT BCM_ERR_CODE chal_memc_config_sequence(
    CHAL_MEMC_HANDLE handle,
    uint8_t seq
    );

MEMC_EXPORT BCM_ERR_CODE chal_memc_config_wrap_nowrap(
    CHAL_MEMC_HANDLE handle,
    uint8_t wrap
   );

MEMC_EXPORT BCM_ERR_CODE chal_memc_config_stats (
    CHAL_MEMC_HANDLE handle,
    uint32_t cycles
    );

MEMC_EXPORT BCM_ERR_CODE chal_memc_get_stats (
    CHAL_MEMC_HANDLE handle,
    CHAL_MEMC_STATS_T *pStats
    );

MEMC_EXPORT BCM_ERR_CODE chal_memc_meu_config_region(
    CHAL_MEMC_HANDLE handle,
    uint8_t  rgn_num,
    uint32_t rgn_type,
    uint32_t start_addr,
    uint32_t end_addr
    );

MEMC_EXPORT BCM_ERR_CODE chal_memc_meu_enable_disable_region(
    CHAL_MEMC_HANDLE handle,
    uint8_t  rgn_num,
    _Bool enable
    );

MEMC_EXPORT BCM_ERR_CODE chal_memc_meu_enable_region_data_scramble(
    CHAL_MEMC_HANDLE handle,
    uint8_t  rgn_num
);

MEMC_EXPORT void chal_memc_meu_enable_global_data_scrambling(
    CHAL_MEMC_HANDLE handle
    );

MEMC_EXPORT void chal_memc_meu_enable_global_addr_scrambling(
    CHAL_MEMC_HANDLE handle
    );

MEMC_EXPORT void chal_memc_meu_lock_global_gid_check(
    CHAL_MEMC_HANDLE handle
    );

MEMC_EXPORT void chal_memc_meu_enable_global_gid_check(
    CHAL_MEMC_HANDLE handle
    );

MEMC_EXPORT void chal_memc_meu_disable_global_gid_check(
    CHAL_MEMC_HANDLE handle
    );

MEMC_EXPORT BCM_ERR_CODE chal_memc_meu_config_keys(
    CHAL_MEMC_HANDLE handle,
    CHAL_MEU_KEY_T key,
    uint32_t key_value
    );

MEMC_EXPORT void chal_memc_meu_program_scrambling_scheme(
    CHAL_MEMC_HANDLE handle,
    uint32_t scheme_value
    );

MEMC_EXPORT void chal_memc_meu_program_scrambling_mask(
    CHAL_MEMC_HANDLE handle,
    uint32_t mask_value
    );

MEMC_EXPORT BCM_ERR_CODE chal_memc_meu_get_access_vio_info(
    CHAL_MEMC_HANDLE handle,
    CHAL_MEU_ACCESS_VIO_INFO_T *info
    );

MEMC_EXPORT _Bool chal_memc_meu_is_gid_check_bit_enabled(
    CHAL_MEMC_HANDLE handle
    );

MEMC_EXPORT _Bool chal_memc_meu_is_gid_check_lock_bit_enabled(
    CHAL_MEMC_HANDLE handle
    );

MEMC_EXPORT uint32_t chal_memc_read_register(
    CHAL_MEMC_HANDLE handle,
    MEMC_REG_BLOCK reg_base,
    uint32_t reg_addr
    );

MEMC_EXPORT _Bool chal_memc_write_register(
    CHAL_MEMC_HANDLE handle,
    MEMC_REG_BLOCK reg_base,
    uint32_t reg_offset,
    uint32_t reg_value
    );

MEMC_EXPORT _Bool chal_memc_write_register_2(
    CHAL_MEMC_HANDLE handle,
    uint32_t reg_addr,
    uint32_t reg_value
    );

MEMC_EXPORT uint32_t chal_memc_read_mode_register(
    CHAL_MEMC_HANDLE handle,
    MEMC_CS_CONNECTION cs,
    uint32_t reg_addr
    );

MEMC_EXPORT _Bool chal_memc_write_mode_register(
    CHAL_MEMC_HANDLE handle,
    MEMC_CS_CONNECTION cs,
    uint32_t reg_addr,
    uint32_t value
    );

MEMC_EXPORT uint32_t chal_memc_get_ddr_pll_status(
    CHAL_MEMC_HANDLE handle
    );

MEMC_EXPORT _Bool chal_memc_get_master_dll_output(
    CHAL_MEMC_HANDLE handle,
    CHAL_MEMC_MASTER_DLL_STATUS_T* status
    );

MEMC_EXPORT _Bool chal_memc_enable_slave_dll_lock(
    CHAL_MEMC_HANDLE handle,
    uint32_t lock
    );

MEMC_EXPORT _Bool chal_memc_set_zq_calibration(
    CHAL_MEMC_HANDLE handle,
    uint32_t enable
    );

MEMC_EXPORT BCM_ERR_CODE chal_memc_reset_ddr2(
    CHAL_MEMC_HANDLE handle
    );

MEMC_EXPORT BCM_ERR_CODE chal_memc_pvt_calibration(
    CHAL_MEMC_HANDLE handle
    );

MEMC_EXPORT BCM_ERR_CODE chal_memc_temperature_polling(
    CHAL_MEMC_HANDLE handle,
    _Bool enable
    );


/***************************************************************
 * NVM
 ***************************************************************/

MEMC_EXPORT BCM_ERR_CODE chal_memc_nvm_disable_overlay_window(
    CHAL_MEMC_HANDLE handle
    );

MEMC_EXPORT BCM_ERR_CODE chal_memc_nvm_enable_overlay_window(
    CHAL_MEMC_HANDLE handle,
    uint32_t base_addr
    );

MEMC_EXPORT _Bool chal_memc_nvm_is_overlay_window_enable(
    CHAL_MEMC_HANDLE handle,
    _Bool read_from_device
    );

MEMC_EXPORT BCM_ERR_CODE chal_memc_nvm_read_overlay_window_status(
    CHAL_MEMC_HANDLE handle,
    uint32_t* status
    );

MEMC_EXPORT BCM_ERR_CODE chal_memc_nvm_clear_overlay_window_status(
    CHAL_MEMC_HANDLE handle,
    uint32_t status_bit
    );

MEMC_EXPORT BCM_ERR_CODE chal_memc_nvm_is_busy(
    CHAL_MEMC_HANDLE handle,
    _Bool *busy
    );

MEMC_EXPORT BCM_ERR_CODE chal_memc_nvm_block_unlock_all(
    CHAL_MEMC_HANDLE handle
    );

MEMC_EXPORT BCM_ERR_CODE chal_memc_nvm_block_lock(
    CHAL_MEMC_HANDLE handle,
    uint32_t start_addr,
    uint32_t end_addr,
    NVM_BLOCK_LOCK_MODE lock
    );

MEMC_EXPORT BCM_ERR_CODE chal_memc_nvm_block_lock_down(
    CHAL_MEMC_HANDLE handle,
    uint32_t start_addr,
    uint32_t end_addr
    );

MEMC_EXPORT BCM_ERR_CODE chal_memc_nvm_block_erase(
    CHAL_MEMC_HANDLE handle,
    uint32_t block_addr,
    uint32_t waitComplete
    );

MEMC_EXPORT BCM_ERR_CODE chal_memc_nvm_write_word(
    CHAL_MEMC_HANDLE handle,
    uint32_t word_addr,
    uint16_t word_value,
    uint32_t overwrite,
    uint32_t waitComplete
    );

MEMC_EXPORT BCM_ERR_CODE chal_memc_nvm_write_buffer(
    CHAL_MEMC_HANDLE handle,
    uint32_t address,
    uint8_t* data,
    uint32_t size,
    uint32_t overwrite,
    uint32_t waitComplete
    );

MEMC_EXPORT BCM_ERR_CODE chal_memc_nvm_suspend(
    CHAL_MEMC_HANDLE handle
    );

MEMC_EXPORT BCM_ERR_CODE chal_memc_nvm_resume(
    CHAL_MEMC_HANDLE handle
    );

MEMC_EXPORT BCM_ERR_CODE chal_memc_nvm_read_block_lock_status(
    CHAL_MEMC_HANDLE handle,
    uint32_t address,
    NVM_BLOCK_LOCK_MODE* lock
    );

MEMC_EXPORT BCM_ERR_CODE chal_memc_nvm_abort(
    CHAL_MEMC_HANDLE handle
    );

#ifdef __cplusplus
}
#endif

#endif /* _MEMC_H_ */


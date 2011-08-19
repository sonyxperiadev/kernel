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

/* This file is intended for BigIsland MEMC*/

/*
 * ===========================================================================
 *  include header file declarations
 */
#include <plat/chal/chal_common.h>
#include <mach/chipregHw_inline.h>
#include <chal/chal_memc.h>
#include <chal/chal_memc_ddr3.h>
#include <chal/bcmregmem.h>
#include <linux/string.h>

//#include <mach/csp/mm_io.h>
#include <mach/rdb/brcm_rdb_sysmap.h>
#include <mach/rdb/brcm_rdb_map.h>
#include <mach/rdb/brcm_rdb_csr.h>
#include <mach/rdb/brcm_rdb_sec.h>
#include <mach/rdb/brcm_rdb_aphy_csr.h>
#include <mach/rdb/brcm_rdb_dphy_csr.h>

#ifdef ARRAY_SIZE
#undef ARRAY_SIZE
#endif
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr)[0])

#define READ_RETRY_MAX    1000

#define DEBUG_REG_WRITE   0

#if DEBUG_REG_WRITE
#undef  BCM_REG_WRITE32
#define BCM_REG_WRITE32(hdl, addr, val) {\
    BCM_DBG_OUT(("Addr 0x%08x, Val 0x%08x\n", addr, val)); \
    (*((volatile unsigned int *)(addr)) = (val)); \
}
#endif
#define BCM_DBG_ERR(...)

/*
 * ===========================================================================
 *  global variable declarations
 * 
 */

/*
 * ===========================================================================
 *  extern variable declarations
 * 
 */

/*
 * ===========================================================================
 *  static function prototype declarations
 * 
 */
static BCM_ERR_CODE lpddr2_mode_reg_read (CHAL_MEMC_HANDLE handle, MEMC_CS_CONNECTION cs, uint32_t addr, uint32_t *pVal);
static BCM_ERR_CODE lpddr2_mode_reg_write (CHAL_MEMC_HANDLE handle, MEMC_CS_CONNECTION cs, uint32_t addr, uint32_t val);
static BCM_ERR_CODE lpddr2_read_config (CHAL_MEMC_HANDLE handle);

static void chal_memc_set_cs0_memory_type(CHAL_MEMC_HANDLE handle);
static void chal_memc_set_lpddr_address(CHAL_MEMC_HANDLE handle);
static void chal_memc_init_burst_length(CHAL_MEMC_HANDLE handle);

static BCM_ERR_CODE lpddr2_wait_memc_register(CHAL_MEMC_HANDLE handle, uint32_t reg, uint32_t mask);
static BCM_ERR_CODE chal_memc_ddr2_reset(CHAL_MEMC_HANDLE handle, MEMC_CS_CONNECTION cs);
static void lpddr2_clock_configuration(CHAL_MEMC_HANDLE handle);

static void memc_misc_ctrl_config(CHAL_MEMC_HANDLE handle, int dqs_oeb, int rdqs_gate);
static void wait_frequency_state_stable(CHAL_MEMC_HANDLE handle);

static BCM_ERR_CODE lpddr2_get_vendor_name_from_id (uint32_t id, char name[]);

static BCM_ERR_CODE chal_memc_nvm_fill_overlay_window_info(CHAL_MEMC_HANDLE handle);
static void chal_memc_nvm_fill_prog_buffer(uint32_t addr_reg, uint8_t* data, uint32_t size);
static _Bool chal_memc_nvm_poll_ready(CHAL_MEMC_HANDLE handle);
static _Bool chal_memc_nvm_write_allow(CHAL_MEMC_HANDLE handle);

/*
 * ===========================================================================
 *  local macro declarations
 * 
 */

#if 0
#define MEMC_DBG_OUT(a) {BCM_DBG_OUT(a);}
#else
#define MEMC_DBG_OUT(a) {}
#endif

#define CHAL_DELAY_MICROS(cnt) { udelay(cnt); }
#define CHAL_DELAY_NANOS(cnt) { udelay(cnt); }   /* There doesn't exist nanosec granularity, OK to use microsec */
#define CHAL_DELAY_LPDDR2(cnt) { udelay(cnt); }

#define READ_AND_PRINT_LPDDR2_MODE_REG(handle, cs, addr) { \
    uint32_t rdvalue=0x00; \
    if ( lpddr2_mode_reg_read(handle, cs, addr, &rdvalue) != (BCM_SUCCESS) ) return (BCM_ERROR); \
    else MEMC_DBG_OUT (("LPDDR2 reg[0x%x]: %s, cs=%d, val=0x%x\n", addr, #addr, cs, rdvalue)); \
}

#define DECLARE(VENDOR) {LPDDR2_ID_##VENDOR, #VENDOR}

/*
 * ===========================================================================
 *  static variables declarations
 * 
 */
static const struct {
    LPDDR2_VENDOR_ID id;
    const char       *name;
} lpddr_vendor_info [] =
{
    DECLARE(SAMSUNG),
    DECLARE(QIMONDA),
    DECLARE(ELPIDA),
    DECLARE(SAMSUNG),
    DECLARE(ETRON),
    DECLARE(NANYA),
    DECLARE(HYNIX),
    DECLARE(MOSEL),
    DECLARE(WINBOND),
    DECLARE(ESMT),
    DECLARE(SPANSION),
    DECLARE(SST),
    DECLARE(ZMOS),
    DECLARE(INTEL),
    DECLARE(NUMONYX),
    DECLARE(MICRON),
};



/*
 * ===========================================================================
 * 
 *   Function Name: chal_memc_soft_reset
 * 
 *   Description:
 *       performs a software reset to the memory controller
 * 
 * ===========================================================================
 */
MEMC_EXPORT BCM_ERR_CODE chal_memc_soft_reset (
    CHAL_MEMC_HANDLE handle
    )
{
    /* can only be done with a cold boot*/
    if (handle->boot_mode == MEMC_BOOT_MODE_COLD)
    {
        /* 1. software reset = 1; hold CKEN low*/
        BCM_REG_WRITE32 (handle, handle->memc_open_reg_base + CSR_SOFT_RESET_OFFSET, 1);

        /* 2. wait for MEMC clock becomes stable*/
        CHAL_DELAY_NANOS (100);   /* 100ns*/

        /* 3. software unreset = 0; pull CKEN high*/
        BCM_REG_WRITE32 (handle, handle->memc_open_reg_base + CSR_SOFT_RESET_OFFSET, 0);

        /* 4. wait for MEMC clock becomes stable*/
        CHAL_DELAY_NANOS (300);  /* 250-300ns*/
    }
    return BCM_SUCCESS;
}

/*
 * ===========================================================================
 * 
 *   Function Name: chal_memc_ddr2_reset
 * 
 *   Description:
 *       performs a reset the DDR2 device
 * 
 * ===========================================================================
 */
static BCM_ERR_CODE chal_memc_ddr2_reset(
    CHAL_MEMC_HANDLE handle,
    MEMC_CS_CONNECTION cs
    )
{
    uint32_t rdvalue = 0x00;
    uint32_t MRW_data = 0;
    uint32_t MR_addr = 0;
    int32_t  result = BCM_SUCCESS;

    if (cs == MEMC_CS_NONE)
        return BCM_SUCCESS;

    /* cold boot; initialize the DDR2 devices*/
    if (handle->boot_mode == MEMC_BOOT_MODE_COLD)
    {
        /* 1. wait; iINIT3*/
        CHAL_DELAY_LPDDR2 (tINIT3);

        /* 2. write a reset*/
        MR_addr = LPDDR2_MODE_REG_RESET;
        MRW_data = 0;
        if (BCM_SUCCESS != lpddr2_mode_reg_write (handle, cs, MR_addr, MRW_data))
        {
            return BCM_ERROR;
        }

        do
        {
            /* 3. wait; iINIT4*/
            CHAL_DELAY_LPDDR2 (tINIT4);

            /* 4. poll until DRAM MR0 DAI bit complete (= 0)*/
            rdvalue = 0;
            MR_addr = LPDDR2_MODE_REG_DEVICE_INFO;
            if (BCM_SUCCESS != lpddr2_mode_reg_read (handle, cs, MR_addr, &rdvalue))
            {
                return BCM_ERROR;
            }
        } while ((rdvalue & 1) != 0);   /* wait until DAI bit = 0*/

        if ( handle->operation_mode == MEMC_OP_MODE_FPGA )
        {
            /* Reset controller following DRAM reset.  Workaround on BigIsland FPGA that
             * allows subsequent lpddr2_mode_reg_read to return correct values upon cold boot */
             chal_memc_soft_reset(handle);
        }

        /* 5. put DRAM bus in high impedance*/
        memc_misc_ctrl_config(handle, 0, 0);

        /* 6. write IO calibration*/
        MR_addr = LPDDR2_MODE_REG_IO_CALIBRATION;
        MRW_data = 0xFF;
        if (BCM_SUCCESS != lpddr2_mode_reg_write (handle, cs, MR_addr, MRW_data))
        {
            result = BCM_ERROR;
        }

        CHAL_DELAY_LPDDR2 (tZQINIT); /* wait tZQINIT*/

        /* 7. put DRAM bus in read access state*/
        memc_misc_ctrl_config(handle, 1, 0);
    }

    return result;
}

/*
 * ===========================================================================
 * 
 *   Function Name: chal_memc_pvt_calibration
 * 
 *   Description:
 *       performs a pvt calibration to the DDR2 device
 * 
 * ===========================================================================
 */
MEMC_EXPORT BCM_ERR_CODE chal_memc_pvt_calibration(CHAL_MEMC_HANDLE handle)
{
    uint32_t rdvalue = 0x00;
    int32_t  result = BCM_SUCCESS;

    /* 1. put DRAM bus in high impedance*/
    memc_misc_ctrl_config(handle, 0, 0);

    /* 2. set the DRAM IO pad impedance; enable PVT compensation on APHY*/
    rdvalue = BCM_REG_READ32 (handle, handle->memc_aphy_reg_base + APHY_CSR_ADDR_PVT_COMP_CNTRL_OFFSET);
    rdvalue |= APHY_CSR_ADDR_PVT_COMP_CNTRL_PVT_COMP_EN_MASK;
    BCM_REG_WRITE32 (handle, handle->memc_aphy_reg_base + APHY_CSR_ADDR_PVT_COMP_CNTRL_OFFSET, rdvalue);

    /* 3. wait until the PVT enable complete*/
    if (BCM_SUCCESS != lpddr2_wait_memc_register(handle, handle->memc_aphy_reg_base + APHY_CSR_ADDR_PVT_COMP_STATUS_OFFSET,
            APHY_CSR_ADDR_PVT_COMP_STATUS_PVT_COMP_DONE_MASK))
    {
        result = BCM_ERROR;
    }

    /* 4. disable PVT compensation on APHY */
    rdvalue &= ~APHY_CSR_ADDR_PVT_COMP_CNTRL_PVT_COMP_EN_MASK;
    BCM_REG_WRITE32 (handle, handle->memc_aphy_reg_base + APHY_CSR_ADDR_PVT_COMP_CNTRL_OFFSET, rdvalue);

    /* 5di. set the DRAM IO pad impedance; enable PVT compensation on DPHY */
    rdvalue = BCM_REG_READ32 (handle, handle->memc_dphy_reg_base + DPHY_CSR_DQ_PVT_COMP_CNTRL_OFFSET);
    rdvalue |= DPHY_CSR_DQ_PVT_COMP_CNTRL_PVT_COMP_EN_MASK;
    BCM_REG_WRITE32 (handle, handle->memc_dphy_reg_base + DPHY_CSR_DQ_PVT_COMP_CNTRL_OFFSET, rdvalue);

    /* 5dii. wait until the PVT enable complete*/
    if (BCM_SUCCESS != lpddr2_wait_memc_register(handle, handle->memc_dphy_reg_base + DPHY_CSR_DQ_PVT_COMP_STATUS_OFFSET,
            DPHY_CSR_DQ_PVT_COMP_STATUS_PVT_COMP_DONE_MASK))
    {
        result = BCM_ERROR;
    }

    /* 5diii. disable PVT compensation on DPHY */
    rdvalue &= ~DPHY_CSR_DQ_PVT_COMP_CNTRL_PVT_COMP_EN_MASK;
    BCM_REG_WRITE32 (handle, handle->memc_dphy_reg_base + DPHY_CSR_DQ_PVT_COMP_CNTRL_OFFSET, rdvalue);



    /* 5. put DRAM bus in read access state*/
    memc_misc_ctrl_config(handle, 1, 0);

    return result;
}

/*
 * ===========================================================================
 *
 *  Function Name: chal_memc_set_cs0_memory_type
 *
 *  Description:
 *      set the memory type connected to CS0
 *
 * ===========================================================================
 */
static void chal_memc_set_cs0_memory_type(CHAL_MEMC_HANDLE handle)
{
    uint32_t dram_config = 0;

    if (handle->mem_device[0].dev_type == MEMC_TYPE_NVM)
    {
        /* if NVM populated, it must be CS0 */
        BCM_REG_WRITE32 (handle, handle->memc_open_reg_base + CSR_CS0_MEMORY_TYPE_OFFSET, 1);
    }
    else
    {
        /* DRAM must be CS0 */
        BCM_REG_WRITE32 (handle, handle->memc_open_reg_base + CSR_CS0_MEMORY_TYPE_OFFSET, 0);
    }

    /* DDR interface width should be equal to the IO data width
     * for Hera/Rhea = 1, 16-bit; for Island = 0, 32-bit
     */ 
    if (handle->mem_device[0].dev_type == MEMC_TYPE_DRAM)
    {
        dram_config = (handle->mem_device[0].dev_info.io_width << SEC_DRAM_CONFIG_DDR_IF_WIDTH_SHIFT) |
                (handle->mem_device[0].dev_info.io_width << SEC_DRAM_CONFIG_DRAM_DEVICE_WIDTH_SHIFT) |
                (handle->mem_device[0].dev_info.density << SEC_DRAM_CONFIG_DRAM_DENSITY_SHIFT) |
                (handle->mem_device[0].dev_info.type << SEC_DRAM_CONFIG_MEM_TYPE_SHIFT);
    }
    else if (handle->mem_device[1].dev_type == MEMC_TYPE_DRAM)
    {
        dram_config = (handle->mem_device[1].dev_info.io_width << SEC_DRAM_CONFIG_DDR_IF_WIDTH_SHIFT) |
                (handle->mem_device[1].dev_info.io_width << SEC_DRAM_CONFIG_DRAM_DEVICE_WIDTH_SHIFT) |
                (handle->mem_device[1].dev_info.density << SEC_DRAM_CONFIG_DRAM_DENSITY_SHIFT) |
                (handle->mem_device[1].dev_info.type << SEC_DRAM_CONFIG_MEM_TYPE_SHIFT);
    }

    if (dram_config > 0)
    {
        BCM_REG_WRITE32 (handle, handle->memc_secure_reg_base + SEC_DRAM_CONFIG_OFFSET, dram_config);
    }
}

/*
 * ===========================================================================
 *
 *  Function Name: chal_memc_set_lpddr_address
 *
 *  Description:
 *      set the DRAM & NVM start and end address
 *
 * ===========================================================================
 */
static void chal_memc_set_lpddr_address(CHAL_MEMC_HANDLE handle)
{
    uint32_t mem_size = 0x00;

    /* NVM can only be on CS0 */
    if (handle->mem_device[0].dev_type == MEMC_TYPE_NVM)
    {
        mem_size = (8 * 1024 * 1024) << handle->mem_device[0].dev_info.density;

        /* set the NVM start and end address */
        BCM_REG_WRITE32 (handle, handle->memc_secure_reg_base + SEC_NVM_START_ADDR_OFFSET, 
            handle->memc_memory_base_nvm);
        BCM_REG_WRITE32 (handle, handle->memc_secure_reg_base + SEC_NVM_END_ADDR_OFFSET, 
                         handle->memc_memory_base_nvm+mem_size-1);
    }

    /* DRAM can be on CS0 or CS1 or both depends on the device */
    mem_size = 0x00;
    if (handle->mem_device[0].dev_type == MEMC_TYPE_DRAM)
    {
        mem_size += (8 * 1024 * 1024) << handle->mem_device[0].dev_info.density;
    }
    if (handle->mem_device[1].dev_type == MEMC_TYPE_DRAM)
    {
        mem_size += (8 * 1024 * 1024) << handle->mem_device[1].dev_info.density;
    }

    if (mem_size)
    {
        /* set the DRAM start and end address */
        BCM_REG_WRITE32 (handle, handle->memc_secure_reg_base + SEC_DRAM_START_ADDR_OFFSET, handle->memc_memory_base_dram);
        BCM_REG_WRITE32 (handle, handle->memc_secure_reg_base + SEC_DRAM_END_ADDR_OFFSET, handle->memc_memory_base_dram+mem_size-1);
        handle->total_bytes = mem_size;
    }
}

/*
 * ===========================================================================
 * 
 *   Function Name: chal_memc_init
 * 
 *   Description:
 *       initialize MEMC controller
 * 
 * ===========================================================================
 */
MEMC_EXPORT BCM_ERR_CODE chal_memc_init (
    CHAL_MEMC_HANDLE handle
)
{
    uint32_t wrvalue = 0x00;
    uint32_t rdvalue = 0x00;
    uint32_t MR_addr = 0;

    if (!handle) {
        BCM_DBG_ERR (("invalid argument: handle==0\n"));
        return BCM_ERROR;
    }

    if ( handle->memc_ddr_type == MEMC_DDR_TYPE_DDR3 )
    {
       return chal_memc_ddr3_init( handle );
    }

    handle->overlay_win_enable = 0;
    handle->overlay_win_base_addr = handle->memc_memory_base_nvm;

    handle->expireReadCnt = 0;
    handle->expireWriteCnt = 0;

    /* 1, 2: reset the DDR2;*/
    chal_memc_soft_reset(handle);

    if (handle->operation_mode == MEMC_OP_MODE_ASIC)
    {

        /* enable hardware freq change control*/
        if ( handle->mem_device[0].dev_config.auto_vref_management )
        {
           /* Clear bit DISABLE_DLL_CALIB_ON_CLK_CHANGE */
           BCM_REG_WRITE32 (handle, handle->memc_open_reg_base + CSR_HW_FREQ_CHANGE_CNTRL_OFFSET,
                             MEMC_HW_FREQ_CHANGE_DEFAULT & ~(CSR_HW_FREQ_CHANGE_CNTRL_DISABLE_DLL_CALIB_ON_CLK_CHANGE_MASK));
        }
        else
        {
           /* Set bit DISABLE_DLL_CALIB_ON_CLK_CHANGE */
           BCM_REG_WRITE32 (handle, handle->memc_open_reg_base + CSR_HW_FREQ_CHANGE_CNTRL_OFFSET,
                             MEMC_HW_FREQ_CHANGE_DEFAULT | CSR_HW_FREQ_CHANGE_CNTRL_DISABLE_DLL_CALIB_ON_CLK_CHANGE_MASK);
        }    

        /* io pad drv and slew rate control*/
        BCM_REG_WRITE32 (handle, handle->memc_aphy_reg_base + APHY_CSR_ADDR_PAD_DRV_SLEW_CNTRL_OFFSET, 
                           (handle->mem_device[0].dev_config.aphy_io_pad_drv<<APHY_CSR_ADDR_PAD_DRV_SLEW_CNTRL_CLK_DRV_SHIFT |
                            handle->mem_device[0].dev_config.aphy_io_pad_drv<<APHY_CSR_ADDR_PAD_DRV_SLEW_CNTRL_CA_DRV_SHIFT |
                            handle->mem_device[0].dev_config.aphy_slew<<APHY_CSR_ADDR_PAD_DRV_SLEW_CNTRL_SLEW_SHIFT) );

        BCM_REG_WRITE32 (handle, handle->memc_dphy_reg_base + DPHY_CSR_DQ_PAD_DRV_SLEW_CNTRL_OFFSET,
                           (handle->mem_device[0].dev_config.dphy_io_pad_drv<<DPHY_CSR_DQ_PAD_DRV_SLEW_CNTRL_DQS_DRV_SHIFT |
                            handle->mem_device[0].dev_config.dphy_io_pad_drv<<DPHY_CSR_DQ_PAD_DRV_SLEW_CNTRL_DQ_DM_DRV_SHIFT |
                            handle->mem_device[0].dev_config.dphy_slew<<DPHY_CSR_DQ_PAD_DRV_SLEW_CNTRL_SLEW_SHIFT) );

        /* 3, 4, 5: PVT calibration*/
        chal_memc_pvt_calibration(handle);

        /* 6a. APHY global DLL reset*/
        BCM_REG_WRITE32 (handle, handle->memc_aphy_reg_base + APHY_CSR_GLOBAL_ADDR_DLL_RESET_OFFSET, 1);

        /* 6b. DPHY global DLL reset*/
        BCM_REG_WRITE32 (handle, handle->memc_dphy_reg_base + DPHY_CSR_GLOBAL_DQ_DLL_RESET_OFFSET, 1);

        /* 6c. wait for the APHY global DLL to lock*/
        if (BCM_SUCCESS != lpddr2_wait_memc_register(handle, handle->memc_aphy_reg_base + APHY_CSR_GLOBAL_ADDR_DLL_LOCK_STATUS_OFFSET,
                APHY_CSR_GLOBAL_ADDR_DLL_LOCK_STATUS_ADDR_MASTER_DLL_LOCK_P_MASK |
                APHY_CSR_GLOBAL_ADDR_DLL_LOCK_STATUS_ADDR_MASTER_DLL_LOCK_N_MASK))
        {
            return BCM_ERROR;
        }

        /* 6d. wait for the DPHY global DLL to lock*/
        if (BCM_SUCCESS != lpddr2_wait_memc_register(handle, handle->memc_dphy_reg_base + DPHY_CSR_GLOBAL_MASTER_DLL_LOCK_STATUS_OFFSET, 0xFF))
        {
            return BCM_ERROR;
        }
    }

    /* 7. reset the DDR2 devices*/
    if (BCM_SUCCESS != chal_memc_ddr2_reset(handle, handle->mem_device[0].dev_cs))
    {
        return BCM_ERROR;
    }
    if (BCM_SUCCESS != chal_memc_ddr2_reset(handle, handle->mem_device[1].dev_cs))
    {
        return BCM_ERROR;
    }

    /* read the memory device so we know the DRAM & NVM size */
    lpddr2_read_config (handle);

    /* 9. set RL/WL to LPDDR2 device */

    /* 10. set memory type connected to CS0 */
    chal_memc_set_cs0_memory_type(handle);

    /* 11. adjust tRCD for NVM if it's connected to CS0 */
    if (handle->mem_device[0].dev_type == MEMC_TYPE_NVM)
    {
        MR_addr = LPDDR2_MODE_REG_NVM_TRCD;
        rdvalue = 0;
        lpddr2_mode_reg_read (handle, handle->mem_device[0].dev_cs, MR_addr, &rdvalue);
        wrvalue = BCM_REG_READ32 (handle, handle->memc_open_reg_base + CSR_NVM_TIMING0_OFFSET);
        wrvalue &= 0xFF;      /* mask out the tRCD bits */
        wrvalue |= rdvalue;   /* add the tRCD */
        BCM_REG_WRITE32 (handle, handle->memc_open_reg_base + CSR_NVM_TIMING0_OFFSET, wrvalue);
    }

    /* 12. init burst length */
    chal_memc_init_burst_length(handle);

    /* 13. set the DRAM & NVM start and end address */
    chal_memc_set_lpddr_address(handle);

    /* Miscellaneous settings */
    wrvalue = 0;
    BCM_REG_WRITE32 (handle, handle->memc_open_reg_base + CSR_AXI_PORT_CTRL_OFFSET, wrvalue);   /* enable all ports*/

    BCM_REG_WRITE32 (handle, handle->memc_secure_reg_base + SEC_RGN_0_CTL_OFFSET, 0); /* disable security region mode*/
    BCM_REG_WRITE32 (handle, handle->memc_secure_reg_base + SEC_RGN_1_CTL_OFFSET, 0); /* disable security region mode*/
    BCM_REG_WRITE32 (handle, handle->memc_secure_reg_base + SEC_RGN_2_CTL_OFFSET, 0); /* disable security region mode*/
    BCM_REG_WRITE32 (handle, handle->memc_secure_reg_base + SEC_RGN_3_CTL_OFFSET, 0); /* disable security region mode*/
    BCM_REG_WRITE32 (handle, handle->memc_secure_reg_base + SEC_RGN_4_CTL_OFFSET, 0); /* disable security region mode*/
    BCM_REG_WRITE32 (handle, handle->memc_secure_reg_base + SEC_RGN_5_CTL_OFFSET, 0); /* disable security region mode*/
    BCM_REG_WRITE32 (handle, handle->memc_secure_reg_base + SEC_RGN_6_CTL_OFFSET, 0); /* disable security region mode*/
    BCM_REG_WRITE32 (handle, handle->memc_secure_reg_base + SEC_RGN_7_CTL_OFFSET, 0); /* disable security region mode*/

    CHAL_DELAY_MICROS (1);  /* wait 1 us*/

    /* 12. init done*/
    wrvalue = CSR_SW_INIT_DONE_DONE_MASK;
    BCM_REG_WRITE32 (handle, handle->memc_open_reg_base + CSR_SW_INIT_DONE_OFFSET, wrvalue);

    CHAL_DELAY_MICROS (1);  /* wait 1 us*/

    /* 15. now configure the clock mode */
    if (handle->boot_mode == MEMC_BOOT_MODE_COLD)
    {
        lpddr2_clock_configuration(handle);

        /* enable device temperature polling */
        chal_memc_temperature_polling (handle, 1);

        /* enable periodic ZQ calibration */
        chal_memc_set_zq_calibration (handle, 1);
    
        /* For BigIsland put LPDDR2 IO pads in lower power mode at lower frequencies */
        rdvalue = BCM_REG_READ32 (handle, handle->memc_open_reg_base + CSR_HW_FREQ_CHANGE_CNTRL_OFFSET);
        rdvalue |= CSR_HW_FREQ_CHANGE_CNTRL_XTAL_LPWR_RX_CNTRL_MASK |
                   CSR_HW_FREQ_CHANGE_CNTRL_SYS_PLL_078MHZ_LPWR_RX_CNTRL_MASK |
                   CSR_HW_FREQ_CHANGE_CNTRL_SYS_PLL_104MHZ_LPWR_RX_CNTRL_MASK |
                   CSR_HW_FREQ_CHANGE_CNTRL_SYS_PLL_156MHZ_LPWR_RX_CNTRL_MASK ;
        BCM_REG_WRITE32 (handle, handle->memc_open_reg_base + CSR_HW_FREQ_CHANGE_CNTRL_OFFSET, rdvalue);

        /* Configure the max power state to run off the DDR PLL */
        BCM_REG_WRITE32 (handle, handle->memc_open_reg_base + CSR_MEMC_MAX_PWR_STATE_OFFSET,
                         MEMC_MAX_POWER_STATE_DDR_PLL << CSR_MEMC_MAX_PWR_STATE_MEMC_MAX_PWR_STATE_SHIFT);

    }

    CHAL_DELAY_MICROS(1);   /* wait 1 us*/

    /* 16. enable auto power management */
    if (handle->operation_mode == MEMC_OP_MODE_ASIC)
    {
       /*
        *  skip for now; enable auto transition can generate a crystal clock on top
        *  of the operating frequency
        */ 
        chal_memc_enable_auto_pwr(handle, FALSE);
    }    

    /* 17. read configurations from the mode registers of the DRAM & NVM device */
    /* this step is already done earlier */
    /*lpddr2_read_config (handle); */

    /* 18. if the device has NVM part, unlock the device and
     * read its overlay window info section
     */
    if (handle->mem_device[0].dev_type == MEMC_TYPE_NVM)
    {
        chal_memc_nvm_enable_overlay_window(handle, handle->memc_memory_base_nvm);
        chal_memc_nvm_fill_overlay_window_info(handle);
        /* chal_memc_nvm_block_unlock_all(handle); */
        chal_memc_nvm_disable_overlay_window(handle);
    }

    return BCM_SUCCESS;
}

/*
 * ===========================================================================
 * 
 *   Function Name: memc_misc_ctrl_config
 * 
 *   Description:
 *       configures the DQS_OEB and RDQS_GATE bits in misc control register
 * 
 * ===========================================================================
 */
static void memc_misc_ctrl_config(CHAL_MEMC_HANDLE handle, int dqs_oeb, int rdqs_gate)
{
    uint32_t rdvalue = 0x00;

    rdvalue = BCM_REG_READ32 (handle, handle->memc_dphy_reg_base + DPHY_CSR_DQ_PHY_MISC_CNTRL_OFFSET);

    if (dqs_oeb)
        rdvalue |= DPHY_CSR_DQ_PHY_MISC_CNTRL_DQ_DQS_OEB_OVERRIDE_MASK;
    else
        rdvalue &= ~DPHY_CSR_DQ_PHY_MISC_CNTRL_DQ_DQS_OEB_OVERRIDE_MASK;

    if (rdqs_gate)
        rdvalue |= DPHY_CSR_DQ_PHY_MISC_CNTRL_RDQS_GATE_OVERRIDE_MASK;
    else
        rdvalue &= ~DPHY_CSR_DQ_PHY_MISC_CNTRL_RDQS_GATE_OVERRIDE_MASK;

    BCM_REG_WRITE32 (handle, handle->memc_dphy_reg_base + DPHY_CSR_DQ_PHY_MISC_CNTRL_OFFSET, rdvalue);
}

/*
 * ===========================================================================
 * 
 *   Function Name: wait_frequency_state_stable
 * 
 *   Description:
 *       wait until the MEMC output frequency to DDR becomes stable
 * 
 * ===========================================================================
 */
static void wait_frequency_state_stable(CHAL_MEMC_HANDLE handle)
{
    uint32_t  currentState;
    uint32_t  targetState;
    uint32_t  rdvalue;
    MEMC_CS_CONNECTION  cs = MEMC_CS_0;
    
    /* Note this is for Hera ONLY */

    if (handle->mem_device[0].dev_type == MEMC_TYPE_DRAM)
        cs = handle->mem_device[0].dev_cs;

    if (handle->mem_device[1].dev_type == MEMC_TYPE_DRAM)
        cs = handle->mem_device[1].dev_cs;

    while(1)
    {
        /* wait until device not in self-refresh state*/
        rdvalue = MEMC_SELF_REFRESH_STATE;
        while (1)
        {
          chal_memc_get_sw_power_down_status(handle, &rdvalue);
          if (rdvalue != MEMC_SELF_REFRESH_STATE)
              break;

          /* generate any read to MRx to bring dram out of self-refresh*/
          lpddr2_mode_reg_read (handle, cs, LPDDR2_MODE_REG_BASIC_CONFIG4, &rdvalue);
        }

        /* verify the current frequency state matches target frequency*/
        rdvalue = BCM_REG_READ32 (handle, handle->memc_open_reg_base + CSR_CURRENT_FREQUENCY_STATE_OFFSET);
        currentState = rdvalue & CSR_CURRENT_FREQUENCY_STATE_CURRENT_STATE_MASK;
        targetState = (rdvalue & CSR_CURRENT_FREQUENCY_STATE_TARGET_STATE_MASK) >>
                        CSR_CURRENT_FREQUENCY_STATE_TARGET_STATE_SHIFT;

        if (currentState == targetState)
            return;
    }
}

/*
 * ===========================================================================
 * 
 *   Function Name: lpddr2_clock_configuration
 * 
 *   Description:
 *       configures the MEMC output frequency to DDR
 * 
 * ===========================================================================
 */
static void lpddr2_clock_configuration(
        CHAL_MEMC_HANDLE handle)
{
    uint32_t  rdvalue, i;
    uint32_t  MR_addr = 0;
    MEMC_CS_CONNECTION cs;

    MEMC_MDIV_T  mdiv;
    uint32_t  pdiv;
    uint32_t  ndiv_int;
    uint32_t  ndiv_frac;

    MEMC_FREQ_MAPPING_SYS_PLL_T sys_pll;
    MEMC_FREQ_MAPPING_DDR_PLL_T ddr_pll;

    /* 1. hold off AXI access*/
    BCM_REG_WRITE32 (handle, handle->memc_open_reg_base + CSR_DEMESH_ARB_DISABLE_OFFSET, 1);

    CHAL_DELAY_MICROS (10); /* wait 10 us*/

    /*
     *  Get configuration parameters for LPDDR2 PLL, based upon the XTAL (Input_Fref)
     *  Fvco = (Input_Fref/PDIV)*(NDIV_INT+(NDIV_FRAC/2^20))
     *  F_ddr2_pll = Fvco / MDIV;
     */
    /* 2a. program DDR2 frequency*/
    if (handle->operation_mode == MEMC_OP_MODE_ASIC )
    {
        switch ( chipregHw_getStrapXtalType () )
        {
           case chipregHw_STRAP_XTAL_TYPE_26MHZ:
           { 
              mdiv = MDIV_2;        /* Divide by 2*/
              pdiv = 1;

              /* The calculation for ndiv_int is dependent on mdiv = 2, pdiv = 1, xtal = 26 and another div-by-2 in the system
               * The resulting ndiv_int = clock_MHz*4/26 = clock_Mhz/6.5 */
              //ndiv_int = 0x3d;        /* F_ddr2_pll = 396.5 MHz */
              //ndiv_int = 0x2e;        /* F_ddr2_pll = 299 MHz */
              //ndiv_int = 0x24;        /* F_ddr2_pll = 234 MHz */
              ndiv_int = handle->clock_Mhz * 2 / 13;
              
              /* Always set ndiv_frac to 0, which will result in final clock configuration 
               * as the highest full multiple of 6.5 MHz, below the expected configuration */
              ndiv_frac = 0x0;
              sys_pll = SYS_PLL_FREQ_DIV_2;
              ddr_pll = DDR_PLL_FREQ_DIV_2;
              break;
           }
           default:
           { 
              BCM_DBG_ERR (("Unsupported XTAL strap\n" ));
              return;
           }
        }


        BCM_REG_WRITE32 (handle, handle->memc_aphy_reg_base + APHY_CSR_DDR_PLL_VCO_FREQ_CNTRL0_OFFSET, (pdiv << 16) | ndiv_int);
        BCM_REG_WRITE32 (handle, handle->memc_aphy_reg_base + APHY_CSR_DDR_PLL_VCO_FREQ_CNTRL1_OFFSET, ndiv_frac);

        /* For BigIsland: Program the 2nd VCO control register */
        BCM_REG_WRITE32 (handle, handle->memc_aphy_reg_base + APHY_CSR_DDR_PLL_VCO_FREQ_CNTRL2_OFFSET, 
                                   ( ndiv_int << APHY_CSR_DDR_PLL_VCO_FREQ_CNTRL2_NDIV_INT_2_SHIFT ) |
                                   ( ndiv_frac << APHY_CSR_DDR_PLL_VCO_FREQ_CNTRL2_NDIV_FRAC_2_SHIFT ));

        BCM_REG_WRITE32 (handle, handle->memc_aphy_reg_base + APHY_CSR_DDR_PLL_MDIV_VALUE_OFFSET, mdiv);

        /* For BigIsland: Program the SYS and DDR PLL divide register */
        BCM_REG_WRITE32 (handle, handle->memc_open_reg_base + CSR_MEMC_FREQ_STATE_MAPPING_OFFSET,
                                   ( sys_pll << CSR_MEMC_FREQ_STATE_MAPPING_SYS_FREQ_DIVIDE_VAL_SHIFT ) |
                                   ( ddr_pll << CSR_MEMC_FREQ_STATE_MAPPING_DDR_FREQ_DIVIDE_VAL_SHIFT ) );

        /* 2b. disable software PLL control*/
        BCM_REG_WRITE32 (handle, handle->memc_aphy_reg_base + APHY_CSR_DISABLE_SOFT_DDR_PLL_CONTROL_OFFSET, 1);
    }

    /*
     *  skip; programming refresh control & DRAM timing cause problems
     *  leave them as default
     *  3a. program the refresh control
     * BCM_REG_WRITE32 (handle, handle->memc_open_reg_base + CSR_REFRESH_CNTRL_OFFSET, handle->refresh_ctrl);
     */

    /*
     *  3b. program the DRAM timing0 control
     * BCM_REG_WRITE32 (handle, handle->memc_open_reg_base + CSR_DRAM_TIMING0_OFFSET, handle->timing0);
     */

    /*
     *  3c. program the DRAM timing1 control
     * BCM_REG_WRITE32 (handle, handle->memc_open_reg_base + CSR_DRAM_TIMING1_OFFSET, handle->timing1);
     */

    /* 3d. skip program the NVM timing control; not supported*/

    /* 4. DQS gating mode*/
    BCM_REG_WRITE32 (handle, handle->memc_dphy_reg_base + DPHY_CSR_NORM_READ_DQS_GATE_CNTRL_OFFSET, 0x50505050);

    for (i=0; i<MEMC_CS_MAX; i++)
    {
        cs = handle->mem_device[i].dev_cs;
        if (cs != MEMC_CS_NONE)
        {
            /* 5a. write device feature1 */
            MR_addr = LPDDR2_MODE_REG_DEVICE_FEATURE1;  /* MR1 */
            lpddr2_mode_reg_write (handle, cs, MR_addr, handle->mem_device[i].dev_config.MR1_device_feature1);

            /* 5b. write device io config */
            MR_addr = LPDDR2_MODE_REG_IO_CONFIG1;       /* MR3 */
            lpddr2_mode_reg_write (handle, cs, MR_addr, handle->mem_device[i].dev_config.MR3_device_ioconfig);

            /* 5c. write device feature2 */
            MR_addr = LPDDR2_MODE_REG_DEVICE_FEATURE2;  /* MR2 */
            lpddr2_mode_reg_write (handle, cs, MR_addr, handle->mem_device[i].dev_config.MR2_device_feature2);
        }
    }

    BCM_REG_WRITE32 (handle, handle->memc_open_reg_base + CSR_DDR2_MR2_OFFSET,
        handle->mem_device[0].dev_config.MR2_device_feature2);

    /* 6. wait 200 ns*/
    CHAL_DELAY_MICROS (200);

    /* 7. enable hardware PLL control; auto-power transition off*/
    rdvalue = BCM_REG_READ32 (handle, handle->memc_open_reg_base + CSR_HW_FREQ_CHANGE_CNTRL_OFFSET);
    rdvalue |= CSR_HW_FREQ_CHANGE_CNTRL_HW_FREQ_CHANGE_EN_MASK;
    rdvalue &= ~CSR_HW_FREQ_CHANGE_CNTRL_HW_AUTO_PWR_TRANSITION_MASK;
    BCM_REG_WRITE32 (handle, handle->memc_open_reg_base + CSR_HW_FREQ_CHANGE_CNTRL_OFFSET, rdvalue);

    /* 8. disable clock-deskew*/
    rdvalue = BCM_REG_READ32 (handle, handle->memc_open_reg_base + CSR_HW_FREQ_CHANGE_CNTRL_OFFSET);
    rdvalue |= CSR_HW_FREQ_CHANGE_CNTRL_DISABLE_DESKEW_DLL_CALIB_ON_CLK_CHANGE_MASK;
    BCM_REG_WRITE32 (handle, handle->memc_open_reg_base + CSR_HW_FREQ_CHANGE_CNTRL_OFFSET, rdvalue);

    {
       /* Initiate auto-refresh command to both CS in order to compensate for the DRAM's
        * internal refresh command being aborted when doing a warm boot as a consequence of
        * a WD reset in the system.  See BisIsland/Rhea MEMC programmers guide. */
       uint32_t  wrvalue;

       wrvalue =  0 << CSR_DRAM_INIT_CONTROL_CS_BITS_SHIFT |
                MEMCX_OPEN_DRAM_INIT_CONTROL_AREF |
                0xde << CSR_DRAM_INIT_CONTROL_MRW_DATA_SHIFT |
                0xaf << CSR_DRAM_INIT_CONTROL_MR_ADDR_SHIFT ;

       BCM_REG_WRITE32 (handle, handle->memc_open_reg_base + CSR_DRAM_INIT_CONTROL_OFFSET, wrvalue);
    }

    /* 9. enable AXI access*/
    BCM_REG_WRITE32 (handle, handle->memc_open_reg_base + CSR_DEMESH_ARB_DISABLE_OFFSET, 0);


    /* 10. generate any read to MRx; skip; unnecessary*/
    CHAL_DELAY_MICROS (100);      /* delay a little bit; say 100 us*/
}

/*
 * ===========================================================================
 * 
 *   Function Name: lpddr2_wait_memc_register
 * 
 *   Description:
 *       wait for status bits in a memc status register
 * 
 * ===========================================================================
 */
static BCM_ERR_CODE lpddr2_wait_memc_register(
        CHAL_MEMC_HANDLE handle,
        uint32_t reg,
        uint32_t mask)
{
    uint32_t expire_cnt = 0;
    uint32_t rdvalue = 0x00;

    while (((rdvalue & mask) == 0) && expire_cnt < READ_RETRY_MAX)
    {
        rdvalue = BCM_REG_READ32 (handle, reg);
        expire_cnt ++;
    }

    /*
     *  if running on FPGA, the status register is bypassed and counter will expire;
     *  if running on actual hardware, it's an error if expired
     */
    if (handle->operation_mode == MEMC_OP_MODE_ASIC && expire_cnt >= READ_RETRY_MAX)
    {
        return BCM_ERROR;
    }
    return BCM_SUCCESS;
}

/*
 * ===========================================================================
 * 
 *   Function Name: lpddr2_mode_reg_read
 * 
 *   Description:
 *       Read a single LPDDR2 mode register
 * 
 * ===========================================================================
 */
static BCM_ERR_CODE lpddr2_mode_reg_read (
    CHAL_MEMC_HANDLE handle,
    MEMC_CS_CONNECTION cs,
    uint32_t addr,
    uint32_t *pVal
)
{
    uint32_t wrvalue=0x00;
    uint32_t rdvalue=0x00, valid=0, retry=0;
    uint32_t base_addr = 0x00;
    uint32_t cs_bits = 0x00;

    *pVal = 0;
    base_addr = handle->memc_open_reg_base;

    if (cs == MEMC_CS_0)
        cs_bits = 0x2;            /* CS0 */
    else if (cs == MEMC_CS_1)
        cs_bits = 0x1;            /* CS1 */
    else
        return (BCM_ERROR);       /* both CS0 & CS1 */

    wrvalue = ((cs_bits<<CSR_DRAM_INIT_CONTROL_CS_BITS_SHIFT) & CSR_DRAM_INIT_CONTROL_CS_BITS_MASK)
        | (MEMCX_OPEN_DRAM_INIT_CONTROL_MRR)
        | ((addr<<CSR_DRAM_INIT_CONTROL_MR_ADDR_SHIFT) & CSR_DRAM_INIT_CONTROL_MR_ADDR_MASK);

    BCM_REG_WRITE32 (handle, base_addr + CSR_DRAM_INIT_CONTROL_OFFSET, wrvalue);

    /* polling valid bit*/
    do {
        rdvalue = BCM_REG_READ32 (handle, base_addr + CSR_DRAM_INIT_RESULT_OFFSET);
        valid = rdvalue & CSR_DRAM_INIT_RESULT_MRR_VALID_MASK;
        if (valid) break;
    } while (retry++<READ_RETRY_MAX);

    if ( retry >= READ_RETRY_MAX ) {
        handle->expireReadCnt++;
        return (BCM_ERROR);
    }

    *pVal = ( rdvalue & CSR_DRAM_INIT_RESULT_MRR_DATA_MASK ) >> CSR_DRAM_INIT_RESULT_MRR_DATA_SHIFT;
    return (BCM_SUCCESS);
}

/*
 * ===========================================================================
 * 
 *   Function Name: lpddr2_mode_reg_write
 * 
 *   Description:
 *       Write a single LPDDR2 mode register
 * 
 * ===========================================================================
 */
static BCM_ERR_CODE lpddr2_mode_reg_write (
    CHAL_MEMC_HANDLE handle,
    MEMC_CS_CONNECTION cs,
    uint32_t addr,
    uint32_t val
)
{
    uint32_t wrvalue=0x00;
    uint32_t base_addr = 0x00;
    uint32_t rdvalue=0x00, busy=0, retry=0;
    uint32_t cs_bits = 0x00;

    if ( !handle ) {
        BCM_DBG_ERR (("invalid argument: handle=0 \n" ));
        return (BCM_ERROR);
    }

    if (cs == MEMC_CS_0)
        cs_bits = 0x2;            /* CS0 */
    else if (cs == MEMC_CS_1)
        cs_bits = 0x1;            /* CS1 */
    else
        return (BCM_ERROR);       /* both CS0 & CS1 */

    base_addr = handle->memc_open_reg_base;

    wrvalue = ((cs_bits<<CSR_DRAM_INIT_CONTROL_CS_BITS_SHIFT) & CSR_DRAM_INIT_CONTROL_CS_BITS_MASK)
        | (MEMCX_OPEN_DRAM_INIT_CONTROL_MRW)
        | ((val<<CSR_DRAM_INIT_CONTROL_MRW_DATA_SHIFT) & CSR_DRAM_INIT_CONTROL_MRW_DATA_MASK)
        | ((addr<<CSR_DRAM_INIT_CONTROL_MR_ADDR_SHIFT) & CSR_DRAM_INIT_CONTROL_MR_ADDR_MASK);

    BCM_REG_WRITE32 (handle, base_addr + CSR_DRAM_INIT_CONTROL_OFFSET, wrvalue);

    /* polling busy bit*/
    do {
        rdvalue = BCM_REG_READ32 (handle, base_addr + CSR_DRAM_INIT_RESULT_OFFSET);
        busy = rdvalue & CSR_DRAM_INIT_RESULT_BUSY_MASK;
        if (busy) break;
    } while (retry++<READ_RETRY_MAX);

    if ( retry >= READ_RETRY_MAX ) {
        handle->expireWriteCnt++;
        /*
         *  busy bit happens for very short time; software may not be able to
         *  catch it; in case we miss it, still consider as success
         * return (BCM_ERROR);
         */
    }

    return BCM_SUCCESS;
}

/* 
 * ===========================================================================
 * 
 *   Function Name: chal_lpddr2_get_base_addr
 * 
 *   Description:
 *       returns the base address of the MEMC register base
 * 
 * ===========================================================================
 */
MEMC_EXPORT BCM_ERR_CODE chal_lpddr2_get_base_addr (
    CHAL_MEMC_HANDLE handle,
    MEMC_REG_BLOCK base
    )
{
    switch(base)
    {
    case MEMC_REG_BLOCK_SEC:
        return handle->memc_secure_reg_base;
    case MEMC_REG_BLOCK_CSR:
        return handle->memc_open_reg_base;
    case MEMC_REG_BLOCK_APHY:
        return handle->memc_aphy_reg_base;
    case MEMC_REG_BLOCK_DPHY:
        return handle->memc_dphy_reg_base;
    default:
        return 0;
    }
}

/*
 * ===========================================================================
 * 
 *   Function Name: lpddr2_read_config
 * 
 *   Description:
 *       read config register from LPDDR2 attached to controller id and selected by cs
 * 
 * ===========================================================================
 */
static BCM_ERR_CODE lpddr2_read_config (
    CHAL_MEMC_HANDLE handle
)
{
    uint32_t config;
    uint32_t i;
    MEMC_CS_CONNECTION cs;

    if ( !handle ) {
        BCM_DBG_ERR (("invalid argument: handle=0 \n" ));
        return (BCM_ERROR);
    }

    for (i=0; i<MEMC_CS_MAX; i++)
    {
        cs = handle->mem_device[i].dev_cs;
        if (cs == MEMC_CS_NONE)
            continue;

        if ( lpddr2_mode_reg_read (handle, cs, LPDDR2_MODE_REG_SDRAM_REFR, &config) != BCM_SUCCESS) {
            BCM_DBG_ERR (("Error read lpddr2 register %d\n", LPDDR2_MODE_REG_SDRAM_REFR));
            return BCM_ERROR;
        }
        handle->mem_device[i].dev_config.MR4_SDRAM_refresh_rate = (uint8_t) config;

        if ( lpddr2_mode_reg_read (handle, cs, LPDDR2_MODE_REG_BASIC_CONFIG1, &config) != BCM_SUCCESS) {
            BCM_DBG_ERR (("Error read lpddr2 register %d\n", LPDDR2_MODE_REG_BASIC_CONFIG1));
            return BCM_ERROR;
        }
        handle->mem_device[i].dev_config.MR5_basic_config1 = (uint8_t) config;

        if ( lpddr2_mode_reg_read (handle, cs, LPDDR2_MODE_REG_BASIC_CONFIG2, &config) != BCM_SUCCESS) {
            BCM_DBG_ERR (("Error read lpddr2 register %d\n", LPDDR2_MODE_REG_BASIC_CONFIG2));
            return BCM_ERROR;
        }
        handle->mem_device[i].dev_config.MR6_basic_config2 = (uint8_t) config;

        if ( lpddr2_mode_reg_read (handle, cs, LPDDR2_MODE_REG_BASIC_CONFIG3, &config) != BCM_SUCCESS) {
            BCM_DBG_ERR (("Error read lpddr2 register %d\n", LPDDR2_MODE_REG_BASIC_CONFIG3));
            return BCM_ERROR;
        }
        handle->mem_device[i].dev_config.MR7_basic_config3 = (uint8_t) config;

        if ( lpddr2_mode_reg_read (handle, cs, LPDDR2_MODE_REG_BASIC_CONFIG4, &config) != BCM_SUCCESS) {
            BCM_DBG_ERR (("Error read lpddr2 register %d\n", LPDDR2_MODE_REG_BASIC_CONFIG4));
            return BCM_ERROR;
        }
        handle->mem_device[i].dev_config.MR8_basic_config4 = (uint8_t) config;

        /* decode the dev config */
        handle->mem_device[i].dev_info.vendor_id = (LPDDR2_VENDOR_ID)handle->mem_device[i].dev_config.MR5_basic_config1;
        lpddr2_get_vendor_name_from_id (handle->mem_device[i].dev_info.vendor_id, handle->mem_device[i].dev_info.vendor_name);

        handle->mem_device[i].dev_info.type = (LPDDR2_TYPE)(handle->mem_device[i].dev_config.MR8_basic_config4 & 3);
        handle->mem_device[i].dev_info.density = (LPDDR2_DENSITY)((handle->mem_device[i].dev_config.MR8_basic_config4>>2) & 0xf);
        handle->mem_device[i].dev_info.io_width = (LPDDR2_IO_WIDTH)((handle->mem_device[i].dev_config.MR8_basic_config4>>6) & 3);
    }

    return (BCM_SUCCESS);
}

/*
 * ===========================================================================
 * 
 *   Function Name: lpddr2_get_vendor_name_from_id
 * 
 *   Description:
 *       Get vendor name from id
 * 
 * ===========================================================================
 */
static BCM_ERR_CODE lpddr2_get_vendor_name_from_id (
    uint32_t id,
    char name[]
)
{
    uint32_t i;

    for (i=0; i<ARRAY_SIZE (lpddr_vendor_info); i++) {
        if (id == lpddr_vendor_info[i].id) {
            strcpy (name, lpddr_vendor_info[i].name);
            return (BCM_SUCCESS);
        }
    }

    return (BCM_ERROR);
}

/*
 * ===========================================================================
 * 
 *   Function Name: chal_memc_set_clock
 * 
 *   Description:
 *       Set MEMC to a new clock
 * 
 * ===========================================================================
 */
MEMC_EXPORT BCM_ERR_CODE chal_memc_set_clock (
    CHAL_MEMC_HANDLE handle,
    MEMC_CLOCK_T clock
)
{
    /* Note this is for Hera ONLY */
    
    if (clock >= FXTOTAL)
        return (BCM_ERROR);

    /* 1. wait until frequency state is stable*/
    wait_frequency_state_stable(handle);

    /*
     *  2. set the target frequency state;
     */ 
    BCM_REG_WRITE32 (handle, handle->memc_open_reg_base + 0x100, clock);

    /* 3. wait the target frequency is set*/
    wait_frequency_state_stable(handle);

    return (BCM_SUCCESS);
}

/*
 * ===========================================================================
 * 
 *   Function Name: chal_memc_get_clock
 * 
 *   Description:
 *       Get the current MEMC clock
 * 
 * ===========================================================================
 */
MEMC_EXPORT BCM_ERR_CODE chal_memc_get_clock (
    CHAL_MEMC_HANDLE handle,
    MEMC_CLOCK_T *clock
)
{
    uint32_t  rdvalue;

    rdvalue = BCM_REG_READ32 (handle, handle->memc_open_reg_base + CSR_CURRENT_FREQUENCY_STATE_OFFSET);
    *clock = (MEMC_CLOCK_T)(rdvalue & CSR_CURRENT_FREQUENCY_STATE_CURRENT_STATE_MASK);

    return (BCM_SUCCESS);
}
    
/*
 * ===========================================================================
 * 
 *   Function Name: chal_memc_get_clock_speed
 * 
 *   Description:
 *       Get the current MEMC clock speed in hz
 * 
 * ===========================================================================
 */
MEMC_EXPORT BCM_ERR_CODE chal_memc_get_clock_speed (
    CHAL_MEMC_HANDLE handle,
    uint32_t *clock_speed_hz
)
{
    uint32_t  rdvalue;
    MEMC_MDIV_T  mdiv;
    uint32_t  mdiv_shift;
    uint32_t  pdiv;
    uint32_t  pdiv_val;
    uint32_t  ndiv_int;
    uint32_t  ndiv_frac;
    uint32_t  xtal_freq;
    MEMC_MAX_POWER_STATE_T  max_power_state;
    MEMC_FREQ_MAPPING_DDR_PLL_T  ddr_pll;
    uint32_t  ddr_pll_div;
    MEMC_FREQ_MAPPING_SYS_PLL_T  sys_pll;
    uint32_t  sys_pll_div;
    uint32_t  f_ddr2_pll;

    if ( handle->memc_ddr_type == MEMC_DDR_TYPE_DDR3 )
    {
       chal_memc_ddr3_get_clock_speed( (void*)handle, clock_speed_hz );
    }    
    else if ( handle->memc_ddr_type == MEMC_DDR_TYPE_LPDDR2 )
    {
       if (handle->operation_mode == MEMC_OP_MODE_ASIC )
       {
          rdvalue = BCM_REG_READ32 (handle, handle->memc_aphy_reg_base + APHY_CSR_DDR_PLL_VCO_FREQ_CNTRL0_OFFSET);
          ndiv_int = (rdvalue & APHY_CSR_DDR_PLL_VCO_FREQ_CNTRL0_NDIV_INT_MASK) >> APHY_CSR_DDR_PLL_VCO_FREQ_CNTRL0_NDIV_INT_SHIFT;

          pdiv = (rdvalue & APHY_CSR_DDR_PLL_VCO_FREQ_CNTRL0_PDIV_MASK) >> APHY_CSR_DDR_PLL_VCO_FREQ_CNTRL0_PDIV_SHIFT;
          pdiv_val = ((pdiv == 0) ? 8 : pdiv);

          rdvalue = BCM_REG_READ32 (handle, handle->memc_aphy_reg_base + APHY_CSR_DDR_PLL_VCO_FREQ_CNTRL1_OFFSET);
          ndiv_frac = (rdvalue & APHY_CSR_DDR_PLL_VCO_FREQ_CNTRL1_NDIV_FRAC_MASK) >> APHY_CSR_DDR_PLL_VCO_FREQ_CNTRL1_NDIV_FRAC_SHIFT;

          rdvalue = BCM_REG_READ32 (handle, handle->memc_aphy_reg_base + APHY_CSR_DDR_PLL_MDIV_VALUE_OFFSET);
          mdiv = (rdvalue & APHY_CSR_DDR_PLL_MDIV_VALUE_MDIV_MASK) >> APHY_CSR_DDR_PLL_MDIV_VALUE_MDIV_SHIFT;
          mdiv_shift = ((mdiv == MDIV_2) ? 1 : 2);

          rdvalue = BCM_REG_READ32 (handle, handle->memc_open_reg_base + CSR_MEMC_FREQ_STATE_MAPPING_OFFSET);

          sys_pll = (rdvalue & CSR_MEMC_FREQ_STATE_MAPPING_SYS_FREQ_DIVIDE_VAL_MASK) >> CSR_MEMC_FREQ_STATE_MAPPING_SYS_FREQ_DIVIDE_VAL_SHIFT;
          switch ( sys_pll )
          {
             case SYS_PLL_FREQ_DIV_2:
             {    
                 sys_pll_div = 2;
                 break;
             }
             case SYS_PLL_FREQ_DIV_4:
             {    
                 sys_pll_div = 4;
                 break;
             }
             case SYS_PLL_FREQ_DIV_6:
             {    
                 sys_pll_div = 6;
                 break;
             }
             default:
             { 
                BCM_DBG_ERR (("Unknown Sys PLL Freq Divisor\n" ));
                *clock_speed_hz = 0;
                return (BCM_ERROR);
             }
          }

          ddr_pll = (rdvalue & CSR_MEMC_FREQ_STATE_MAPPING_DDR_FREQ_DIVIDE_VAL_MASK) >> CSR_MEMC_FREQ_STATE_MAPPING_DDR_FREQ_DIVIDE_VAL_SHIFT;
          switch ( ddr_pll )
          {
             case DDR_PLL_FREQ_DIV_2:
             {    
                 ddr_pll_div = 2;
                 break;
             }
             case DDR_PLL_FREQ_DIV_4:
             {    
                 ddr_pll_div = 4;
                 break;
             }
             case DDR_PLL_FREQ_DIV_6:
             {    
                 ddr_pll_div = 6;
                 break;
             }
             case DDR_PLL_FREQ_DIV_8:
             {    
                 ddr_pll_div = 8;
                 break;
             }
             default:
             { 
                BCM_DBG_ERR (("Unknown DDR PLL Freq Divisor\n" ));
                *clock_speed_hz = 0;
                return (BCM_ERROR);
             }
          }

          rdvalue = BCM_REG_READ32 (handle, handle->memc_open_reg_base + CSR_MEMC_MAX_PWR_STATE_OFFSET);

          max_power_state = (rdvalue & CSR_MEMC_MAX_PWR_STATE_MEMC_MAX_PWR_STATE_MASK) >> CSR_MEMC_MAX_PWR_STATE_MEMC_MAX_PWR_STATE_SHIFT;

          switch ( chipregHw_getStrapXtalType() )
          {
             case chipregHw_STRAP_XTAL_TYPE_26MHZ:
             { 
                xtal_freq = 26*1000*1000;
                break;
             }
             default:
             { 
                BCM_DBG_ERR (("Unsupported XTAL strap\n" ));
                *clock_speed_hz = 0;
                return (BCM_ERROR);
             }
          }

          /*
           *  Fvco = (Input_Fref/PDIV)*(NDIV_INT+(NDIV_FRAC/2^20))
           *  F_ddr2_pll = Fvco / MDIV;
           */

          f_ddr2_pll = (((xtal_freq*ndiv_int)/pdiv + ((uint32_t)((xtal_freq*(uint64_t)ndiv_frac)>>20))/pdiv_val) >> mdiv_shift);
                
          /* Configured LPDDR2 clock speed */
          switch ( max_power_state )
          {
             case MEMC_MAX_POWER_STATE_XTAL:
             { 
                *clock_speed_hz = xtal_freq;
                break;
             }
             case MEMC_MAX_POWER_STATE_SYS_PLL:
             { 
                *clock_speed_hz = f_ddr2_pll / sys_pll_div ;
                break;
             }
             case MEMC_MAX_POWER_STATE_DDR_PLL:
             { 
                *clock_speed_hz = f_ddr2_pll / ddr_pll_div ;
                break;
             }
             default:
             { 
                BCM_DBG_ERR (("Invalid max power state\n" ));
                *clock_speed_hz = 0;
                return (BCM_ERROR);
             }
          }
       }
       else
       {
          /* The operation mode is MEMC_OP_MODE_FPGA */ 
          /* LPDDR2 clock speed is hardcoded */
          *clock_speed_hz = 27000000;  
       } 

    }
    return (BCM_SUCCESS);
}

/*
 * ===========================================================================
 * 
 *   Function Name: chal_memc_set_auto_power_down_cycles
 * 
 *   Description:
 *       Set auto power down cycles
 * 
 * ===========================================================================
 */
MEMC_EXPORT BCM_ERR_CODE chal_memc_set_auto_power_down_cycles (
    CHAL_MEMC_HANDLE handle,
    uint32_t cycles
)
{
    uint32_t base_addr;
    uint32_t value;

    if ( !handle ) {
        BCM_DBG_ERR (("invalid argument: handle=0 \n" ));
        return (BCM_ERROR);
    }

    if ( cycles < 0x4 ) {
        BCM_DBG_ERR (("invalid argument: cycles<4"));
        return (BCM_ERROR);
    }

    base_addr = handle->memc_open_reg_base;

    value = BCM_REG_READ32 (handle, base_addr + CSR_DDR_SW_POWER_DOWN_CONTROL_OFFSET);
    value &= ~CSR_DDR_SW_POWER_DOWN_CONTROL_IDLE_TIME_MASK;  /* clear the bits*/
    value |= cycles << CSR_DDR_SW_POWER_DOWN_CONTROL_IDLE_TIME_SHIFT;

    BCM_REG_WRITE32 (handle, base_addr + CSR_DDR_SW_POWER_DOWN_CONTROL_OFFSET, value);

    return (BCM_SUCCESS);
}

/*
 * ===========================================================================
 * 
 *   Function Name: chal_memc_get_auto_power_down_cycles
 * 
 *   Description:
 *       Get auto power down cycles
 * 
 * ===========================================================================
 */
MEMC_EXPORT BCM_ERR_CODE chal_memc_get_auto_power_down_cycles(
    CHAL_MEMC_HANDLE handle,
    uint32_t *cycles
)
{
    uint32_t base_addr;
    uint32_t value;

    *cycles = 0;

    if ( !handle ) {
        BCM_DBG_ERR (("invalid argument: handle=0 \n" ));
        return (BCM_ERROR);
    }

    base_addr = handle->memc_open_reg_base;

    value = BCM_REG_READ32 (handle, base_addr + CSR_DDR_SW_POWER_DOWN_CONTROL_OFFSET);

    *cycles = (value & CSR_DDR_SW_POWER_DOWN_CONTROL_IDLE_TIME_MASK) >>
            CSR_DDR_SW_POWER_DOWN_CONTROL_IDLE_TIME_SHIFT ;

    return (BCM_SUCCESS);
}

/*
 * ===========================================================================
 * 
 *   Function Name: chal_memc_enable_auto_pwr
 * 
 *   Description:
 *       enable auto power management
 * 
 * ===========================================================================
 */
MEMC_EXPORT BCM_ERR_CODE chal_memc_enable_auto_pwr(
    CHAL_MEMC_HANDLE handle,
    _Bool enable
)
{
    uint32_t base_addr;
    uint32_t value;

    if ( !handle ) {
        BCM_DBG_ERR (("invalid argument: handle=0 \n" ));
        return (BCM_ERROR);
    }

    /* 1. set active power down mode*/
    chal_memc_enter_sw_power_down(handle, MEMC_ACTIVE_POWER_DOWN);

    /* 2. configure auto power down*/
    base_addr = handle->memc_open_reg_base;

    value = BCM_REG_READ32 (handle, base_addr + CSR_HW_FREQ_CHANGE_CNTRL_OFFSET);
    value |= CSR_HW_FREQ_CHANGE_CNTRL_HW_FREQ_CHANGE_EN_MASK;
    if ( enable )
    {
       value |= CSR_HW_FREQ_CHANGE_CNTRL_HW_AUTO_PWR_TRANSITION_MASK;
    }
    else
    {
       value &= ~CSR_HW_FREQ_CHANGE_CNTRL_HW_AUTO_PWR_TRANSITION_MASK;
    }    
    

    BCM_REG_WRITE32 (handle, base_addr + CSR_HW_FREQ_CHANGE_CNTRL_OFFSET, value);

    return (BCM_SUCCESS);
}

/*
 * ===========================================================================
 * 
 *   Function Name: chal_memc_enter_sw_power_down
 * 
 *   Description:
 *       Set MEMC to SW power down mode
 * 
 * ===========================================================================
 */
MEMC_EXPORT BCM_ERR_CODE chal_memc_enter_sw_power_down (
    CHAL_MEMC_HANDLE handle,
    CHAL_MEMC_POWER_DOWN_MODE mode
)
{
    uint32_t base_addr;
    uint32_t value = 0;

    if ( !handle ) {
        BCM_DBG_ERR (("invalid argument: handle=0 \n" ));
        return (BCM_ERROR);
    }

    base_addr = handle->memc_open_reg_base;

    value = BCM_REG_READ32 (handle, base_addr + CSR_DDR_SW_POWER_DOWN_CONTROL_OFFSET);
    value &= ~CSR_DDR_SW_POWER_DOWN_CONTROL_POWER_DOWN_MODE_MASK;   /* clear the bits*/
    if (mode != MEMC_SELF_REFRESH)
    {
       value |= mode << CSR_DDR_SW_POWER_DOWN_CONTROL_POWER_DOWN_MODE_SHIFT;
    }

    /* make sure power down is enable*/
    value |= CSR_DDR_SW_POWER_DOWN_CONTROL_POWER_DOWN_ENABLE_MASK;

    BCM_REG_WRITE32 (handle, base_addr + CSR_DDR_SW_POWER_DOWN_CONTROL_OFFSET, value);

    if (mode == MEMC_SELF_REFRESH)
    {
        /* to enter self-refresh mode, we need to enable auto power transition*/
        value = BCM_REG_READ32 (handle, base_addr + CSR_HW_FREQ_CHANGE_CNTRL_OFFSET);
        value |= CSR_HW_FREQ_CHANGE_CNTRL_HW_AUTO_PWR_TRANSITION_MASK;
        BCM_REG_WRITE32 (handle, base_addr + CSR_HW_FREQ_CHANGE_CNTRL_OFFSET, value);

        BCM_REG_WRITE32 (handle, base_addr + CSR_APPS_MIN_PWR_STATE_OFFSET, MEMC_MIN_POWER_STATE_SELF_REFRESH );
        BCM_REG_WRITE32 (handle, base_addr + CSR_MODEM_MIN_PWR_STATE_OFFSET, MEMC_MIN_POWER_STATE_SELF_REFRESH );
    }

    return (BCM_SUCCESS);
}

/*
 * ===========================================================================
 * 
 *   Function Name: chal_memc_exit_selfrefresh
 * 
 *   Description:
 *       Exit self-refresh mode
 * 
 * ===========================================================================
 */
MEMC_EXPORT BCM_ERR_CODE chal_memc_exit_selfrefresh (
    CHAL_MEMC_HANDLE handle
)
{
    uint32_t wrvalue = 0;

    if ( !handle ) {
        BCM_DBG_ERR (("invalid argument: handle=0 \n" ));
        return (BCM_ERROR);
    }

    chal_memc_enable_auto_pwr( handle, FALSE );

    BCM_REG_WRITE32 (handle, handle->memc_open_reg_base + CSR_APPS_MIN_PWR_STATE_OFFSET, MEMC_MIN_POWER_STATE_XTAL );
    BCM_REG_WRITE32 (handle, handle->memc_open_reg_base + CSR_MODEM_MIN_PWR_STATE_OFFSET, MEMC_MIN_POWER_STATE_XTAL );

    /* Issue auto-refresh command to the DRAM */
    wrvalue =  0 << CSR_DRAM_INIT_CONTROL_CS_BITS_SHIFT |
                MEMCX_OPEN_DRAM_INIT_CONTROL_AREF |
                0xde << CSR_DRAM_INIT_CONTROL_MRW_DATA_SHIFT |
                0xaf << CSR_DRAM_INIT_CONTROL_MR_ADDR_SHIFT ;

    BCM_REG_WRITE32 (handle, handle->memc_open_reg_base + CSR_DRAM_INIT_CONTROL_OFFSET, wrvalue);

    return (BCM_SUCCESS);
}    

/*
 * ===========================================================================
 * 
 *   Function Name: chal_memc_sw_power_fully_on
 * 
 *   Description:
 *       device in full active power mode; always on
 * 
 * ===========================================================================
 */
MEMC_EXPORT BCM_ERR_CODE chal_memc_sw_power_fully_on (
    CHAL_MEMC_HANDLE handle
)
{
    uint32_t base_addr;
    uint32_t value = 0;

    if ( !handle ) {
        BCM_DBG_ERR (("invalid argument: handle=0 \n" ));
        return (BCM_ERROR);
    }

    base_addr = handle->memc_open_reg_base;

    value = BCM_REG_READ32 (handle, base_addr + CSR_DDR_SW_POWER_DOWN_CONTROL_OFFSET);

    /* clear the bits to put it back to normal/default mode*/
    value &= ~CSR_DDR_SW_POWER_DOWN_CONTROL_POWER_DOWN_MODE_MASK;

    /* device always on*/
    value &= ~CSR_DDR_SW_POWER_DOWN_CONTROL_POWER_DOWN_ENABLE_MASK;

    BCM_REG_WRITE32 (handle, base_addr + CSR_DDR_SW_POWER_DOWN_CONTROL_OFFSET, value);

    return (BCM_SUCCESS);
}

/*
 * ===========================================================================
 * 
 *   Function Name: chal_memc_get_sw_power_down_status
 * 
 *   Description:
 *       get the current SW power down status
 * 
 * ===========================================================================
 */
MEMC_EXPORT BCM_ERR_CODE chal_memc_get_sw_power_down_status (
    CHAL_MEMC_HANDLE handle,
    uint32_t *status
)
{
    uint32_t base_addr;
    uint32_t value;

    *status = 0;

    if ( !handle ) {
        BCM_DBG_ERR (("invalid argument: handle=0 \n" ));
        return (BCM_ERROR);
    }

    base_addr = handle->memc_open_reg_base;

    value = BCM_REG_READ32 (handle, base_addr + CSR_DDR_POWER_DOWN_STATUS_OFFSET);
    value &= CSR_DDR_POWER_DOWN_STATUS_POWER_DOWN_STATUS_MASK;
    *status = value;

    return (BCM_SUCCESS);
}

/* 
 * ===========================================================================
 * 
 *   Function Name: chal_lpddr2_get_dev_info
 * 
 *   Description:
 *       Get lpddr2 device info
 * 
 * ===========================================================================
 */
MEMC_EXPORT BCM_ERR_CODE chal_lpddr2_get_dev_info (
    CHAL_MEMC_HANDLE handle,
    MEMC_CS_CONNECTION cs,
    CHAL_LPDDR2_DEV_INFO_T *pDevInfo
)
{
    uint32_t i = 0;

    if ( !handle ) {
        BCM_DBG_ERR (("invalid argument: handle=0 \n" ));
        return (BCM_ERROR);
    }

    if ( !pDevInfo) {
        BCM_DBG_ERR (("invalid argument: pConfig==0\n"));
        return (BCM_ERROR);
    }

    lpddr2_read_config( handle );

    pDevInfo->vendor_id = handle->mem_device[cs].dev_info.vendor_id;
    pDevInfo->type = handle->mem_device[cs].dev_info.type;
    pDevInfo->density = handle->mem_device[cs].dev_info.density;
    pDevInfo->io_width = handle->mem_device[cs].dev_info.io_width;

    for (i=0; i<15; i++)
    {
        pDevInfo->vendor_name[i] = handle->mem_device[cs].dev_info.vendor_name[i];
    }

    handle->total_bytes = CHAL_REG_READ32( handle->memc_secure_reg_base + SEC_DRAM_END_ADDR_OFFSET) -
                          CHAL_REG_READ32( handle->memc_secure_reg_base + SEC_DRAM_START_ADDR_OFFSET) + 1;


    return (BCM_SUCCESS);
}

/* 
 * ===========================================================================
 * 
 *   Function Name: chal_lpddr2_get_mem_size
 * 
 *   Description:
 *       Get lpddr2 memory size
 * 
 * ===========================================================================
 */
MEMC_EXPORT BCM_ERR_CODE chal_lpddr2_get_mem_size (
    CHAL_MEMC_HANDLE handle,
    MEMC_CS_CONNECTION cs,
    uint32_t *memSize
)
{
    uint32_t size = 0;
    uint32_t baseSize = 8;   /* 64 Mb =  8MB */

    if ( !handle ) {
        BCM_DBG_ERR (("invalid argument: handle=0 \n" ));
        return (BCM_ERROR);
    }

    if (cs >= MEMC_CS_MAX)
    {
        BCM_DBG_ERR (("invalid argument: handle=0 \n" ));
        return (BCM_ERROR);
    }

    size = handle->mem_device[cs].dev_info.density;
    if (size > LPDDR2_DENSITY_64M && size <= LPDDR2_DENSITY_32G)
        size = baseSize << size;

    *memSize = size;

    return (BCM_SUCCESS);
}

/*
 * ===========================================================================
 * 
 *   Function Name: chal_memc_config_stats
 * 
 *   Description:
 *       Set configration for statistics collection
 * 
 * ===========================================================================
 */
MEMC_EXPORT BCM_ERR_CODE chal_memc_config_stats (
    CHAL_MEMC_HANDLE handle,
    uint32_t period
)
{
    uint32_t base_addr;

    base_addr = handle->memc_open_reg_base;

    BCM_REG_WRITE32 (handle, base_addr + CSR_STATISTICS_PERIOD_OFFSET, period);
    BCM_REG_WRITE32 (handle, base_addr + CSR_STATISTICS_CONTROL_OFFSET, 1);

    return (BCM_SUCCESS);
}

/*
 * ===========================================================================
 * 
 *   Function Name: chal_memc_get_stats
 * 
 *   Description:
 *       Get statistics collection if done
 * 
 * ===========================================================================
 */
MEMC_EXPORT BCM_ERR_CODE chal_memc_get_stats (
    CHAL_MEMC_HANDLE handle,
    CHAL_MEMC_STATS_T *pStats
)
{
    uint32_t base_addr = handle->memc_open_reg_base;

    if (!pStats) {
        BCM_DBG_ERR (("invalid argument: pStats==0\n"));
        return (BCM_ERROR);
    }

    /* if not done*/
    if ( ! (BCM_REG_READ32 (handle, base_addr + CSR_STATISTICS_READ_DATA_CYCLES_OFFSET)) ) {
        return (BCM_ERROR);
    }

    pStats->read_data = BCM_REG_READ32 (handle, base_addr + CSR_STATISTICS_READ_DATA_CYCLES_OFFSET);
    pStats->write_data = BCM_REG_READ32 (handle, base_addr + CSR_STATISTICS_WRITE_DATA_CYCLES_OFFSET);
    pStats->page_hit_data = BCM_REG_READ32 (handle, base_addr + CSR_STATISTICS_PAGE_HIT_DATA_CYCLES_OFFSET);
    pStats->page_miss_data = BCM_REG_READ32 (handle, base_addr + CSR_STATISTICS_PAGE_MISS_DATA_CYCLES_OFFSET);
    pStats->page_hit_transaction = BCM_REG_READ32 (handle, base_addr + CSR_STATISTICS_PAGE_HIT_TRANSACTIONS_OFFSET);
    pStats->page_miss_transaction = BCM_REG_READ32 (handle, base_addr + CSR_STATISTICS_PAGE_MISS_TRANSACTIONS_OFFSET);
    pStats->auto_ref = BCM_REG_READ32 (handle, base_addr + CSR_STATISTICS_AUTO_REF_CYCLES_OFFSET);
    pStats->bank_active = BCM_REG_READ32 (handle, base_addr + CSR_STATISTICS_BANK_ACTIVE_CYCLES_OFFSET);

    return (BCM_SUCCESS);
}
/*
 * ===========================================================================
 * 
 *   Function Name: chal_memc_init_burst_length
 * 
 *   Description:
 *       configures the memc transfer burst length
 * 
 * ===========================================================================
 */
static void chal_memc_init_burst_length(
    CHAL_MEMC_HANDLE handle
)
{
    uint8_t lenBurst = LPDDR2_BL8;
    uint8_t i = 0;
    MEMC_CS_CONNECTION cs;

    /* TODO: need to confirm if NVM needs to set burst length and what value */
    for (i=0; i<MEMC_CS_MAX; i++)
    {
        cs = handle->mem_device[i].dev_cs;
        if (cs != MEMC_CS_NONE && handle->mem_device[i].dev_type == MEMC_TYPE_DRAM)
        {
            /* mask bits 0,1,2 */
            lenBurst =  handle->mem_device[i].dev_config.MR1_device_feature1 & 0x7;

            /* configure burst length */
            chal_memc_config_burst_length(handle, lenBurst);
            break;
        }
    }
}

/* ===========================================================================
 * 
 *   Function Name: chal_memc_config_burst_length
 * 
 *   Description:
 *       configures the memc transfer burst length
 * 
 * ===========================================================================
 */

MEMC_EXPORT BCM_ERR_CODE chal_memc_config_burst_length(
    CHAL_MEMC_HANDLE handle,
    uint8_t blen
)
{
    uint8_t devfeature1, i;
    MEMC_CS_CONNECTION cs;

    for (i=0; i<MEMC_CS_MAX; i++)
    {
        cs = handle->mem_device[i].dev_cs;
        if (cs != MEMC_CS_NONE && handle->mem_device[i].dev_type == MEMC_TYPE_DRAM)
        {
            devfeature1 =  handle->mem_device[i].dev_config.MR1_device_feature1 & 0xF8;     /* mask bits 0,1,2 */
            devfeature1 = devfeature1 | blen;                      /* OR the burst length value */
            handle->mem_device[i].dev_config.MR1_device_feature1 = devfeature1;

            /* configure burst length */
            lpddr2_mode_reg_write (handle, cs, LPDDR2_MODE_REG_DEVICE_FEATURE1, devfeature1);
        }
    }
    return BCM_SUCCESS;
}

/*
 * ===========================================================================
 * 
 *   Function Name: chal_memc_config_sequence
 * 
 *   Description:
 *       configures the memc transfer consequence
 * 
 * ===========================================================================
 */
MEMC_EXPORT BCM_ERR_CODE chal_memc_config_sequence(
    CHAL_MEMC_HANDLE handle,
    uint8_t seq
)
{
    uint8_t devfeature1, i;
    MEMC_CS_CONNECTION cs;

    for (i=0; i<MEMC_CS_MAX; i++)
    {
        cs = handle->mem_device[i].dev_cs;
        if (cs != MEMC_CS_NONE && handle->mem_device[i].dev_type == MEMC_TYPE_DRAM)
        {
            devfeature1 = handle->mem_device[i].dev_config.MR1_device_feature1 & 0xF7;   /* mask bit3 */
            devfeature1 = devfeature1 | seq;                    /* OR the BT value */
            handle->mem_device[i].dev_config.MR1_device_feature1 = devfeature1;

            lpddr2_mode_reg_write (handle, cs, LPDDR2_MODE_REG_DEVICE_FEATURE1, devfeature1);
        }
    }

    return BCM_SUCCESS;
}

/*
 * ===========================================================================
 * 
 *   Function Name: chal_memc_config_wrap_nowrap
 * 
 *   Description:
 *       configures the memc transfer wrap mode
 * 
 * ===========================================================================
 */
MEMC_EXPORT BCM_ERR_CODE chal_memc_config_wrap_nowrap(
    CHAL_MEMC_HANDLE handle,
    uint8_t wrap
)
{
    uint8_t devfeature1, i;
    MEMC_CS_CONNECTION cs;

    for (i=0; i<MEMC_CS_MAX; i++)
    {
        cs = handle->mem_device[i].dev_cs;
        if (cs != MEMC_CS_NONE && handle->mem_device[i].dev_type == MEMC_TYPE_DRAM)
        {
            devfeature1 =  handle->mem_device[cs].dev_config.MR1_device_feature1 & 0xEF;  /* mask bit4 */
            devfeature1 = devfeature1 | wrap;                   /* OR the wrap value */
            handle->mem_device[cs].dev_config.MR1_device_feature1 = devfeature1;

            /* configure wrap */
            lpddr2_mode_reg_write (handle, cs, LPDDR2_MODE_REG_DEVICE_FEATURE1, devfeature1);
        }
    }

    return BCM_SUCCESS;
}

/*
 * ===========================================================================
 * 
 *   Function Name: chal_memc_meu_config_region
 * 
 *   Description:
 *       configures the memc security regions
 * 
 * ===========================================================================
 */
MEMC_EXPORT BCM_ERR_CODE chal_memc_meu_config_region(
    CHAL_MEMC_HANDLE handle,
    uint8_t  rgn_num,
    uint32_t rgn_type,
    uint32_t start_addr,
    uint32_t end_addr
)
{
   uint32_t mask;
   uint32_t shift;
   uint32_t start_addr_reg;
   uint32_t end_addr_reg;
   uint32_t rgn_ctrl_reg;
   uint32_t value;
   uint32_t enable = 0;

   switch(rgn_num)
    {
        case REGION_0:
            start_addr_reg  = handle->memc_secure_reg_base + SEC_RGN_0_START_ADDR_OFFSET;
            end_addr_reg = handle->memc_secure_reg_base + SEC_RGN_0_END_ADDR_OFFSET;
            rgn_ctrl_reg = handle->memc_secure_reg_base + SEC_RGN_0_CTL_OFFSET;
            shift   = SEC_RGN_0_CTL_RGN_0_MEM_TYPE_SHIFT;
            mask    = SEC_RGN_0_CTL_RGN_0_MEM_TYPE_MASK;
            break;
        case REGION_1:
            start_addr_reg  = handle->memc_secure_reg_base + SEC_RGN_1_START_ADDR_OFFSET;
            end_addr_reg = handle->memc_secure_reg_base + SEC_RGN_1_END_ADDR_OFFSET;
            rgn_ctrl_reg = handle->memc_secure_reg_base + SEC_RGN_1_CTL_OFFSET;
            shift   = SEC_RGN_1_CTL_RGN_1_MEM_TYPE_SHIFT;
            mask    = SEC_RGN_1_CTL_RGN_1_MEM_TYPE_MASK;
            break;
        case REGION_2:
            start_addr_reg  = handle->memc_secure_reg_base + SEC_RGN_2_START_ADDR_OFFSET;
            end_addr_reg = handle->memc_secure_reg_base + SEC_RGN_2_END_ADDR_OFFSET;
            rgn_ctrl_reg = handle->memc_secure_reg_base + SEC_RGN_2_CTL_OFFSET;
            shift   = SEC_RGN_2_CTL_RGN_2_MEM_TYPE_SHIFT;
            mask    = SEC_RGN_2_CTL_RGN_2_MEM_TYPE_MASK;
            break;
        case REGION_3:
            start_addr_reg  = handle->memc_secure_reg_base + SEC_RGN_3_START_ADDR_OFFSET;
            end_addr_reg = handle->memc_secure_reg_base + SEC_RGN_3_END_ADDR_OFFSET;
            rgn_ctrl_reg = handle->memc_secure_reg_base + SEC_RGN_3_CTL_OFFSET;
            shift   = SEC_RGN_3_CTL_RGN_3_MEM_TYPE_SHIFT;
            mask    = SEC_RGN_3_CTL_RGN_3_MEM_TYPE_MASK;
            break;
        case REGION_4:
            start_addr_reg  = handle->memc_secure_reg_base + SEC_RGN_4_START_ADDR_OFFSET;
            end_addr_reg = handle->memc_secure_reg_base + SEC_RGN_4_END_ADDR_OFFSET;
            rgn_ctrl_reg = handle->memc_secure_reg_base + SEC_RGN_4_CTL_OFFSET;
            shift   = SEC_RGN_4_CTL_RGN_4_MEM_TYPE_SHIFT;
            mask    = SEC_RGN_4_CTL_RGN_4_MEM_TYPE_MASK;
            break;
        case REGION_5:
            start_addr_reg  = handle->memc_secure_reg_base + SEC_RGN_5_START_ADDR_OFFSET;
            end_addr_reg = handle->memc_secure_reg_base + SEC_RGN_5_END_ADDR_OFFSET;
            rgn_ctrl_reg = handle->memc_secure_reg_base + SEC_RGN_5_CTL_OFFSET;
            shift   = SEC_RGN_5_CTL_RGN_5_MEM_TYPE_SHIFT;
            mask    = SEC_RGN_5_CTL_RGN_5_MEM_TYPE_MASK;
            break;
        case REGION_6:
            start_addr_reg  = handle->memc_secure_reg_base + SEC_RGN_6_START_ADDR_OFFSET;
            end_addr_reg = handle->memc_secure_reg_base + SEC_RGN_6_END_ADDR_OFFSET;
            rgn_ctrl_reg = handle->memc_secure_reg_base + SEC_RGN_6_CTL_OFFSET;
            shift   = SEC_RGN_6_CTL_RGN_6_MEM_TYPE_SHIFT;
            mask    = SEC_RGN_6_CTL_RGN_6_MEM_TYPE_MASK;
            break;
        case REGION_7:
            start_addr_reg  = handle->memc_secure_reg_base + SEC_RGN_7_START_ADDR_OFFSET;
            end_addr_reg = handle->memc_secure_reg_base + SEC_RGN_7_END_ADDR_OFFSET;
            rgn_ctrl_reg = handle->memc_secure_reg_base + SEC_RGN_7_CTL_OFFSET;
            shift   = SEC_RGN_7_CTL_RGN_7_MEM_TYPE_SHIFT;
            mask    = SEC_RGN_7_CTL_RGN_7_MEM_TYPE_MASK;
            break;
        default: return BCM_ERROR;  /* Do nothing and return error */
    }

   switch(rgn_type)
   {
   case RGN_SECURE:
   case RGN_SUPERVISOR:
       enable = 1;
       break;
   case RGN_TZ:
   case RGN_USER:
       enable = 0;
       break;
   }

    /* Config region start address */
    BCM_REG_WRITE32(handle, start_addr_reg, start_addr);
    BCM_REG_WRITE32(handle, end_addr_reg, end_addr);
    value = BCM_REG_READ32(handle, rgn_ctrl_reg);
    value &= ~mask;                                 /* mask the mem type bit */
    value &= ~0x1;                                  /* mask the enable bit */
    value = (value | (rgn_type << shift) | enable);          /* or region type  */
    BCM_REG_WRITE32(handle, rgn_ctrl_reg, value);   /* update region type */

    return BCM_SUCCESS;
}

/*
 * ===========================================================================
 * 
 *   Function Name: NAME:chal_memc_meu_enable_disable_region
 * 
 *   Description:
 *       enable/disable memc security regions
 * 
 * ===========================================================================
 */
MEMC_EXPORT BCM_ERR_CODE chal_memc_meu_enable_disable_region(
    CHAL_MEMC_HANDLE handle,
    uint8_t  rgn_num,
    _Bool enable
)
{
    uint32_t ctl_reg_addr;
    uint32_t mask;
    uint32_t value;

   switch(rgn_num)
    {
        case REGION_0:
            ctl_reg_addr  = handle->memc_secure_reg_base + SEC_RGN_0_CTL_OFFSET;
            mask          = SEC_RGN_0_CTL_RGN_0_EN_MASK;
            break;
        case REGION_1:
            ctl_reg_addr  = handle->memc_secure_reg_base + SEC_RGN_1_CTL_OFFSET;
            mask          = SEC_RGN_1_CTL_RGN_1_EN_MASK;
            break;
        case REGION_2:
            ctl_reg_addr  = handle->memc_secure_reg_base + SEC_RGN_2_CTL_OFFSET;
            mask          = SEC_RGN_2_CTL_RGN_2_EN_MASK;
            break;
        case REGION_3:
            ctl_reg_addr  = handle->memc_secure_reg_base + SEC_RGN_3_CTL_OFFSET;
            mask          = SEC_RGN_3_CTL_RGN_3_EN_MASK;
            break;
        case REGION_4:
            ctl_reg_addr  = handle->memc_secure_reg_base + SEC_RGN_4_CTL_OFFSET;
            mask          = SEC_RGN_4_CTL_RGN_4_EN_MASK;
            break;
        case REGION_5:
            ctl_reg_addr  = handle->memc_secure_reg_base + SEC_RGN_5_CTL_OFFSET;
            mask          = SEC_RGN_5_CTL_RGN_5_EN_MASK;
            break;
        case REGION_6:
            ctl_reg_addr  = handle->memc_secure_reg_base + SEC_RGN_6_CTL_OFFSET;
            mask          = SEC_RGN_6_CTL_RGN_6_EN_MASK;
            break;
        case REGION_7:
            ctl_reg_addr  = handle->memc_secure_reg_base + SEC_RGN_7_CTL_OFFSET;
            mask          = SEC_RGN_7_CTL_RGN_7_EN_MASK;
            break;
        default: return BCM_ERROR;  /* Do nothing and return error */

    }

    /* Read the control register value  */
    value = BCM_REG_READ32(handle, ctl_reg_addr);

    /* Mask the enable bit( make enable bit 0)  */
    value = (~mask)& value;

    /* If enable required, then set the bit.  */
    if(enable == TRUE)
    {
        value = (value | mask );
    }
    else
    {
     /* Here do nothing because this bit is cleared above when masked */
    }

    /* Write back value to register  */
    BCM_REG_WRITE32(handle, ctl_reg_addr, value);

    return BCM_SUCCESS;
}

/****************************************************************************
*
* NAME:chal_memc_meu_enable_region_data_scramble
*
*
*  Description: Enables data scrambing for a region
*
*
*  Parameters:
*
*
*  Returns:
*
*  Notes:
*
****************************************************************************/
MEMC_EXPORT BCM_ERR_CODE chal_memc_meu_enable_region_data_scramble(
    CHAL_MEMC_HANDLE handle,
    uint8_t  rgn_num
)
{
    uint32_t ctl_reg_addr;
    uint32_t mask;
    uint32_t value;

   switch(rgn_num)
    {
        case REGION_0:
            ctl_reg_addr  = handle->memc_secure_reg_base + SEC_RGN_0_CTL_OFFSET;
            mask          = SEC_RGN_0_CTL_RGN_0_DATA_SCR_EN_MASK;
            break;
        case REGION_1:
            ctl_reg_addr  = handle->memc_secure_reg_base + SEC_RGN_1_CTL_OFFSET;
            mask          = SEC_RGN_1_CTL_RGN_1_DATA_SCR_EN_MASK;
            break;
        case REGION_2:
            ctl_reg_addr  = handle->memc_secure_reg_base + SEC_RGN_2_CTL_OFFSET;
            mask          = SEC_RGN_2_CTL_RGN_2_DATA_SCR_EN_MASK;
            break;
        case REGION_3:
            ctl_reg_addr  = handle->memc_secure_reg_base + SEC_RGN_3_CTL_OFFSET;
            mask          = SEC_RGN_3_CTL_RGN_3_DATA_SCR_EN_MASK;
            break;
        case REGION_4:
            ctl_reg_addr  = handle->memc_secure_reg_base + SEC_RGN_4_CTL_OFFSET;
            mask          = SEC_RGN_4_CTL_RGN_4_DATA_SCR_EN_MASK;
            break;
        case REGION_5:
            ctl_reg_addr  = handle->memc_secure_reg_base + SEC_RGN_5_CTL_OFFSET;
            mask          = SEC_RGN_5_CTL_RGN_5_DATA_SCR_EN_MASK;
            break;
        case REGION_6:
            ctl_reg_addr  = handle->memc_secure_reg_base + SEC_RGN_6_CTL_OFFSET;
            mask          = SEC_RGN_6_CTL_RGN_6_DATA_SCR_EN_MASK;
            break;
        case REGION_7:
            ctl_reg_addr  = handle->memc_secure_reg_base + SEC_RGN_7_CTL_OFFSET;
            mask          = SEC_RGN_7_CTL_RGN_7_DATA_SCR_EN_MASK;
            break;
        default: return BCM_ERROR;  /* Do nothing and return error */

    }

    /* Read the control register value  */
    value = BCM_REG_READ32(handle, ctl_reg_addr);

    /* Enable and write back value to register  */
    BCM_REG_WRITE32(handle, ctl_reg_addr, value | mask);

    return BCM_SUCCESS;
}

/*
 * ===========================================================================
 * 
 *   Function Name: chal_memc_meu_enable_global_data_scrambling
 * 
 *   Description:
 *       enable global data scrambling
 * 
 * ===========================================================================
 */
MEMC_EXPORT void chal_memc_meu_enable_global_data_scrambling(
    CHAL_MEMC_HANDLE handle

)
{
    uint32_t ctl_reg_addr;
    uint32_t value;

    /* Get the register address  */
    ctl_reg_addr  = handle->memc_secure_reg_base + SEC_GLB_CTL_OFFSET;

    /* use read, modify and write */
    value = BCM_REG_READ32(handle, ctl_reg_addr);
    /* Mask the enable bit( make enable bit 0)  */
/*    value = value & (~SEC_GLB_CTL_GLB_DATA_SCR_EN_MASK);*/
    value = value | (SEC_GLB_CTL_GLB_DATA_SCR_EN_MASK);

    /* Write back value to register  */
    BCM_REG_WRITE32(handle, ctl_reg_addr, value);
}

/*
 * ===========================================================================
 * 
 *   Function Name: chal_memc_meu_enable_global_addr_scrambling
 * 
 *   Description:
 *       enable global address scrambling
 * 
 * ===========================================================================
 */
MEMC_EXPORT void chal_memc_meu_enable_global_addr_scrambling(
    CHAL_MEMC_HANDLE handle

)
{
    uint32_t ctl_reg_addr;
    uint32_t value;

    /* Get the register address  */
    ctl_reg_addr  = handle->memc_secure_reg_base + SEC_GLB_CTL_OFFSET;

    /* use read, modify and write */
    value = BCM_REG_READ32(handle, ctl_reg_addr);
    /* Mask the enable bit( make enable bit 1)  */
/*    value = value & (~SEC_GLB_CTL_GLB_ADDR_SCR_EN_MASK);*/
    value = value | (SEC_GLB_CTL_GLB_ADDR_SCR_EN_MASK);

    /* Write back value to register  */
    BCM_REG_WRITE32(handle, ctl_reg_addr, value);
}

/*
 * ===========================================================================
 * 
 *   Function Name: chal_memc_meu_enable_global_gid_check
 * 
 *   Description:
 *       enable group ID checking
 * 
 * ===========================================================================
 */
MEMC_EXPORT void chal_memc_meu_enable_global_gid_check(
    CHAL_MEMC_HANDLE handle

)
{
    uint32_t ctl_reg_addr;
    uint32_t value;

    /* Get the register address  */
    ctl_reg_addr  = handle->memc_secure_reg_base + SEC_GLB_CTL_OFFSET;

    /* use read, modify and write */
    value = BCM_REG_READ32(handle, ctl_reg_addr);
    /* Mask the enable bit( make enable bit 1)  */

    /* This will set(make 1) bit 4  */
    value = value | SEC_GLB_CTL_GLB_GID_CHECK_EN_MASK;

    /* Write back value to register  */
    BCM_REG_WRITE32(handle, ctl_reg_addr, value);
}

/*
 * ===========================================================================
 * 
 *   Function Name: chal_memc_meu_disable_global_gid_check
 * 
 *   Description:
 *       disable group ID checking
 * 
 * ===========================================================================
 */
MEMC_EXPORT void chal_memc_meu_disable_global_gid_check(
    CHAL_MEMC_HANDLE handle

)
{
    uint32_t ctl_reg_addr;
    uint32_t value;

    /* Get the register address  */
    ctl_reg_addr  = handle->memc_secure_reg_base + SEC_GLB_CTL_OFFSET;

    /* use read, modify and write */
    value = BCM_REG_READ32(handle, ctl_reg_addr);
    /* Mask the enable bit( make enable bit 1)  */

    /* This will reset( make 0) bit 4  */
    value &= ~SEC_GLB_CTL_GLB_GID_CHECK_EN_MASK;

    /* Write back value to register  */
    BCM_REG_WRITE32(handle, ctl_reg_addr, value);
}

/*
 * ===========================================================================
 * 
 *   Function Name: chal_memc_meu_config_keys
 * 
 *   Description:
 *       Programs address and data key types
 * 
 * ===========================================================================
 */
MEMC_EXPORT BCM_ERR_CODE chal_memc_meu_config_keys(
    CHAL_MEMC_HANDLE handle,
    CHAL_MEU_KEY_T key,
    uint32_t key_value
)
{
   uint32_t reg_addr;

    switch(key)
    {
        case DATA_KEY_WK_IN:
            reg_addr = handle->memc_secure_reg_base + SEC_DATA_KEY_WK_IN_OFFSET;
        break;
        case DATA_KEY_WK_OUT:
            reg_addr = handle->memc_secure_reg_base + SEC_DATA_KEY_WK_OUT_OFFSET;
        break;
        case ADDR_KEY_WK_IN:
           reg_addr = handle->memc_secure_reg_base + SEC_ADDR_KEY_WK_IN_OFFSET;
        break;
        case ADDR_KEY_WK_OUT:
            reg_addr = handle->memc_secure_reg_base + SEC_ADDR_KEY_WK_OUT_OFFSET;
        break;
        case RND_0_DATA_KEY:
            reg_addr = handle->memc_secure_reg_base + SEC_RND_0_DATA_KEY_OFFSET;
        break;
        case RND_1_DATA_KEY:
            reg_addr = handle->memc_secure_reg_base + SEC_RND_1_DATA_KEY_OFFSET;
        break;
        case RND_2_DATA_KEY:
            reg_addr = handle->memc_secure_reg_base + SEC_RND_2_DATA_KEY_OFFSET;
        break;
        case RND_0_ADDR_KEY:
            reg_addr = handle->memc_secure_reg_base + SEC_RND_0_ADDR_KEY_OFFSET;
        break;
        case RND_1_ADDR_KEY:
            reg_addr = handle->memc_secure_reg_base + SEC_RND_1_ADDR_KEY_OFFSET;
        break;
        case RND_2_ADDR_KEY:
            reg_addr = handle->memc_secure_reg_base + SEC_RND_2_ADDR_KEY_OFFSET;
        break;
        default: return BCM_ERROR;
    }

    BCM_REG_WRITE32(handle, reg_addr, key_value);

    return BCM_SUCCESS;
}

/*
 * ===========================================================================
 * 
 *   Function Name: chal_memc_meu_program_scrambling_scheme
 * 
 *   Description:
 *       Programs address scrambling scheme
 * 
 * ===========================================================================
 */
MEMC_EXPORT void chal_memc_meu_program_scrambling_scheme(
    CHAL_MEMC_HANDLE handle,
    uint32_t scheme_value
)
{
    uint32_t reg_addr;

    reg_addr = handle->memc_secure_reg_base + SEC_SW_ADDR_SCR_SCH_SEL_OFFSET;
    scheme_value = scheme_value & SEC_SW_ADDR_SCR_SCH_SEL_SW_ADDR_SCR_SCH_SEL_MASK;
    BCM_REG_WRITE32(handle, reg_addr, scheme_value);
}

/*
 * ===========================================================================
 * 
 *   Function Name: chal_memc_meu_program_scrambling_mask
 * 
 *   Description:
 *       Programs address scrambling mask
 * 
 * ===========================================================================
 */
MEMC_EXPORT void chal_memc_meu_program_scrambling_mask(
    CHAL_MEMC_HANDLE handle,
    uint32_t mask_value
)
{
    uint32_t reg_addr;

    reg_addr = handle->memc_secure_reg_base + SEC_ADDR_SCR_MASK_OFFSET;
    mask_value = mask_value & SEC_ADDR_SCR_MASK_ADDR_SCR_MASK_MASK;
    BCM_REG_WRITE32(handle, reg_addr, mask_value);
}

/*
 * ===========================================================================
 * 
 *   Function Name: chal_memc_meu_get_access_vio_info
 * 
 *   Description:
 *       returns access violation info
 * 
 * ===========================================================================
 */
MEMC_EXPORT BCM_ERR_CODE chal_memc_meu_get_access_vio_info(
    CHAL_MEMC_HANDLE handle,
    CHAL_MEU_ACCESS_VIO_INFO_T *info
)
{
    uint32_t reg_addr;
    uint32_t value;
    uint32_t tempdata;

    /* Get address of register where access violation address is stored */
    reg_addr = handle->memc_secure_reg_base + SEC_ACC_VIO_ADDR_OFFSET;

    /* Read the address for which access violation happened */
    value = BCM_REG_READ32(handle, reg_addr);
    info->acc_vio_addr = value;

    /* Get address of register where access violatoin info is stored */
    reg_addr = handle->memc_secure_reg_base + SEC_ACC_VIO_INFO_OFFSET;
    /* Read access violation info */
    value = BCM_REG_READ32(handle, reg_addr);

    /* read interrupt flag */
    info->interrupted = ( value & SEC_ACC_VIO_INFO_ACC_VIO_INTR_MASK ) ? TRUE : FALSE;

    /* If bit0=1; read type access violation else write type access violation   */
    if((value & SEC_ACC_VIO_INFO_ACC_VIO_TYPE_MASK) == SEC_ACC_VIO_INFO_ACC_VIO_TYPE_MASK)
        info->vio_type = VIO_WRITE;
    else
        info->vio_type = VIO_READ;


    /*Get the client ID  */
    tempdata = value & SEC_ACC_VIO_INFO_ACC_VIO_CLIENT_ID_MASK;
    tempdata = (tempdata >> SEC_ACC_VIO_INFO_ACC_VIO_CLIENT_ID_SHIFT);
    info->client_id = tempdata;

    /* Get the region number for which access violation occured */
    tempdata = value & SEC_ACC_VIO_INFO_ACC_VIO_REGION_MASK;
    tempdata = (tempdata >> SEC_ACC_VIO_INFO_ACC_VIO_REGION_SHIFT);

    if(tempdata & 0x00000001)
        info->region_num[REGION_0] = TRUE;
    else
        info->region_num[REGION_0] = FALSE;

    if(tempdata & 0x00000002)
        info->region_num[REGION_1] = TRUE;
    else
        info->region_num[REGION_1] = FALSE;

    if(tempdata & 0x00000004)
        info->region_num[REGION_2] = TRUE;
    else
        info->region_num[REGION_2] = FALSE;

    if(tempdata & 0x00000008)
        info->region_num[REGION_3] = TRUE;
    else
        info->region_num[REGION_3] = FALSE;

    if(tempdata & 0x00000010)
        info->region_num[REGION_4] = TRUE;
    else
        info->region_num[REGION_4] = FALSE;

    if(tempdata & 0x00000020)
        info->region_num[REGION_5] = TRUE;
    else
        info->region_num[REGION_5] = FALSE;

    if(tempdata & 0x00000040)
        info->region_num[REGION_6] = TRUE;
    else
        info->region_num[REGION_6] = FALSE;

    if(tempdata & 0x00000080)
        info->region_num[REGION_7] = TRUE;
    else
        info->region_num[REGION_7] = FALSE;


    /*Get the port ID  */
    tempdata = value & SEC_ACC_VIO_INFO_ACC_VIO_PORT_ID_MASK;
    tempdata = (tempdata >> SEC_ACC_VIO_INFO_ACC_VIO_PORT_ID_SHIFT);
    info->port_id = tempdata;

    /* Get reason for access violation */
    tempdata = value & SEC_ACC_VIO_INFO_ACC_VIO_REASON_MASK;
    tempdata = (tempdata >> SEC_ACC_VIO_INFO_ACC_VIO_REASON_SHIFT);

    if(tempdata == 0x00000003)
        info->acc_vio_reason = GROUP_ID_MATCHED;
    else if(tempdata == 0x00000002)
        info->acc_vio_reason = REGION_MATCHED;
    else if(tempdata == 0x00000001)
        info->acc_vio_reason = SECURITY_VIO;
    else
        info->acc_vio_reason = RESERVED;

    return BCM_SUCCESS;
}

/*
 * ===========================================================================
 * 
 *   Function Name: chal_memc_meu_lock_global_gid_check
 * 
 *   Description:
 *       locks the global GID check enable bit
 * 
 * ===========================================================================
 */
MEMC_EXPORT void chal_memc_meu_lock_global_gid_check(
    CHAL_MEMC_HANDLE handle
)
{
    uint32_t reg_addr;
    uint32_t value;

    reg_addr = handle->memc_secure_reg_base + SEC_GLB_CTL_OFFSET;
    value = BCM_REG_READ32(handle, reg_addr);
    value |= SEC_GLB_CTL_GLB_GID_CHECK_EN_LOCK_MASK;

    BCM_REG_WRITE32(handle, reg_addr, value);
}

/*
 * ===========================================================================
 * 
 *   Function Name: chal_memc_meu_is_gid_check_bit_enabled
 * 
 *   Description:
 *       returns the current status of GID check bit
 * 
 * ===========================================================================
 */
MEMC_EXPORT _Bool chal_memc_meu_is_gid_check_bit_enabled(
    CHAL_MEMC_HANDLE handle
)
{
    uint32_t reg_addr;
    uint32_t value;

    reg_addr = handle->memc_secure_reg_base + SEC_GLB_CTL_OFFSET;
    value = BCM_REG_READ32(handle, reg_addr);
    value = value & SEC_GLB_CTL_GLB_GID_CHECK_EN_MASK;

    if( value == SEC_GLB_CTL_GLB_GID_CHECK_EN_MASK)
        return TRUE;

    return FALSE;
}

/*
 * ===========================================================================
 * 
 *   Function Name: chal_memc_meu_is_gid_check_lock_bit_enabled
 * 
 *   Description:
 *       returns the current status of GID check lock bit
 * 
 * ===========================================================================
 */
MEMC_EXPORT _Bool chal_memc_meu_is_gid_check_lock_bit_enabled(
    CHAL_MEMC_HANDLE handle
)
{
    uint32_t reg_addr;
    uint32_t value;

    reg_addr = handle->memc_secure_reg_base + SEC_GLB_CTL_OFFSET;
    value = BCM_REG_READ32(handle, reg_addr);
    value = value & SEC_GLB_CTL_GLB_GID_CHECK_EN_LOCK_MASK;

    if( value == SEC_GLB_CTL_GLB_GID_CHECK_EN_LOCK_MASK)
        return TRUE;

    return FALSE;
}

/*
 * ===========================================================================
 * 
 *   Function Name: chal_memc_read_register
 * 
 *   Description:
 *       reads a memc register content
 * 
 * ===========================================================================
 */
MEMC_EXPORT uint32_t chal_memc_read_register(
    CHAL_MEMC_HANDLE handle,
    MEMC_REG_BLOCK reg_base,
    uint32_t reg_addr
    )
{
    if (handle)
    {
        switch(reg_base)
        {
        case MEMC_REG_BLOCK_SEC:
            return BCM_REG_READ32(handle, handle->memc_secure_reg_base+reg_addr);
        case MEMC_REG_BLOCK_CSR:
            return BCM_REG_READ32(handle, handle->memc_open_reg_base+reg_addr);
        case MEMC_REG_BLOCK_APHY:
            return BCM_REG_READ32(handle, handle->memc_aphy_reg_base+reg_addr);
        case MEMC_REG_BLOCK_DPHY:
            return BCM_REG_READ32(handle, handle->memc_dphy_reg_base+reg_addr);
        default:
            return 0xFFFFFFFF;
        }
    }

    return 0xFFFFFFFF;
}

/*
 * ===========================================================================
 * 
 *   Function Name: chal_memc_write_register
 * 
 *   Description:
 *       writes to a memc register
 * 
 * ===========================================================================
 */
MEMC_EXPORT _Bool chal_memc_write_register(
    CHAL_MEMC_HANDLE handle,
    MEMC_REG_BLOCK reg_base,
    uint32_t reg_offset,
    uint32_t reg_value
    )
{
    if (handle)
    {
        switch(reg_base)
        {
        case MEMC_REG_BLOCK_SEC:
            BCM_REG_WRITE32(handle, handle->memc_secure_reg_base+reg_offset, reg_value);
            break;
        case MEMC_REG_BLOCK_CSR:
            BCM_REG_WRITE32(handle, handle->memc_open_reg_base+reg_offset, reg_value);
            break;
        case MEMC_REG_BLOCK_APHY:
            BCM_REG_WRITE32(handle, handle->memc_aphy_reg_base+reg_offset, reg_value);
            break;
        case MEMC_REG_BLOCK_DPHY:
            BCM_REG_WRITE32(handle, handle->memc_dphy_reg_base+reg_offset, reg_value);
            break;
        default:
            return FALSE;
        }
        return TRUE;
    }

    return FALSE;
}

/*
 * ===========================================================================
 * 
 *   Function Name: chal_memc_write_register_2
 * 
 *   Description:
 *       writes to a memc register
 * 
 * ===========================================================================
 */
MEMC_EXPORT _Bool chal_memc_write_register_2(
    CHAL_MEMC_HANDLE handle,
    uint32_t reg_addr,
    uint32_t reg_value
    )
{
    if (handle)
    {
        if (((reg_addr & SYS_EMI_SECURE_BASE_ADDR) == SYS_EMI_SECURE_BASE_ADDR) ||
            ((reg_addr & SYS_EMI_OPEN_BASE_ADDR) == SYS_EMI_OPEN_BASE_ADDR) ||
            ((reg_addr & SYS_EMI_DDR3_CTL_BASE_ADDR) == SYS_EMI_DDR3_CTL_BASE_ADDR) ||
            ((reg_addr & SYS_EMI_DDR3_PHY_ADDR_CTL_BASE_ADDR) == SYS_EMI_DDR3_PHY_ADDR_CTL_BASE_ADDR))
        {
            BCM_REG_WRITE32(handle, reg_addr, reg_value);
            return TRUE;
        }
    }

    return FALSE;
}

/*
 * ===========================================================================
 * 
 *   Function Name: chal_memc_read_mode_register
 * 
 *   Description:
 *       reads a DDR mode register content
 * 
 * ===========================================================================
 */
MEMC_EXPORT uint32_t chal_memc_read_mode_register(
    CHAL_MEMC_HANDLE handle,
    MEMC_CS_CONNECTION cs,
    uint32_t reg_addr
    )
{
    uint32_t val = 0;
    if ( handle->memc_ddr_type == MEMC_DDR_TYPE_DDR3 )
    {
       val = chal_memc_ddr3_read_mode_register ( (void*)handle, (uint32_t)cs, reg_addr );
    }
    else if ( handle->memc_ddr_type == MEMC_DDR_TYPE_LPDDR2 )
    {
       lpddr2_mode_reg_read (handle, cs, reg_addr, &val);
    }

    return val;
}

/*
 * ===========================================================================
 * 
 *   Function Name: chal_memc_write_mode_register
 * 
 *   Description:
 *       write to a DDR mode register
 * 
 * ===========================================================================
 */
MEMC_EXPORT _Bool chal_memc_write_mode_register(
    CHAL_MEMC_HANDLE handle,
    MEMC_CS_CONNECTION cs,
    uint32_t reg_addr,
    uint32_t value
    )
{
    if ( handle->memc_ddr_type == MEMC_DDR_TYPE_DDR3 )
    {
       return chal_memc_ddr3_write_mode_register( (void*)handle, (uint32_t)cs, reg_addr, value );
    }
    else if ( handle->memc_ddr_type == MEMC_DDR_TYPE_LPDDR2 )
    {
       if (BCM_SUCCESS == lpddr2_mode_reg_write (handle, cs, reg_addr, value))
       {
           return TRUE;
       }
    }
    return FALSE;
}

/*
 * ===========================================================================
 * 
 *   Function Name: chal_memc_get_ddr_pll_status
 * 
 *   Description:
 *       get the memc DDR PLL status
 * 
 * ===========================================================================
 */
MEMC_EXPORT uint32_t chal_memc_get_ddr_pll_status(
    CHAL_MEMC_HANDLE handle
    )
{
    uint32_t value;

    value = BCM_REG_READ32(handle, handle->memc_aphy_reg_base + APHY_CSR_DDR_PLL_LOCK_STATUS_OFFSET);
    value = value & APHY_CSR_DDR_PLL_LOCK_STATUS_LOCK_BIT_MASK;

    return value;
}

/*
 * ===========================================================================
 * 
 *   Function Name: chal_memc_get_master_dll_output
 * 
 *   Description:
 *       get the memc master DDR DLL status
 * 
 * ===========================================================================
 */
MEMC_EXPORT _Bool chal_memc_get_master_dll_output(
    CHAL_MEMC_HANDLE handle,
    CHAL_MEMC_MASTER_DLL_STATUS_T* status
    )
{
    status->CSR_CURRENT_FREQUENCY_STATE = BCM_REG_READ32(handle, handle->memc_open_reg_base + CSR_CURRENT_FREQUENCY_STATE_OFFSET);

    status->APHY_CSR_ADDR_MASTER_DLL_OUTPUT = BCM_REG_READ32(handle, handle->memc_aphy_reg_base + APHY_CSR_ADDR_MASTER_DLL_OUTPUT_OFFSET);

    status->APHY_CSR_GLOBAL_ADDR_DLL_LOCK_STATUS = BCM_REG_READ32(handle, handle->memc_aphy_reg_base + APHY_CSR_GLOBAL_ADDR_DLL_LOCK_STATUS_OFFSET);

    status->DPHY_CSR_BYTE0_MASTER_DLL_OUTPUT = BCM_REG_READ32(handle, handle->memc_dphy_reg_base + DPHY_CSR_BYTE0_MASTER_DLL_OUTPUT_OFFSET);

    status->DPHY_CSR_BYTE1_MASTER_DLL_OUTPUT = BCM_REG_READ32(handle, handle->memc_dphy_reg_base + DPHY_CSR_BYTE1_MASTER_DLL_OUTPUT_OFFSET);

    status->DPHY_CSR_GLOBAL_MASTER_DLL_LOCK_STATUS = BCM_REG_READ32(handle, handle->memc_dphy_reg_base + DPHY_CSR_GLOBAL_MASTER_DLL_LOCK_STATUS_OFFSET);

    return TRUE;
}

/*
 * ===========================================================================
 * 
 *   Function Name: chal_memc_enable_slave_dll_lock
 * 
 *   Description:
 *       lock the memc slave DLL
 * 
 * ===========================================================================
 */
MEMC_EXPORT _Bool chal_memc_enable_slave_dll_lock(
    CHAL_MEMC_HANDLE handle,
    uint32_t lock
    )
{
    uint32_t value;

    value = BCM_REG_READ32(handle, handle->memc_open_reg_base + CSR_ATE_MODE_CONTROL_OFFSET);
    if (lock)
        value |= CSR_ATE_MODE_CONTROL_SLAVE_DLL_LOCK_ENABLE_MASK;
    else
        value &= ~CSR_ATE_MODE_CONTROL_SLAVE_DLL_LOCK_ENABLE_MASK;

    BCM_REG_WRITE32(handle, handle->memc_open_reg_base + CSR_ATE_MODE_CONTROL_OFFSET, value);

    return TRUE;
}

/*
 * ===========================================================================
 * 
 *   Function Name: chal_memc_set_zq_calibration
 * 
 *   Description:
 *       enable/disable memc periodic ZQ calibration
 * 
 * ===========================================================================
 */
MEMC_EXPORT _Bool chal_memc_set_zq_calibration(
    CHAL_MEMC_HANDLE handle,
    uint32_t enable)
{
    uint32_t value, regAddr;

    if (handle)
    {
        regAddr = handle->memc_open_reg_base + CSR_PERIODIC_ZQ_CALIBRATION_CONTROL_OFFSET;
        value = BCM_REG_READ32(handle, regAddr);

        if (enable)
        {
            if ( handle->mem_device[MEMC_CS_0].dev_config.aphy_zq_calibration )
            {
               value |= CSR_PERIODIC_ZQ_CALIBRATION_CONTROL_ENABLE_DDR_APHY_ZQ_CALIBRATION_MASK;
            }
            
            if ( handle->mem_device[MEMC_CS_0].dev_config.dphy_zq_calibration )
            {
               value |= CSR_PERIODIC_ZQ_CALIBRATION_CONTROL_ENABLE_DDR_DPHY_ZQ_CALIBRATION_MASK;
            }
            
            if ( handle->mem_device[MEMC_CS_0].dev_config.device_zq_calibration )
            {
               value |= CSR_PERIODIC_ZQ_CALIBRATION_CONTROL_ENABLE_DDR_CS0_CALIBRATION_MASK;
            }

            /* Only configure CS1 if mounted */
            if ( (handle->mem_device[MEMC_CS_1].dev_cs != MEMC_CS_NONE) && 
                  handle->mem_device[MEMC_CS_1].dev_config.device_zq_calibration )
            {
               value |= CSR_PERIODIC_ZQ_CALIBRATION_CONTROL_ENABLE_DDR_CS1_CALIBRATION_MASK;
            }

            /* If the device is Micron MT42L64M32D1KL-25IT:A then there is a known problem
             * with ZQ Short Calibration that does not meet 90ns spec.  Disable ZQ Short Calibration for these parts */
            if (handle->mem_device[MEMC_CS_0].dev_config.MR5_basic_config1 == LPDDR2_ID_MICRON &&
                handle->mem_device[MEMC_CS_0].dev_config.MR6_basic_config2 == 0 &&
                handle->mem_device[MEMC_CS_0].dev_config.MR7_basic_config3 == 0 &&
                handle->mem_device[MEMC_CS_0].dev_config.MR8_basic_config4 == ((LPDDR2_WIDTH_X32<<6) | (LPDDR2_DENSITY_2G<<2) | LPDDR2_TYPE_S4) )
            {
                value &= ~CSR_PERIODIC_ZQ_CALIBRATION_CONTROL_ENABLE_DDR_CS0_CALIBRATION_MASK;
            }    
            if (handle->mem_device[MEMC_CS_1].dev_config.MR5_basic_config1 == LPDDR2_ID_MICRON &&
                handle->mem_device[MEMC_CS_1].dev_config.MR6_basic_config2 == 0 &&
                handle->mem_device[MEMC_CS_1].dev_config.MR7_basic_config3 == 0 &&
                handle->mem_device[MEMC_CS_1].dev_config.MR8_basic_config4 == ((LPDDR2_WIDTH_X32<<6) | (LPDDR2_DENSITY_2G<<2) | LPDDR2_TYPE_S4) )
            {
                value &= ~CSR_PERIODIC_ZQ_CALIBRATION_CONTROL_ENABLE_DDR_CS1_CALIBRATION_MASK;
            }    

        }
        else
        {
            value &= ~CSR_PERIODIC_ZQ_CALIBRATION_CONTROL_ENABLE_DDR_CS0_CALIBRATION_MASK;
            value &= ~CSR_PERIODIC_ZQ_CALIBRATION_CONTROL_ENABLE_DDR_APHY_ZQ_CALIBRATION_MASK;
            value &= ~CSR_PERIODIC_ZQ_CALIBRATION_CONTROL_ENABLE_DDR_DPHY_ZQ_CALIBRATION_MASK;
            /* Only configure CS1 if mounted */
            if ( handle->mem_device[MEMC_CS_1].dev_cs != MEMC_CS_NONE )
            {
               value &= ~CSR_PERIODIC_ZQ_CALIBRATION_CONTROL_ENABLE_DDR_CS1_CALIBRATION_MASK;
            }
        }

        BCM_REG_WRITE32(handle, regAddr, value);

        return TRUE;
    }
    return FALSE;
}

/*
 * ===========================================================================
 * 
 *   Function Name: chal_memc_temperature_polling
 * 
 *   Description:
 *       enable/disable memc periodic temperature polling
 * 
 * ===========================================================================
 */
MEMC_EXPORT BCM_ERR_CODE chal_memc_temperature_polling(
    CHAL_MEMC_HANDLE handle,
    _Bool enable
    )
{
    uint32_t value;

    value = BCM_REG_READ32 (handle, handle->memc_open_reg_base + CSR_LPDDR2_DEV_TEMP_PERIOD_OFFSET);

    if (enable)
    {
        value |= CSR_LPDDR2_DEV_TEMP_PERIOD_ENABLE_DDR_CS0_DEV_TEMP_MASK;

        /* Only configure CS1 if mounted */
        if ( handle->mem_device[MEMC_CS_1].dev_cs != MEMC_CS_NONE )
        {
           value |= CSR_LPDDR2_DEV_TEMP_PERIOD_ENABLE_DDR_CS1_DEV_TEMP_MASK;
        }
    }
    else
    {
        value &= ~CSR_LPDDR2_DEV_TEMP_PERIOD_ENABLE_DDR_CS0_DEV_TEMP_MASK;
        
        /* Only configure CS1 if mounted */
        if ( handle->mem_device[MEMC_CS_1].dev_cs != MEMC_CS_NONE )
        {
           value &= ~CSR_LPDDR2_DEV_TEMP_PERIOD_ENABLE_DDR_CS1_DEV_TEMP_MASK;
        }
    }

    BCM_REG_WRITE32 (handle, handle->memc_open_reg_base + CSR_LPDDR2_DEV_TEMP_PERIOD_OFFSET, value);

    return TRUE;
}

/* 
 * ===========================================================================
 * NVM
 * ===========================================================================
 */

/* 
 * ===========================================================================
 * 
 *   Function Name: chal_memc_nvm_is_overlay_window_enable
 * 
 *   Description:
 *       check if the overlay window is enabled
 * 
 * ===========================================================================
 */
MEMC_EXPORT _Bool chal_memc_nvm_is_overlay_window_enable(
    CHAL_MEMC_HANDLE handle,
    _Bool read_from_device
    )
{
    int32_t result;

    if (read_from_device)
    {
        result = chal_memc_read_mode_register (handle, handle->mem_device[0].dev_cs, NVM_OVERLAY_WINDOW);
        handle->overlay_win_enable = (result == NVM_OVERLAY_WIN_ENABLE) ? 1 : 0;
    }
    return handle->overlay_win_enable;
}

/* 
 * ===========================================================================
 * 
 *   Function Name: chal_memc_nvm_disable_overlay_window
 * 
 *   Description:
 *       disable the overlay window
 * 
 * ===========================================================================
 */
MEMC_EXPORT BCM_ERR_CODE chal_memc_nvm_disable_overlay_window(
    CHAL_MEMC_HANDLE handle
    )
{
    handle->overlay_win_enable = 0;
    handle->overlay_win_base_addr = handle->memc_memory_base_nvm;

    /* 1. disable the overlay window on memc */
    BCM_REG_WRITE32 (handle, handle->memc_secure_reg_base + SEC_NVM_OW_CTRL_OFFSET, 0);

    /* 2. disable the overlay window on device */
    if (FALSE == chal_memc_write_mode_register (handle, handle->mem_device[0].dev_cs,
            NVM_OVERLAY_WINDOW, NVM_OVERLAY_WIN_DISABLE))
    {
        return BCM_ERROR;
    }

    /* 3. verify the overlay window is disabled */
    if (FALSE == chal_memc_nvm_is_overlay_window_enable (handle, TRUE))
    {
        /* window is disabled as requested */
        return BCM_SUCCESS;
    }

    return BCM_ERROR;
}

/* 
 * ===========================================================================
 * 
 *   Function Name: chal_memc_nvm_enable_overlay_window
 * 
 *   Description:
 *       enable the overlay window
 * 
 * ===========================================================================
 */
MEMC_EXPORT BCM_ERR_CODE chal_memc_nvm_enable_overlay_window(
    CHAL_MEMC_HANDLE handle,
    uint32_t base_addr
    )
{
    int32_t addr;

    handle->overlay_win_enable = 0;
    handle->overlay_win_base_addr = handle->memc_memory_base_nvm;

    /* 1. if overlay window is enabled, disable it first */
    if (chal_memc_nvm_is_overlay_window_enable(handle, TRUE))
    {
        if (BCM_SUCCESS != chal_memc_nvm_disable_overlay_window (handle))
        {
            return BCM_ERROR;
        }
    }

    /* 2. set the overlay window base address; 8 KB boundary */
    addr = (base_addr >> 12) & 0xFE;    /* bit0 = 0 */
    if (FALSE == chal_memc_write_mode_register (handle, handle->mem_device[0].dev_cs,
            NVM_OVERLAY_WINDOW_ADDR1, addr))
    {
        return BCM_ERROR;
    }

    addr = (base_addr >> 20) & 0xFF;
    if (FALSE == chal_memc_write_mode_register (handle, handle->mem_device[0].dev_cs,
            NVM_OVERLAY_WINDOW_ADDR2, addr))
    {
        return BCM_ERROR;
    }

    addr = (base_addr >> 28) & 0x0F;
    if (FALSE == chal_memc_write_mode_register (handle, handle->mem_device[0].dev_cs,
            NVM_OVERLAY_WINDOW_ADDR3, addr))
    {
        return BCM_ERROR;
    }

    /* 3. enable the overlay window on device */
    if (FALSE == chal_memc_write_mode_register (handle, handle->mem_device[0].dev_cs,
            NVM_OVERLAY_WINDOW, NVM_OVERLAY_WIN_ENABLE))
    {
        return BCM_ERROR;
    }

    /* 4. check if the window is enable/disable as requested */
    if (!chal_memc_nvm_is_overlay_window_enable (handle, TRUE))
    {
        return BCM_ERROR;
    }

    /* 6. set the memc overlay address registers and enable it */
    BCM_REG_WRITE32 (handle, handle->memc_secure_reg_base + SEC_NVM_OWBA_START_ADDR_OFFSET, base_addr);
    BCM_REG_WRITE32 (handle, handle->memc_secure_reg_base + SEC_NVM_OWBA_END_ADDR_OFFSET,
            base_addr+handle->nvm_window_info.window_size-1);
    BCM_REG_WRITE32 (handle, handle->memc_secure_reg_base + SEC_NVM_OW_CTRL_OFFSET, 1);

    handle->overlay_win_enable = 1;
    handle->overlay_win_base_addr = base_addr;

    return BCM_SUCCESS;
}

/* 
 * ===========================================================================
 * 
 *   Function Name: chal_memc_nvm_fill_overlay_window_info
 * 
 *   Description:
 *       read the overlay window info section
 * 
 * ===========================================================================
 */
static BCM_ERR_CODE chal_memc_nvm_fill_overlay_window_info(
    CHAL_MEMC_HANDLE handle
    )
{
    uint32_t base_addr = handle->overlay_win_base_addr;

    if (!chal_memc_nvm_is_overlay_window_enable(handle, FALSE))
        return BCM_ERROR;

    handle->nvm_window_info.query_string_0 = CHAL_REG_READ16(base_addr);
    handle->nvm_window_info.query_string_1 = CHAL_REG_READ16(base_addr+0x2);
    handle->nvm_window_info.query_string_2 = CHAL_REG_READ16(base_addr+0x4);
    handle->nvm_window_info.query_string_3 = CHAL_REG_READ16(base_addr+0x6);

    handle->nvm_window_info.window_id = CHAL_REG_READ16(base_addr+0x8);
    handle->nvm_window_info.window_revision = CHAL_REG_READ16(base_addr+0xA);
    handle->nvm_window_info.window_size = CHAL_REG_READ16(base_addr+0xC);

    handle->nvm_window_info.prog_buffer_offset = CHAL_REG_READ16(base_addr+0x10);
    handle->nvm_window_info.prog_buffer_size = CHAL_REG_READ16(base_addr+0x12);

    handle->nvm_window_info.manufacturer_id = CHAL_REG_READ16(base_addr+0x20);
    handle->nvm_window_info.device_id = CHAL_REG_READ16(base_addr+0x22);

    return BCM_SUCCESS;
}

/* 
 * ===========================================================================
 * 
 *   Function Name: chal_memc_read_overlay_window_status
 * 
 *   Description:
 *       read the overlay window status register
 * 
 * ===========================================================================
 */
MEMC_EXPORT BCM_ERR_CODE chal_memc_nvm_read_overlay_window_status(
    CHAL_MEMC_HANDLE handle,
    uint32_t* status
    )
{
    uint32_t base = handle->overlay_win_base_addr + NVM_STATUS_REG_OFFSET;

    *status = 0;
    if (chal_memc_nvm_is_overlay_window_enable(handle, FALSE))
    {
        *status = CHAL_REG_READ16(base);

        return BCM_SUCCESS;
    }
    return BCM_ERROR;
}

/* 
 * ===========================================================================
 * 
 *   Function Name: chal_memc_nvm_clear_overlay_window_status
 * 
 *   Description:
 *       clear the overlay window status register
 * 
 * ===========================================================================
 */
MEMC_EXPORT BCM_ERR_CODE chal_memc_nvm_clear_overlay_window_status(
    CHAL_MEMC_HANDLE handle,
    uint32_t status
    )
{
    uint16_t value;
    uint32_t base = handle->overlay_win_base_addr + NVM_STATUS_REG_OFFSET;
    (void)status;

    if (chal_memc_nvm_is_overlay_window_enable(handle, FALSE))
    {
        value = CHAL_REG_READ16(base);

        /* write 1 to clear the status bit */
        value |= NVM_STATUS_ESB;
        value |= NVM_STATUS_PSB;
        value |= NVM_STATUS_VSESB;
        value |= NVM_STATUS_BLSB;

        /* DRB, ESSB, PSSB, bit 0 must be 0 */
        value &= ~(NVM_STATUS_DRB + NVM_STATUS_ESSB + NVM_STATUS_PSSB + 1);

        CHAL_REG_WRITE16(base, value);

        return BCM_SUCCESS;
    }
    return BCM_ERROR;
}

/* 
 * ===========================================================================
 * 
 *   Function Name: chal_memc_nvm_is_busy
 * 
 *   Description:
 *       check if the NVM device is busy
 * 
 * ===========================================================================
 */
MEMC_EXPORT BCM_ERR_CODE chal_memc_nvm_is_busy(
    CHAL_MEMC_HANDLE handle,
    _Bool* busy
    )
{
    uint32_t status;

    *busy = FALSE;

    if (BCM_SUCCESS == chal_memc_nvm_read_overlay_window_status(handle, &status))
    {
        /* 0 = busy; 1 = ready */
        if ((status & NVM_STATUS_DRB_MASK) == 0)
            *busy = TRUE;
        return BCM_SUCCESS;
    }
    return BCM_ERROR;
}

/* 
 * ===========================================================================
 * 
 *   Function Name: chal_memc_nvm_poll_ready
 * 
 *   Description:
 *       poll the NVM device until it's ready
 * 
 * ===========================================================================
 */
static _Bool chal_memc_nvm_poll_ready(
    CHAL_MEMC_HANDLE handle
    )
{
    _Bool busy = FALSE;
    uint32_t expire = 0;
    uint32_t max_expire = 1000;

    /* some NVM operation is short while others may be long (e.g. erase);
     * make sure we wait long enough
     */ 
    while (expire < max_expire)
    {
        chal_memc_nvm_is_busy(handle, &busy);
        if (busy == FALSE)
            break;  /* it's ready */
        CHAL_DELAY_MICROS(1);
        expire ++;
    }

    if (expire >= max_expire)
        return FALSE;

    return TRUE;
}

/* 
 * ===========================================================================
 * 
 *   Function Name: chal_memc_nvm_write_allow
 * 
 *   Description:
 *       check if write to the NVM device is allowed
 * 
 * ===========================================================================
 */
static _Bool chal_memc_nvm_write_allow(
    CHAL_MEMC_HANDLE handle
    )
{
    _Bool busy = FALSE;

    /* a write operation requires an overlay window enable and device ready */
    if (!chal_memc_nvm_is_overlay_window_enable(handle, FALSE))
        return FALSE;

    if (BCM_ERROR == chal_memc_nvm_is_busy(handle, &busy))
        return FALSE;

    return (!busy);
}
/* 
 * ===========================================================================
 * 
 *   Function Name: chal_memc_nvm_block_unlock_all
 * 
 *   Description:
 *       unlocks all blocks
 * 
 * ===========================================================================
 */
MEMC_EXPORT BCM_ERR_CODE chal_memc_nvm_block_unlock_all(
    CHAL_MEMC_HANDLE handle
    )
{
    uint32_t start_addr, end_addr;
    uint32_t memSize;

    chal_lpddr2_get_mem_size (handle, handle->mem_device[0].dev_cs, &memSize);

    start_addr = 1;
    end_addr = memSize * 0x100000 - 1;

    return chal_memc_nvm_block_lock(handle, start_addr, end_addr, NVM_BLOCK_UNLOCK);
}

/* 
 * ===========================================================================
 * 
 *   Function Name: chal_memc_nvm_block_lock
 * 
 *   Description:
 *       locks a address region within a block
 * 
 * ===========================================================================
 */
MEMC_EXPORT BCM_ERR_CODE chal_memc_nvm_block_lock(
    CHAL_MEMC_HANDLE handle,
    uint32_t start_addr,
    uint32_t end_addr,
    NVM_BLOCK_LOCK_MODE lock
    )
{
    uint32_t base = handle->overlay_win_base_addr;

    /* the overlay window must be open and device ready */
    if (!chal_memc_nvm_write_allow(handle))
        return BCM_ERROR;

    if (lock == NVM_BLOCK_LOCK)
    {
        /* 1. Write 0x0061 to the Command Code Register */
        CHAL_REG_WRITE16(base+NVM_COMMAND_CODE_REG_OFFSET, NVM_COMMAND_BLOCK_LOCK);
    }
    else
    {
        /* 1. Write 0x0062 to the Command Code Register */
        CHAL_REG_WRITE16(base+NVM_COMMAND_CODE_REG_OFFSET, NVM_COMMAND_BLOCK_UNLOCK);
    }

    /* 2. Write start address to lock to the Command Address Register */
    CHAL_REG_WRITE32(base+NVM_COMMAND_ADDR_REG_OFFSET, start_addr);

    /* 3. Write end address to lock to the Multi-Purpose Register, */
    CHAL_REG_WRITE32(base+NVM_MULTI_PURPOSE_REG_OFFSET, end_addr);

    /* 4. Write 0x0001 to the Command Execute Register. */
    CHAL_REG_WRITE16(base+NVM_COMMAND_EXE_REG_OFFSET, 0x1);

    /* 5. poll device until ready */
    if (!chal_memc_nvm_poll_ready(handle))
        return BCM_ERROR;

    return BCM_SUCCESS;
}

/* 
 * ===========================================================================
 * 
 *   Function Name: chal_memc_nvm_block_lock_down
 * 
 *   Description:
 *       locks down a block(s)
 * 
 * ===========================================================================
 */
MEMC_EXPORT BCM_ERR_CODE chal_memc_nvm_block_lock_down(
    CHAL_MEMC_HANDLE handle,
    uint32_t start_addr,
    uint32_t end_addr
    )
{
    uint32_t base = handle->overlay_win_base_addr;

    /* the overlay window must be open and device ready */
    if (!chal_memc_nvm_write_allow(handle))
        return BCM_ERROR;

    /* 1. Write 0x0063 to the Command Code Register */
    CHAL_REG_WRITE16(base+NVM_COMMAND_CODE_REG_OFFSET, NVM_COMMAND_BLOCK_LOCK_DOWN);

    /* 2. Write start address to lock to the Command Address Register */
    CHAL_REG_WRITE32(base+NVM_COMMAND_ADDR_REG_OFFSET, start_addr);

    /* 3. Write end address to lock to the Multi-Purpose Register, */
    CHAL_REG_WRITE32(base+NVM_MULTI_PURPOSE_REG_OFFSET, end_addr);

    /* 4. Write 0x0001 to the Command Execute Register. */
    CHAL_REG_WRITE16(base+NVM_COMMAND_EXE_REG_OFFSET, 0x1);

    /* 5. poll device until ready */
    if (!chal_memc_nvm_poll_ready(handle))
        return BCM_ERROR;

    return BCM_SUCCESS;
}


/* 
 * ===========================================================================
 * 
 *   Function Name: chal_memc_nvm_block_erase
 * 
 *   Description:
 *       erase a block
 *       block_addr: any byte address within the block
 * 
 * ===========================================================================
 */
MEMC_EXPORT BCM_ERR_CODE chal_memc_nvm_block_erase(
    CHAL_MEMC_HANDLE handle,
    uint32_t block_addr,
    uint32_t waitComplete
    )
{
    uint32_t base = handle->overlay_win_base_addr;
    uint32_t status = 0;

    /* the overlay window must be open and device ready */
    if (!chal_memc_nvm_write_allow(handle))
        return BCM_ERROR;

    /* 1. Write 0x0020 to the Command Code Register */
    CHAL_REG_WRITE16(base+NVM_COMMAND_CODE_REG_OFFSET, NVM_COMMAND_BLOCK_ERASE);

    /* 2. Write any address within the desired start block to the Command Address Register, */
    CHAL_REG_WRITE32(base+NVM_COMMAND_ADDR_REG_OFFSET, block_addr);

    /* 3. Write 0x0001 to the Command Execute Register. */
    CHAL_REG_WRITE16(base+NVM_COMMAND_EXE_REG_OFFSET, 0x1);

    if (waitComplete)
    {
        /* 4. poll device until ready */
        if (!chal_memc_nvm_poll_ready(handle))
            return BCM_ERROR;

        /* 5. read command execute status */
        chal_memc_nvm_read_overlay_window_status(handle, &status);

        if ((status & NVM_ERROR_BLOCK_ERASE_FAIL) == NVM_ERROR_BLOCK_ERASE_FAIL)
        {
            if ((status & NVM_ERROR_BLOCK_ERASE_LOCK) == NVM_ERROR_BLOCK_ERASE_LOCK)
            {
                handle->nvm_operation_error = NVM_ERROR_BLOCK_ERASE_LOCK;
            }
            else if ((status & NVM_ERROR_BLOCK_ERASE_VCC) == NVM_ERROR_BLOCK_ERASE_VCC)
            {
                handle->nvm_operation_error = NVM_ERROR_BLOCK_ERASE_VCC;
            }
            else if ((status & NVM_ERROR_OPERATION_BAD_ADDR) == NVM_ERROR_OPERATION_BAD_ADDR)
            {
                handle->nvm_operation_error = NVM_ERROR_OPERATION_BAD_ADDR;
            }
            else
            {
                handle->nvm_operation_error = NVM_ERROR_BLOCK_ERASE_FAIL;
            }
            chal_memc_nvm_clear_overlay_window_status(handle, status);
            return BCM_ERROR;
        }
    }
    return BCM_SUCCESS;
}

/* 
 * ===========================================================================
 * 
 *   Function Name: chal_memc_nvm_write_word
 * 
 *   Description:
 *       write a word to a block
 * 
 * ===========================================================================
 */
MEMC_EXPORT BCM_ERR_CODE chal_memc_nvm_write_word(
    CHAL_MEMC_HANDLE handle,
    uint32_t word_addr,
    uint16_t word_value,
    uint32_t overwrite,
    uint32_t waitComplete
    )
{
    uint32_t base = handle->overlay_win_base_addr;
    uint32_t status = 0;

    /* the overlay window must be open and device ready */
    if (!chal_memc_nvm_write_allow(handle))
        return BCM_ERROR;

    if (!overwrite)
    {
        /* 1. Write 0x0041 to the Command Code Register */
        CHAL_REG_WRITE16(base+NVM_COMMAND_CODE_REG_OFFSET, NVM_COMMAND_PROG_WORD);
    }
    else
    {
        /* 1. Write 0x0042 to the Command Code Register */
        CHAL_REG_WRITE16(base+NVM_COMMAND_CODE_REG_OFFSET, NVM_COMMAND_OVERWRITE_WORD);
    }

    /* 2. Write address to the Command Address Register, */
    CHAL_REG_WRITE32(base+NVM_COMMAND_ADDR_REG_OFFSET, word_addr);

    /* 3. Write data to the Command Data Register; high-word must be 0 */
    CHAL_REG_WRITE32(base+NVM_COMMAND_DATA_REG_OFFSET, word_value & 0xFFFF);

    /* 4. Write 0x0001 to the Command Execute Register. */
    CHAL_REG_WRITE16(base+NVM_COMMAND_EXE_REG_OFFSET, 0x1);

    if (waitComplete)
    {
        /* 5. poll device until ready */
        if (!chal_memc_nvm_poll_ready(handle))
            return BCM_ERROR;

        /* 6. read command execute status */
        chal_memc_nvm_read_overlay_window_status(handle, &status);

        if ((status & NVM_ERROR_PROGRAM_FAIL) == NVM_ERROR_PROGRAM_FAIL)
        {
            if ((status & NVM_ERROR_PROGRAM_LOCK) == NVM_ERROR_PROGRAM_LOCK)
            {
                handle->nvm_operation_error = NVM_ERROR_PROGRAM_LOCK;
            }
            else if ((status & NVM_ERROR_PROGRAM_VCC) == NVM_ERROR_PROGRAM_VCC)
            {
                handle->nvm_operation_error = NVM_ERROR_PROGRAM_VCC;
            }
            else if ((status & NVM_ERROR_OPERATION_BAD_ADDR) == NVM_ERROR_OPERATION_BAD_ADDR)
            {
                handle->nvm_operation_error = NVM_ERROR_OPERATION_BAD_ADDR;
            }
            else
            {
                handle->nvm_operation_error = NVM_ERROR_PROGRAM_FAIL;
            }
            chal_memc_nvm_clear_overlay_window_status(handle, status);
            return BCM_ERROR;
        }
    }
    return BCM_SUCCESS;
}

/* 
 * ===========================================================================
 * 
 *   Function Name: chal_memc_nvm_fill_prog_buffer
 * 
 *   Description:
 *       fill NVM program buffer
 * 
 * ===========================================================================
 */
static void chal_memc_nvm_fill_prog_buffer(
    uint32_t addr_reg,
    uint8_t* data,
    uint32_t size
    )
{
    uint32_t i;
    uint32_t addr = addr_reg;

    for (i=0; i<size; i++)
    {
        CHAL_REG_WRITE8(addr, data[i]);
        addr ++;
    }
}

/* 
 * ===========================================================================
 * 
 *   Function Name: chal_memc_nvm_write_buffer
 * 
 *   Description:
 *       write a buffer to a block
 * 
 * ===========================================================================
 */
MEMC_EXPORT BCM_ERR_CODE chal_memc_nvm_write_buffer(
    CHAL_MEMC_HANDLE handle,
    uint32_t address,
    uint8_t* data,
    uint32_t size,
    uint32_t overwrite,
    uint32_t waitComplete
    )
{
    uint32_t base = handle->overlay_win_base_addr;
    uint32_t status = 0;

    if (size > handle->nvm_window_info.prog_buffer_size)
        return BCM_ERROR;

    /* the overlay window must be open and device ready */
    if (!chal_memc_nvm_write_allow(handle))
        return BCM_ERROR;

    if (!overwrite)
    {
        /* 1. Write 0x00E9 to the Command Code Register */
        CHAL_REG_WRITE16(base+NVM_COMMAND_CODE_REG_OFFSET, NVM_COMMAND_BUFFERED_PROG);
    }
    else
    {
        /* 1. Write 0x00EA to the Command Code Register */
        CHAL_REG_WRITE16(base+NVM_COMMAND_CODE_REG_OFFSET, NVM_COMMAND_BUFFERED_OVERWRITE);
    }

    /* 2. Write address to the Command Address Register, */
    CHAL_REG_WRITE32(base+NVM_COMMAND_ADDR_REG_OFFSET, address);

    /* 3. Write data size to the Multi-Purpose Data Register, */
    CHAL_REG_WRITE32(base+NVM_MULTI_PURPOSE_REG_OFFSET, size);

    /* 4. copy data to the Program Buffer, */
    /*memcpy((void*)(base+NVM_PROG_BUFFER_REG_OFFSET), data, size);*/
    chal_memc_nvm_fill_prog_buffer(base+NVM_PROG_BUFFER_REG_OFFSET, data, size);

    /* 5. Write 0x0001 to the Command Execute Register. */
    CHAL_REG_WRITE16(base+NVM_COMMAND_EXE_REG_OFFSET, 0x1);

    if (waitComplete)
    {
        /* 6. poll device until ready */
        if (!chal_memc_nvm_poll_ready(handle))
            return BCM_ERROR;

        /* 7. read command execute status */
        chal_memc_nvm_read_overlay_window_status(handle, &status);

        if ((status & NVM_ERROR_PROGRAM_FAIL) == NVM_ERROR_PROGRAM_FAIL)
        {
            if ((status & NVM_ERROR_PROGRAM_LOCK) == NVM_ERROR_PROGRAM_LOCK)
            {
                handle->nvm_operation_error = NVM_ERROR_PROGRAM_LOCK;
            }
            else if ((status & NVM_ERROR_PROGRAM_VCC) == NVM_ERROR_PROGRAM_VCC)
            {
                handle->nvm_operation_error = NVM_ERROR_PROGRAM_VCC;
            }
            else if ((status & NVM_ERROR_OPERATION_BAD_ADDR) == NVM_ERROR_OPERATION_BAD_ADDR)
            {
                handle->nvm_operation_error = NVM_ERROR_OPERATION_BAD_ADDR;
            }
            else
            {
                handle->nvm_operation_error = NVM_ERROR_PROGRAM_FAIL;
            }
            chal_memc_nvm_clear_overlay_window_status(handle, status);
            return BCM_ERROR;
        }
    }
    return BCM_SUCCESS;
}

/* 
 * ===========================================================================
 * 
 *   Function Name: chal_memc_nvm_suspend
 * 
 *   Description:
 *       suspend an operation; can be program or erase
 * 
 * ===========================================================================
 */
MEMC_EXPORT BCM_ERR_CODE chal_memc_nvm_suspend(
    CHAL_MEMC_HANDLE handle
    )
{
    uint32_t base = handle->overlay_win_base_addr;

    /* the overlay window must be open */
    if (!chal_memc_nvm_is_overlay_window_enable(handle, FALSE))
        return BCM_ERROR;

    /* 1. Write 0x1 to the Suspend Register */
    CHAL_REG_WRITE16(base+NVM_SUSPEND_REG_OFFSET, 0x1);

    /* 2. poll device until ready */
    if (!chal_memc_nvm_poll_ready(handle))
        return BCM_ERROR;

    return BCM_SUCCESS;
}

/* 
 * ===========================================================================
 * 
 *   Function Name: chal_memc_nvm_resume
 * 
 *   Description:
 *       resume an operation; can be program or erase
 * 
 * ===========================================================================
 */
MEMC_EXPORT BCM_ERR_CODE chal_memc_nvm_resume(
    CHAL_MEMC_HANDLE handle
    )
{
    uint32_t base = handle->overlay_win_base_addr;

    /* the overlay window must be open */
    if (!chal_memc_nvm_is_overlay_window_enable(handle, FALSE))
        return BCM_ERROR;

    /* 1. Write 0x00D0 to the Command Code Register */
    CHAL_REG_WRITE16(base+NVM_COMMAND_CODE_REG_OFFSET, NVM_COMMAND_RESUME);

    /* 2. Write 0x0001 to the Command Execute Register. */
    CHAL_REG_WRITE16(base+NVM_COMMAND_EXE_REG_OFFSET, 0x1);

    /* 3. poll device until ready */
    if (!chal_memc_nvm_poll_ready(handle))
        return BCM_ERROR;

    return BCM_SUCCESS;
}

/* 
 * ===========================================================================
 * 
 *   Function Name: chal_memc_nvm_abort
 * 
 *   Description:
 *       abort a NVM operation
 * 
 * ===========================================================================
 */
MEMC_EXPORT BCM_ERR_CODE chal_memc_nvm_abort(
    CHAL_MEMC_HANDLE handle
    )
{
    uint32_t base = handle->overlay_win_base_addr;
    uint32_t status;

    /* the overlay window must be open */
    if (!chal_memc_nvm_is_overlay_window_enable(handle, FALSE))
        return BCM_ERROR;

    /* 1. Write 0x0001 to the Abort Register */
    CHAL_REG_WRITE16(base+NVM_ABORT_REG_OFFSET, 0x1);

    /* 2. poll device until ready */
    if (!chal_memc_nvm_poll_ready(handle))
        return BCM_ERROR;
    
    /* 3. read command execute status */
    chal_memc_nvm_read_overlay_window_status(handle, &status);

    /* 4. check and clear abort status */
    if (((status & NVM_ERROR_PROGRAM_FAIL) == NVM_ERROR_PROGRAM_FAIL) ||
        ((status & NVM_ERROR_BLOCK_ERASE_FAIL) == NVM_ERROR_BLOCK_ERASE_FAIL) ||
        ((status & NVM_ERROR_OPERATION_BAD_ADDR) == NVM_ERROR_OPERATION_BAD_ADDR))
    {
        chal_memc_nvm_clear_overlay_window_status(handle, status);
    }

    return BCM_SUCCESS;
}

/* 
 * ===========================================================================
 * 
 *   Function Name: chal_memc_nvm_read_block_lock_status
 * 
 *   Description:
 *       read a block lock status
 * 
 * ===========================================================================
 */
MEMC_EXPORT BCM_ERR_CODE chal_memc_nvm_read_block_lock_status(
    CHAL_MEMC_HANDLE handle,
    uint32_t address,
    NVM_BLOCK_LOCK_MODE* lock
    )
{
    uint32_t base = handle->overlay_win_base_addr;
    _Bool busy = FALSE;
    uint32_t status;

    /* device should be ready */
    if (BCM_SUCCESS != chal_memc_nvm_is_busy(handle, &busy))
        return BCM_ERROR;

    if (busy)
        return BCM_ERROR;

    /* 1. Write 0x0065 to the Command Code Register */
    CHAL_REG_WRITE16(base+NVM_COMMAND_CODE_REG_OFFSET, NVM_COMMAND_READ_LOCK_STATUS);

    /* 2. Write address to the Command Address Register, */
    CHAL_REG_WRITE32(base+NVM_COMMAND_ADDR_REG_OFFSET, address);

    /* 3. Write 0x0001 to the Command Execute Register. */
    CHAL_REG_WRITE16(base+NVM_COMMAND_EXE_REG_OFFSET, 0x1);

    /* 4. poll device until ready */
    if (!chal_memc_nvm_poll_ready(handle))
        return BCM_ERROR;

    /* 5. read command execute status */
    chal_memc_nvm_read_overlay_window_status(handle, &status);

    /* any bit [6:1] of status register set (= 1) indicates an error */
    if (status & 0x7E)
        return BCM_ERROR;

    /* 6. read lock status from Data register; */
    status = CHAL_REG_READ16(base+NVM_COMMAND_DATA_REG_OFFSET);

    /* bits [1:0] indicate the lock status */
    status &= 0x3;

    switch (status)
    {
    case 0:
        *lock = NVM_BLOCK_UNLOCK;
        break;
    case 1:
        *lock = NVM_BLOCK_LOCK;
        break;
    case 3:
        *lock = NVM_BLOCK_LOCK_DOWN;
        break;
    default:
        return BCM_ERROR;
    }

    return BCM_SUCCESS;
}

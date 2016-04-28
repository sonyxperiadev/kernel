/*****************************************************************************
*  Copyright 2001 - 2008 Broadcom Corporation.  All rights reserved.
*
*  Unless you and Broadcom execute a separate written software license
*  agreement governing use of this software, this software is licensed to you
*  under the terms of the GNU General Public License version 2, available at
*  http://www.gnu.org/licenses/old-license/gpl-2.0.html (the "GPL").
*
*  Notwithstanding the above, under no circumstances may you combine this
*  software in any way with any other Broadcom software provided under a
*  license other than the GPL, without Broadcom's express prior written
*  consent.
*
*****************************************************************************/
#ifndef __KERNEL__
#include <stdio.h>
#endif
#include <mach/rdb/brcm_rdb_i2c_mm_hs.h>
#include <mach/rdb/brcm_rdb_i2c_mm_hs_slave.h>

/*
 * Map the B0 macros to the appropriate A0 macros to get both the variants
 *working - This has to be done as a part of adding the changes in the B0
 *specific RDB to the A0 RDB
 */
#if defined(CONFIG_ARCH_RHEA_BX) || defined(CONFIG_ARCH_HAWAII) || defined(CONFIG_ARCH_JAVA)
#define I2C_MM_HS_CS_CMD_CMD_GEN_START          I2C_MM_HS_CS_CMD_CMD_START_RESTART
#define I2C_MM_HS_CS_EN_CMD_RST_BSC             I2C_MM_HS_CS_EN_CMD_RESET_BSC
#define I2C_MM_HS_TIM_PRESCALE_CMD_NODIV        I2C_MM_HS_TIM_PRESCALE_CMD_DIV1
#define I2C_MM_HS_CS_CMD_CMD_GEN_STOP           I2C_MM_HS_CS_CMD_CMD_STOP
#define I2C_MM_HS_CS_CMD_CMD_RD_A_BYTE          I2C_MM_HS_CS_CMD_CMD_READ_BYTE
#define I2C_MM_HS_PADCTL_PAD_SLEW_RATE_SHIFT    I2C_MM_HS_PADCTL_PULLUP_EN_SHIFT
#define I2C_MM_HS_PADCTL_PAD_SLEW_RATE_MASK     I2C_MM_HS_PADCTL_PULLUP_EN_MASK
#endif

/*#define BSC_HS *//* Athena has HS controller */

#define BSC_WRITE_REG_FIELD(addr, mask, shift, data) (writel((((data << shift) & (mask)) | \
			(readl((void __iomem __force *)addr) & ~mask)) , (void __iomem __force *)addr))
#define BSC_READ_REG_FIELD(addr, mask, shift)       ((readl((void __iomem __force *)addr) & mask) >> shift)
#define BSC_WRITE_REG(addr, data)                  (writel(data, (void __iomem __force *)addr))
#define BSC_READ_REG(addr)                        (readl((void __iomem __force *)addr))

#define BSC_DEBUG_PRINT  0
#if BSC_DEBUG_PRINT
#define bsc_dprintf(e, f, s...)    { printf(f, ## s); }
#else
#define bsc_dprintf(e, f, s...)
#endif

/**
* Supported BSC bus speeds
*****************************************************************************/
typedef enum {
	BSC_SPD_32K = 0,	/*< 32KHZ */
	BSC_SPD_50K,		/*< 50KHZ */
	BSC_SPD_100K,		/*< 100KHZ */
	BSC_SPD_230K,		/*< 230KHZ */
	BSC_SPD_380K,		/*< 380KHZ */
	BSC_SPD_400K,		/*< 400KHZ */
	BSC_SPD_430K,		/*< 430KHZ */
	BSC_SPD_HS,		/*< HIGH SPEED */
	BSC_SPD_HS_1MHZ,	/*< HIGH SPEED */
	BSC_SPD_HS_2MHZ,	/*< HIGH SPEED */
	BSC_SPD_HS_1625KHZ,	/*< HIGH SPEED */
	BSC_SPD_HS_2600KHZ,	/*< HIGH SPEED */
	BSC_SPD_100K_FPGA,	/*< 100KHZ based on a 26MHz incoming clock */
	BSC_SPD_400K_FPGA,	/*< 400KHZ based on a 26MHz incoming clock */
	BSC_SPD_HS_FPGA,	/*< HIGH SPEED based on a 26MHz incoming clock */
	BSC_SPD_MAXIMUM		/*< 460K, assume auto sense turn off. */
} BSC_SPEED_t;

/**
* Supported BSC mode
*****************************************************************************/
typedef enum {
	BSC_MODE_MASTER = 0,	/*< Master mode */
	BSC_MODE_SLAVE		/*< Slave mode */
} BSC_MODE_t;

/**
* Supported BSC interrupts
*****************************************************************************/

/**
* BSC Command
*****************************************************************************/
typedef enum {
	BSC_CMD_NOACTION = 0,	/*< NOACTION */
	BSC_CMD_START,		/*< START command */
	BSC_CMD_RESTART,	/*< RESTART command */
	BSC_CMD_STOP,		/*< STOP command */
	BSC_CMD_READ_ACK,	/*< READ ACK command */
	BSC_CMD_READ_NAK,	/*< READ NAK command */
	BSC_CMD_HS_STOP		/*< High speed STOP command */
} BSC_CMD_t;

static inline void bsc_set_bus_speed(uint32_t baseAddr, BSC_SPEED_t speed,
				     enum bsc_ref_clk ref_clk);
static inline void isl_bsc_init(uint32_t baseAddr);
static inline void bsc_disable_intr(uint32_t baseAddr, uint32_t mask);
static inline void bsc_clear_intr_status(uint32_t baseAddr, uint32_t mask);
static inline void bsc_set_tx_fifo(uint32_t baseAddr, unsigned char enable);
static inline void bsc_set_autosense(uint32_t baseAddr, unsigned char on,
				     unsigned char timeout_enable);
static inline unsigned char bsc_get_timeout(uint32_t baseAddr);
static inline void bsc_enable_intr(uint32_t baseAddr, uint32_t mask);
static inline void bsc_deinit(uint32_t baseAddr);
static inline void bsc_stop(uint32_t baseAddr);
static inline void bsc_reset(uint32_t baseAddr);
static inline void bsc_set_sda(uint32_t baseAddr, unsigned char state);
static inline void bsc_set_scl(uint32_t baseAddr, unsigned char state);
static inline void bsc_set_timeout(uint32_t baseAddr, unsigned char on);
static inline void isl_bsc_send_cmd(uint32_t baseAddr, BSC_CMD_t cmd);
static inline void bsc_start(uint32_t baseAddr);
static inline void bsc_set_tim(uint32_t baseAddr, uint32_t val);
static inline void bsc_start_highspeed(uint32_t baseAddr);
static inline void bsc_stop_highspeed(uint32_t baseAddr);
static inline uint32_t bsc_get_enabled_intr(uint32_t baseAddr);
static inline uint32_t bsc_read_intr_status(uint32_t baseAddr);
static inline uint32_t bsc_read_data(uint32_t baseAddr, uint8_t *pBuffer,
				     uint32_t size);
static inline uint32_t bsc_write_data(uint32_t baseAddr, uint8_t *pBuffer,
				      uint32_t size);
static inline unsigned char bsc_get_autosense(uint32_t baseAddr);
static inline unsigned char bsc_get_bus_status(uint32_t baseAddr);
static inline unsigned char bsc_get_ack(uint32_t baseAddr);
static inline unsigned char bsc_get_sda(uint32_t baseAddr);
static inline unsigned char bsc_get_scl(uint32_t baseAddr);
static inline void bsc_set_soft_reset(uint32_t baseAddr, unsigned char enable);
static inline unsigned char bsc_get_soft_reset_ready(uint32_t baseAddr);
static inline unsigned char bsc_get_soft_reset(uint32_t baseAddr);
static inline BSC_MODE_t bsc_set_mode(uint32_t baseAddr, BSC_MODE_t mode);
static inline void bsc_enable_pad_output(uint32_t baseAddr, bool enable);
static inline void bsc_enable_thigh_ctrl(uint32_t baseAddr, bool enable);
static inline void bsc_flush_tx_fifo(uint32_t baseAddr);
static inline unsigned char bsc_get_tx_fifo_count(uint32_t baseAddr);

/**
*
*  @brief  Initialize BSC for the passed BSC instance
*
*  @param  baseAddr  (in) mapped address of this BSC instance
*
*  @return
*****************************************************************************/
static inline void isl_bsc_init(uint32_t baseAddr)
{
	bsc_dprintf(CDBG_INFO, "isl_bsc_init, base=0x%x\n", baseAddr);

	bsc_set_sda(baseAddr, 1);
	bsc_set_scl(baseAddr, 1);

	/* Time out */
	bsc_set_timeout(baseAddr, 1);

	/* set to max 1023 cycles */
	BSC_WRITE_REG_FIELD((baseAddr + I2C_MM_HS_TOUT_OFFSET),
			    I2C_MM_HS_TOUT_TOUT_LOW_MASK,
			    I2C_MM_HS_TOUT_TOUT_LOW_SHIFT, 0x7F);
	BSC_WRITE_REG_FIELD((baseAddr + I2C_MM_HS_TOUT_OFFSET),
			    I2C_MM_HS_TOUT_TOUT_HIGH_MASK,
			    I2C_MM_HS_TOUT_TOUT_HIGH_SHIFT, 0x3);

	/* FIFO */
	bsc_set_tx_fifo(baseAddr, 0);

	/* CRC */
	BSC_WRITE_REG_FIELD((baseAddr + I2C_MM_HS_RCM_OFFSET),
			    I2C_MM_HS_RCM_EN_MASK, I2C_MM_HS_RCM_EN_SHIFT, 0);
	BSC_WRITE_REG_FIELD((baseAddr + I2C_MM_HS_RCM_OFFSET),
			    I2C_MM_HS_RCM_LSB0_MASK, I2C_MM_HS_RCM_LSB0_SHIFT,
			    1);
	BSC_WRITE_REG((baseAddr + I2C_MM_HS_RCP_OFFSET), 0x89);

	/* Clear Interrupts */
	bsc_clear_intr_status(baseAddr, 0xff);

	/* Clear CMD */
	isl_bsc_send_cmd(baseAddr, BSC_CMD_NOACTION);

#ifdef BSC_HS
	/*pad control, slew rate 60ns<Tr(Tf)<100ns
	   The PAD enable polarity is reverted. Need to use 0 instead of 1. */
	BSC_WRITE_REG_FIELD((baseAddr + I2C_MM_HS_PADCTL_OFFSET),
			    I2C_MM_HS_PADCTL_PAD_OUT_EN_MASK,
			    I2C_MM_HS_PADCTL_PAD_OUT_EN_SHIFT, 0);
	/* start with non-HS, use 13MHz */
	BSC_WRITE_REG_FIELD((baseAddr + I2C_MM_HS_TIM_OFFSET),
			    I2C_MM_HS_TIM_PRESCALE_MASK,
			    I2C_MM_HS_TIM_PRESCALE_SHIFT,
			    I2C_MM_HS_TIM_PRESCALE_CMD_NODIV);
	BSC_WRITE_REG_FIELD((baseAddr + I2C_MM_HS_TIM_OFFSET),
			    I2C_MM_HS_TIM_NO_DIV_MASK,
			    I2C_MM_HS_TIM_NO_DIV_SHIFT, 0);
#endif

	/* start */
	bsc_start(baseAddr);
}

/**
*
*  @brief  De-Initialize BSC for the passed BSC instance
*
*  @param  baseAddr  (in) mapped address of this BSC instance
*
*  @return none
*****************************************************************************/
static inline void bsc_deinit(uint32_t baseAddr)
{
	bsc_stop(baseAddr);
}

/**
*
*  @brief  Start BSC device
*
*  @param  baseAddr  (in) mapped address of this BSC instance
*
*  @return none
*****************************************************************************/

static inline void bsc_start(uint32_t baseAddr)
{
	BSC_WRITE_REG_FIELD((baseAddr + I2C_MM_HS_CLKEN_OFFSET),
			    I2C_MM_HS_CLKEN_CLKEN_MASK,
			    I2C_MM_HS_CLKEN_CLKEN_SHIFT, 1);
	BSC_WRITE_REG_FIELD((baseAddr + I2C_MM_HS_CS_OFFSET),
			    I2C_MM_HS_CS_CMD_MASK, I2C_MM_HS_CS_CMD_SHIFT,
			    I2C_MM_HS_CS_CMD_CMD_NO_ACTION);
	BSC_WRITE_REG_FIELD((baseAddr + I2C_MM_HS_CS_OFFSET),
			    I2C_MM_HS_CS_ACK_MASK, I2C_MM_HS_CS_ACK_SHIFT,
			    I2C_MM_HS_CS_ACK_CMD_GEN_START);
	BSC_WRITE_REG_FIELD((baseAddr + I2C_MM_HS_CS_OFFSET),
			    I2C_MM_HS_CS_EN_MASK, I2C_MM_HS_CS_EN_SHIFT,
			    I2C_MM_HS_CS_EN_CMD_ENABLE_BSC);
}

/**
*
*  @brief  Stop BSC device
*
*  @param  baseAddr  (in) mapped address of this BSC instance
*
*  @return none
*****************************************************************************/
static inline void bsc_stop(uint32_t baseAddr)
{
	BSC_WRITE_REG_FIELD((baseAddr + I2C_MM_HS_CS_OFFSET),
			    I2C_MM_HS_CS_EN_MASK, I2C_MM_HS_CS_EN_SHIFT,
			    I2C_MM_HS_CS_EN_CMD_RST_BSC);
	BSC_WRITE_REG_FIELD((baseAddr + I2C_MM_HS_CLKEN_OFFSET),
			    I2C_MM_HS_CLKEN_CLKEN_MASK,
			    I2C_MM_HS_CLKEN_CLKEN_SHIFT, 0);
}

static inline void bsc_reset(uint32_t baseAddr)
{
	BSC_WRITE_REG_FIELD((baseAddr + I2C_MM_HS_CS_OFFSET),
			    I2C_MM_HS_CS_EN_MASK, I2C_MM_HS_CS_EN_SHIFT,
			    I2C_MM_HS_CS_EN_CMD_RST_BSC);
	bsc_set_sda(baseAddr, 1);
	bsc_set_scl(baseAddr, 1);
	BSC_WRITE_REG_FIELD((baseAddr + I2C_MM_HS_CS_OFFSET),
			    I2C_MM_HS_CS_CMD_MASK, I2C_MM_HS_CS_CMD_SHIFT,
			    I2C_MM_HS_CS_CMD_CMD_NO_ACTION);
	BSC_WRITE_REG_FIELD((baseAddr + I2C_MM_HS_CS_OFFSET),
			    I2C_MM_HS_CS_ACK_MASK, I2C_MM_HS_CS_ACK_SHIFT,
			    I2C_MM_HS_CS_ACK_CMD_GEN_START);
	BSC_WRITE_REG_FIELD((baseAddr + I2C_MM_HS_CS_OFFSET),
			    I2C_MM_HS_CS_EN_MASK, I2C_MM_HS_CS_EN_SHIFT,
			    I2C_MM_HS_CS_EN_CMD_ENABLE_BSC);
}

/**
*
*  @brief  Set BSC bus speed
*
*  @param  baseAddr  (in) mapped address of this BSC instance
*  @param  speed   (in) speed to be set
*
*  @return none
*****************************************************************************/
/* Master clock is 13MHz */
#define BSCTIM_DIV_812500HZ     (0 << I2C_MM_HS_TIM_DIV_SHIFT)
#define BSCTIM_DIV_1625000HZ    (1 << I2C_MM_HS_TIM_DIV_SHIFT)
#define BSCTIM_DIV_3250000HZ    (2 << I2C_MM_HS_TIM_DIV_SHIFT)
#define BSCTIM_DIV_6500000HZ    (3 << I2C_MM_HS_TIM_DIV_SHIFT)

#ifndef FPGA
/* HS Timings for 26MHz source:
 *	Hold=1, HighPhase=3, Setup=4,
 *	HS CLK = 26/(1+3+4) = 3.25MHz
 */
#define BSC_HS_HSMODE_TIMING_26MHZ        0x0000024
#define BSC_HS_HSMODE_1MHZ_26MHZ          0x000014D
#define BSC_HS_HSMODE_1625KHZ_26MHZ       0x00000A8
#define BSC_HS_HSMODE_2600KHZ_26MHZ       0x0000064
#define BSC_HS_HSMODE_2MHZ_26MHZ          0x0000086
#define BSC_HS_HSMODE_TIMING              0x00000513
#define BSC_HS_HSMODE_1MHZ                0x00002BC
#define BSC_HS_HSMODE_1625KHZ             0x0000172
#define BSC_HS_HSMODE_2600KHZ             0x00000EA
#define BSC_HS_HSMODE_2MHZ                0x00002BC
#else
#define BSC_HS_HSMODE_TIMING              0x00000001
#endif
/* HS speed always uses 26MHZ source and FS uses 13MHZ */
static inline void bsc_set_bus_speed(uint32_t baseAddr, BSC_SPEED_t speed,
				     enum bsc_ref_clk ref_clk)
{
	uint8_t DIV = 0, M = 0, N = 0, P = 0, NO_DIV = 0, PRESCALE =
	    I2C_MM_HS_TIM_PRESCALE_CMD_NODIV;
	NO_DIV = 0;
	PRESCALE = I2C_MM_HS_TIM_PRESCALE_CMD_NODIV;
	bsc_dprintf(CDBG_INFO, "bsc_set_bus_speed, %d\n", speed);
	switch (speed) {
	case BSC_SPD_32K:
		M = 0x01;
		N = 0x01;
		P = 0x04;
		if (ref_clk == BSC_BUS_REF_13MHZ) {
			PRESCALE = I2C_MM_HS_TIM_PRESCALE_CMD_NODIV;
			NO_DIV = 0;
			DIV = I2C_MM_HS_TIM_DIV_CMD_DIV16;
		} else if (ref_clk == BSC_BUS_REF_26MHZ) {
			PRESCALE = I2C_MM_HS_TIM_PRESCALE_CMD_DIV2;
			NO_DIV = 0;
			DIV = I2C_MM_HS_TIM_DIV_CMD_DIV16;
		} else if (ref_clk == BSC_BUS_REF_104MHZ) {
			PRESCALE = I2C_MM_HS_TIM_PRESCALE_CMD_DIV8;
			NO_DIV = 0;
			DIV = I2C_MM_HS_TIM_DIV_CMD_DIV16;
		}
		break;
	case BSC_SPD_50K:
		M = 0x01;
		N = 0x01;
		P = 0x02;
		if (ref_clk == BSC_BUS_REF_13MHZ) {
			PRESCALE = I2C_MM_HS_TIM_PRESCALE_CMD_NODIV;
			NO_DIV = 0;
			DIV = I2C_MM_HS_TIM_DIV_CMD_DIV16;
		} else if (ref_clk == BSC_BUS_REF_26MHZ) {
			PRESCALE = I2C_MM_HS_TIM_PRESCALE_CMD_DIV2;
			NO_DIV = 0;
			DIV = I2C_MM_HS_TIM_DIV_CMD_DIV16;
		} else if (ref_clk == BSC_BUS_REF_104MHZ) {
			PRESCALE = I2C_MM_HS_TIM_PRESCALE_CMD_DIV8;
			NO_DIV = 0;
			DIV = I2C_MM_HS_TIM_DIV_CMD_DIV16;
		}
		break;
	case BSC_SPD_230K:
		M = 0x07;
		N = 0x00;
		P = 0x00;
		if (ref_clk == BSC_BUS_REF_13MHZ) {
			PRESCALE = I2C_MM_HS_TIM_PRESCALE_CMD_NODIV;
			NO_DIV = 0;
			DIV = I2C_MM_HS_TIM_DIV_CMD_DIV4;
		} else if (ref_clk == BSC_BUS_REF_26MHZ) {
			PRESCALE = I2C_MM_HS_TIM_PRESCALE_CMD_NODIV;
			NO_DIV = 0;
			DIV = I2C_MM_HS_TIM_DIV_CMD_DIV8;
		} else if (ref_clk == BSC_BUS_REF_104MHZ) {
			PRESCALE = I2C_MM_HS_TIM_PRESCALE_CMD_DIV4;
			NO_DIV = 0;
			DIV = I2C_MM_HS_TIM_DIV_CMD_DIV8;
		}
		break;
	case BSC_SPD_380K:
		M = 0x02;
		N = 0x04;
		P = 0x01;
		if (ref_clk == BSC_BUS_REF_13MHZ) {
			PRESCALE = I2C_MM_HS_TIM_PRESCALE_CMD_NODIV;
			NO_DIV = 0;
			DIV = I2C_MM_HS_TIM_DIV_CMD_DIV2;
		} else if (ref_clk == BSC_BUS_REF_26MHZ) {
			PRESCALE = I2C_MM_HS_TIM_PRESCALE_CMD_NODIV;
			NO_DIV = 0;
			DIV = I2C_MM_HS_TIM_DIV_CMD_DIV4;
		} else if (ref_clk == BSC_BUS_REF_104MHZ) {
			PRESCALE = I2C_MM_HS_TIM_PRESCALE_CMD_DIV4;
			NO_DIV = 0;
			DIV = I2C_MM_HS_TIM_DIV_CMD_DIV4;
		}
		break;
	case BSC_SPD_400K:
		M = 0x01;	/* 2; */
		N = 0x01;	/* 2; */
		P = 0x06;	/* 0x01; */
		if (ref_clk == BSC_BUS_REF_13MHZ) {
			PRESCALE = I2C_MM_HS_TIM_PRESCALE_CMD_NODIV;
			NO_DIV = 1;
			DIV = I2C_MM_HS_TIM_DIV_CMD_DIV4;
		} else if (ref_clk == BSC_BUS_REF_26MHZ) {
			PRESCALE = I2C_MM_HS_TIM_PRESCALE_CMD_NODIV;
			NO_DIV = 0;
			DIV = I2C_MM_HS_TIM_DIV_CMD_DIV2;
		} else if (ref_clk == BSC_BUS_REF_104MHZ) {
			PRESCALE = I2C_MM_HS_TIM_PRESCALE_CMD_NODIV;
			NO_DIV = 0;
			DIV = I2C_MM_HS_TIM_DIV_CMD_DIV8;
		}
		break;
	case BSC_SPD_430K:
		M = 0x03;
		N = 0x01;
		P = 0x01;
		if (ref_clk == BSC_BUS_REF_13MHZ) {
			PRESCALE = I2C_MM_HS_TIM_PRESCALE_CMD_NODIV;
			NO_DIV = 0;
			DIV = I2C_MM_HS_TIM_DIV_CMD_DIV2;
		} else if (ref_clk == BSC_BUS_REF_26MHZ) {
			PRESCALE = I2C_MM_HS_TIM_PRESCALE_CMD_NODIV;
			NO_DIV = 0;
			DIV = I2C_MM_HS_TIM_DIV_CMD_DIV4;
		} else if (ref_clk == BSC_BUS_REF_104MHZ) {
			PRESCALE = I2C_MM_HS_TIM_PRESCALE_CMD_DIV4;
			NO_DIV = 0;
			DIV = I2C_MM_HS_TIM_DIV_CMD_DIV4;
		}
		break;
	case BSC_SPD_MAXIMUM:
		M = 0x02;
		N = 0x01;
		P = 0x01;
		if (ref_clk == BSC_BUS_REF_13MHZ) {
			PRESCALE = I2C_MM_HS_TIM_PRESCALE_CMD_NODIV;
			NO_DIV = 0;
			DIV = I2C_MM_HS_TIM_DIV_CMD_DIV2;
		} else if (ref_clk == BSC_BUS_REF_26MHZ) {
			PRESCALE = I2C_MM_HS_TIM_PRESCALE_CMD_NODIV;
			NO_DIV = 0;
			DIV = I2C_MM_HS_TIM_DIV_CMD_DIV4;
		} else if (ref_clk == BSC_BUS_REF_104MHZ) {
			PRESCALE = I2C_MM_HS_TIM_PRESCALE_CMD_DIV4;
			NO_DIV = 0;
			DIV = I2C_MM_HS_TIM_DIV_CMD_DIV4;
		}
		break;
		/* M, N & P values are not valid for HS mode */
	case BSC_SPD_HS:
		M = 0x04;	/* 2; */
		N = 0x01;	/* 2; */
		P = 0x05;	/* 0x01; */
		DIV = I2C_MM_HS_TIM_DIV_CMD_DIV16;
		PRESCALE = I2C_MM_HS_TIM_PRESCALE_CMD_DIV1;
		NO_DIV = 1;
		if (ref_clk == BSC_BUS_REF_26MHZ)
			BSC_WRITE_REG((baseAddr + I2C_MM_HS_HSTIM_OFFSET),
				      BSC_HS_HSMODE_TIMING_26MHZ);
		else if (ref_clk == BSC_BUS_REF_104MHZ)
			BSC_WRITE_REG((baseAddr + I2C_MM_HS_HSTIM_OFFSET),
				      BSC_HS_HSMODE_TIMING);
		break;
	case BSC_SPD_HS_1MHZ:
		M = 0x04;	/* 2; */
		N = 0x01;	/* 2; */
		P = 0x05;	/* 0x01; */
		if (ref_clk == BSC_BUS_REF_26MHZ) {
			DIV = I2C_MM_HS_TIM_DIV_CMD_DIV2;
			PRESCALE = I2C_MM_HS_TIM_PRESCALE_CMD_DIV4;
			NO_DIV = 1;
			BSC_WRITE_REG((baseAddr + I2C_MM_HS_HSTIM_OFFSET),
				      BSC_HS_HSMODE_1MHZ_26MHZ);
		} else if (ref_clk == BSC_BUS_REF_104MHZ) {
			DIV = I2C_MM_HS_TIM_DIV_CMD_DIV2;
			PRESCALE = I2C_MM_HS_TIM_PRESCALE_CMD_DIV1;
			NO_DIV = 0;
			BSC_WRITE_REG((baseAddr + I2C_MM_HS_HSTIM_OFFSET),
				      BSC_HS_HSMODE_1MHZ);
		}
		break;
	case BSC_SPD_HS_2MHZ:
		M = 0x04;	/* 2; */
		N = 0x01;	/* 2; */
		P = 0x05;	/* 0x01; */
		if (ref_clk == BSC_BUS_REF_26MHZ) {
			DIV = I2C_MM_HS_TIM_DIV_CMD_DIV2;
			PRESCALE = I2C_MM_HS_TIM_PRESCALE_CMD_DIV4;
			NO_DIV = 1;
			BSC_WRITE_REG((baseAddr + I2C_MM_HS_HSTIM_OFFSET),
				      BSC_HS_HSMODE_2MHZ_26MHZ);
		} else if (ref_clk == BSC_BUS_REF_104MHZ) {
			DIV = I2C_MM_HS_TIM_DIV_CMD_DIV2;
			PRESCALE = I2C_MM_HS_TIM_PRESCALE_CMD_DIV1;
			NO_DIV = 1;
			BSC_WRITE_REG((baseAddr + I2C_MM_HS_HSTIM_OFFSET),
				      BSC_HS_HSMODE_2MHZ);
		}
		break;
	case BSC_SPD_HS_1625KHZ:
		M = 0x04;	/* 2; */
		N = 0x01;	/* 2; */
		P = 0x05;	/* 0x01; */
		if (ref_clk == BSC_BUS_REF_26MHZ) {
			DIV = I2C_MM_HS_TIM_DIV_CMD_DIV2;
			PRESCALE = I2C_MM_HS_TIM_PRESCALE_CMD_DIV4;
			NO_DIV = 1;
			BSC_WRITE_REG((baseAddr + I2C_MM_HS_HSTIM_OFFSET),
				      BSC_HS_HSMODE_1625KHZ_26MHZ);
		} else if (ref_clk == BSC_BUS_REF_104MHZ) {
			DIV = I2C_MM_HS_TIM_DIV_CMD_DIV2;
			PRESCALE = I2C_MM_HS_TIM_PRESCALE_CMD_DIV1;
			NO_DIV = 0;
			BSC_WRITE_REG((baseAddr + I2C_MM_HS_HSTIM_OFFSET),
				      BSC_HS_HSMODE_1625KHZ);
		}
		break;
	case BSC_SPD_HS_2600KHZ:
		M = 0x04;	/* 2; */
		N = 0x01;	/* 2; */
		P = 0x05;	/* 0x01; */
		if (ref_clk == BSC_BUS_REF_26MHZ) {
			DIV = I2C_MM_HS_TIM_DIV_CMD_DIV2;
			PRESCALE = I2C_MM_HS_TIM_PRESCALE_CMD_DIV4;
			NO_DIV = 1;
			BSC_WRITE_REG((baseAddr + I2C_MM_HS_HSTIM_OFFSET),
				      BSC_HS_HSMODE_2600KHZ_26MHZ);
		} else if (ref_clk == BSC_BUS_REF_104MHZ) {
			DIV = I2C_MM_HS_TIM_DIV_CMD_DIV2;
			PRESCALE = I2C_MM_HS_TIM_PRESCALE_CMD_DIV1;
			NO_DIV = 0;
			BSC_WRITE_REG((baseAddr + I2C_MM_HS_HSTIM_OFFSET),
				      BSC_HS_HSMODE_2600KHZ);
		}
		break;
		/* master clock is 26MHz for FPGA */
	case BSC_SPD_100K_FPGA:
		M = 0x01;
		N = 0x01;
		DIV = 2;
		P = 0x06;
		PRESCALE = 2;
		break;
	case BSC_SPD_400K_FPGA:
		M = 0x04;
		N = 0x01;
		DIV = 0x3;
		P = 0x05;
		PRESCALE = 3;
		break;
	case BSC_SPD_HS_FPGA:
		M = 0x04;
		N = 0x01;
		NO_DIV = 0;
		DIV = 3;
		PRESCALE = I2C_MM_HS_TIM_PRESCALE_CMD_NODIV;
		P = 0x07;
		BSC_WRITE_REG((baseAddr + I2C_MM_HS_HSTIM_OFFSET),
			      BSC_HS_HSMODE_TIMING);
		break;
	case BSC_SPD_100K:
	default:
		M = 0x01;
		N = 0x01;
		P = 0x06;	/* 2; */
		if (ref_clk == BSC_BUS_REF_13MHZ) {
			PRESCALE = I2C_MM_HS_TIM_PRESCALE_CMD_NODIV;
			NO_DIV = 0;
			DIV = I2C_MM_HS_TIM_DIV_CMD_DIV4;
		} else if (ref_clk == BSC_BUS_REF_26MHZ) {
			PRESCALE = I2C_MM_HS_TIM_PRESCALE_CMD_NODIV;
			NO_DIV = 0;
			DIV = I2C_MM_HS_TIM_DIV_CMD_DIV8;
		} else if (ref_clk == BSC_BUS_REF_104MHZ) {
			PRESCALE = I2C_MM_HS_TIM_PRESCALE_CMD_DIV4;
			NO_DIV = 0;
			DIV = I2C_MM_HS_TIM_DIV_CMD_DIV8;
		}
		break;
	}

	BSC_WRITE_REG_FIELD((baseAddr + I2C_MM_HS_TIM_OFFSET),
			    I2C_MM_HS_TIM_P_MASK, I2C_MM_HS_TIM_P_SHIFT, P);
	BSC_WRITE_REG_FIELD((baseAddr + I2C_MM_HS_TIM_OFFSET),
			    I2C_MM_HS_TIM_DIV_MASK, I2C_MM_HS_TIM_DIV_SHIFT,
			    DIV);
	BSC_WRITE_REG_FIELD((baseAddr + I2C_MM_HS_TIM_OFFSET),
			    I2C_MM_HS_TIM_NO_DIV_MASK,
			    I2C_MM_HS_TIM_NO_DIV_SHIFT, NO_DIV);
	BSC_WRITE_REG_FIELD((baseAddr + I2C_MM_HS_TIM_OFFSET),
			    I2C_MM_HS_TIM_PRESCALE_MASK,
			    I2C_MM_HS_TIM_PRESCALE_SHIFT, PRESCALE);
	BSC_WRITE_REG_FIELD((baseAddr + I2C_MM_HS_CLKEN_OFFSET),
			    I2C_MM_HS_CLKEN_M_MASK, I2C_MM_HS_CLKEN_M_SHIFT, M);
	BSC_WRITE_REG_FIELD((baseAddr + I2C_MM_HS_CLKEN_OFFSET),
			    I2C_MM_HS_CLKEN_N_MASK, I2C_MM_HS_CLKEN_N_SHIFT, N);
}

/**
*
*  @brief  Get the current value of the BSC_TIM register (used for high-speed)
*
*  @param  baseAddr  (in) mapped address of this BSC instance
*
*  @return current value of the BSC_TIM register
*****************************************************************************/
static inline uint32_t bsc_get_tim(uint32_t baseAddr)
{
	return (BSC_READ_REG(baseAddr + I2C_MM_HS_TIM_OFFSET));
}

/**
*
*  @brief  Set the BSC_TIM register (used for high-speed)
*
*  @param  baseAddr  (in) mapped address of this BSC instance
*  @param  baseAddr  (in) value to set
*
*  @return none
*****************************************************************************/
static inline void bsc_set_tim(uint32_t baseAddr, uint32_t val)
{
	BSC_WRITE_REG((baseAddr + I2C_MM_HS_TIM_OFFSET), val);
}

/**
*
*  @brief  Start High speed
*
*  @param  baseAddr  (in) mapped address of this BSC instance
*  @param  speed     (in) speed to be set
*
*  @return none
*****************************************************************************/

static inline void bsc_start_highspeed(uint32_t baseAddr)
{
#ifdef FPGA
	uint32_t val;

	val = (7 << I2C_MM_HS_TIM_P_SHIFT) |
	    (I2C_MM_HS_TIM_PRESCALE_CMD_NODIV << I2C_MM_HS_TIM_PRESCALE_SHIFT) |
	    (I2C_MM_HS_TIM_DIV_CMD_DIV2);

	BSC_WRITE_REG((baseAddr + I2C_MM_HS_TIM_OFFSET), val);
#endif

	/* Configure Hs-mode timing register */
	BSC_WRITE_REG_FIELD((baseAddr + I2C_MM_HS_HSTIM_OFFSET),
			    I2C_MM_HS_HSTIM_HS_MODE_MASK,
			    I2C_MM_HS_HSTIM_HS_MODE_SHIFT, 1);
}

/**
*
*  @brief  Stop High speed
*
*  @param  baseAddr  (in) mapped address of this BSC instance
*  @param  speed     (in) speed to be set
*
*  @return none
*****************************************************************************/
static inline void bsc_stop_highspeed(uint32_t baseAddr)
{
	/* Disable Hs-mode - leave other Hs-mode timing values untouched */
	BSC_WRITE_REG_FIELD((baseAddr + I2C_MM_HS_HSTIM_OFFSET),
			    I2C_MM_HS_HSTIM_HS_MODE_MASK,
			    I2C_MM_HS_HSTIM_HS_MODE_SHIFT, 0);
}

/**
*
*  @brief  Flush the TX fifo
*
*  @param  baseAddr  (in) mapped address of this BSC instance
*
*  @return none
*****************************************************************************/
static inline void bsc_flush_tx_fifo(uint32_t baseAddr)
{
		BSC_WRITE_REG_FIELD((baseAddr + I2C_MM_HS_TXFCR_OFFSET),
				    I2C_MM_HS_TXFCR_FIFO_FLUSH_MASK,
				    I2C_MM_HS_TXFCR_FIFO_FLUSH_SHIFT, 1);
}

/**
*
*  @brief  Enable BSC FIFO
*
*  @param  baseAddr  (in) mapped address of this BSC instance
*  @param  enable    (in) enable=1 to enable FIFO, enable=0 to
*                         disable FIFO
*
*  @return set mode
*****************************************************************************/
static inline void bsc_set_tx_fifo(uint32_t baseAddr, unsigned char enable)
{
	if (enable) {
		BSC_WRITE_REG_FIELD((baseAddr + I2C_MM_HS_TXFCR_OFFSET),
				    I2C_MM_HS_TXFCR_FIFO_FLUSH_MASK,
				    I2C_MM_HS_TXFCR_FIFO_FLUSH_SHIFT, 1);
		BSC_WRITE_REG_FIELD((baseAddr + I2C_MM_HS_TXFCR_OFFSET),
				    I2C_MM_HS_TXFCR_FIFO_EN_MASK,
				    I2C_MM_HS_TXFCR_FIFO_EN_SHIFT, 1);
	} else {
		BSC_WRITE_REG_FIELD((baseAddr + I2C_MM_HS_TXFCR_OFFSET),
				    I2C_MM_HS_TXFCR_FIFO_FLUSH_MASK,
				    I2C_MM_HS_TXFCR_FIFO_FLUSH_SHIFT, 1);
		BSC_WRITE_REG_FIELD((baseAddr + I2C_MM_HS_TXFCR_OFFSET),
				    I2C_MM_HS_TXFCR_FIFO_EN_MASK,
				    I2C_MM_HS_TXFCR_FIFO_EN_SHIFT, 0);
	}
}

static inline void bsc_start_rx_fifo(uint32_t baseAddr, unsigned int noack,
				     unsigned int len)
{
	uint32_t val;

	val = ((noack ? 1 : 0) << I2C_MM_HS_RXFCR_NACK_EN_SHIFT) |
	    ((len << I2C_MM_HS_RXFCR_READ_COUNT_SHIFT) &
	     I2C_MM_HS_RXFCR_READ_COUNT_MASK);

	BSC_WRITE_REG((baseAddr + I2C_MM_HS_RXFCR_OFFSET), val);
}

static uint8_t bsc_read_from_rx_fifo(uint32_t baseAddr)
{
	return (BSC_READ_REG(baseAddr + I2C_MM_HS_RXFIFORDOUT_OFFSET) &
		I2C_MM_HS_RXFIFORDOUT_RXFIFO_RDOUT_MASK);
}

/**
*
*  @brief  Set BSC mode
*
*  @param  baseAddr  (in) mapped address of this BSC instance
*  @param  mode      (in) mode to be set
*
*  @return set mode
*****************************************************************************/
static inline BSC_MODE_t bsc_set_mode(uint32_t baseAddr, BSC_MODE_t mode)
{
	(void)baseAddr;

	if (mode != BSC_MODE_MASTER) {
		bsc_dprintf(CDBG_ERRO,
			    "ERROR: bsc_set_mode: no slave support\n");
	}

	return BSC_MODE_MASTER;
}

/**
*
*  @brief  Enable BSC interrupt
*
*  @param  baseAddr  (in) mapped address of this BSC instance
*  @param  mask      (in) interrupts to enable
*
*  @return none
*****************************************************************************/
static inline void bsc_enable_intr(uint32_t baseAddr, uint32_t mask)
{
	/* clear pending interrupts */
	BSC_WRITE_REG((baseAddr + I2C_MM_HS_ISR_OFFSET), mask);

	BSC_WRITE_REG((baseAddr + I2C_MM_HS_IER_OFFSET),
		      (bsc_get_enabled_intr(baseAddr) | mask));
}

/**
*
*  @brief  Disable BSC interrupt
*
*  @param  baseAddr  (in) mapped address of this BSC instance
*  @param  mask      (in) interrupts to disable
*
*  @return none
*****************************************************************************/
static inline void bsc_disable_intr(uint32_t baseAddr, uint32_t mask)
{
	BSC_WRITE_REG((baseAddr + I2C_MM_HS_IER_OFFSET),
		      (bsc_get_enabled_intr(baseAddr) & (~mask)));
}

/**
*
*  @brief  Get enabled BSC interrupt
*
*  @param  baseAddr  (in) mapped address of this BSC instance
*  @param  mask      (in) interrupts to disable
*
*  @return none
*****************************************************************************/
static inline uint32_t bsc_get_enabled_intr(uint32_t baseAddr)
{
	return (BSC_READ_REG(baseAddr + I2C_MM_HS_IER_OFFSET) &
		(~I2C_MM_HS_IER_RESERVED_MASK));
}

/**
*
*  @brief  Read BSC interrupt status
*
*  @param  baseAddr  (in) mapped address of this BSC instance
*
*  @return status mask
*
*  @note
*****************************************************************************/
static inline uint32_t bsc_read_intr_status(uint32_t baseAddr)
{
	return (BSC_READ_REG(baseAddr + I2C_MM_HS_ISR_OFFSET) &
		(~I2C_MM_HS_ISR_RESERVED_MASK));
}

/**
*
*  @brief  Clear BSC interrupt status
*
*  @param  baseAddr  (in) mapped address of this BSC instance
*
*  @return status mask
*
*  @note
*****************************************************************************/
static inline void bsc_clear_intr_status(uint32_t baseAddr, uint32_t mask)
{
	BSC_WRITE_REG((baseAddr + I2C_MM_HS_ISR_OFFSET),
		      (bsc_read_intr_status(baseAddr) & (mask)));
}

/**
*
*  @brief  Send a cmd to BSC controller
*
*  @param  baseAddr  (in) mapped address of this BSC instance
*  @param  cmd       (in) cmd to be executed
*
*  @return none
*****************************************************************************/
static inline void isl_bsc_send_cmd(uint32_t baseAddr, BSC_CMD_t cmd)
{
	uint8_t temp;

	temp = BSC_READ_REG((baseAddr + I2C_MM_HS_CS_OFFSET));

	bsc_dprintf(CDBG_INFO, "bsc_send_cmd, %d\n", cmd);
	switch (cmd) {
	case BSC_CMD_NOACTION:
		temp =
		    (temp & ~I2C_MM_HS_CS_CMD_MASK & ~I2C_MM_HS_CS_ACK_MASK) |
		    (I2C_MM_HS_CS_CMD_CMD_NO_ACTION << I2C_MM_HS_CS_CMD_SHIFT);
		break;
	case BSC_CMD_START:
		temp =
		    (temp & ~I2C_MM_HS_CS_ACK_MASK & ~I2C_MM_HS_CS_CMD_MASK) |
		    (I2C_MM_HS_CS_CMD_CMD_GEN_START << I2C_MM_HS_CS_CMD_SHIFT);
		break;
	case BSC_CMD_RESTART:
		temp = (temp & ~I2C_MM_HS_CS_CMD_MASK) | I2C_MM_HS_CS_ACK_MASK |
		    (I2C_MM_HS_CS_CMD_CMD_GEN_START << I2C_MM_HS_CS_CMD_SHIFT);
		break;
	case BSC_CMD_STOP:
		temp = (temp & ~I2C_MM_HS_CS_CMD_MASK) |
		    I2C_MM_HS_CS_CMD_CMD_GEN_STOP << I2C_MM_HS_CS_CMD_SHIFT;
		break;

	case BSC_CMD_READ_ACK:
		temp =
		    (temp & ~I2C_MM_HS_CS_ACK_MASK & ~I2C_MM_HS_CS_CMD_MASK) |
		    (I2C_MM_HS_CS_CMD_CMD_RD_A_BYTE << I2C_MM_HS_CS_CMD_SHIFT);
		break;
	case BSC_CMD_READ_NAK:
		temp = (temp & ~I2C_MM_HS_CS_CMD_MASK) | I2C_MM_HS_CS_ACK_MASK |
		    (I2C_MM_HS_CS_CMD_CMD_RD_A_BYTE << I2C_MM_HS_CS_CMD_SHIFT);
		break;
	case BSC_CMD_HS_STOP:
		/* For high speed stop, need to set different value for BSC_TIM */
		{
			uint32_t val;
			/* Configure PRESCALE, p & DIV bit field */
			val =
			    (I2C_MM_HS_TIM_PRESCALE_CMD_DIV4 <<
			     I2C_MM_HS_TIM_PRESCALE_SHIFT) | (5 <<
							      I2C_MM_HS_TIM_P_SHIFT)
			    | (I2C_MM_HS_TIM_DIV_CMD_DIV2);
			BSC_WRITE_REG((baseAddr + I2C_MM_HS_TIM_OFFSET), val);
		}
		temp = (temp & ~I2C_MM_HS_CS_CMD_MASK) |
		    I2C_MM_HS_CS_CMD_CMD_GEN_STOP << I2C_MM_HS_CS_CMD_SHIFT;
		break;
	}

	BSC_WRITE_REG((baseAddr + I2C_MM_HS_CS_OFFSET), temp);
}

/**
*
*  @brief  Read data from BSC FIFO
*
*  @param  baseAddr  (in) mapped address of this BSC instance
*  @param  pBuffer   (i/o) Buffer for data read
*  @param  size      (in) Buffer size
*
*  @return Number of bytes read
*****************************************************************************/
static inline uint32_t bsc_read_data(uint32_t baseAddr, uint8_t *pBuffer,
				     uint32_t size)
{
	(void)size;

	pBuffer[0] = BSC_READ_REG(baseAddr + I2C_MM_HS_DAT_OFFSET);

	/* no RX FIFO */
	return 1;
}

/**
*
*  @brief  Write data to BSC FIFO
*
*  @param  baseAddr  (in) mapped address of this BSC instance
*  @param  pBuffer   (in) Buffer to write data from
*  @param  size      (in) Buffer size
*
*  @return Number of bytes written
*****************************************************************************/

static inline uint32_t bsc_write_data(uint32_t baseAddr, uint8_t *pBuffer,
				      uint32_t size)
{
	uint16_t i = 0;
	for (i = 0; i < size; i++) {
		BSC_WRITE_REG((baseAddr + I2C_MM_HS_DAT_OFFSET), pBuffer[i]);
	}
	return size;
}

/**
*
*  @brief  Set timeout
*
*  @param  baseAddr  (in) mapped address of this BSC instance
*
*  @return none
*****************************************************************************/
static inline void bsc_set_timeout(uint32_t baseAddr, unsigned char on)
{
	if (on)
		BSC_WRITE_REG_FIELD((baseAddr + I2C_MM_HS_TOUT_OFFSET),
				    I2C_MM_HS_TOUT_TE_MASK,
				    I2C_MM_HS_TOUT_TE_SHIFT, 1);
	else
		BSC_WRITE_REG_FIELD((baseAddr + I2C_MM_HS_TOUT_OFFSET),
				    I2C_MM_HS_TOUT_TE_MASK,
				    I2C_MM_HS_TOUT_TE_SHIFT, 0);
}

/**
*
*  @brief  Get timeout
*
*  @param  baseAddr  (in) mapped address of this BSC instance
*
*  @return none
*****************************************************************************/
static inline unsigned char bsc_get_timeout(uint32_t baseAddr)
{
	uint8_t temp;

	temp =
	    BSC_READ_REG_FIELD((baseAddr + I2C_MM_HS_TOUT_OFFSET),
			       I2C_MM_HS_TOUT_TE_MASK, I2C_MM_HS_TOUT_TE_SHIFT);

	return temp;
}

/**
*
*  @brief  Get auto sense status
*
*  @param  baseAddr  (in) mapped address of this BSC instance
*
*  @return TRUE for on; FALSE for off
*****************************************************************************/
static inline unsigned char bsc_get_autosense(uint32_t baseAddr)
{
	uint8_t temp;

	temp =
	    BSC_READ_REG_FIELD((baseAddr + I2C_MM_HS_CLKEN_OFFSET),
			       I2C_MM_HS_CLKEN_AUTOSENSE_OFF_MASK,
			       I2C_MM_HS_CLKEN_AUTOSENSE_OFF_SHIFT);

	return (!(temp));
}

/**
*
*  @brief  Set auto sense status
*
*  @param  baseAddr  (in) mapped address of this BSC instance
*
*  @return none
*****************************************************************************/
static inline void bsc_set_autosense(uint32_t baseAddr, unsigned char on,
				     unsigned char timeout_enable)
{
	if (on) {
		BSC_WRITE_REG_FIELD((baseAddr + I2C_MM_HS_CLKEN_OFFSET),
				    I2C_MM_HS_CLKEN_AUTOSENSE_OFF_MASK,
				    I2C_MM_HS_CLKEN_AUTOSENSE_OFF_SHIFT, 0);
		/* When autosense is ON, a device may stretch clock very long time
		   We use max timeout value (1023 6.5MHz cycles) */
		BSC_WRITE_REG_FIELD((baseAddr + I2C_MM_HS_TOUT_OFFSET),
				    I2C_MM_HS_TOUT_TOUT_LOW_MASK,
				    I2C_MM_HS_TOUT_TOUT_LOW_SHIFT, 0x7F);
		BSC_WRITE_REG_FIELD((baseAddr + I2C_MM_HS_TOUT_OFFSET),
				    I2C_MM_HS_TOUT_TOUT_HIGH_MASK,
				    I2C_MM_HS_TOUT_TOUT_HIGH_SHIFT, 0x7);
		bsc_set_timeout(baseAddr, timeout_enable);
	} else {
		BSC_WRITE_REG_FIELD((baseAddr + I2C_MM_HS_CLKEN_OFFSET),
				    I2C_MM_HS_CLKEN_AUTOSENSE_OFF_MASK,
				    I2C_MM_HS_CLKEN_AUTOSENSE_OFF_SHIFT, 1);
		/* Timeout feature is not used when autosense is off */
		bsc_set_timeout(baseAddr, 0);
	}
}

/**
*
*  @brief  Get bus status
*
*  @param  baseAddr  (in) mapped address of this BSC instance
*
*  @return none
*****************************************************************************/
static inline unsigned char bsc_get_bus_status(uint32_t baseAddr)
{
	uint8_t temp;

	temp =
	    BSC_READ_REG_FIELD((baseAddr + I2C_MM_HS_TXCOUNT_OFFSET),
			       I2C_MM_HS_TXCOUNT_STATUS_MASK,
			       I2C_MM_HS_TXCOUNT_STATUS_SHIFT);

	return temp;
}

/**
*
*  @brief  Get the number of bytes in the TX FIFO
*
*  @param  baseAddr  (in) mapped address of this BSC instance
*
*  @return Number of bytes present in the TX FIFO
*****************************************************************************/
static inline unsigned char bsc_get_tx_fifo_count(uint32_t baseAddr)
{
	uint8_t temp;

	temp =
	    BSC_READ_REG_FIELD((baseAddr + I2C_MM_HS_TXCOUNT_OFFSET),
			       I2C_MM_HS_TXCOUNT_FIFOCNT_MASK,
			       I2C_MM_HS_TXCOUNT_FIFOCNT_SHIFT);

	return temp;
}

static inline int bsc_tx_fifo_is_full(uint32_t baseAddr)
{
	uint32_t val;

	val =
	    BSC_READ_REG(baseAddr +
			 I2C_MM_HS_FIFO_STATUS_OFFSET) &
	    I2C_MM_HS_FIFO_STATUS_TXFIFO_FULL_MASK;
	return (val ? 1 : 0);
}

static inline int bsc_tx_fifo_is_empty(uint32_t baseAddr)
{
	uint32_t val;

	val =
	    BSC_READ_REG(baseAddr +
			 I2C_MM_HS_FIFO_STATUS_OFFSET) &
	    I2C_MM_HS_FIFO_STATUS_TXFIFO_EMPTY_MASK;
	return (val ? 1 : 0);
}

static inline int bsc_rx_fifo_is_full(uint32_t baseAddr)
{
	uint32_t val;

	val =
	    BSC_READ_REG(baseAddr +
			 I2C_MM_HS_FIFO_STATUS_OFFSET) &
	    I2C_MM_HS_FIFO_STATUS_RXFIFO_FULL_MASK;
	return (val ? 1 : 0);
}

static inline int bsc_rx_fifo_is_empty(uint32_t baseAddr)
{
	uint32_t val;

	val =
	    BSC_READ_REG(baseAddr +
			 I2C_MM_HS_FIFO_STATUS_OFFSET) &
	    I2C_MM_HS_FIFO_STATUS_RXFIFO_EMPTY_MASK;
	return (val ? 1 : 0);
}

/**
*
*  @brief  Get ACK after writing a byte
*
*  @param  baseAddr  (in) mapped address of this BSC instance
*
*  @return none
*****************************************************************************/
static inline unsigned char bsc_get_ack(uint32_t baseAddr)
{
	uint8_t temp;

	temp =
	    BSC_READ_REG_FIELD((baseAddr + I2C_MM_HS_CS_OFFSET),
			       I2C_MM_HS_CS_ACK_MASK, I2C_MM_HS_CS_ACK_SHIFT);

	/* ACK is active low */
	if (temp)
		return 0;
	else
		return 1;
}

/**
*
*  @brief  Get SDA line state for Bit-Banging implementation
*
*  @param  baseAddr  (in) mapped address of this BSC instance
*
*  @return STA line state
*****************************************************************************/
static inline unsigned char bsc_get_sda(uint32_t baseAddr)
{
	uint8_t temp;

	temp =
	    BSC_READ_REG_FIELD((baseAddr + I2C_MM_HS_CS_OFFSET),
			       I2C_MM_HS_CS_SDA_MASK, I2C_MM_HS_CS_SDA_SHIFT);

	return temp;
}

/**
*
*  @brief  Set SDA line for Bit-Banging implementation
*
*  @param  baseAddr  (in) mapped address of this BSC instance
*  @param  state     (in) state to be set
*
*  @return none
*****************************************************************************/
static inline void bsc_set_sda(uint32_t baseAddr, unsigned char state)
{
	if (state)
		BSC_WRITE_REG_FIELD((baseAddr + I2C_MM_HS_CS_OFFSET),
				    I2C_MM_HS_CS_SDA_MASK,
				    I2C_MM_HS_CS_SDA_SHIFT, 1);
	else
		BSC_WRITE_REG_FIELD((baseAddr + I2C_MM_HS_CS_OFFSET),
				    I2C_MM_HS_CS_SDA_MASK,
				    I2C_MM_HS_CS_SDA_SHIFT, 0);
}

/**
*
*  @brief  Get SCL line state for Bit-Bangling implementation
*
*  @param  baseAddr  (in) mapped address of this BSC instance
*
*  @return SCL line state
*****************************************************************************/
static inline unsigned char bsc_get_scl(uint32_t baseAddr)
{
	uint8_t temp;

	temp =
	    BSC_READ_REG_FIELD((baseAddr + I2C_MM_HS_CS_OFFSET),
			       I2C_MM_HS_CS_SCL_MASK, I2C_MM_HS_CS_SCL_SHIFT);

	return temp;
}

/**
*
*  @brief  Set SCL line for Bit-Bangling implementation
*
*  @param  baseAddr  (in) mapped address of this BSC instance
*  @param  state     (in) state to be set
*
*  @return none
*****************************************************************************/
static inline void bsc_set_scl(uint32_t baseAddr, unsigned char state)
{
	if (state)
		BSC_WRITE_REG_FIELD((baseAddr + I2C_MM_HS_CS_OFFSET),
				    I2C_MM_HS_CS_SCL_MASK,
				    I2C_MM_HS_CS_SCL_SHIFT, 1);
	else
		BSC_WRITE_REG_FIELD((baseAddr + I2C_MM_HS_CS_OFFSET),
				    I2C_MM_HS_CS_SCL_MASK,
				    I2C_MM_HS_CS_SCL_SHIFT, 0);
}

/**
*
*  @brief  Set SCL line for Bit-Bangling implementation
*
*  @param  baseAddr  (in) mapped address of this BSC instance
*  @param  state     (in) state to be set
*
*  @return none
*****************************************************************************/
static inline void bsc_set_soft_reset(uint32_t baseAddr, unsigned char enable)
{
	if (enable)
		BSC_WRITE_REG_FIELD((baseAddr + I2C_MM_HS_SFTRST_OFFSET),
				    I2C_MM_HS_SFTRST_SWRST_MASK,
				    I2C_MM_HS_SFTRST_SWRST_SHIFT, 1);
	else
		BSC_WRITE_REG_FIELD((baseAddr + I2C_MM_HS_SFTRST_OFFSET),
				    I2C_MM_HS_SFTRST_SWRST_MASK,
				    I2C_MM_HS_SFTRST_SWRST_SHIFT, 0);
}

/**
*
*  @brief  Set SCL line for Bit-Bangling implementation
*
*  @param  baseAddr  (in) mapped address of this BSC instance
*  @param  state     (in) state to be set
*
*  @return none
*****************************************************************************/
static inline unsigned char bsc_get_soft_reset(uint32_t baseAddr)
{
	return (BSC_READ_REG_FIELD
		((baseAddr + I2C_MM_HS_SFTRST_OFFSET),
		 I2C_MM_HS_SFTRST_SWRST_MASK, I2C_MM_HS_SFTRST_SWRST_SHIFT));
}

/**
*
*  @brief  Set SCL line for Bit-Bangling implementation
*
*  @param  baseAddr  (in) mapped address of this BSC instance
*  @param  state     (in) state to be set
*
*  @return none
*****************************************************************************/
static inline unsigned char bsc_get_soft_reset_ready(uint32_t baseAddr)
{
	return (BSC_READ_REG_FIELD
		((baseAddr + I2C_MM_HS_SFTRST_OFFSET),
		 I2C_MM_HS_SFTRST_SWRST_RDY_MASK,
		 I2C_MM_HS_SFTRST_SWRST_RDY_SHIFT));
}

/**
*
*  @brief  Enable/Disable pad output
*
*  @param  baseAddr  (in) mapped address of this BSC instance
*  @param  enable    (in) state of the pad output to be set
*
*  @return none
*****************************************************************************/
static inline void bsc_enable_pad_output(uint32_t baseAddr, bool enable)
{
	if (enable)
		BSC_WRITE_REG_FIELD((baseAddr + I2C_MM_HS_PADCTL_OFFSET),
				    I2C_MM_HS_PADCTL_PAD_OUT_EN_MASK,
				    I2C_MM_HS_PADCTL_PAD_OUT_EN_SHIFT, 0);
	else
		BSC_WRITE_REG_FIELD((baseAddr + I2C_MM_HS_PADCTL_OFFSET),
				    I2C_MM_HS_PADCTL_PAD_OUT_EN_MASK,
				    I2C_MM_HS_PADCTL_PAD_OUT_EN_SHIFT, 1);
}

/**
*
*  @brief  Enable/Disable thigh ctrl
*
*  @param  baseAddr  (in) mapped address of this BSC instance
*  @param  enable    (in) state of the thigh ctrl bit
*
*  @return none
*****************************************************************************/
static inline void bsc_enable_thigh_ctrl(uint32_t baseAddr, bool enable)
{
	if (enable)
		BSC_WRITE_REG_FIELD((baseAddr + I2C_MM_HS_SLAVE_RCM_OFFSET),
			I2C_MM_HS_SLAVE_RCM_THIGH_CTRL_MASK,
			I2C_MM_HS_SLAVE_RCM_THIGH_CTRL_SHIFT, 1);
	else
		BSC_WRITE_REG_FIELD((baseAddr + I2C_MM_HS_SLAVE_RCM_OFFSET),
			I2C_MM_HS_SLAVE_RCM_THIGH_CTRL_MASK,
			I2C_MM_HS_SLAVE_RCM_THIGH_CTRL_SHIFT, 0);

}

/*
 * Copyright (c) 2016-2020 The Linux Foundation. All rights reserved.
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

#include "tcl_data_cmd.h"
#include "mac_tcl_reg_seq_hwioreg.h"
#include "phyrx_rssi_legacy.h"
#include "hal_hw_headers.h"
#include "hal_internal.h"
#include "cdp_txrx_mon_struct.h"
#include "qdf_trace.h"
#include "hal_rx.h"
#include "hal_tx.h"
#include "dp_types.h"
#include "hal_api_mon.h"

/**
 * hal_tx_desc_set_dscp_tid_table_id_6290() - Sets DSCP to TID conversion
 *						table ID
 * @desc: Handle to Tx Descriptor
 * @id: DSCP to tid conversion table to be used for this frame
 *
 * Return: void
 */
#if defined(QCA_WIFI_QCA6290_11AX)
static void hal_tx_desc_set_dscp_tid_table_id_6290(void *desc,
						   uint8_t id)
{
	HAL_SET_FLD(desc, TCL_DATA_CMD_5,
		    DSCP_TID_TABLE_NUM) |=
	HAL_TX_SM(TCL_DATA_CMD_5,
		  DSCP_TID_TABLE_NUM, id);
}
#else
static void hal_tx_desc_set_dscp_tid_table_id_6290(void *desc,
						   uint8_t id)
{
	HAL_SET_FLD(desc, TCL_DATA_CMD_3,
		    DSCP_TO_TID_PRIORITY_TABLE_ID) |=
			HAL_TX_SM(TCL_DATA_CMD_3,
				  DSCP_TO_TID_PRIORITY_TABLE_ID, id);
}
#endif


#define DSCP_TID_TABLE_SIZE 24
#define NUM_WORDS_PER_DSCP_TID_TABLE (DSCP_TID_TABLE_SIZE / 4)

#if defined(QCA_WIFI_QCA6290_11AX)
/**
 * hal_tx_set_dscp_tid_map_6290() - Configure default DSCP to TID map table
 * @soc: HAL SoC context
 * @map: DSCP-TID mapping table
 * @id: mapping table ID - 0-31
 *
 * DSCP are mapped to 8 TID values using TID values programmed
 * in any of the 32 DSCP_TID_MAPS (id = 0-31).
 *
 * Return: none
 */
static void hal_tx_set_dscp_tid_map_6290(struct hal_soc *soc,
					 uint8_t *map,
					 uint8_t id)
{
	int i;
	uint32_t addr, cmn_reg_addr;
	uint32_t value = 0, regval;
	uint8_t val[DSCP_TID_TABLE_SIZE], cnt = 0;

	if (id >= HAL_MAX_HW_DSCP_TID_MAPS_11AX)
		return;

	cmn_reg_addr = HWIO_TCL_R0_CONS_RING_CMN_CTRL_REG_ADDR(
					SEQ_WCSS_UMAC_MAC_TCL_REG_OFFSET);

	addr = HWIO_TCL_R0_DSCP_TID_MAP_n_ADDR(
				SEQ_WCSS_UMAC_MAC_TCL_REG_OFFSET,
				id * NUM_WORDS_PER_DSCP_TID_TABLE);

	/* Enable read/write access */
	regval = HAL_REG_READ(soc, cmn_reg_addr);
	regval |=
	(1 << HWIO_TCL_R0_CONS_RING_CMN_CTRL_REG_DSCP_TID_MAP_PROGRAM_EN_SHFT);

	HAL_REG_WRITE(soc, cmn_reg_addr, regval);

	/* Write 8 (24 bits) DSCP-TID mappings in each interation */
	for (i = 0; i < 64; i += 8) {
		value = (map[i] |
			(map[i + 1] << 0x3) |
			(map[i + 2] << 0x6) |
			(map[i + 3] << 0x9) |
			(map[i + 4] << 0xc) |
			(map[i + 5] << 0xf) |
			(map[i + 6] << 0x12) |
			(map[i + 7] << 0x15));

		qdf_mem_copy(&val[cnt], &value, 3);
		cnt += 3;
	}

	for (i = 0; i < DSCP_TID_TABLE_SIZE; i += 4) {
		regval = *(uint32_t *)(val + i);
		HAL_REG_WRITE(soc, addr,
			      (regval & HWIO_TCL_R0_DSCP_TID_MAP_n_RMSK));
		addr += 4;
	}

	/* Diasble read/write access */
	regval = HAL_REG_READ(soc, cmn_reg_addr);
	regval &=
	~(HWIO_TCL_R0_CONS_RING_CMN_CTRL_REG_DSCP_TID_MAP_PROGRAM_EN_BMSK);

	HAL_REG_WRITE(soc, cmn_reg_addr, regval);
}
#else
static void hal_tx_set_dscp_tid_map_6290(struct hal_soc *soc,
					 uint8_t *map,
					 uint8_t id)
{
	int i;
	uint32_t addr;
	uint32_t value;

	if (id == HAL_TX_DSCP_TID_MAP_TABLE_DEFAULT) {
		addr =
			HWIO_TCL_R0_DSCP_TID1_MAP_0_ADDR(
					SEQ_WCSS_UMAC_MAC_TCL_REG_OFFSET);
	} else {
		addr =
			HWIO_TCL_R0_DSCP_TID2_MAP_0_ADDR(
					SEQ_WCSS_UMAC_MAC_TCL_REG_OFFSET);
	}

	for (i = 0; i < 64; i += 10) {
		value = (map[i] |
			(map[i+1] << HWIO_TCL_R0_DSCP_TID1_MAP_0_DSCP_1_SHFT) |
			(map[i+2] << HWIO_TCL_R0_DSCP_TID1_MAP_0_DSCP_2_SHFT) |
			(map[i+3] << HWIO_TCL_R0_DSCP_TID1_MAP_0_DSCP_3_SHFT) |
			(map[i+4] << HWIO_TCL_R0_DSCP_TID1_MAP_0_DSCP_4_SHFT) |
			(map[i+5] << HWIO_TCL_R0_DSCP_TID1_MAP_0_DSCP_5_SHFT) |
			(map[i+6] << HWIO_TCL_R0_DSCP_TID1_MAP_0_DSCP_6_SHFT) |
			(map[i+7] << HWIO_TCL_R0_DSCP_TID1_MAP_0_DSCP_7_SHFT) |
			(map[i+8] << HWIO_TCL_R0_DSCP_TID1_MAP_0_DSCP_8_SHFT) |
			(map[i+9] << HWIO_TCL_R0_DSCP_TID1_MAP_0_DSCP_9_SHFT));

		HAL_REG_WRITE(soc, addr,
				(value & HWIO_TCL_R0_DSCP_TID1_MAP_1_RMSK));

		addr += 4;
	}
}
#endif

#ifdef QCA_WIFI_QCA6290_11AX
/**
 * hal_tx_update_dscp_tid_6290() - Update the dscp tid map table as updated
 *					by the user
 * @soc: HAL SoC context
 * @map: DSCP-TID mapping table
 * @id : MAP ID
 * @dscp: DSCP_TID map index
 *
 * Return: void
 */
static void hal_tx_update_dscp_tid_6290(struct hal_soc *soc, uint8_t tid,
					uint8_t id, uint8_t dscp)
{
	int index;
	uint32_t addr;
	uint32_t value;
	uint32_t regval;

	addr = HWIO_TCL_R0_DSCP_TID_MAP_n_ADDR(
			SEQ_WCSS_UMAC_MAC_TCL_REG_OFFSET, id);

	index = dscp % HAL_TX_NUM_DSCP_PER_REGISTER;
	addr += 4 * (dscp / HAL_TX_NUM_DSCP_PER_REGISTER);
	value = tid << (HAL_TX_BITS_PER_TID * index);

	regval = HAL_REG_READ(soc, addr);
	regval &= ~(HAL_TX_TID_BITS_MASK << (HAL_TX_BITS_PER_TID * index));
	regval |= value;

	HAL_REG_WRITE(soc, addr, (regval & HWIO_TCL_R0_DSCP_TID_MAP_n_RMSK));
}
#else
static void hal_tx_update_dscp_tid_6290(struct hal_soc *soc, uint8_t tid,
					uint8_t id, uint8_t dscp)
{
	int index;
	uint32_t addr;
	uint32_t value;
	uint32_t regval;

	if (id == HAL_TX_DSCP_TID_MAP_TABLE_DEFAULT)
		addr =
			HWIO_TCL_R0_DSCP_TID1_MAP_0_ADDR(
					SEQ_WCSS_UMAC_MAC_TCL_REG_OFFSET);
	else
		addr =
			HWIO_TCL_R0_DSCP_TID2_MAP_0_ADDR(
					SEQ_WCSS_UMAC_MAC_TCL_REG_OFFSET);

	index = dscp % HAL_TX_NUM_DSCP_PER_REGISTER;
	addr += 4 * (dscp/HAL_TX_NUM_DSCP_PER_REGISTER);
	value = tid << (HAL_TX_BITS_PER_TID * index);

	/* Read back previous DSCP TID config and update
	 * with new config.
	 */
	regval = HAL_REG_READ(soc, addr);
	regval &= ~(HAL_TX_TID_BITS_MASK << (HAL_TX_BITS_PER_TID * index));
	regval |= value;

	HAL_REG_WRITE(soc, addr,
			(regval & HWIO_TCL_R0_DSCP_TID1_MAP_1_RMSK));
}
#endif

#ifdef QCA_WIFI_QCA6290_11AX
/**
 * hal_tx_desc_set_lmac_id - Set the lmac_id value
 * @desc: Handle to Tx Descriptor
 * @lmac_id: mac Id to ast matching
 *		     b00 – mac 0
 *		     b01 – mac 1
 *		     b10 – mac 2
 *		     b11 – all macs (legacy HK way)
 *
 * Return: void
 */
static void hal_tx_desc_set_lmac_id_6290(void *desc, uint8_t lmac_id)
{
	HAL_SET_FLD(desc, TCL_DATA_CMD_4, LMAC_ID) |=
		HAL_TX_SM(TCL_DATA_CMD_4, LMAC_ID, lmac_id);
}
#else
static void hal_tx_desc_set_lmac_id_6290(void *desc, uint8_t lmac_id)
{
}
#endif

/**
 * hal_tx_init_cmd_credit_ring_6290() - Initialize command/credit SRNG
 * @hal_soc_hdl: Handle to HAL SoC structure
 * @hal_srng: Handle to HAL SRNG structure
 *
 * Return: none
 */
static inline void hal_tx_init_cmd_credit_ring_6290(hal_soc_handle_t hal_soc_hdl,
						    hal_ring_handle_t hal_ring_hdl)
{
}

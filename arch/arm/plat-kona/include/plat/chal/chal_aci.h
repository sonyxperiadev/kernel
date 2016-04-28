/*****************************************************************************
*  Copyright 2001 - 2009 Broadcom Corporation.  All rights reserved.
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
/**
*
*  @file   chal_aci.h
*  @brief  ACI Accessory cHAL interface
*  @note
*
*****************************************************************************/

#ifndef	_CHAL_ACI_H_
#define	_CHAL_ACI_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "chal_types.h"

/**
 * @addtogroup cHAL_Interface
 * @{
 */

/* -------------------------------------------------------------------------- */
/*
 * ACI Accessory Hardware block
 *
 */
/* -------------------------------------------------------------------------- */

#define HAL_ACC_FILTER_BLOCK_CLOCK   32768	/* Filter block operating
						 rate in Hz */

/**
* Microphone line route control type
******************************************************************************/
	typedef enum {
		CHAL_ACI_MIC_ROUTE_TV,	/*< MIC Line routed to TV out */
		CHAL_ACI_MIC_ROUTE_MIC,	/*< MIC line routed to analouge
					 microphone input */
		CHAL_ACI_MIC_ROUTE_ACI_OPEN,	/*< MIC line routed to ACI
					 block; data line is open */
		CHAL_ACI_MIC_ROUTE_ACI_CLOSED,	/*< MIC line routed to ACI
				 block; data line is closed (connected) */
		CHAL_ACI_MIC_ROUTE_ALL_OFF	/*< MIC OFF; all detection
						 blocks are powered down */
	} CHAL_ACI_mic_route_t;

/**
* ACI Accessory Filter action types
******************************************************************************/
	typedef enum {
		CHAL_ACI_BLOCK_ACTION_ENABLE,
		CHAL_ACI_BLOCK_ACTION_DISABLE,
		CHAL_ACI_BLOCK_ACTION_CONFIGURE_FILTER,
		CHAL_ACI_BLOCK_ACTION_COMP_THRESHOLD,
		CHAL_ACI_BLOCK_ACTION_ADC_RANGE,
		CHAL_ACI_BLOCK_ACTION_RESET_FILTER,
		CHAL_ACI_BLOCK_ACTION_INTERRUPT_ENABLE,
		CHAL_ACI_BLOCK_ACTION_INTERRUPT_DISABLE,
		CHAL_ACI_BLOCK_ACTION_INTERRUPT_ACKNOWLEDGE,
		CHAL_ACI_BLOCK_ACTION_MIC_BIAS,
		CHAL_ACI_BLOCK_ACTION_MIC_POWERDOWN_HIZ_IMPEDANCE,
		CHAL_ACI_BLOCK_ACTION_VREF,
		CHAL_ACI_BLOCK_ACTION_MAX
	} CHAL_ACI_block_action_t;

/**
* ACI Accessory Block hardware unit identifiers
******************************************************************************/
	typedef enum {
		CHAL_ACI_BLOCK_COMP1,
		CHAL_ACI_BLOCK_COMP2,
		CHAL_ACI_BLOCK_COMP2_INV,
		CHAL_ACI_BLOCK_COMP,	/* Action applies to both Comperators */
		CHAL_ACI_BLOCK_ADC1,
		CHAL_ACI_BLOCK_ADC2,
		CHAL_ACI_BLOCK_ADC,	/* Action applies to both ADC's */
		CHAL_ACI_BLOCK_DIGITAL,
		CHAL_ACI_BLOCK_GENERIC,
		CHAL_ACI_BLOCK_ID_MAX
	} CHAL_ACI_block_id_t;

/**
* ACI Accessory Comparator Filter mode types
******************************************************************************/
	typedef enum {
		CHAL_ACI_FILTER_MODE_INTEGRATE,
		CHAL_ACI_FILTER_MODE_DECIMATE
	} CHAL_ACI_filter_comp_mode_t;

/**
* ACI Accessory Comparator Filter Reset types
******************************************************************************/
	typedef enum {
		CHAL_ACI_FILTER_RESET_FIRMWARE,
		CHAL_ACI_FILTER_RESET_COMP
	} CHAL_ACI_filter_comp_reset_t;

/**
* ACI Accessory Comparator Filter Configuration parameter types
******************************************************************************/
	typedef struct {
		CHAL_ACI_filter_comp_mode_t mode;
		CHAL_ACI_filter_comp_reset_t reset_type;
		cUInt32 s;
		cUInt32 t;
		cUInt32 m;
		cUInt32 mt;
	} CHAL_ACI_filter_config_comp_t;

/**
* ACI Accessory ADC Filter Configuration parameter types
******************************************************************************/
	typedef struct {
		cBool chopping_enable;
		cUInt32 n;
	} CHAL_ACI_filter_config_adc_t;

/**
* ACI Accessory ADC voltage range parameter types
******************************************************************************/
	typedef enum {
		CHAL_ACI_BLOCK_ADC_FULL_RANGE,	/*< ADC can measure full
			 range voltages (0 - 2.5v) with low resolution */
		CHAL_ACI_BLOCK_ADC_HIGH_VOLTAGE,	/*< Same as above */
		CHAL_ACI_BLOCK_ADC_LOW_VOLTAGE,	/*< ADC can measure low
			 voltages (0 - 0.3125v) with hig resolution */
		CHAL_ACI_BLOCK_ADC_OFF
	} CHAL_ACI_range_config_adc_t;

/**
* ACI Accessory Block Output identifiers
******************************************************************************/
	typedef enum {
		CHAL_ACI_BLOCK_COMP_RAW,
		CHAL_ACI_BLOCK_COMP_FILTER1,
		CHAL_ACI_BLOCK_COMP_FILTER2,
		CHAL_ACI_BLOCK_COMP_INTERRUPT,
		CHAL_ACI_BLOCK_ADC_RAW,
		CHAL_ACI_BLOCK_ADC_FILTER,
		CHAL_ACI_BLOCK_OUTPUT_MAX
	} CHAL_ACI_block_output_id_t;

/**
* ACI Accessory Block Comperator filter Output values
******************************************************************************/
	typedef enum {
		CHAL_ACI_BLOCK_COMP_LINE_HIGH = 0,
		CHAL_ACI_BLOCK_COMP_LINE_LOW = 1,
		CHAL_ACI_BLOCK_COMP_LINE_UNDEFINED = -1
	} CHAL_ACI_block_comp_output_t;

/**
 * ACI Accessory MIC Bias mode configuration type
******************************************************************************/
	typedef enum {
		CHAL_ACI_MIC_BIAS_ON,
		CHAL_ACI_MIC_BIAS_DISCONTINUOUS,
		CHAL_ACI_MIC_BIAS_OFF,
		CHAL_ACI_MIC_BIAS_GND,
		CHAL_ACI_MIC_BIAS_HIZ
	} CHAL_ACI_micbias_mode_t;

/**
* ACI Accessory MIC Bias voltage configuration type Note:
* enumeration is also used to configure the hw register (AUXMIC_AUXEN_REG)
******************************************************************************/
	typedef enum {
		CHAL_ACI_MIC_BIAS_0_45V = 0,	/* bias = 0.45V */
		CHAL_ACI_MIC_BIAS_2_1V = (1 << 0),	/* enable,
							 bias = 2.1V, */
		CHAL_ACI_MIC_BIAS_2_5V = ((1 << 0) | (1 << 1))
						/* enable, bias = 2.5V, */
	} CHAL_ACI_micbias_voltage_t;

/**
* The MIC Bias probe cycle used in Discontinuous mode.
* Note: enumeration is also used to configure the hw register
* (AUXMIC_PRB_CYC_REG)
******************************************************************************/
	typedef enum {
		CHAL_ACI_MIC_BIAS_PRB_CYC_4MS = 0x00,
		CHAL_ACI_MIC_BIAS_PRB_CYC_8MS = 0x01,
		CHAL_ACI_MIC_BIAS_PRB_CYC_16MS = 0x02,
		CHAL_ACI_MIC_BIAS_PRB_CYC_32MS = 0x03,
		CHAL_ACI_MIC_BIAS_PRB_CYC_64MS = 0x04,
		CHAL_ACI_MIC_BIAS_PRB_CYC_128MS = 0x05,
		CHAL_ACI_MIC_BIAS_PRB_CYC_256MS = 0x06,
		CHAL_ACI_MIC_BIAS_PRB_CYC_512MS = 0x07
	} CHAL_ACI_micbias_probe_cycle_t;

/**
* The MIC Bias measurement delay used in Discontinuous mode.
* Note: enumeration is also used to configure the hw register
* (AUXMIC_MSR_DLY_REG)
******************************************************************************/
	typedef enum {
		CHAL_ACI_MIC_BIAS_MSR_DLY_1MS = 0x00,
		CHAL_ACI_MIC_BIAS_MSR_DLY_2MS = 0x01,
		CHAL_ACI_MIC_BIAS_MSR_DLY_4MS = 0x02,
		CHAL_ACI_MIC_BIAS_MSR_DLY_8MS = 0x03,
		CHAL_ACI_MIC_BIAS_MSR_DLY_16MS = 0x04,
		CHAL_ACI_MIC_BIAS_MSR_DLY_32MS = 0x05,
		CHAL_ACI_MIC_BIAS_MSR_DLY_64MS = 0x06,
		CHAL_ACI_MIC_BIAS_MSR_DLY_128MS = 0x07
	} CHAL_ACI_micbias_measure_delay_t;

/**
* The MIC Bias measurement interval used is in Discontinuous
* mode.
* Note: enumeration is also used to configure the hw register
* (AUXMIC_MSR_INTVL_REG)
******************************************************************************/
	typedef enum {
		CHAL_ACI_MIC_BIAS_MSR_INTVL_2MS = 0x00,
		CHAL_ACI_MIC_BIAS_MSR_INTVL_4MS = 0x01,
		CHAL_ACI_MIC_BIAS_MSR_INTVL_8MS = 0x02,
		CHAL_ACI_MIC_BIAS_MSR_INTVL_16MS = 0x03,
		CHAL_ACI_MIC_BIAS_MSR_INTVL_32MS = 0x04,
		CHAL_ACI_MIC_BIAS_MSR_INTVL_64MS = 0x05,
		CHAL_ACI_MIC_BIAS_MSR_INTVL_128MS = 0x06,
		CHAL_ACI_MIC_BIAS_MSR_INTVL_256MS = 0x07
	} CHAL_ACI_micbias_measure_interval_t;

/**
* The MIC Bias measurement interval control is used in
* Discontinuous mode. Note: enumeration is also used to
* configure the hw register
******************************************************************************/
	typedef enum {
		CHAL_ACI_MIC_BIAS_1_MEASUREMENT = 0x00,
		CHAL_ACI_MIC_BIAS_2_MEASUREMENT = 0x01
	} CHAL_ACI_micbias_measure_control_t;

/**
*  ACI Accessory MIC Bias Configuration parameter type
******************************************************************************/
	typedef struct {
		CHAL_ACI_micbias_mode_t mode;
		CHAL_ACI_micbias_voltage_t voltage;
		CHAL_ACI_micbias_probe_cycle_t probe_cycle;
		CHAL_ACI_micbias_measure_delay_t measure_delay;
		CHAL_ACI_micbias_measure_interval_t measure_interval;
		CHAL_ACI_micbias_measure_control_t measure_control;
	} CHAL_ACI_micbias_config_t;

/**
 * ACI Accessory Vref mode configuration type
******************************************************************************/
	typedef enum {
		CHAL_ACI_VREF_FAST_ON,
		CHAL_ACI_VREF_ON,
		CHAL_ACI_VREF_OFF
	} CHAL_ACI_vref_mode_t;

/**
*  ACI Accessory Vref Configuration parameter type
******************************************************************************/
	typedef struct {
		CHAL_ACI_vref_mode_t mode;
	} CHAL_ACI_vref_config_t;

/* -------------------------------------------------------------------------- */
/*
 * ACI Accessory ACI Hardware block
 *
 */
/* -------------------------------------------------------------------------- */

/**
* Command parameter for chal_aci_rx_input_set
******************************************************************************/
	typedef enum {
		CHAL_ACI_RX_DIRECT,
		CHAL_ACI_RX_VOTE
	} CHAL_ACI_RX_INPUT_TYPE_t;

/**
* Command parameter for chal_aci_bus_hold_set
******************************************************************************/
	typedef enum {
		CHAL_ACI_BUS_HOLD_2MS,
		CHAL_ACI_BUS_HOLD_10MS
	} CHAL_ACI_BUS_HOLD_TIME_t;

/**
* Command parameter for chal_aci_rxdar_use
******************************************************************************/
	typedef enum {
		CHAL_ACI_RXDAR_NORMAL,
		CHAL_ACI_RXDAR_DBIDIS
	} CHAL_ACI_RXDAR_USE_t;

/**
* Command parameter for chal_aci_rxdar_source
******************************************************************************/
	typedef enum {
		CHAL_ACI_RXDAR_SPEED,
		CHAL_ACI_RXDAR_ACIRX
	} CHAL_ACI_RXDAR_SOURCE_t;

/**
* Command parameter for chal_aci_xxx Interrupt control functions Note:
* Enumeration is equal to the register bit position
******************************************************************************/
	typedef enum {
		CHAL_ACI_NOPINT = (1 << 8),
		CHAL_ACI_COLL = (1 << 7),
		CHAL_ACI_RESERR = (1 << 6),
		CHAL_ACI_FRAERR = (1 << 5),
		CHAL_ACI_COMERR = (1 << 4),
		CHAL_ACI_SPDSET = (1 << 3),
		CHAL_ACI_DSENT = (1 << 2),
		CHAL_ACI_DREC = (1 << 1),
		CHAL_ACI_ACCINT = (1 << 0),
		CHAL_ACI_ALL_ACI_INT = 0x01FF,
		CHAL_ACI_ALL_ECI_INT = 0x01FE
	} CHAL_ACI_INTERRUPT_SELECT_t, CHAL_ACI_INTERRUPT_MASK_t;

/**
* Command parameter for chal_aci_command
* Note: Enumeration is equal to the register values in ACI Command Register
******************************************************************************/
	typedef enum {
		CHAL_ACI_CMD_NOP = 0x0,	/*< No Operation (=NOP) */
		CHAL_ACI_CMD_RESET_LEARN = 0x1,	/*< Generate reset &
							 learn sequence */
		CHAL_ACI_CMD_SEND_DATA = 0x2,	/*< Send data when written
							 into ACITxDaR */
		CHAL_ACI_CMD_RECV_BYTE = 0x3,	/*< Receive 8-bit data via
							 ACIRxDaR */
		CHAL_ACI_CMD_RECV_BYTE_LEARN = 0x4,	/*< Receive 1+8-bit
						 data with learn seq. */
		CHAL_ACI_CMD_MAX = 0x4
	} CHAL_ACI_COMMAND_t;

/* ---- Function Prototypes --------------------------------------------- */

/**
*
*  @brief Standard Init entry point for Accessory driver which should be the
*		first function to call.
*
*  @param  baseAddr  (in) mapped address of this ACI Accessory instance
*
*  @return CHAL_HANDLE
****************************************************************************/
	CHAL_HANDLE chal_aci_init(void __iomem *baseAddr);

/**
*
*  @brief De-Initialize CHAL Accessory for the passed Accessory instance
*
*  @param  handle (in) this Accessory instance
*
*  @return none
****************************************************************************/
	cVoid chal_aci_deinit(CHAL_HANDLE handle);

/* Accessory Hardware block access functions */

/**
*
*  @brief
*
*  @param  handle (in) this Accessory instance
*
*  @return none
****************************************************************************/
	cVoid chal_aci_set_mic_route(CHAL_HANDLE handle,
				     CHAL_ACI_mic_route_t route);

/**
*
*  @brief
*
*  @param  handle (in) this Accessory instance
*
*  @return none
****************************************************************************/
	cVoid chal_aci_block_ctrl(CHAL_HANDLE handle,
				  CHAL_ACI_block_action_t action,
				  CHAL_ACI_block_id_t id, ...);

/**
*
*  @brief
*
*  @param  handle (in) this Accessory instance
*
*  @return none
****************************************************************************/
	cInt32 chal_aci_block_read(CHAL_HANDLE handle, CHAL_ACI_block_id_t id,
				   CHAL_ACI_block_output_id_t output);

/* Accessory ACI Hardware block access functions */

/**
*
*  @brief
*
*  @param  handle (in) this Accessory instance
*
*  @return none
****************************************************************************/
	cVoid chal_aci_enable_aci(void);

/**
*
*  @brief
*
*  @param  handle (in) this Accessory instance
*
*  @return none
****************************************************************************/
	cVoid chal_aci_disable_aci(void);

/**
*
*  @brief
*
*  @param  handle (in) this Accessory instance
*
*  @return none
****************************************************************************/
	cBool chal_aci_is_aci_enabled(void);

/**
*
*  @brief
*
*  @param  handle (in) this Accessory instance
*
*  @return none
****************************************************************************/
	cVoid chal_aci_rx_input_set(CHAL_ACI_RX_INPUT_TYPE_t input_type);

/**
*
*  @brief
*
*  @param  handle (in) this Accessory instance
*
*  @return none
****************************************************************************/
	cVoid chal_aci_bus_hold_set(CHAL_ACI_BUS_HOLD_TIME_t bus_hold);

/**
*
*  @brief
*
*  @param  handle (in) this Accessory instance
*
*  @return none
****************************************************************************/
	cVoid chal_aci_start_pulse_set(cUInt8 start_pulse_length);

/**
*
*  @brief
*
*  @param  handle (in) this Accessory instance
*
*  @return none
****************************************************************************/
	cVoid chal_aci_rxdar_use(CHAL_ACI_RXDAR_USE_t use);

/**
*
*  @brief
*
*  @param  handle (in) this Accessory instance
*
*  @return none
****************************************************************************/
	cVoid chal_aci_rxdar_source(CHAL_ACI_RXDAR_SOURCE_t source);

/**
*
*  @brief
*
*  @param  handle (in) this Accessory instance
*
*  @return none
****************************************************************************/
	cVoid chal_aci_tx_output_high(cBool high);

/**
*
*  @brief
*
*  @param  handle (in) this Accessory instance
*
*  @return none
****************************************************************************/
	CHAL_ACI_INTERRUPT_MASK_t chal_aci_interrupt_read(void);

/**
*
*  @brief
*
*  @param  handle (in) this Accessory instance
*
*  @return none
****************************************************************************/
	cUInt16 chal_aci_rx_read(void);

/**
*
*  @brief
*
*  @param  handle (in) this Accessory instance
*
*  @return none
****************************************************************************/
	cVoid chal_aci_tx_write(cUInt16 data);

/**
*
*  @brief
*
*  @param  handle (in) this Accessory instance
*
*  @return none
****************************************************************************/
	cVoid chal_aci_command(CHAL_ACI_COMMAND_t command);

/**
*
*  @brief
*
*  @param  handle (in) this Accessory instance
*
*  @return none
****************************************************************************/
	cVoid chal_aci_interrupt_enable(CHAL_ACI_INTERRUPT_SELECT_t mask);

/**
*
*  @brief
*
*  @param  handle (in) this Accessory instance
*
*  @return none
****************************************************************************/
	cVoid chal_aci_interrupt_disable(CHAL_ACI_INTERRUPT_SELECT_t mask);

/**
*
*  @brief
*
*  @param  handle (in) this Accessory instance
*
*  @return none
****************************************************************************/
	cVoid chal_aci_interrupt_acknowledge(CHAL_ACI_INTERRUPT_SELECT_t mask);

/**
*
*  @brief This function will power on the ACI part for AUX MIC for audio
*
*  @param  Void
*
*  @return none
****************************************************************************/
	cVoid chal_aci_powerup_auxmic(void);

/** @} */

#ifdef __cplusplus
}
#endif
#endif				/* _CHAL_ACI_H_ */

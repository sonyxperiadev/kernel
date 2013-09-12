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
*  @file   chal_trace.h
*  @brief  TRACE cHAL interface
*  @note
*
*****************************************************************************/

#ifndef _CHAL_TRACE_H_
#define _CHAL_TRACE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "chal_common.h"

/**
 * @addtogroup cHAL Interface
 * @{
 */

/**
* funnels
******************************************************************************/
	typedef enum {
		/* HUB_FUNNEL register block (Hub.APB9) */
		CHAL_TRACE_HUB_FUNNEL = 0,
		/* FIN_FUNNEL register block (Hub.APB9) */
		CHAL_TRACE_FIN_FUNNEL,
		/* A9_ATB_FUNNEL register block (Comm_Subsystem.APB13) */
		CHAL_TRACE_COMMS_FUNNEL,
#if defined(_HERA_)
		/* MM_FUNNEL register block (MM_Subsystem.APB12) */
		CHAL_TRACE_MM_FUNNEL,
#endif /*_HERA_ */
		/* FAB_FUNNEL1 register block (Apps_Subsystem.Fabric.APB8) */
		CHAL_TRACE_FAB_FUNNEL1,
		/* FUNNEL register block (Apps_Subsystem.Processor.APB0) */
		CHAL_TRACE_FUNNEL,
		CHAL_TRACE_MAX_FUNNEL
	} CHAL_TRACE_FUNNEL_t;

/**
* axi trace
******************************************************************************/
	typedef enum {
		CHAL_TRACE_AXITRACE0 = 0,
		CHAL_TRACE_AXITRACE1,
		CHAL_TRACE_AXITRACE2,
		CHAL_TRACE_AXITRACE3,
		CHAL_TRACE_AXITRACE4,
		CHAL_TRACE_AXITRACE5,
		CHAL_TRACE_AXITRACE6,
		CHAL_TRACE_AXITRACE7,
		CHAL_TRACE_AXITRACE8,
		CHAL_TRACE_AXITRACE9,
		CHAL_TRACE_AXITRACE10,
		CHAL_TRACE_AXITRACE11,
		CHAL_TRACE_AXITRACE12,
		CHAL_TRACE_AXITRACE13,
		CHAL_TRACE_AXITRACE14,
		CHAL_TRACE_AXITRACE15,
		CHAL_TRACE_AXITRACE16,
		CHAL_TRACE_AXITRACE17,
		CHAL_TRACE_AXITRACE18,
		CHAL_TRACE_AXITRACE19,
		CHAL_TRACE_MAX_AXITRACE
	} CHAL_TRACE_AXITRACE_t;

/**
* axi trace performance counters
******************************************************************************/
	typedef enum {
		WRITE_COMMANDS,
		READ_COMMANDS,
		WRITE_ADDRESS_CYCLES,
		READ_ADDRESS_CYCLES,
		WRITE_DATA_CYCLES,
		READ_DATA_CYCLES,
		WRITE_RESPONSE_CYCLES,
		WRITE_ADDRESS_BUSY_CYCLES,
		READ_ADDRESS_BUSY_CYCLES,
		WRITE_DATA_BUSY_CYCLES,
		READ_DATA_BUSY_CYCLES,
		WRITE_RESPONSE_BUSY_CYCLES,
		WRITE_LATENCY_SUM_CYCLES,
		READ_LATENCY_SUM_CYCLES,
		WRITE_LATENCY_MINIMUM_CYCLES,
		READ_LATENCY_MINIMUM_CYCLES,
		WRITE_LATENCY_MAXIMUM_CYCLES,
		READ_LATENCY_MAXIMUM_CYCLES,
		WRITE_DATA_BEATS,
		READ_DATA_BEATS,
		WRITE_OUTSTANDING_COMMANDS_CYCLES,
		READ_OUTSTANDING_COMMANDS_CYCLES,
		CHAL_TRACE_AXITRACE_MAX_COUNT,
	} CHAL_TRACE_AXITRACE_COUNT_t;

/**
* CTI
******************************************************************************/
	typedef enum {
		CHAL_TRACE_HUB_CTI = 0,
		CHAL_TRACE_MM_CTI,
		CHAL_TRACE_FAB_CTI,
		CHAL_TRACE_A9CTI0,
		CHAL_TRACE_R4_CTI,
#if defined(_HERA_)
		CHAL_TRACE_R4CS_CTI,
		CHAL_TRACE_R4MPSS_CTI,
#endif /*_HERA_*/
		CHAL_TRACE_MAX_CTI
	} CHAL_TRACE_CTI_t;

/**
* local command
******************************************************************************/
	typedef enum {
		CHAL_TRACE_COUNTER_CLEAR = 0,	/* 00=counter clear */
		CHAL_TRACE_COUNTER_STOP,	/* 01=counter stop */
		CHAL_TRACE_COUNTER_START	/* 10=counter start */
	} CHAL_TRACE_LOCAL_CMD_t;

/**
* global performance counter command
******************************************************************************/
	typedef enum {
		CHAL_TRACE_GLB_CLEAR = 0,	/* 0=counter clear */
		CHAL_TRACE_GLB_STOP = 1,	/* 1=stop */
		CHAL_TRACE_GLB_START_RESUME = 2,	/* 2=start&resume */
		CHAL_TRACE_GLB_LOAD = 4,	/* 4=load */
		CHAL_TRACE_GLB_STOP_RESET = 7,	/* 7=stop&reset. */
	} CHAL_TRACE_GLB_CMD_t;

/**
* GIC latency
******************************************************************************/
	typedef enum {
		CHAL_TRACE_GIC_FIQ_LAT0 = 0,	/* FIQ Latency 0 */
		CHAL_TRACE_GIC_IRQ_LAT0,	/* IRQ Latency 0 */
		CHAL_TRACE_GIC_FIQ_LAT1,	/* FIQ Latency 1 */
		CHAL_TRACE_GIC_IRQ_LAT1,	/* IRQ Latency 1 */
	} CHAL_TRACE_GIC_LAT_t;

/**
* SW STM
******************************************************************************/
	typedef enum {
		CHAL_TRACE_SWSTM = 0,	/* Software STM */
		CHAL_TRACE_SWSTM_ST,	/* Software STM with ST */
		CHAL_TRACE_MAX_SWSTM
	} CHAL_TRACE_SWSTM_t;

/**
* CHAL trace base address
******************************************************************************/
	typedef struct {
#if defined(_HERA_)
		/* APB2ATB register block (Apps_Subsystem.Fabric.APB8) */
		cUInt32 APB2ATB_base;
		/* ATB2PTI register block (Hub.APB9)*/
		cUInt32 ATB2PTI_base;
		/* ATB2OCP register block (Hub.APB9) */
		cUInt32 ATB2OCP_base;
		/* WGM_OCP2ATB register block (Comm_Subsystem.APB13) */
		cUInt32 WGM_OCP2ATB_base;
		/* HUBOCP2ATB register block (Hub.APB9) */
		cUInt32 HUBOCP2ATB_base;
#endif				/*_HERA_ */
		/* GIC Trace (Apps_Subsystem.Processor.APB0) */
		void __iomem *GICTR_base;
		void __iomem *CHIPREGS_base;	/* CHIPREGS Block  */
		/* PWRMGR Block  (Hub.Power_Manager) */
		void __iomem *PWRMGR_base;
		void __iomem *FUNNEL_base[CHAL_TRACE_MAX_FUNNEL];
		void __iomem *AXITRACE_base[CHAL_TRACE_MAX_AXITRACE];
		void __iomem *CTI_base[CHAL_TRACE_MAX_CTI];
		void __iomem *ETB_base;
		void __iomem *ETB2AXI_base;
		void __iomem *GLOBPERF_base;	/* Global Counters Registers */
		/* ATB_STM register block (Hub.APB9) */
		void __iomem *ATB_STM_base;
		/* SWSTM register block (Apps_Subsystem.Processor.APB0) */
		void __iomem *SW_STM_base[CHAL_TRACE_MAX_SWSTM];
	} CHAL_TRACE_DEV_t;

/**
* CHAL axi ATM CONFIG
******************************************************************************/
	typedef struct {
		/* bit filed 31:19, Busy ID
		* Defines ID value for busy-cycle counting
		* of AXI channel cycles.
		* Meaningful only if BUSY_FILTERED=1.
		* This field should only be changed while
		* the counting is in the stopped state
		*/
		cUInt16 BUSY_ID;

		/* bit filed 18:16, Outstanding Threshold
		* Defines number of AXI transactions that must be
		* exceeded to enable counting of cycles for
		* oustanding commands.
		* This field should only be changed while the counting
		* is in the stopped state.
		*/
		cUInt8 OUTS_THRESH;

		/* bit filed 15, Outstanding Filtered
		* When set, outstanding command cycle counting uses
		* AXI transactions passing the programmed filter(s).
		* When cleared, all AXI transactions are used.
		* This bit should only be changed while the counting
		* is in the stopped state.
		*/
		cBool OUTS_FILTERED;

		/* bit filed 14, Outstanding Count Enable
		* This bit is loaded into the outstanding command cycle counting
		* enable state for a counter-start command.
		* The state is synchronized to the AXI clock.
		*/
		cBool OUTS_EN;
#if defined(_RHEA_)
		/* bit filed 14, All Cycles Enable
		* When set, BCYCLES counts all cycles regardless of any
		* handshakes and BBUSY is meaningless.
		* BUSY_EN must be set for this to work. When cleared,
		* BCYCLES works normally.
		* This bit should only be changed while the counting is
		* in the stopped state.
		*/
		cBool ALL_CYCLES_EN;
#endif				/*_RHEA_*/
		/* bit filed 12, Read Latency Mode
		* When set, read latency measurements use the
		* RLAST cycle to end a single transaction.
		* When cleared, measurements use the first read data cycle.
		* This bit should only be changed while the counting
		* is in the stopped state.
		*/
		cBool RDLAT_MODE;

		/* bit filed 11, Latency Filtered
		* When set, latency measurements use AXI
		* transactions passing the
		* programmed filter(s).
		* When cleared, all AXI transactions are used.
		* This bit should only be changed while the
		* counting is in the stopped state.
		*/
		cBool LATENCY_FILTERED;

		/* bit filed 10, Latency Measurement Enable
		* This bit is loaded into the latency
		* measurement enable state for a
		* counter-start command.
		* The state is synchronized to the AXI clock.
		*/
		cBool LATENCY_EN;

		/* bit filed 09, Busy Filtered
		* When set, BUSY_ID is used.
		* When cleared, all AXI transactions
		* are used for busy-cycle counting.
		* This bit should only be changed while
		* the counting is in the stopped state.
		*/
		cBool BUSY_FILTERED;

		/* bit filed 08, Busy Counting Enable
		* This bit is loaded into the busy-cycle counting enable
		* state for a counter-start
		* command.  The state is synchronized to the AXI clock.
		*/
		cBool BUSY_EN;

		/* bit filed 07, Beats Filtered
		* When set, data beat counting uses AXI
		* transactions passing the programmed filter(s).
		* When cleared, all AXI transactions are used.
		* This bit should only be changed while the
		* counting is in the stopped state.
		*/
		cBool BEATS_FILTERED;

		/* bit filed 06, Beats Counting Enable
		* This bit is loaded into the data beats counting
		* enable state for a counter-start
		* command. The state is synchronized to the AXI clock.
		*/
		cBool BEATS_EN;

		/* bit filed 05, Commands Filtered
		* When set, commands counting uses AXI
		* transactions passing the
		* programmed filter(s). When cleared, all AXI
		* transactions are used. This bit should only be
		* changed while the counting is in the stopped state.
		*/
		cBool CMDS_FILTERED;

		/* bit filed 04, Commands Counting Enable
		* This bit is loaded into the commands counting
		* enable state for a counter-start command.
		* The state is synchronized to the AXI clock.
		*/
		cBool CMDS_EN;

		/* bit filed 03, Trace Enable
		* When set, trace format FIFO loads are enabled.
		* This bit is synchronized to the AXI clock.
		*/
		cBool TRACE_EN;

		/* bit filed 02, Flush
		* Set by firmware and cleared by hardware when complete.
		* When set, capture and filter pipelines, as well as trace
		* format FIFO's, are emptied.
		*/
		cBool FLUSH;

		/* bit filed 01, Saturate Enable
		* When set, performance counters stop incrementing
		* when they reach
		* maximum (typically 0xFFFF_FFFF). When cleared,
		* the counters roll over to 0.
		* This bit is synchronized to the AXI clock.
		*/
		cBool SAT_EN;

		/* bit filed 00, Control Source
		* When set, performance counters are under local control.
		* When cleared, the counters are under global control.
		*/
		cBool CTRL_SRC;
	} CHAL_TRACE_ATM_CONF_t;

/**
* CHAL axi ATM Filter CONFIG
******************************************************************************/
	typedef struct {
		/* bit filed 31, Trigger Enable
		* When set, AXI transactions passing this filter generate
		* a trigger output.
		*/
		cBool TRIGGER_EN;

		/* bit filed 30, Filter Secure
		* When set, secure AXI transactions pass this filter.
		* When cleared, secure transactions do not.
		*/
		cBool FILTER_SEC;

		/* bit filed 29, Filter Open
		* When set, open AXI transactions pass this filter.
		* When cleared, open transactions do not.
		*/
		cBool FILTER_OPEN;

		/* bit filed 28:16, Filter ID
		* Combined with ID_MASK, this field defines the AXI ID
		* for transactions passing this filter.
		*/
		cUInt16 FILTER_ID;

		/* bit filed 15:8, ID Mask
		* Set bits in this mask define AXI ID bits that are compared
		* between transactions and FILTER_ID.
		* Cleared bits are don't-cares.
		*/
		cUInt8 ID_MASK;

		/* bit filed 7:4, Filter Length
		* Defines the comparison value for AXI length according
		* to LEN_MODE for AXI transactions passing this filter.
		*/
		cUInt8 FILTER_LEN;

		/* bit filed 3:2, Length Mode
		* 0x=AXI length must be == FILTER_LEN; 10=AXI length
		* must be <= FILTER_LEN; 11=AXI length must be > FILTER_LEN.
		*/
		cUInt8 LEN_MODE;

		/* bit filed 1, Filter Write
		* When set, AXI write transactions pass this filter.
		*/
		cBool FILTER_WRITE;

		/* bit filed 0, Filter Read
		* When set, AXI read transactions pass this filter.
		*/
		cBool FILTER_READ;
	} CHAL_TRACE_AXI_FILTER_CONF_t;

/**
* CHAL GIC Trace CONFIG
******************************************************************************/
	typedef struct {
		/* bit filed 10, Latency Measurement Enable.
		* This bit is loaded into the latency measurement enable
		* state for a counter-start command.
		*/
		cBool LATENCY_EN;

		/* bit filed 3, Trace Enable
		* When set, trace data is enabled. This bit is
		* synchronized to the ATB clock.
		*/
		cBool TRACE_EN;

		/* bit filed 0, Control Source
		* When set, performance counters are under local control.
		* When cleared, the counters are under global control.
		*/
		cBool CTRL_SRC;
	} CHAL_TRACE_GIC_CONF_t;

/**
* CHAL ETB Formatter and Flush Control
******************************************************************************/
	typedef struct {
		/* bit filed 13
		* Stop formatter for a trigger event.
		*/
		cBool StopTrig;

		/* bit filed 12
		* Stop formatter when flush completes.
		*/
		cBool StopFl;

		/* bit filed 10
		* Indicate a trigger on flush completion.
		*/
		cBool TrigFl;

		/* bit filed 9
		* Indicate a trigger on a trigger event.
		*/
		cBool TrigEvt;

		/* bit filed 8
		* Indicate a trigger on TRIGIN.
		*/
		cBool Trigin;

		/* bit filed 6
		* Manually generate a flush.
		*/
		cBool FOnMan;

		/* bit filed 5
		* Generate flush for a trigger event.
		*/
		cBool FOnTrig;

		/* bit filed 4
		* Generate flush on TRIGIN.
		*/
		cBool FOnFlIn;

		/* bit filed 1
		* Continuous formatting.
		*/
		cBool EnFCont;

		/* bit filed 0
		* Enable formatting.
		*/
		cBool EnFTC;
	} CHAL_TRACE_ETB_FF_CONF_t;

/*****************************************************************************
* function declarations
*****************************************************************************/

/**
*
*  @brief  function does nothing
*
*  @param   *pTraceDev_baseaddr (in) base address of trace devices.
*
*  @return  TRUE
*
*  @note
*
******************************************************************************/
	cBool chal_trace_init(CHAL_TRACE_DEV_t *pTraceDev_baseaddr);

#if defined(_HERA_)
/**
*
*  @brief  ATB to PTI Configuration
*
*  @param   handle (in) Handle returned in chal_trace_init()
*  @param   twobit_mode (in) When set, selects 2-bit output mode. When clear,
*		4-bit mode is active.
*  @param   intlv_mode (in) When set, selects interleaved mode of operation for
*		STP messages from hub XTI OCP output mixed with ATB messages.
*		When clear, only ATB messages are sent to PTI.
*  @param   pti_en (in) When set, enables PTI output.
*  @param   match_id (in) Defines ATB ID value to match for data output.
*
*  @return  status
*
*  @note
*
******************************************************************************/
	cBool chal_trace_cfg_hub_atb2pti(CHAL_HANDLE handle, cBool twobit_mode,
					 cBool intlv_mode, cBool pti_en,
					 cUInt8 match_id);

/**
*
*  @brief  ATB to OCP SW Source ID's
*
*  @param   handle (in) Handle returned in chal_trace_init()
*  @param   sw (in) sw number (0 ~ 3)
*  @param   atb_id (in) Defines ATB ID value to match for SW source
*		(from APB2ATB). Bit 0 = don't care (source uses for
*		timestamp enable).
*
*  @return  status
*
*  @note
*
******************************************************************************/
	cBool chal_trace_cfg_hub_atb2ocp_sw_id(CHAL_HANDLE handle, cUInt8 sw,
					       cUInt8 atb_id);

/**
*
*  @brief  ATB to OCP Filter-Out ID's
*
*  @param   handle (in) Handle returned in chal_trace_init()
*  @param   filter (in) filter number (0 ~ 3)
*  @param   atb_id (in) Defines ATB ID value to match for filtering-out data output.
*
*  @return  status
*
*  @note
*
******************************************************************************/
	cBool chal_trace_cfg_hub_atb2ocp_filter_id(CHAL_HANDLE handle,
						   cUInt8 filter,
						   cUInt8 atb_id);

/**
*
*  @brief  Config fabric APB to ATB atb ID
*
*  @param   handle (in) Handle returned in chal_trace_init()
*  @param   cpu_n (in) CPU number (0 ~ 3)
*  @param   atb_id (in) ATB ID value
*
*  @return  status
*
*  @note
*
******************************************************************************/
	cBool chal_trace_cfg_fabric_apb2atb_atb_id(CHAL_HANDLE handle,
						   cUInt8 cpu_n, cUInt8 atb_id);

/**
*
*  @brief  Config fabric APB to ATB stall mode
*
*  @param   handle (in) Handle returned in chal_trace_init()
*  @param   stall_mode (in) When set, stalls APB bus for full output FIFO.
*	When clear, writes are dropped for full output FIFO.
*
*  @return  status
*
*  @note
*
******************************************************************************/
	cBool chal_trace_cfg_fabric_apb2atb_stall_mode(CHAL_HANDLE handle,
						       cBool stall_mode);

/**
*
*  @brief  Config COMMS OCP to ATB
*
*  @param   handle (in) Handle returned in chal_trace_init()
*  @param   atb_id (in) ATB ID
*
*  @return  status
*
*  @note
*
******************************************************************************/
	cBool chal_trace_cfg_comms_ocp2atb(CHAL_HANDLE handle, cUInt8 atb_id);

/**
*
*  @brief  Config HUB OCP to ATB
*
*  @param   handle (in) Handle returned in chal_trace_init()
*  @param   atb_id (in) ATB ID
*
*  @return  status
*
*  @note
*
******************************************************************************/
	cBool chal_trace_cfg_hub_ocp2atb(CHAL_HANDLE handle, cUInt8 atb_id);
#endif				/*_HERA_*/

/**
*
*  @brief  Config Funnel_Control bits CSTF Control Register enable bits for each port
*
*  @param   handle (in) Handle returned in chal_trace_init()
*  @param   funnel_type (in) type of funnel.
*  @param   port_n (in) port number (0 ~ 7)
*  @param   enable (in) Setting this bit enables this input, or slave, port.
*
*  @return  status
*
*  @note    Setting this bit enables this input, or slave, port. If the bit is not set then this has
*           the effect of excluding the port from the priority selection
*		scheme. The reset value
*           is all clear, that is, all ports disabled.
*           Reset value is 0x0.
*
******************************************************************************/
	cBool chal_trace_funnel_set_enable(CHAL_HANDLE handle,
					   CHAL_TRACE_FUNNEL_t funnel_type,
					   cUInt8 port_n, cBool enable);

/**
*
*  @brief  Config Funnel_Control bits CSTF Control Register
*	Minimum_hold_time_3_0 field
*
*  @param   handle (in) Handle returned in chal_trace_init()
*  @param   funnel_type (in) type of funnel.
*  @param   min_hold_time (in) hold time of the slave ports
*
*  @return  status
*
*  @note    The formatting scheme can easily become inefficient if fast
*		switching occurs, so,
*           where possible, this must be minimized. If a source has
*		nothing to transmit, then
*           another source is selected irrespective of the minimum
*		number of cycles. Reset
*           is 0x3. The CSTF holds for the minimum hold time and one
*		additional cycle.
*           The maximum value that can be entered is 0xE and this equates
*		to 15 cycles. 0xF
*           is reserved. Reset value is 0x3.
*
******************************************************************************/
	cBool chal_trace_funnel_set_min_hold_time(CHAL_HANDLE handle,
						  CHAL_TRACE_FUNNEL_t
						  funnel_type,
						  cUInt8 min_hold_time);

/**
*
*  @brief  Config Priority_Control bits CSTF Priority Control Register
*
*  @param   handle (in) Handle returned in chal_trace_init()
*  @param   funnel_type (in) type of funnel.
*  @param   port_n (in) port number (0 ~ 7)
*  @param   priority (in) Priority value of the port
*
*  @return  status
*
*  @note    Priority value of the first slave port. The value written into this
*           location is the value that you want to assign the first
*		slave port.
*           At reset the default configuration assigns priority 0
*		to port 0, to priority 1 to port 1, and so on.
*           If you want to give highest priority to a particular slave port, the
*		corresponding port
*           must be programmed with the lowest value. Typically this
*		is likely to be a port that
*           has more important data or that has a small FIFO and is
*		therefore likely to overflow.
*           If you want to give lowest priority to a particular slave port,
*		the corresponding slave port
*           must be programmed with the highest value. Typically this is
*		likely to be a device that
*           has a large FIFO that is less likely to overflow or a source that
*		has information that is of lower importance.
*           A port programmed with value 0 gets the highest priority. A
*		port programmed with
*           value 7 gets the lowest priority. Priority must always go to
*		the highest priority	source
*           that has valid data available, if enabled. If a priority value
*		has been entered for multiple
*           different slave ports then the arbitration logic selects the
*		lowest port number of them.
*           This register must only be altered when the trace system
*		is disabled, that is, trace
*           sources are off and the system is drained.
*
******************************************************************************/
	cBool chal_trace_funnel_set_priority(CHAL_HANDLE handle,
					     CHAL_TRACE_FUNNEL_t funnel_type,
					     cUInt8 port_n, cUInt8 priority);

/**
*
*  @brief  Config ItCtrl bits Integration Mode Control Registers.
*
*  @param   handle (in) Handle returned in chal_trace_init()
*  @param   funnel_type (in) type of funnel.
*  @param   mode (in) mode
*
*  @return  status
*
*  @note    If set, the component reverts to an integration mode to enable topology detection
*           or to enable integration testing.
*           When no integration functionality has been put into a component,
*	because it is not
*           required, writing a HIGH to this location must be ignored and
*	must return a LOW
*           on read.
*           At reset integration functionality is disabled.
*
******************************************************************************/
	cBool chal_trace_funnel_set_itctrl(CHAL_HANDLE handle,
					   CHAL_TRACE_FUNNEL_t funnel_type,
					   cUInt8 mode);

/**
*
*  @brief  Config Claim_Tag_Set bits Claim Tag Set Registers.
*
*  @param   handle (in) Handle returned in chal_trace_init()
*  @param   funnel_type (in) type of funnel.
*  @param   ctv (in) Claim Tag Value
*
*  @return  status
*
*  @note    A bit-programmable register bank that reads the Claim Tag Value (CTV)
*           Reads:
*           A read returns 32'h000000FF indicating that this claim tag is eight
*	bits wide.
*           32'h0000000F indicates that this claim tag is four bits wide.
*	Logic 1indicates that this
*           bit can be set. Logic 0 indicates that this bit is unimplemented,
*	that is, it cannot be set.
*           Writes:
*           Bit-clear has no effect on the CTV.
*           Bit-set marks the bit in the CTV.
*
******************************************************************************/
	cBool chal_trace_funnel_set_claim_tag_set(CHAL_HANDLE handle,
						  CHAL_TRACE_FUNNEL_t
						  funnel_type, cUInt8 ctv);

/**
*
*  @brief  Config Claim_Tag_Clear bits Claim Tag Clear Registers.
*
*  @param   handle (in) Handle returned in chal_trace_init()
*  @param   funnel_type (in) type of funnel.
*  @param   ctv (in) Claim Tag Value
*
*  @return  status
*
*  @note    A bit-programmable register bank that sets the CTV.
*           Reads:
*           A read returns a value indicating the current claim value.
*           Writes:
*           Bit-clear has no effect on CTV.
*           Bit-set removes the bit in the CTV.
*           This is 0 on reset.
*
******************************************************************************/
	cBool chal_trace_funnel_set_claim_tag_clear(CHAL_HANDLE handle,
						    CHAL_TRACE_FUNNEL_t
						    funnel_type, cUInt8 ctv);

/**
*
*  @brief  Set LockAccess bits Lock Access Registers.
*
*  @param   handle (in) Handle returned in chal_trace_init()
*  @param   funnel_type (in) type of funnel.
*  @param   write_access_code (in) write access code
*
*  @return  status
*
*  @note    A write of 0xC5ACCE55 enables further write access to this component. An
*           invalid write has the effect of removing write access.
*	Reserved RAZ/SBZP.
*
******************************************************************************/
	cBool chal_trace_funnel_set_lock_access(CHAL_HANDLE handle,
						CHAL_TRACE_FUNNEL_t funnel_type,
						cUInt32 write_access_code);

/**
*
*  @brief  Read LockStatus bits Lock Status Registers.
*
*  @param   handle (in) Handle returned in chal_trace_init()
*  @param   funnel_type (in) type of funnel.
*
*  @return  LockStatus bits Lock Status Registers.
*
*  @note    Bit Field 2 Lock_Size
*           1 = device implements an 8-bit lock register.
*           0 = device implements a 32-bit lock register.
*
*           Bit Field 1 Lock_status
*           1 = device write access is locked.
*           0 = device write access is granted.
*
*           Bit Field 0 Lock_Mechanism
*           Indicates that a lock control mechanism exists for this component.
*           1 = lock mechanism is implemented.
*           0 = lock mechanism is not implemented.
*           All CoreSight components implement 32-bit lock access registers but
*		they are only
*           present on accesses when PADDRDBG31 is LOW. PADDRDBG31 is
*		used to bypass
*           the lock control mechanism and must be connected to PADDRDBG[31]
*		which is only
*           HIGH on accesses from external tools. For more information
*		about the Lock Access
*           Register and PADDRDBG[31] see the CoreSight
*		Architecture Specification.
*
******************************************************************************/
	cUInt32 chal_trace_funnel_get_lock_status(CHAL_HANDLE handle,
						  CHAL_TRACE_FUNNEL_t
						  funnel_type);

/**
*
*  @brief  Return TRUE when the lock mechanism is implemented and device write access
*		is locked.
*
*  @param   handle (in) Handle returned in chal_trace_init()
*  @param   funnel_type (in) type of funnel.
*
*  @return  TRUE/FALSE
*
*  @note
*
******************************************************************************/
	cBool chal_trace_funnel_locked(CHAL_HANDLE handle,
				       CHAL_TRACE_FUNNEL_t funnel_type);

/**
*
*  @brief  Read AuthStatus bits Authentication Status Registers.
*
*  @param   handle (in) Handle returned in chal_trace_init()
*  @param   funnel_type (in) type of funnel.
*
*  @return  AuthStatus bits Authentication Status Registers.
*
*  @note    Bit Field 7:6 Secure_Noninvasive_Debug
*
*           Bit Field 5:4 Secure_Invasive_Debug
*
*           Bit Field 3:2 Nonsecure_Noninvasive_Debug
*
*           Bit Field 1:0 Nonsecure_Invasive_Debug
*
*           Authentication Status Register pairs
*           Value Description
*           2'b00 Functionality not implemented or controlled elsewhere.
*           2'b01 Reserved.
*           2'b10 Functionality disabled.
*           2'b11 Functionality enabled.
*           See the CoreSight Architecture Specification for more information.
*
******************************************************************************/
	cUInt32 chal_trace_funnel_get_auth_status(CHAL_HANDLE handle,
						  CHAL_TRACE_FUNNEL_t
						  funnel_type);

/**
*
*  @brief  Read Device_ID bits Component Configuration Registers.
*
*  @param   handle (in) Handle returned in chal_trace_init()
*  @param   funnel_type (in) type of funnel.
*
*  @return  Device_ID bits Component Configuration Registers.
*
*  @note    Bit Field 7:4 PRIORITY
*           The CSTF implements a static priority scheme.
*
*           Bit Field 3:0 PORTCOUNT
*           This is the value of the Verilog define PORTCOUNT and
*           represents the number of input ports connected. By default all 8
*           ports are connected. 0x0 and 0x1 are illegal values.
*
******************************************************************************/
	cUInt32 chal_trace_funnel_get_device_id(CHAL_HANDLE handle,
						CHAL_TRACE_FUNNEL_t
						funnel_type);

/**
*
*  @brief  Read Device_Type_Identifier bits Device Type Identifier Registers.
*
*  @param   handle (in) Handle returned in chal_trace_init()
*  @param   funnel_type (in) type of funnel.
*
*  @return  Device_Type_Identifier bits Device Type Identifier Registers.
*
*  @note    Bit Field 7:4 Sub_type
*           Sub type, for trace sources this is broken down into cores,
*		DSPs or buses.
*
*           Bit Field 3:0 Main_type
*           Main type/class.
*
******************************************************************************/
	cUInt32 chal_trace_funnel_get_device_type_identifier(
							CHAL_HANDLE handle,
							CHAL_TRACE_FUNNEL_t
							funnel_type);

/**
*
*  @brief  Set GIC Trace Config
*
*  @param   handle (in) Handle returned in chal_trace_init()
*  @param   p_gic_config (in) gic trace config
*
*  @return  status
*
*  @note
*
******************************************************************************/
	cBool chal_trace_gic_set_config(CHAL_HANDLE handle,
					CHAL_TRACE_GIC_CONF_t *p_gic_config);

/**
*
*  @brief  Get GIC Trace Config
*
*  @param   handle (in) Handle returned in chal_trace_init()
*  @param   p_gic_config (in) gic trace config
*
*  @return  status
*
*  @note
*
******************************************************************************/
	cBool chal_trace_gic_get_config(CHAL_HANDLE handle,
					CHAL_TRACE_GIC_CONF_t *p_gic_config);

/**
*
*  @brief  Set GIC Trace Output ID's
*
*  @param   handle (in) Handle returned in chal_trace_init()
*  @param   atb_id (in) ATB ID. Defines the ATB ID value for trace packets.
*
*  @return  status
*
*  @note
*
******************************************************************************/
	cBool chal_trace_gic_set_outid(CHAL_HANDLE handle, cUInt8 atb_id);

/**
*
*  @brief  Set GIC trace Local Command
*
*  @param   handle (in) Handle returned in chal_trace_init()
*  @param   command (in) Local Command
*
*  @return  status
*
*  @note    When CTRL_SRC=1, writes to this register create a counter command.
*           When CTRL_SRC=0 writes are ignored.
*
******************************************************************************/
	cBool chal_trace_gic_set_cmd(CHAL_HANDLE handle,
				     CHAL_TRACE_LOCAL_CMD_t command);

/**
*
*  @brief  Get GIC Trace Status
*
*  @param   handle (in) Handle returned in chal_trace_init()
*
*  @return  GIC Trace Status
*
*  @note    Latency Measurement State. When set, measurements are enabled.
*
******************************************************************************/
	cBool chal_trace_gic_get_status(CHAL_HANDLE handle);

/**
*
*  @brief  Get FIQ/IRQ Latency
*
*  @param   handle (in) Handle returned in chal_trace_init()
*  @param   fiq_irq (in) type of interrupt (FIQ0,1/IRQ0,1)
*
*  @return  FIQ/IRQ Latency
*
*  @note    Reports the number of APB clock cycles the last time that the interrupt was active.
*
******************************************************************************/
	cUInt32 chal_trace_gic_get_latency(CHAL_HANDLE handle,
					   CHAL_TRACE_GIC_LAT_t fiq_irq);

/**
*
*  @brief  Set PM ATB trace ID
*
*  @param   handle (in) Handle returned in chal_trace_init()
*  @param   atb_id (in) Value of the PM ATB trace ID - if set to zero then trace is disabled
*
*  @return  status
*
*  @note
*
******************************************************************************/
	cBool chal_trace_pwrmgr_set_atb_id(CHAL_HANDLE handle, cUInt8 atb_id);

/**
*
*  @brief  Set AXI Trace Config
*
*  @param   handle (in) Handle returned in chal_trace_init()
*  @param   axitrace_type (in) type of axitrace
*  @param   *p_atm_config (in) axi trace config
*
*  @return  status
*
*  @note
*
******************************************************************************/
	cBool chal_trace_axi_set_atm_config(CHAL_HANDLE handle,
					    CHAL_TRACE_AXITRACE_t axitrace_type,
					    CHAL_TRACE_ATM_CONF_t *
					    p_atm_config);

/**
*
*  @brief  Get AXI Trace Config
*
*  @param   handle (in) Handle returned in chal_trace_init()
*  @param   axitrace_type (in) type of axitrace
*  @param   *p_atm_config (in) axi trace config
*
*  @return  status
*
*  @note
*
******************************************************************************/
	cBool chal_trace_axi_get_atm_config(CHAL_HANDLE handle,
					    CHAL_TRACE_AXITRACE_t axitrace_type,
					    CHAL_TRACE_ATM_CONF_t *
					    p_atm_config);

/**
*
*  @brief  Set AXI Trace Output ID's
*
*  @param   handle (in) Handle returned in chal_trace_init()
*  @param   axitrace_type (in) type of axitrace
*  @param   w_atb_id (in) Write ATB ID. Defines the ATB ID value for write trace packets.
*  @param   r_atb_id (in) Read ATB ID. Defines the ATB ID value for read trace packets.
*
*  @return  status
*
*  @note
*
******************************************************************************/
	cBool chal_trace_axi_set_atm_outid(CHAL_HANDLE handle,
					   CHAL_TRACE_AXITRACE_t axitrace_type,
					   cUInt8 w_atb_id, cUInt8 r_atb_id);

/**
*
*  @brief  Set AXI trace Local Command
*
*  @param   handle (in) Handle returned in chal_trace_init()
*  @param   axitrace_type (in) type of axitrace
*  @param   cmd (in) Local Command
*
*  @return  status
*
*  @note    When CTRL_SRC=1, writes to this register create a counter command.
*           When CTRL_SRC=0 writes are ignored.
*
******************************************************************************/
	cBool chal_trace_axi_set_atm_cmd(CHAL_HANDLE handle,
					 CHAL_TRACE_AXITRACE_t axitrace_type,
					 CHAL_TRACE_LOCAL_CMD_t cmd);

/**
*
*  @brief  Return AXI Trace Status
*
*  @param   handle (in) Handle returned in chal_trace_init()
*  @param   axitrace_type (in) type of axitrace
*
*  @return  AXI Trace Status
*
*  @note    Bit Field 13 Outstanding Commands Cycle Counting State. When set,
*           counting is enabled.
*           Bit Field 12 Latency Measurement State. When set,
*			measurements are enabled.
*           Bit Field 11 Data Beat Counting State. When set,
*			counting is enabled.
*           Bit Field 10 Busy Cycle Counting State. When set,
*			counting is enabled.
*           Bit Field 9 Commands Counting State. When set, counting
*			is enabled.
*           Bit Field 8 Saturation Stopped. When set, at least one counter
*			reached
*                       saturation with SAT_EN=1.
*                       While set, counters are stopped. Cleared by a counter
*                       clear command.
*           Bit Field 3 Write Trace Formatter FIFO Empty.
*           Bit Field 2 Read Trace Formatter FIFO Empty.
*           Bit Field 1 Write Trace Clock-Crossing FIFO Empty.
*           Bit Field 0 Read Trace Clock-Crossing FIFO Empty.
*
******************************************************************************/
	cUInt32 chal_trace_axi_get_status(CHAL_HANDLE handle,
					  CHAL_TRACE_AXITRACE_t axitrace_type);

/**
*
*  @brief  Return AXI Trace Status SAT_STOPPED field
*
*  @param   handle (in) Handle returned in chal_trace_init()
*  @param   axitrace_type (in) type of axitrace
*
*  @return  AXI Trace Status SAT_STOPPED field
*
*  @note
*
******************************************************************************/
	cUInt32 chal_trace_axi_get_status_sat(CHAL_HANDLE handle,
					      CHAL_TRACE_AXITRACE_t
					      axitrace_type);

/**
*
*  @brief  Return AXI Trace Performance Counters
*
*  @param   handle (in) Handle returned in chal_trace_init()
*  @param   axitrace_type (in) type of axitrace
*  @param   counter (in) type of axitrace performance counter
*
*  @return  AXI Trace Status
*
*  @note
*
******************************************************************************/
	cUInt32 chal_trace_axi_get_count(CHAL_HANDLE handle,
					 CHAL_TRACE_AXITRACE_t axitrace_type,
					 CHAL_TRACE_AXITRACE_COUNT_t counter);

/**
*
*  @brief  Set AXI filter configuratin/address low/address high
*
*  @param   handle (in) Handle returned in chal_trace_init()
*  @param   axitrace_type (in) type of axitrace
*  @param   filter_num (in) filter number (0 ~ 7)
*  @param   *filter_config (in) Filter Configuration
*  @param   addr_low (in) Address Low. AXI addresses must
*  be >= this value (with bits 7:0 assumed 0x00) to pass this filter.
*  @param   addr_high (in) Address High. AXI addresses must be
*   <= this value (with bits 7:0 assumed 0xFF) to pass this filter.
*
*  @return  status
*
*  @note
*
******************************************************************************/
	cBool chal_trace_axi_set_filter(CHAL_HANDLE handle,
					CHAL_TRACE_AXITRACE_t axitrace_type,
					cUInt8 filter_num,
					CHAL_TRACE_AXI_FILTER_CONF_t *
					filter_config, cUInt32 addr_low,
					cUInt32 addr_high);

/**
*
*  @brief  Set CTI Global enable
*
*  @param   handle (in) Handle returned in chal_trace_init()
*  @param   cti_type (in) type of CTI
*  @param   enable (in) Global enable
*
*  @return  status
*
*  @note
*
******************************************************************************/
	cBool chal_trace_cti_set_control(CHAL_HANDLE handle,
					 CHAL_TRACE_CTI_t cti_type,
					 cBool enable);

/**
*
*  @brief  Acknowledges nCTIIRQ outputs.
*
*  @param   handle (in) Handle returned in chal_trace_init()
*  @param   cti_type (in) type of CTI
*  @param   int_ack (in) CTIIRQ number for each bit [0:7].
*
*  @return  status
*
*  @note
*
******************************************************************************/
	cBool chal_trace_cti_set_int_ack(CHAL_HANDLE handle,
					 CHAL_TRACE_CTI_t cti_type,
					 cUInt8 int_ack);

/**
*
*  @brief  Generates event for channels.
*
*  @param   handle (in) Handle returned in chal_trace_init()
*  @param   cti_type (in) type of CTI
*  @param   channel (in) channel number for each bits [0:3].
*
*  @return  status
*
*  @note
*
******************************************************************************/
	cBool chal_trace_cti_set_app_set(CHAL_HANDLE handle,
					 CHAL_TRACE_CTI_t cti_type,
					 cUInt8 channel);

/**
*
*  @brief  Clears event for channels.
*
*  @param   handle (in) Handle returned in chal_trace_init()
*  @param   cti_type (in) type of CTI
*  @param   channel (in) channel number for each bits [0:3].
*
*  @return  status
*
*  @note
*
******************************************************************************/
	cBool chal_trace_cti_set_app_clear(CHAL_HANDLE handle,
					   CHAL_TRACE_CTI_t cti_type,
					   cUInt8 channel);

/**
*
*  @brief  Generates event pulse for channels.
*
*  @param   handle (in) Handle returned in chal_trace_init()
*  @param   cti_type (in) type of CTI
*  @param   channel (in) channel number for each bits [0:3].
*
*  @return  status
*
*  @note
*
******************************************************************************/
	cBool chal_trace_cti_set_app_pulse(CHAL_HANDLE handle,
					   CHAL_TRACE_CTI_t cti_type,
					   cUInt8 channel);

/**
*
*  @brief  Enables a cross trigger event for CTITRIGIN.
*
*  @param   handle (in) Handle returned in chal_trace_init()
*  @param   cti_type (in) type of CTI
*  @param   n_trigger (in) cti trigger number (0 ~ 7).
*  @param   channel (in) channel number for each bits [0:3].
*
*  @return  status
*
*  @note
*
******************************************************************************/
	cBool chal_trace_cti_set_in_en(CHAL_HANDLE handle,
				       CHAL_TRACE_CTI_t cti_type,
				       cUInt8 n_trigger, cUInt8 channel);

/**
*
*  @brief  Enables a channel event to generate CTITRIGOUT.
*
*  @param   handle (in) Handle returned in chal_trace_init()
*  @param   cti_type (in) type of CTI
*  @param   n_trigger (in) cti trigger number (0 ~ 7).
*  @param   channel (in) channel number for each bits [0:3].
*
*  @return  status
*
*  @note
*
******************************************************************************/
	cBool chal_trace_cti_set_out_en(CHAL_HANDLE handle,
					CHAL_TRACE_CTI_t cti_type,
					cUInt8 n_trigger, cUInt8 channel);

/**
*
*  @brief  Reads CTI Trigger In Status
*
*  @param   handle (in) Handle returned in chal_trace_init()
*  @param   cti_type (in) type of CTI
*
*  @return  CTI Trigger In Status
*
*  @note    Bit Field 7:0 TRIGINSTATUS
*
******************************************************************************/
	cUInt32 chal_trace_cti_get_trig_in_status(CHAL_HANDLE handle,
						  CHAL_TRACE_CTI_t cti_type);

/**
*
*  @brief  Reads CTI Trigger Out Status
*
*  @param   handle (in) Handle returned in chal_trace_init()
*  @param   cti_type (in) type of CTI
*
*  @return  CTI Trigger Out Status
*
*  @note    Bit Field 7:0 TRIGOUTSTATUS
*
******************************************************************************/
	cUInt32 chal_trace_cti_get_trig_out_status(CHAL_HANDLE handle,
						   CHAL_TRACE_CTI_t cti_type);

/**
*
*  @brief  Reads CTI Channel in Status
*
*  @param   handle (in) Handle returned in chal_trace_init()
*  @param   cti_type (in) type of CTI
*
*  @return  CTI Channel in Status
*
*  @note    Bit Field 3:0 CHINSTATUS
*
******************************************************************************/
	cUInt32 chal_trace_cti_get_ch_in_status(CHAL_HANDLE handle,
						CHAL_TRACE_CTI_t cti_type);

/**
*
*  @brief  Reads CTI Channel Out Status
*
*  @param   handle (in) Handle returned in chal_trace_init()
*  @param   cti_type (in) type of CTI
*
*  @return  CTI Channel Out Status
*
*  @note    Bit Field 3:0 TRIGOUTSTATUS
*
******************************************************************************/
	cUInt32 chal_trace_cti_get_ch_out_status(CHAL_HANDLE handle,
						 CHAL_TRACE_CTI_t cti_type);

/**
*
*  @brief  Enables CTI output channels
*
*  @param   handle (in) Handle returned in chal_trace_init()
*  @param   cti_type (in) type of CTI
*  @param   channel (in) channel number for each bits [0:3].
*
*  @return  status
*
*  @note
*
******************************************************************************/
	cBool chal_trace_cti_set_ch_gate(CHAL_HANDLE handle,
					 CHAL_TRACE_CTI_t cti_type,
					 cUInt8 channel);

/**
*
*  @brief  Enables edge detection for trigger output
*
*  @param   handle (in) Handle returned in chal_trace_init()
*  @param   cti_type (in) type of CTI
*  @param   triger_output (in)
*
*  @return  status
*
*  @note    Bit Field 5 PMUEXTIN1EDGE
*           Bit Field 4 PMUEXTIN0EDGE
*           Bit Field 3 ETMEXTIN4EDGE
*           Bit Field 2 ETMEXTIN3EDGE
*           Bit Field 1 ETMEXTIN2EDGE
*           Bit Field 0 ETMEXTIN1EDGE
*
******************************************************************************/
	cBool chal_trace_cti_set_asic_control(CHAL_HANDLE handle,
					      CHAL_TRACE_CTI_t cti_type,
					      cUInt8 triger_output);

/**
*
*  @brief  Set Integ. Test Channel In Ack
*
*  @param   handle (in) Handle returned in chal_trace_init()
*  @param   cti_type (in) type of CTI
*  @param   channel (in) channel number for each bits [0:3].
*
*  @return  status
*
*  @note
*
******************************************************************************/
	cBool chal_trace_cti_set_it_ch_in_ack(CHAL_HANDLE handle,
					      CHAL_TRACE_CTI_t cti_type,
					      cUInt8 channel);

/**
*
*  @brief  Set Integ. Test Trigger In Ack
*
*  @param   handle (in) Handle returned in chal_trace_init()
*  @param   cti_type (in) type of CTI
*  @param   trigger (in) TRIG number for each bit [0:7].
*
*  @return  status
*
*  @note
*
******************************************************************************/
	cBool chal_trace_cti_set_it_trig_in_ack(CHAL_HANDLE handle,
						CHAL_TRACE_CTI_t cti_type,
						cUInt8 trigger);

/**
*
*  @brief  Set Integ. Test Channel Out
*
*  @param   handle (in) Handle returned in chal_trace_init()
*  @param   cti_type (in) type of CTI
*  @param   channel (in) channel number for each bits [0:3].
*
*  @return  status
*
*  @note
*
******************************************************************************/
	cBool chal_trace_cti_set_it_ch_out(CHAL_HANDLE handle,
					   CHAL_TRACE_CTI_t cti_type,
					   cUInt8 channel);

/**
*
*  @brief  Set Integ. Test Trigger Out
*
*  @param   handle (in) Handle returned in chal_trace_init()
*  @param   cti_type (in) type of CTI
*  @param   trigger (in) TRIG number for each bit [0:7].
*
*  @return  status
*
*  @note
*
******************************************************************************/
	cBool chal_trace_cti_set_it_trig_out(CHAL_HANDLE handle,
					     CHAL_TRACE_CTI_t cti_type,
					     cUInt8 trigger);

/**
*
*  @brief  Read Integ. Test Channel Out Ack
*
*  @param   handle (in) Handle returned in chal_trace_init()
*  @param   cti_type (in) type of CTI
*
*  @return  Integ. Test Channel Out Ack
*
*  @note    Bit Field 3:0 CTCHOUTACK. Reads value of CTCHOUTACK.
*
******************************************************************************/
	cUInt32 chal_trace_cti_get_it_ch_out_ack(CHAL_HANDLE handle,
						 CHAL_TRACE_CTI_t cti_type);

/**
*
*  @brief  Read Integ. Test Trigger Out Ack
*
*  @param   handle (in) Handle returned in chal_trace_init()
*  @param   cti_type (in) type of CTI
*
*  @return  Integ. Test Trigger Out Ack
*
*  @note    Bit Field 7:0 CTTRIGOUTACK. Reads value of CTTRIGOUTACK.
*
******************************************************************************/
	cUInt32 chal_trace_cti_get_it_trig_out_ack(CHAL_HANDLE handle,
						   CHAL_TRACE_CTI_t cti_type);

/**
*
*  @brief  Read Integ. Test Channel In
*
*  @param   handle (in) Handle returned in chal_trace_init()
*  @param   cti_type (in) type of CTI
*
*  @return  Integ. Test Channel In
*
*  @note    Bit Field 3:0 CTCHIN. Reads value of CTCHIN.
*
******************************************************************************/
	cUInt32 chal_trace_cti_get_it_ch_in(CHAL_HANDLE handle,
					    CHAL_TRACE_CTI_t cti_type);

/**
*
*  @brief  Read Integ. External Output Control
*
*  @param   handle (in) Handle returned in chal_trace_init()
*  @param   cti_type (in) type of CTI
*
*  @return  Integ. External Output Control
*
*  @note    Bit Field 7:0 CTTRIGIN. Reads value of CTTRIGIN.
*
******************************************************************************/
	cUInt32 chal_trace_cti_get_it_trig_in(CHAL_HANDLE handle,
					      CHAL_TRACE_CTI_t cti_type);

/**
*
*  @brief  Set Integ. Mode Control
*
*  @param   handle (in) Handle returned in chal_trace_init()
*  @param   cti_type (in) type of CTI
*  @param   enable (in) Integragtion mode enable.
*
*  @return  status
*
*  @note
*
******************************************************************************/
	cBool chal_trace_cti_set_int_mode_control(CHAL_HANDLE handle,
						  CHAL_TRACE_CTI_t cti_type,
						  cBool enable);

/**
*
*  @brief  Set Claim Tag Set
*
*  @param   handle (in) Handle returned in chal_trace_init()
*  @param   cti_type (in) type of CTI
*  @param   channel (in) channel number for each bits [0:3].
*
*  @return  status
*
*  @note
*
******************************************************************************/
	cBool chal_trace_cti_set_claim_set(CHAL_HANDLE handle,
					   CHAL_TRACE_CTI_t cti_type,
					   cUInt8 channel);

/**
*
*  @brief  Set Claim Tag Clear
*
*  @param   handle (in) Handle returned in chal_trace_init()
*  @param   cti_type (in) type of CTI
*  @param   channel (in) channel number for each bits [0:3].
*
*  @return  status
*
*  @note
*
******************************************************************************/
	cBool chal_trace_cti_set_claim_clr(CHAL_HANDLE handle,
					   CHAL_TRACE_CTI_t cti_type,
					   cUInt8 channel);

/**
*
*  @brief  Set Lock Access
*
*  @param   handle (in) Handle returned in chal_trace_init()
*  @param   cti_type (in) type of CTI
*  @param   control (in) Lock access control.
*
*  @return  status
*
*  @note
*
******************************************************************************/
	cBool chal_trace_cti_set_lock_access(CHAL_HANDLE handle,
					     CHAL_TRACE_CTI_t cti_type,
					     cUInt32 control);

/**
*
*  @brief  Read Lock Status
*
*  @param   handle (in) Handle returned in chal_trace_init()
*  @param   cti_type (in) type of CTI
*
*  @return  Lock Status
*
*  @note    Bit Field 1 Locked.
*           Bit Field 0 Lock implemented.
*
******************************************************************************/
	cUInt32 chal_trace_cti_get_lock_status(CHAL_HANDLE handle,
					       CHAL_TRACE_CTI_t cti_type);

/**
*
*  @brief  Read Authentication Status
*
*  @param   handle (in) Handle returned in chal_trace_init()
*  @param   cti_type (in) type of CTI
*
*  @return  Authentication Status
*
*  @note    Bit Field 3:2 Open noninvasive debug enabled.
*           Bit Field 1:0 Open invasive debug enabled.
*
******************************************************************************/
	cUInt32 chal_trace_cti_get_auth_status(CHAL_HANDLE handle,
					       CHAL_TRACE_CTI_t cti_type);

/**
*
*  @brief  Read Device ID
*
*  @param   handle (in) Handle returned in chal_trace_init()
*  @param   cti_type (in) type of CTI
*
*  @return  Device ID
*
*  @note    Bit Field 19:16 Number of CTI channels available.
*           Bit Field 15:8 Number of CTI triggers available.
*           Bit Field 4:0 Main class.
*
******************************************************************************/
	cUInt32 chal_trace_cti_get_dev_id(CHAL_HANDLE handle,
					  CHAL_TRACE_CTI_t cti_type);

/**
*
*  @brief  Read Device Type
*
*  @param   handle (in) Handle returned in chal_trace_init()
*  @param   cti_type (in) type of CTI
*
*  @return  Device Type
*
*  @note    Bit Field 7:4 Sub-type
*           Bit Field 3:0 Main class.
*
******************************************************************************/
	cUInt32 chal_trace_cti_get_dev_type(CHAL_HANDLE handle,
					    CHAL_TRACE_CTI_t cti_type);

/**
*
*  @brief  Read Peripheral ID
*
*  @param   handle (in) Handle returned in chal_trace_init()
*  @param   cti_type (in) type of CTI
*  @param   n_peripheral (in) number of peripheral (0 ~ 4)
*
*  @return  Peripheral ID
*
*  @note    Bit Field 7:0 Peripheral ID n
*
******************************************************************************/
	cUInt32 chal_trace_cti_get_per_id(CHAL_HANDLE handle,
					  CHAL_TRACE_CTI_t cti_type,
					  cUInt8 n_peripheral);

/**
*
*  @brief  Read Component ID
*
*  @param   handle (in) Handle returned in chal_trace_init()
*  @param   cti_type (in) type of CTI
*  @param   n_component (in) number of component (0 ~ 3)
*
*  @return  Component ID
*
*  @note    Bit Field 7:0 Component ID n
*
******************************************************************************/
	cUInt32 chal_trace_cti_get_comp_id(CHAL_HANDLE handle,
					   CHAL_TRACE_CTI_t cti_type,
					   cUInt8 n_component);

/**
*
*  @brief  Return Ram Depth
*
*  @param   handle (in) Handle returned in chal_trace_init()
*
*  @return  Ram Depth
*
*  @note    Bit Field 31:0 RDP. Depth in words of trace memory, fixed at synthesis.
*
******************************************************************************/
	cUInt32 chal_trace_etb_get_ram_depth(CHAL_HANDLE handle);

/**
*
*  @brief  Read ETB Status
*
*  @param   handle (in) Handle returned in chal_trace_init()
*
*  @return  ETB Status
*
*  @note    Bit Field 3 FtEmpty. Formatter pipeline empty.
*           Bit Field 2 AcqComp. Acquisition complete.
*           Bit Field 1 Triggered. Trigger has been observed.
*           Bit Field 0 Full. RAM write pointer has wrapped.
*
******************************************************************************/
	cUInt32 chal_trace_etb_get_status(CHAL_HANDLE handle);

/**
*
*  @brief  Read RAM Read Data
*
*  @param   handle (in) Handle returned in chal_trace_init()
*
*  @return  RAM Read Data
*
*  @note    Bit Field 31:0 RRD. Read data from trace RAM.
*
******************************************************************************/
	cUInt32 chal_trace_etb_get_ram_read_data(CHAL_HANDLE handle);

/**
*
*  @brief  Set RAM Read Pointer
*
*  @param   handle (in) Handle returned in chal_trace_init()
*  @param   rrp (in) Read pointer.
*
*  @return  status
*
*  @note
*
******************************************************************************/
	cBool chal_trace_etb_set_ram_read_pointer(CHAL_HANDLE handle,
						  cUInt32 rrp);

/**
*
*  @brief  Set RAM Write Pointer
*
*  @param   handle (in) Handle returned in chal_trace_init()
*  @param   rwp (in) Write pointer.
*
*  @return  status
*
*  @note
*
******************************************************************************/
	cBool chal_trace_etb_set_ram_write_pointer(CHAL_HANDLE handle,
						   cUInt32 rwp);

/**
*
*  @brief  Set Trigger Counter
*
*  @param   handle (in) Handle returned in chal_trace_init()
*  @param   trg (in) Trigger Counter.
*
*  @return  status
*
*  @note
*
******************************************************************************/
	cBool chal_trace_etb_set_trigger_counter(CHAL_HANDLE handle,
						 cUInt32 trg);

/**
*
*  @brief  Set ETB Control
*
*  @param   handle (in) Handle returned in chal_trace_init()
*  @param   enable (in) Trace Capture Enable.
*
*  @return  status
*
*  @note
*
******************************************************************************/
	cBool chal_trace_etb_set_control(CHAL_HANDLE handle, cBool enable);

/**
*
*  @brief  Set RAM Write Data
*
*  @param   handle (in) Handle returned in chal_trace_init()
*  @param   rwd (in) Write data to the trace RAM.
*
*  @return  status
*
*  @note
*
******************************************************************************/
	cBool chal_trace_etb_set_ram_write_data(CHAL_HANDLE handle,
						cUInt32 rwd);

/**
*
*  @brief  Read Formatter and Flush Status
*
*  @param   handle (in) Handle returned in chal_trace_init()
*
*  @return  Formatter and Flush Status
*
*  @note    Bit Field 1 FtStopped. Formatter stopped.
*           Bit Field 0 FlInProg. Flush in progress.
*
******************************************************************************/
	cUInt32 chal_trace_etb_get_ff_status(CHAL_HANDLE handle);

/**
*
*  @brief  Set Formatter and Flush Control
*
*  @param   handle (in) Handle returned in chal_trace_init()
*  @param   *ffcr (in) pointer of Formatter and Flush Control configuration.
*
*  @return  status
*
*  @note
*
******************************************************************************/
	cBool chal_trace_etb_set_ff_control(CHAL_HANDLE handle,
					    CHAL_TRACE_ETB_FF_CONF_t *ffcr);

/**
*
*  @brief  Set Integ. Test Misc. Output 0
*
*  @param   handle (in) Handle returned in chal_trace_init()
*  @param   full (in) Set value of Full.
*  @param   acq_comp (in) Set value of AcqComp.
*
*  @return  status
*
*  @note
*
******************************************************************************/
	cBool chal_trace_etb_set_it_misc_op0(CHAL_HANDLE handle, cBool full,
					     cBool acq_comp);

/**
*
*  @brief  Set Integ. Test Trigger In and Flush In Ack
*
*  @param   handle (in) Handle returned in chal_trace_init()
*  @param   trig (in) Set value of TRIGINACK.
*  @param   flush (in) Set value of FLUSHINACK.
*
*  @return  status
*
*  @note
*
******************************************************************************/
	cBool chal_trace_etb_set_it_tr_fl_in_ack(CHAL_HANDLE handle, cBool trig,
						 cBool flush);

/**
*
*  @brief  Read Integ. Test Trigger In and Flush In
*
*  @param   handle (in) Handle returned in chal_trace_init()
*
*  @return  Integ. Test Trigger In and Flush In
*
*  @note    Bit Field 1 FLUSHIN. Read value of FLUSHIN.
*           Bit Field 0 TRIGIN. Read value of TRIGIN.
*
******************************************************************************/
	cUInt32 chal_trace_etb_get_it_tr_fl_in(CHAL_HANDLE handle);

/**
*
*  @brief  Read Integ. Test ATB Data 0
*
*  @param   handle (in) Handle returned in chal_trace_init()
*
*  @return  Integ. Test ATB Data 0
*
*  @note    Bit Field 4 ATDATA_31. Read value of ATDATAS[31].
*           Bit Field 3 ATDATA_23. Read value of ATDATAS[23].
*           Bit Field 2 ATDATA_15. Read value of ATDATAS[15].
*           Bit Field 1 ATDATA_7. Read value of ATDATAS[7].
*           Bit Field 0 ATDATA_0. Read value of ATDATAS[0].
*
******************************************************************************/
	cUInt32 chal_trace_etb_get_it_atb_data0(CHAL_HANDLE handle);

/**
*
*  @brief  Set Integ. Test ATB Control 2
*
*  @param   handle (in) Handle returned in chal_trace_init()
*  @param   afvalids (in) Set value of AFVALIDS.
*  @param   atreadys (in) Set value of ATREADYS.
*
*  @return  status
*
*  @note
*
******************************************************************************/
	cBool chal_trace_etb_set_it_atb_ctrl2(CHAL_HANDLE handle,
					      cBool afvalids, cBool atreadys);

/**
*
*  @brief  Read Integ. Test ATB Control 1
*
*  @param   handle (in) Handle returned in chal_trace_init()
*
*  @return  Integ. Test ATB Control 1
*
*  @note    Bit Field 6:0 ITATIDS. Read value of ATIDS.
*
******************************************************************************/
	cUInt32 chal_trace_etb_get_it_atb_ctrl1(CHAL_HANDLE handle);

/**
*
*  @brief  Read Integ. Test ATB Control 0
*
*  @param   handle (in) Handle returned in chal_trace_init()
*
*  @return  Integ. Test ATB Control 0
*
*  @note    Bit Field 9:8 ATBYTES. Read value of ATBYTES.
*           Bit Field 1 AFREADY. Read value of AFREADYS.
*           Bit Field 0 ATVALID. Read value of ATVALIDS.
*
******************************************************************************/
	cUInt32 chal_trace_etb_get_it_atb_ctrl0(CHAL_HANDLE handle);

/**
*
*  @brief  Set Integ. Mode Control
*
*  @param   handle (in) Handle returned in chal_trace_init()
*  @param   enable (in) Integragtion mode enable.
*
*  @return  status
*
*  @note
*
******************************************************************************/
	cBool chal_trace_etb_set_int_mode_control(CHAL_HANDLE handle,
						  cBool enable);

/**
*
*  @brief  Set Claim Tag Set
*
*  @param   handle (in) Handle returned in chal_trace_init()
*  @param   channel (in) channel number for each bits [0:3].
*
*  @return  status
*
*  @note
*
******************************************************************************/
	cBool chal_trace_etb_set_claim_set(CHAL_HANDLE handle, cUInt8 channel);

/**
*
*  @brief  Set Claim Tag Clear
*
*  @param   handle (in) Handle returned in chal_trace_init()
*  @param   channel (in) channel number for each bits [0:3].
*
*  @return  status
*
*  @note
*
******************************************************************************/
	cBool chal_trace_etb_set_claim_clr(CHAL_HANDLE handle, cUInt8 channel);

/**
*
*  @brief  Set Lock Access
*
*  @param   handle (in) Handle returned in chal_trace_init()
*  @param   control (in) Lock access control.
*
*  @return  status
*
*  @note
*
******************************************************************************/
	cBool chal_trace_etb_set_lock_access(CHAL_HANDLE handle,
					     cUInt32 control);

/**
*
*  @brief  Read Lock Status
*
*  @param   handle (in) Handle returned in chal_trace_init()
*
*  @return  Lock Status
*
*  @note    Bit Field 2 32-bit access required.
*           Bit Field 1 Locked.
*           Bit Field 0 Lock implemented.
*
******************************************************************************/
	cUInt32 chal_trace_etb_get_lock_status(CHAL_HANDLE handle);

/**
*
*  @brief  Read Authentication Status
*
*  @param   handle (in) Handle returned in chal_trace_init()
*
*  @return  Authentication Status
*
*  @note    Bit Field 7:6 Secure noninvasive debug enabled.
*           Bit Field 5:4 Secure invasive debug enabled.
*           Bit Field 3:2 Open noninvasive debug enabled.
*           Bit Field 1:0 Open invasive debug enabled.
*
******************************************************************************/
	cUInt32 chal_trace_etb_get_auth_status(CHAL_HANDLE handle);

/**
*
*  @brief  Read Device ID
*
*  @param   handle (in) Handle returned in chal_trace_init()
*
*  @return  Device ID
*
*  @note    Bit Field 31:0 Device ID.
*
******************************************************************************/
	cUInt32 chal_trace_etb_get_dev_id(CHAL_HANDLE handle);

/**
*
*  @brief  Read Device Type
*
*  @param   handle (in) Handle returned in chal_trace_init()
*
*  @return  Device Type
*
*  @note    Bit Field 7:4 Sub-type
*           Bit Field 3:0 Main class.
*
******************************************************************************/
	cUInt32 chal_trace_etb_get_dev_type(CHAL_HANDLE handle);

/**
*
*  @brief  Read Peripheral ID
*
*  @param   handle (in) Handle returned in chal_trace_init()
*  @param   n_peripheral (in) number of peripheral (0 ~ 4)
*
*  @return  Peripheral ID
*
*  @note    Bit Field 7:0 Peripheral ID n
*
******************************************************************************/
	cUInt32 chal_trace_etb_get_per_id(CHAL_HANDLE handle,
					  cUInt8 n_peripheral);

/**
*
*  @brief  Read Component ID
*
*  @param   handle (in) Handle returned in chal_trace_init()
*  @param   n_component (in) number of component (0 ~ 3)
*
*  @return  Component ID
*
*  @note    Bit Field 7:0 Component ID n
*
******************************************************************************/
	cUInt32 chal_trace_etb_get_comp_id(CHAL_HANDLE handle,
					   cUInt8 n_component);

/**
*
*  @brief  Set ETB to AXI Configuration
*
*  @param   handle (in) Handle returned in chal_trace_init()
*  @param   flush (in) Controls flushing of FIFO's to memory.
* Auto-cleared by hardware when flushing complete.
*
*  @return  status
*
*  @note
*
******************************************************************************/
	cBool chal_trace_etb2axi_set_config(CHAL_HANDLE handle, cBool flush);

/**
*
*  @brief  Set ETB to AXI Configuration
*
*  @param   handle (in) Handle returned in chal_trace_init()
*  @param   wr_ptr (in) Base write pointer of memory where data stored.
*
*  @return  status
*
*  @note
*
******************************************************************************/
	cBool chal_trace_etb2axi_set_wr_ptr(CHAL_HANDLE handle, cUInt32 wr_ptr);

/**
*
*  @brief  Read ETB to AXI Status
*
*  @param   handle (in) Handle returned in chal_trace_init()
*
*  @return  ETB to AXI Status
*
*  @note    Bit Field 7:0 RAM_SIZE
*           Power-of-two for memory size for data storage minus
*	2 (e.g., 18 = 1 MB).
*
******************************************************************************/
	cUInt32 chal_trace_etb2axi_get_status(CHAL_HANDLE handle);

#if !defined(_SAMOA_)
/**
*
*  @brief  Set GLOBPERF_GLB_CONFIG - Counter Config
*
*  @param   handle (in) Handle returned in chal_trace_init()
*  @param   counter_stop_en (in) Enables counter-controlled global stop command generation.
*  @param   timeout (in) counter load value.
*
*  @return  status
*
*  @note
*
******************************************************************************/
	cBool chal_trace_globperf_set_config(CHAL_HANDLE handle,
					     cBool counter_stop_en,
					     cUInt32 timeout);

/**
*
*  @brief  Set GLOBPERF_GLB_CMD - Global Command
*
*  @param   handle (in) Handle returned in chal_trace_init()
*  @param   cmd (in) 0=clear, 1=stop, 2=start&resume, 4=load, 7=stop&reset.
*
*  @return  status
*
*  @note
*
******************************************************************************/
	cBool chal_trace_globperf_set_cmd(CHAL_HANDLE handle,
					  CHAL_TRACE_GLB_CMD_t cmd);

/**
*
*  @brief  Read GLOBPERF_GLB_STATUS - Request Status
*
*  @param   handle (in) Handle returned in chal_trace_init()
*
*  @return  GLOBPERF_GLB_STATUS - Request Status
*
*  @note    Bit Field [20] Request status for PM Trace
*           Bit Field [19:1] Request status for AXI Trace [19:1]
*           Bit Field [0] Request status for GIC Trace
*
******************************************************************************/
	cUInt32 chal_trace_globperf_get_status(CHAL_HANDLE handle);

/**
*
*  @brief  Read GLOBPERF_GLB_COUNT - Counter Status
*
*  @param   handle (in) Handle returned in chal_trace_init()
*
*  @return  GLOBPERF_GLB_COUNT - Counter Status
*
*  @note    Bit Field [31] counter enable status
*           Bit Field [27:0] global down-counter value.
*
******************************************************************************/
	cUInt32 chal_trace_globperf_get_count(CHAL_HANDLE handle);
#endif				/* !defined(_SAMOA_) */

#if !defined(_HERA_)
/**
*
*  @brief  Set ATB_STM Configuration
*
*  @param   handle (in) Handle returned in chal_trace_init()
*  @param   twobit_mode (in) Two bit mode. When set, PTI output is 2 bits wide.
*	When cleared, PTI output is 4 bits wide.
*  @param   break_limit (in) Break Limit.
*           This field sets the STP continuous message limit before forced
*	master ID insertion:
*           00=always break, 01=after 7th message, 10=after 15th,
*	and 11=after 31st.
*           For software messages the number of continous messages includes
*		any channel number.
*  @param   output_mode (in) Output Mode.
*           This field sets the output mode: 00=off, 01=PTI, 10=ATB, 11=rsvd.
*		When the output mode is off all input is dropped.
*           When the output mode is PTI, output data is intended
*	for the PTI pins.
*           When the output mode is ATB, output data is sent to the ATB Funnel
*		for TPIU consumption.
*  @param   atb_id (in) ATB output ID. This field sets the value of the ATB ID
*		for the bus output to the Funnel.
*
*  @return  status
*
*  @note
*
******************************************************************************/
	cBool chal_trace_atb_stm_set_config(CHAL_HANDLE handle,
					    cBool twobit_mode,
					    cUInt8 break_limit,
					    cUInt8 output_mode, cUInt8 atb_id);

/**
*
*  @brief  Set ATB_STM Master Enables Low/High
*
*  @param   handle (in) Handle returned in chal_trace_init()
*  @param   low_half (in) Master ID enables (low half).
*           This field has bits for enabling (value 1) STP master ID's 31:0.
*           Writes from disabled (value 0) masters are discarded
*		after handshaking from the input ATB.
*  @param   high_half (in) Master ID enables (high half).
*           This field has bits for enabling (value 1) STP master ID's 63:32.
*           Writes from disabled (value 0) masters are discarded after
*		handshaking from the input ATB.
*
*  @return  status
*
*  @note
*
******************************************************************************/
	cBool chal_trace_atb_stm_set_en(CHAL_HANDLE handle, cUInt32 low_half,
					cUInt32 high_half);

/**
*
*  @brief  Return ATB_STM Master Enables Low/High
*
*  @param   handle (in) Handle returned in chal_trace_init()
*  @param   high (in) 1 will select high_half register, 0 will select low_half register
*
*  @return  ATB_STM Master Enables Low/High
*
*  @note
*
******************************************************************************/
	cUInt32 chal_trace_atb_stm_get_en(CHAL_HANDLE handle, cBool high);

/**
*
*  @brief  Set ATB_STM Master Types Low/High
*
*  @param   handle (in) Handle returned in chal_trace_init()
*  @param   low_half (in) Master ID types (low half).
*           This field has bits for setting the type (1=SW, 0=HW)
*	for STP master ID's 31:0.
*           Software masters are expected to follow the SW_STM 2-cycle
*	protocol on the input ATB.
*           Hardware masters produce one value per ATB cycle.
*  @param   high_half (in) Master ID types (high half).
*           This field has bits for setting the type (1=SW, 0=HW)
*	for STP master ID's 63:32.
*           Software masters are expected to follow the SW_STM 2-cycle
*	protocol on the input ATB.
*           Hardware masters produce one value per ATB cycle.
*
*  @return  status
*
*  @note
*
******************************************************************************/
	cBool chal_trace_atb_stm_set_sw(CHAL_HANDLE handle, cUInt32 low_half,
					cUInt32 high_half);

/**
*
*  @brief  Return ATB_STM Master SW Low/High
*
*  @param   handle (in) Handle returned in chal_trace_init()
*  @param   high (in) 1 will select high_half register, 0 will select low_half register
*
*  @return  ATB_STM Master SW Low/High
*
*  @note
*
******************************************************************************/
	cUInt32 chal_trace_atb_stm_get_sw(CHAL_HANDLE handle, cBool high);

/**
*
*  @brief  Set software STM Configuration
*
*  @param   handle (in) Handle returned in chal_trace_init()
*  @param   swstm_st (in) indicate to use SWSTM or SWSTM_ST block instance
*  @param   stall_mode (in) When set, stalls APB bus for full output FIFO.
*	When clear, writes are dropped for full output FIFO.
*  @param   atb_id (in) ATB ID value. Timestamp data sets bit 0 on
*	ATB bus, non-timestamp data clears it.
*           Resulting master ID always even (bit 0 will be cleared).
*
*  @return  status
*
*  @note    atb_id used in this function is 6 bit unlike to other functions.
*
******************************************************************************/
	cBool chal_trace_sw_stm_set_config(CHAL_HANDLE handle,
					   CHAL_TRACE_SWSTM_t swstm_st,
					   cBool stall_mode, cUInt8 atb_id);

/**
*
*  @brief  Creates n-byte value with Channel nn.
*
*  @param   handle (in) Handle returned in chal_trace_init()
*  @param   swstm_st (in) indicate to use SWSTM or SWSTM_ST block instance
*  @param   n_channel (in) channel number (0 ~ 255)
*  @param   n_bytes (in) indicate to use 1, 2 or 4 byte channel
*  @param   value (in) data to be written to the channel
*
*  @return  status
*
*  @note
*
******************************************************************************/
	cBool chal_trace_sw_stm_write(CHAL_HANDLE handle,
				      CHAL_TRACE_SWSTM_t swstm_st,
				      cUInt8 n_channel, cUInt8 n_bytes,
				      cUInt32 value);
#endif				/* !defined(_HERA_) */

/** @} */

#ifdef __cplusplus
}
#endif
#endif				/* _CHAL_TRACE_H_ */

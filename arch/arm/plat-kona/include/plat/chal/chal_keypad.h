/*****************************************************************************
 * *  Copyright 2001 - 2008 Broadcom Corporation.  All rights reserved.
 * *
 * *  Unless you and Broadcom execute a separate written software license
 * *  agreement governing use of this software, this software is licensed to you
 * *  under the terms of the GNU General Public License version 2, available at
 * *  http://www.gnu.org/licenses/old-license/gpl-2.0.html (the "GPL").
 * *
 * *  Notwithstanding the above, under no circumstances may you combine this
 * *  software in any way with any other Broadcom software provided under a
 * *  license other than the GPL, without Broadcom's express prior written
 * *  consent.
 * *
 * ***************************************************************************/
/**
*  @file   chal_keypad.h
*
*  @brief  Keypad driver interface header file.
*
*  @note   Include "chal_common.h" before including this header file.
*
****************************************************************************/
#ifdef tempINTERFACE_OSDAL_KEYPAD

#ifndef _CHAL_KEYPAD_H_
#define _CHAL_KEYPAD_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup cHAL_Interface
 * @{
 */

#define MAX_SIMULTANEOUS_KEY_EVENTS             4
/* max valid key events in one interrupt - hardware defined */

	typedef enum {
		CHAL_KEYPAD_INTERRUPT_RISING_EDGE = 1,
		/* Interrupt triggered on rising edge */
		CHAL_KEYPAD_INTERRUPT_FALLING_EDGE = 2,
		/* Interrupt triggered on falling edge */
		CHAL_KEYPAD_INTERRUPT_BOTH_EDGES = 3,
		/* Interrupt triggerd on rising and falling edges */
		CHAL_KEYPAD_INTERRUPT_EDGE_MAX
		/* Enumeration maximum value */
	} CHAL_KEYPAD_INTERRUPT_EDGE_t;

	typedef enum {
		CHAL_KEYPAD_DEBOUNCE_1_ms = 0,
		/* Key debounced for 1 ms before triggering interrupt */
		CHAL_KEYPAD_DEBOUNCE_2_ms = 1,
		/* Key debounced for 2 ms before triggering interrupt */
		CHAL_KEYPAD_DEBOUNCE_4_ms = 2,
		/* Key debounced for 4 ms before triggering interrupt */
		CHAL_KEYPAD_DEBOUNCE_8_ms = 3,
		/* Key debounced for 8 ms before triggering interrupt */
		CHAL_KEYPAD_DEBOUNCE_16_ms = 4,
		/* Key debounced for 16 ms before triggering interrupt */
		CHAL_KEYPAD_DEBOUNCE_32_ms = 5,
		/* Key debounced for 32 ms before triggering interrupt */
		CHAL_KEYPAD_DEBOUNCE_64_ms = 6,
		/* Key debounced for 64 ms before triggering interrupt */
		CHAL_KEYPAD_DEBOUNCE_128_ms = 7,
		/* Key debounced for 128 ms before triggering interrupt */
		CHAL_KEYPAD_DEBOUNCE_MAX
		/* Enumeration maximum value */
	} CHAL_KEYPAD_DEBOUNCE_TIME_t;

	typedef enum {
		CHAL_KEYPAD_KEY_NO_ACTION,
		/* No key action occurred */
		CHAL_KEYPAD_KEY_PRESS,
		/* Key was pressed */
		CHAL_KEYPAD_KEY_RELEASE,
		/* Key was released */
		CHAL_KEYPAD_ACTION_MAX
		/* Enumneration maximum value */
	} CHAL_KEYPAD_ACTION_t;

	typedef cUInt8 CHAL_KEYPAD_KEY_ID_t;
	/* Identifier of the key that was pressed. */
	/* If no keymap is applied then the format of the number is 0xCR where
	 * C = Column number
	 * R = Row number
	 * of the physical key location
	 */

	typedef struct {
		cUInt8 rows;
		/* Number of rows in the physical keypad layout */
		cUInt8 columns;
		/* Number of columns in the physical keypad layout */
		Boolean pullUpMode;
		/* TRUE = Pull Up keypad output signals by default */
		/* FALSE = Pull Down keypad output signals */
		CHAL_KEYPAD_INTERRUPT_EDGE_t interruptEdge;
		/* Interrurpt triggering edge */
		CHAL_KEYPAD_DEBOUNCE_TIME_t debounceTime;
		/* Debounce for key state detection */
	} CHAL_KEYPAD_CONFIG_t;

	typedef struct {
		cUInt32 ssr0;
		cUInt32 ssr1;
		cUInt32 isr0;
		cUInt32 isr1;
	} CHAL_KEYPAD_REGISTER_SET_t;
	/* register set data for 1 interrupt event. */

	typedef struct {
		CHAL_KEYPAD_KEY_ID_t keyId;
		/* Identifier of a key */
		CHAL_KEYPAD_ACTION_t keyAction;
		/* Action occurring on that key */
	} CHAL_KEYPAD_EVENT_t;

	typedef CHAL_KEYPAD_EVENT_t
	 CHAL_KEYPAD_KEY_EVENT_LIST_t[MAX_SIMULTANEOUS_KEY_EVENTS];

/**
*
*  @brief  Inititalize keypad hardware
*
*  @param  config   (in)    keypad hardware configuration
*
*  @return none
*
****************************************************************************/
	CHAL_HANDLE chal_keypad_init(void __iomem *baseAddr);

/**
*
*  @brief  Enable/Disable hardware block
*
*  @param  handle   (in)    keypad hardware block id
*                enable   (in)    TRUE=enable keypad hw
*                                      FALSE=disable keyapd hw
*
*  @return none
*
****************************************************************************/
	void chal_keypad_set_enable(CHAL_HANDLE handle, Boolean enable);

/**
*
*  @brief  Set hardware into PullUp or PullDown Mode
*
*  @param  handle   (in)    keypad hardware block id
*                pullUp    (in)    TRUE=set keypad hw to PullUp mode
*                                      FALSE=set keyapd hw to PullDown Mode
*
*  @return none
*
****************************************************************************/
	void chal_keypad_set_pullup_mode(CHAL_HANDLE handle, Boolean pullUp);

/**
*
*  @brief  Set column filter debounce settigns
*
*  @param  handle   (in)    keypad hardware block id
*                enable   (in)    TRUE=enable debouce
*                                      FALSE=disable debounce
*                debounce  (in)  debounce time setting
*
*  @return none
*
****************************************************************************/
	void chal_keypad_set_column_filter(CHAL_HANDLE handle, Boolean enable,
					   CHAL_KEYPAD_DEBOUNCE_TIME_t
					   debounce);

/**
*
*  @brief  Set status filter debounce settigns
*
*  @param  handle   (in)    keypad hardware block id
*                enable   (in)    TRUE=enable debouce
*                                      FALSE=disable debounce
*                debounce  (in)  debounce time setting
*
*  @return none
*
****************************************************************************/
	void chal_keypad_set_status_filter(CHAL_HANDLE handle, Boolean enable,
					   CHAL_KEYPAD_DEBOUNCE_TIME_t
					   debounce);

/**
*
*  @brief  Set keypad column width control
*
*  @param  handle    (in)    keypad hardware block id
*                columns  (in)   number of column used in keypad configuration
*
*  @return none
*
****************************************************************************/
	void chal_keypad_set_column_width(CHAL_HANDLE handle, cUInt32 columns);

/**
*
*  @brief  Set keypad row width control
*
*  @param  handle    (in)    keypad hardware block id
*                rows       (in)   number of rows used in keypad configuration
*
*  @return none
*
****************************************************************************/
	void chal_keypad_set_row_width(CHAL_HANDLE handle, cUInt32 rows);

/**
*
*  @brief  Set keypad row output control
*
*  @param  handle    (in)    keypad hardware block id
*                rows       (in)   number of rows used in keypad configuration
*
*  @return none
*
****************************************************************************/
	void chal_keypad_set_row_output_control(CHAL_HANDLE handle,
						cUInt32 rows);

/**
*
*  @brief  Set keypad column output control
*
*  @param  handle    (in)    keypad hardware block id
*                columns  (in)   number of columns used in keypad configuration
*
*  @return none
*
****************************************************************************/
	void chal_keypad_set_column_output_control(CHAL_HANDLE handle,
						   cUInt32 cols);

/**
*
*  @brief  Set keypad interrupt edge control
*
*  @param  handle    (in)    keypad hardware block id
*                edge      (in)    edge configuration for interrupt generation
*
*  @return none
*
****************************************************************************/
	void chal_keypad_set_interrupt_edge(CHAL_HANDLE handle,
					    CHAL_KEYPAD_INTERRUPT_EDGE_t edge);

/**
*
*  @brief  Set keypad interrupt edge control
*
*  @param  handle    (in)    keypad hardware block id
*                rows       (in)   number of rows used in keypad configuration
*                columns  (in)   number of columns used in keypad configuration
*
*  @return none
*
****************************************************************************/
	void chal_keypad_set_interrupt_mask(CHAL_HANDLE handle, cUInt32 rows,
					    cUInt32 columns);

/**
*
*  @brief  Get keypad pullup mode status
*
*  @param  none
*
*  @return TRUE = Pullup mode
*              FALSE = Pulldown mode
*
****************************************************************************/
	Boolean chal_keypad_get_pullup_status(CHAL_HANDLE handle);

/**
*
*  @brief  Disable keypad event interrupts
*
*  @param  swap (in) TRUE = set swap row and column mode ON
*                              FALSE =  set swap row/column mode OFF
*
*  @return none
*
****************************************************************************/
	void chal_keypad_swap_row_and_column(CHAL_HANDLE handle, Boolean swap);

/**
*
*  @brief  Shutdown keypad hardware
*
*  @param  none
*
*  @return none
*
****************************************************************************/
	void chal_keypad_shutdown(CHAL_HANDLE handle);

/**
*
*  @brief  Clear all keypad interrupt set bits
*
*  @param  none
*
*  @return none
*
****************************************************************************/
	void chal_keypad_clear_interrupts(CHAL_HANDLE handle);

/**
*
*  @brief  Enable keypad event interrupts
*
*  @param  none
*
*  @return none
*
****************************************************************************/
	void chal_keypad_enable_interrupts(CHAL_HANDLE handle);

/**
*
*  @brief  Disable keypad event interrupts
*
*  @param  none
*
*  @return none
*
****************************************************************************/
	void chal_keypad_disable_interrupts(CHAL_HANDLE handle);

/**
*
*  @brief  Retrieve a key event from the stored event queue.
*
*  @param  event   (out) Key event info
*
*  @return none
*
*  @note  This function retrieves the first event from the key
*         event FIFO that was recorded druing interrupt
*         processing. If there are no events in the FIFO, then
*         keyAction in the CHAL_KEYPAD_ACTION_t is set to
*         CHAL_KEYPAD_KEY_NO_ACTION.
*
****************************************************************************/
	void chal_keypad_retrieve_key_event_registers(CHAL_HANDLE handle,
						      CHAL_KEYPAD_REGISTER_SET_t
						      *regState);

/**
*
*  @brief  Decode the data returned from chal_keypad_retreive_key_event_registers()
*
*  @param  regState  (in)   structure containing keypad registers with event data
*                keyEvents (out) list of key events that have occurred
*
*  @return  count of total number of key events.
*
****************************************************************************/
	cUInt32 chal_keypad_process_key_event_registers(CHAL_HANDLE *handle,
						CHAL_KEYPAD_REGISTER_SET_t
						*regState,
						CHAL_KEYPAD_KEY_EVENT_LIST_t
						keyEvents);
/**
*
*  @brief  Read Status 1 register
*
*  @param  none
*
*  @return UInt32   - register val
*
****************************************************************************/
	cUInt32 chal_keypad_config_read_status1(void);

/**
*
*  @brief  Read Status 2 register
*
*  @param  none
*
*  @return UInt32   - register val
*
****************************************************************************/
	cUInt32 chal_keypad_config_read_status2(void);

/** @} */

#ifdef __cplusplus
}
#endif
#endif				/* _CHAL_KEYPAD_H_ */
#else /* tempINTERFACE_OSDAL_KEYPAD */

#ifndef _CHAL_KEYPAD_H_
#define _CHAL_KEYPAD_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup cHAL_Interface
 * @{
 */

	typedef enum {
		CHAL_KEYPAD_INTERRUPT_RISING_EDGE = 1,
		/* Interrupt triggered on rising edge */
		CHAL_KEYPAD_INTERRUPT_FALLING_EDGE = 2,
		/* Interrupt triggered on falling edge */
		CHAL_KEYPAD_INTERRUPT_BOTH_EDGES = 3,
		/* Interrupt triggerd on rising and falling edges */
		CHAL_KEYPAD_INTERRUPT_EDGE_MAX
		/* Enumeration maximum value */
	} CHAL_KEYPAD_INTERRUPT_EDGE_t;

	typedef enum {
		CHAL_KEYPAD_DEBOUNCE_1_ms = 0,
		/* Key debounced for 1 ms before triggering interrupt */
		CHAL_KEYPAD_DEBOUNCE_2_ms = 1,
		/* Key debounced for 2 ms before triggering interrupt */
		CHAL_KEYPAD_DEBOUNCE_4_ms = 2,
		/* Key debounced for 4 ms before triggering interrupt */
		CHAL_KEYPAD_DEBOUNCE_8_ms = 3,
		/* Key debounced for 8 ms before triggering interrupt */
		CHAL_KEYPAD_DEBOUNCE_16_ms = 4,
		/* Key debounced for 16 ms before triggering interrupt */
		CHAL_KEYPAD_DEBOUNCE_32_ms = 5,
		/* Key debounced for 32 ms before triggering interrupt */
		CHAL_KEYPAD_DEBOUNCE_64_ms = 6,
		/* Key debounced for 64 ms before triggering interrupt */
		CHAL_KEYPAD_DEBOUNCE_128_ms = 7,
		/* Key debounced for 128 ms before triggering interrupt */
		CHAL_KEYPAD_DEBOUNCE_MAX
		/* Enumeration maximum value */
	} CHAL_KEYPAD_DEBOUNCE_TIME_t;

	typedef enum {
		CHAL_KEYPAD_KEY_NO_ACTION,
		/* No key action occurred */
		CHAL_KEYPAD_KEY_PRESS,
		/* Key was pressed */
		CHAL_KEYPAD_KEY_RELEASE,
		/* Key was released */
		CHAL_KEYPAD_ACTION_MAX
		/* Enumneration maximum value */
	} CHAL_KEYPAD_ACTION_t;

	typedef cUInt8 CHAL_KEYPAD_KEY_ID_t;
	/* Identifier of the key that was pressed. */
	/* If no keymap is applied then the format of the number is 0xCR where
	 * C = Column number
	 * R = Row number
	 * of the physical key location
	 */

	typedef struct {
		cUInt8 rows;
		/* Number of rows in the physical keypad layout */
		cUInt8 columns;
		/* Number of columns in the physical keypad layout */
		Boolean pullUpMode;
		/* TRUE = Pull Up keypad output signals by default */
		/* FALSE = Pull Down keypad output signals */
		CHAL_KEYPAD_INTERRUPT_EDGE_t interruptEdge;
		/* Interrurpt triggering edge */
		CHAL_KEYPAD_DEBOUNCE_TIME_t debounceTime;
		/* Debounce for key state detection */
	} CHAL_KEYPAD_CONFIG_t;

	typedef struct {
		CHAL_KEYPAD_KEY_ID_t keyId;
		/* Identifier of a key */
		CHAL_KEYPAD_ACTION_t keyAction;
		/* Action occurring on that key */
	} CHAL_KEYPAD_EVENT_t;

/**
*
*  @brief  Inititalize keypad hardware
*
*  @param  config   (in)    keypad hardware configuration
*
*  @return none
*
****************************************************************************/
	void chal_keypad_init(CHAL_KEYPAD_CONFIG_t config);

/**
*
*  @brief  Shutdown keypad hardware
*
*  @param  none
*
*  @return none
*
****************************************************************************/
	void chal_keypad_shutdown(void);

/**
*
*  @brief  Perform hardware dependant interrupt processing
*
*  @param  none
*
*  @return none
*
*  @note  This function performs the steps necessary to record a
*         keypad event from within an interrupt. The event can
*         later be retrieved using calls to
*         chal_keypad_retrieve_event() from less time-dependant
*         code.
*
****************************************************************************/
	void chal_keypad_handle_interrupt(void);

/**
*
*  @brief  Retrieve a key event from the stored event queue.
*
*  @param  event   (out) Key event info
*
*  @return none
*
*  @note  This function retrieves the first event from the key
*         event FIFO that was recorded druing interrupt
*         processing. If there are no events in the FIFO, then
*         keyAction in the CHAL_KEYPAD_ACTION_t is set to
*         CHAL_KEYPAD_KEY_NO_ACTION.
*
****************************************************************************/
	void chal_keypad_retrieve_event(CHAL_KEYPAD_EVENT_t *event);

/**
*
*  @brief  Clear all keypad interrupt set bits
*
*  @param  none
*
*  @return none
*
****************************************************************************/
	void chal_keypad_clear_interrupts(void);

/**
*
*  @brief  Enable keypad event interrupts
*
*  @param  none
*
*  @return none
*
****************************************************************************/
	void chal_keypad_enable_interrupts(void);

/**
*
*  @brief  Disable keypad event interrupts
*
*  @param  none
*
*  @return none
*
****************************************************************************/
	void chal_keypad_disable_interrupts(void);

/**
*
*  @brief  Reset the keypad block.
*
*  @param  none
*
*  @return none
*
****************************************************************************/
	void chal_keypad_config_reset(void);

/**
*
*  @brief  Read Status 1 register
*
*  @param  none
*
*  @return UInt32   - register val
*
****************************************************************************/
	cUInt32 chal_keypad_config_read_status1(void);

/**
*
*  @brief  Read Status 2 register
*
*  @param  none
*
*  @return UInt32   - register val
*
****************************************************************************/
	cUInt32 chal_keypad_config_read_status2(void);

/**
*
*  @brief  Clear Interrupt Status 1 register
*
*  @param  none
*
*  @return none
*
****************************************************************************/
	void chal_keypad_update_interrupt_clear_register0(cUInt32 value);

/**
*
*  @brief  Clear Interrupt Status 2 register
*
*  @param  none
*
*  @return none
*
****************************************************************************/
	void chal_keypad_update_interrupt_clear_register1(cUInt32 value);

/**
*
*  @brief  Read Keypad Pullup status
*
*  @param  none
*
*  @return UInt32  -  pullup status
*
****************************************************************************/
	cUInt32 chal_keypad_read_pullup_status(void);

/**
*
*  @brief  Set the Swap row and column feature
*
*  @param  swap     (in)     TRUE = turn swap row and column ON
*                           FALSE = set swap row and column OFF
*  @return none
*
****************************************************************************/
	void chal_keypad_swap_row_and_column(Boolean swap);

/** @} */

#ifdef __cplusplus
}
#endif
#endif				/* _CHAL_KEYPAD_H_ */
#endif				/* tempINTERFACE_OSDAL_KEYPAD */

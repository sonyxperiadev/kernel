//*********************************************************************
//
// (c)1999-2011 Broadcom Corporation
//
// Unless you and Broadcom execute a separate written software license agreement governing use of this software,
// this software is licensed to you under the terms of the GNU General Public License version 2,
// available at http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
//
//*********************************************************************
/**
*
*   @file   logapi.h
*
*   @brief  This file defines the interface for log API.
*
****************************************************************************/
/**

*   @defgroup   LogAPIGroup   Log API
*   @ingroup    LoggingGroup
*
*   @brief      This group defines the interfaces and the utilities related to
				MS logging API.

\section logid Logging ID
This ID defines a functional group that the debugging traces apply to.

- Logging ID's 0 - 254, and 256 are reserved for binary logging.
- Logging ID's 384 - 511 are customers ASCII logging groups.

- 0 - 191 are stack binary logging.  Defined in msp/stack/inc/proc_id.h
- 192 - 223 are customer binary logging ID's.
- 224 - 254, and 256 are Broadcom non-stack binary logging
- 251 = integrated WTT log (900kbps)
- 252 = inter-task messages
- 256 = stand-alone WTT log (UARTC)
- Further values are reserved in the range 255, 257 - ::LAST_RESERVED_ID.  They 
can be found in the file msp/system/inc/logapi.h
	
By default, logging ID's 0-250, and 252-256 are enabled.

Enable logging by issuing Log_EnableLogging().  A range of logging IDs could 
also be enabled by issuing Log_EnableRange().

\section Logformat Logging Format

Log_SelectLoggingFormat() selects the format of the output traces.  
- Select ::LOGFORMAT_ASCII if a text terminal program will be used to display the traces, 
- Select ::LOGFORMAT_MOBILE_ANALYZER if the Mobile Analyzer (MA) will be used to display the
traces
- Select ::LOGFORMAT_BINARY if the MTT will be used.

The default format set in the sysparm is to use MTT.

\subsection LogCommands ASCII Logging Functions

For strings, the logging function is Log_DebugOutputString().\n
For values, the logging functions are Log_DebugOutputValue(), 
Log_DebugOutputValue2(), ...Log_DebugOutputValue7()\n
For arrays, the logging function is Log_DebugOutputArray().\n
For a general printf, the logging function is Log_DebugPrintf().\n

\subsection Legacy Legacy ASCII Logging Functions

Legacy ASCII logging functions has been re-mapped to the above functions with the
corresponding logging IDs:
 
    Legacy driver logging -- TRACE_Printf_Sio()  --> 275\n
    Legacy MMI logging -- TRACE_Printf_Log() --> 276\n
 
These legacy functions should not be used in the new codes.

\subsection Binary Binary Logging Functions

For binary signals, the logging function is Log_DebugSignal().\n
For binary signals described by a link list, the logging function is Log_DebugLinkList().\n

Use these as needed.

\subsection Misc Miscellaneous

Log_LimitedLogging() is a specialized command only used by the customer's customer.  It 
limits the logging to specialized items to debug specialized network problems.

\section ATlogging AT commands that control logging

Broadcom proprietary at*mtest commands control CP logging and non-FUSE logging. 
Before issuing any BRCM proprietary AT command to CP or non-FUSE UE, one must 
issue at*mtest=1,1,"coolcard" first.

Broadcom proprietary at*apmtest commands control AP logging. Before issuing any 
BRCM proprietary AT command to AP, one must issue at*apmtest=1,1,"coolcard" first.

AP and CP have identical sets of at*mtest commands. We explain the CP set only below.

@param at*mtest=2,3 It triggers a crash.
@param at*mtest=3,x,y It turns on logs with logging ID from x to y on CP or non-FUSE UE.
@param at*mtest=4,x,y It turns off logs with logging ID from x to y on CP or non-FUSE UE.
@param at*mtest=5,x It changes logging format: x=0 means ASCII format; x=1 means Mobile 
Analyzer format; x=2 means MTT format. This command had better be used before phone starts running.
@param  at*mtest=8,t It starts periodical CPU profiling. t is the cycle, in unit of one minute. 
If t= 0, it stops periodical CPU profiling; otherwise, every t minutes, the profiling 
result is logged. If profiling with logging disabled and one-minute cycle is desired, 
one can issue at*mtest=4,0,511; at*mtest=3,33,33; and at*mtest=8,1. The reason why 
at*mtest=3,33,33 (turn on SIM logging) is needed is to log CPU profiling results at minimum 
logging cost; otherwise, if all 512 logging IDs are turned off, CPU profiling 
cannot be logged either.

\section sysparmlogging System parameters that control logging

@param TRACE_OFF If TRACE_OFF is 0, logging is enabled; otherwise, logging is disabled.

@param DEFAULT_LOG_PORT It can be set to be A (UART A), B (UART B), C (UART C, not 
supported by most of platforms), U (USB), or N (Null). If it is set to be N,
please use JTAG/ETM to debug.

@param LOG_FORMAT It can be 0 (ASCII logging), 1 (Mobile Analyzer logging), and 2 (MTT logging)

@param UART_TYPE the high 16 bits specifies UART A's baud rate, and low 16 bits specifies 
UART B's baud rate. The meaningful baud rate codes are: 6 (115kbps), 7 (230kbps), 
8 (460kbps), 9 (921kbps), 0x0E (2Mps, hacked FTDI driver needed), 0x0F (3Mbps, hacked
FTDI driver needed).

*	
****************************************************************************/

#ifndef __LOGAPI_H__
#define __LOGAPI_H__

#include "mobcom_types.h"
#include "string.h"

/**
 * @addtogroup LogAPIGroup
 * @{
 */

#define		LOGFORMAT_BINARY			2 ///< Logging format for the MTT.
#define		LOGFORMAT_MOBILE_ANALYZER	1 ///< Logging Format for the Mobile Analyzer.
#define		LOGFORMAT_ASCII				0 ///< Logging format in ASCII, use a text terminal program to read (like Hyperterm)

#define	MAX_CLIENT_STRING_LENGTH	128		///< maximum string length that can be logged
#define	MAX_CLIENT_SIGNAL_SIZE		0x4000	///< maximum signal size that can be logged

//Pre-reserve the space for  the stack process ID defined in proc_id.h

#define	P_log_general			250
#define P_umtsfw				251
#define	P_itmsg					252
#define P_no_receiver			253
#define P_many_receiver			254

#define FIRST_STACK_SDL_LOGID	0
#define LAST_STACK_SDL_LOGID	254
#define MAX_LOGGING_PROCESSES	8192
#define	MAX_ITMSG_GROUPS		256
#define	MAX_STACK_LOG_GROUPS	256

//Pre-Reserve the space LAST_STACK_SDL_LOGID+1 to LAST_STACK_SDL_LOGID+31
//for the other MS process log 

#define LOGID_L1				255
#define LOGID_UMTS_FW			256
#define LOGID_L1_MAC_BLOCK		257
#define LOGID_USB				258
#define	LOGID_ATC				259
#define LOGID_MNDS				260
#define LOGID_PCH				261
#define LOGID_MPX				262
#define LOGID_FLW				263
#define LOGID_ECDC				264
#define LOGID_MAC				265
#define LOGID_STACK				266
#define LOGID_PPP				267
#define LOGID_SIM				268
#define LOGID_MISC				269
#define LOGID_CAPI2_BASIC		270
#define LOGID_CAPI2_DETAIL		271
#define LOGID_IPC				272
#define LOGID_LCS_GLL			273
#define LOGID_LCS_API			274
#define	LOGID_DRIVER			275
#define	LOGID_MMI				276
#define LOGID_FFS				277
#define LOGID_DATA_DETAIL		278
#define	LOGID_BLUETOOTH			279
#define	LOGID_AUDIO				280
#define	LOGID_AUDIO_DETAIL		280
#define	LOGID_SOC_AUDIO			280
#define	LOGID_SOC_AUDIO_DETAIL	280
#define	LOGID_OMX_AUDIO			280
#define	LOGID_OMX_AUDIO_DETAIL	280

#define	LOGID_MMS				281
#define	LOGID_AUXADC			282
#define	LOGID_WLAN_ERR			283
#define	LOGID_WLAN_INFO			284
#define	LOGID_SDIO_INFO			285
#define	LOGID_PPP_DETAIL	    286  /* for dlink_multi detailed traces, e.g. to debug TCP memory usage, NET_RESOURCE, and more. */
#define	LOGID_MINDREADER		287
#define LOGID_WLAN_GENERIC_DBG	288

#ifdef CNEON_COMMON
#define LOGID_GLUEAPI_ID1       301
#define LOGID_GLUEAPI_ID2       302
#define LOGID_GLUEAPI_ID3       303
#define LOGID_GLUEAPI_ID4       304
#define LOGID_GLUEAPI_ID5       305
#define LOGID_GLUEAPI_ID6       306
#define LOGID_GLUEAPI_ID7       307
#define LOGID_GLUEAPI_ID8       308
#define LOGID_GLUEAPI_ID9       309
#define LOGID_GLUEAPI_MISC      310

#define LOGID_CHIPSET_ASSERT_LVL0    311
#define LOGID_CHIPSET_ASSERT_LVL1    312
#define LOGID_CHIPSET_ASSERT_LVL2    313
#else
#define LOGID_GLUEAPI_ID1       301
#define LOGID_GLUEAPI_ID2       302
#define LOGID_GLUEAPI_ID3       303
#define LOGID_GLUEAPI_ID4       304
#define LOGID_GLUEAPI_ID5       305
#define LOGID_GLUEAPI_ID6       306
#define LOGID_GLUEAPI_ID7       307
#define LOGID_GLUEAPI_ID8       308
#define LOGID_GLUEAPI_ID9       309
#define LOGID_GLUEAPI_ID10      310
#endif


#define	LOGID_ATC_HANDLERS						311
#define	LOGID_ATC_PARSER						312
#define	LOGID_ATC_TOOL							313

#define	LOGID_DATASERVICES_IP					314
#define	LOGID_DATASERVICES_APPS					315

#define	LOGID_MODEM_CAPI_RPC					316
#define	LOGID_MODEM_CAPI_CC						317
#define	LOGID_MODEM_CAPI_DATAACCT				318
#define	LOGID_MODEM_CAPI_DS						319
#define	LOGID_MODEM_CAPI_ECDC					264		// same as LOGID_ECDC
#define	LOGID_MODEM_CAPI_LCS					320
#define	LOGID_MODEM_CAPI_PCH					261		// same as LOGID_PCH
#define	LOGID_MODEM_CAPI_PHONEBK				321
#define	LOGID_MODEM_CAPI_PHONECTRL				322
#define	LOGID_MODEM_CAPI_SIM					268		// same as LOGID_SIM
#define	LOGID_MODEM_CAPI_SIMLOCK				323
#define	LOGID_MODEM_CAPI_SMS					324
#define	LOGID_MODEM_CAPI_SS						325
#define	LOGID_MODEM_CAPI_SYSTEM					326
#define	LOGID_MODEM_CAPI_UTIL					327

#define	LOGID_SYSINTERFACE_SERIALMGR_BASIC		328
#define	LOGID_SYSINTERFACE_SERIALMGR_DETAIL		329
#define	LOGID_SYSINTERFACE_RPC_BASIC			330
#define	LOGID_SYSINTERFACE_RPC_DETAIL			331

#define LOGID_CONNECTIVITY_FM_DRIVER			332
#define LOGID_CONNECTIVITY_AGPS_DRIVER			333
#define LOGID_CONNECTIVITY_AGPS_LCS_GLL			334
#define LOGID_CONNECTIVITY_AGPS_LCS_API			335

#define	LOGID_CONNECTIVITY_BLUETOOTH			279		// same as LOGID_BLUETOOTH

#define LOGID_CONNECTIVITY_WLAN					285		// same as LOGID_WLAN_INFO

#define LOGID_SOC_CSL_KEYPAD					336
#define LOGID_SOC_CSL_PWM						337
#define LOGID_SOC_CSL_SDIO						338
#define LOGID_SOC_CSL_SPI						339
#define	LOGID_SOC_CSL_USB						340

#define LOGID_SYSINTERFACE_HAL_GPIO				341
#define LOGID_SYSINTERFACE_HAL_KEYPAD			342
#define LOGID_SYSINTERFACE_HAL_LIGHTING			343
#define LOGID_SYSINTERFACE_HAL_VIBRATOR			344
#define LOGID_SYSINTERFACE_HAL_ACCESSORY		345
#define LOGID_SYSINTERFACE_HAL_ADC				346
#define LOGID_SYSINTERFACE_EM_MEASMGR			347

#define	LOGID_PERIPHERALS_LCD					348
#define	LOGID_PERIPHERALS_TVOUT					349

#define LOGID_DAG								350		// (display and graphics)
#define LOGID_MULTIMEIDA						351
#define LOGID_MULTIMEDIA_CODEC					352
#define LOGID_OMX								353
#define LOGID_SOC_CSL_MULTIMEDIA				354

#define LOGID_CAM				                355
#define LOGID_SYSINTERFACE_HAL_CAM				356
#define	LOGID_PERIPHERALS_CAM					357
#define	LOGID_SOC_CSL_CAM						358

#define LOGID_STKAPP                            359

#define	LOGID_SOC_PM_PRM						360
#define	LOGID_SOC_PM_PRM_TEST					361
#define	LOGID_SOC_PM_DORMANT					362
#define	LOGID_SOC_PM_DFS						363

#define	LOGID_SYSINTERFACE_PM_DVFS				364
#define	LOGID_SOC_PM_DVFS						365

#define	LOGID_V3D								366

#define	LOGID_PMU		                		367

#define	LOGID_SOC_CSL_DSP						368

#define	LOGID_WARNING							400

#define LOGID_LTE_GENERAL						401
#define LOGID_LTE_INIT							402
#define LOGID_LTE_ASSERT						403
#define LOGID_LTE_SKL							404
#define LOGID_LTE_MIMO_MAIN						405
#define LOGID_LTE_USER_CONTROLLER				406
#define LOGID_LTE_CLI_MAIN						407
#define LOGID_LTE_AT_PARSER						408
#define LOGID_LTE_GEN_COMMAND_PARSER			409
#define LOGID_LTE_NAS_UE_CLI_SUPPORT			410
#define LOGID_LTE_CONFIQ_PARSER					411
#define LOGID_LTE_MOB_CTRL_CLI_SUPP				412
#define LOGID_LTE_USER_AL						413
#define LOGID_LTE_NV_MEM_CONT					414
#define LOGID_LTE_NAM_CAPI						415
#define LOGID_LTE_EMM_SM						416
#define LOGID_LTE_NAM							417
#define LOGID_LTE_PLM							418
#define LOGID_LTE_ADM							419
#define LOGID_LTE_SCM							420
#define LOGID_LTE_TAM							421
#define LOGID_LTE_ESM							422
#define LOGID_LTE_CDB							423
#define LOGID_LTE_USIM_CONT						424
#define LOGID_LTE_AS_AL							425
#define LOGID_LTE_SEC_AL						426
#define LOGID_LTE_CRYPTO						427
#define LOGID_LTE_ASM							428
#define LOGID_LTE_ASM_API						429
#define LOGID_LTE_ASM_COMMON_RAT_API			430
#define LOGID_LTE_ASM_DYNAMIC_CONFIGS			431
#define LOGID_LTE_ASM_ENTRY_POINT				432
#define LOGID_LTE_ASM_MSG_HANDLER				433
#define LOGID_LTE_UE_ASM_ICM_STATS				434
#define LOGID_LTE_UE_DYNAMIC_CONFIGS			435
#define LOGID_LTE_AS_ENTRY_POINT				436
#define LOGID_LTE_PERF_MEASURE					437
#define LOGID_LTE_HO_MEASURE					438
#define LOGID_LTE_CFG							439
#define LOGID_LTE_RLM							440
#define LOGID_LTE_RMH							441
#define LOGID_LTE_SIH							442
#define LOGID_LTE_ULA							443
#define LOGID_LTE_ULA_API						444
#define LOGID_LTE_TRAFFIC						445
#define LOGID_LTE_PDCP_HEADER					446
#define LOGID_LTE_PDCP_TX						447
#define LOGID_LTE_PDCP_RX						448
#define LOGID_LTE_RLC_HEADER					449
#define LOGID_LTE_RLC_TX						450
#define LOGID_LTE_RLC_RX						451
#define LOGID_LTE_MAC_HH						452
#define LOGID_LTE_MAC_TX						453
#define LOGID_LTE_MAC_RX						454
#define LOGID_LTE_RA							455
#define LOGID_LTE_PAL							456
#define LOGID_LTE_HAL							457
#define LOGID_LTE_SAL							458
#define LOGID_LTE_RSVL1							459
#define LOGID_LTE_UE_AS_ICM_STATS				460

/* 461-470 reserved for SCC modules */
#define LOGID_IMS_SUE_APP						461
#define LOGID_IMS_SAC_APP						462
#define LOGID_IMS_SMS_APP						463
#define LOGID_IMS_VOLTE_APP						464
#define LOGID_IMS_SUE_ENGINE					465
#define LOGID_IMS_SAC_ENGINE					466
#define LOGID_IMS_SMS_ENGINE					467
#define LOGID_IMS_VOLTE_ENGINE					468
#define LOGID_IMS_PDN_MGR						469
#define LOGID_KERNEL_PRINT						470

#define LOGID_LTE_CFG_CRITICAL					501
#define LOGID_LTE_CFG_MAJOR						502
#define LOGID_LTE_CFG_MINOR						503
#define LOGID_LTE_ASM_CRITICAL					504
#define LOGID_LTE_ASM_MAJOR						505
#define LOGID_LTE_ASM_MINOR						506
#define LOGID_LTE_RLM_CRITICAL					507
#define LOGID_LTE_RLM_MAJOR						508
#define LOGID_LTE_RLM_MINOR						509
#define LOGID_LTE_RMH_CRITICAL					510
#define LOGID_LTE_RMH_MAJOR						511
#define LOGID_LTE_RMH_MINOR						512
#define LOGID_LTE_SIH_CRITICAL					513
#define LOGID_LTE_SIH_MAJOR						514
#define LOGID_LTE_SIH_MINOR						515

#define LOGID_LTE_PDCP_TX_CRITICAL				516
#define LOGID_LTE_PDCP_RX_CRITICAL				517
#define LOGID_LTE_RLC_TX_CRITICAL				518
#define LOGID_LTE_RLC_RX_CRITICAL				519
#define LOGID_LTE_MAC_TX_CRITICAL				520
#define LOGID_LTE_MAC_RX_CRITICAL				521
#define LOGID_LTE_MAC_RA_CRITICAL				522
#define LOGID_LTE_MAC_HH_CRITICAL				523
#define LOGID_LTE_NAM_CRITICAL					524
#define LOGID_LTE_PLM_CRITICAL					525
#define LOGID_LTE_ADM_CRITICAL					526
#define LOGID_LTE_TAM_CRITICAL					527
#define LOGID_LTE_SCM_CRITICAL					528
#define LOGID_LTE_ESM_CRITICAL					529
#define LOGID_LTE_TCM_CRITICAL					530
#define LOGID_LTE_ULA_CRITICAL					531
#define LOGID_LTE_NAS_CRITICAL					532
#define LOGID_LTE_LMAC_CRITICAL         		533

#define LOGID_LTE_PDCP_TX_MAJOR					543
#define LOGID_LTE_PDCP_RX_MAJOR					544
#define LOGID_LTE_RLC_TX_MAJOR					545
#define LOGID_LTE_RLC_RX_MAJOR					546
#define LOGID_LTE_MAC_TX_MAJOR					547
#define LOGID_LTE_MAC_RX_MAJOR					548
#define LOGID_LTE_MAC_RA_MAJOR					549
#define LOGID_LTE_MAC_HH_MAJOR					550
#define LOGID_LTE_NAM_MAJOR						551
#define LOGID_LTE_PLM_MAJOR						552
#define LOGID_LTE_ADM_MAJOR						553
#define LOGID_LTE_TAM_MAJOR						554
#define LOGID_LTE_SCM_MAJOR						555
#define LOGID_LTE_ESM_MAJOR						556
#define LOGID_LTE_TCM_MAJOR						557
#define LOGID_LTE_ULA_MAJOR						558
#define LOGID_LTE_NAS_MAJOR						559
#define LOGID_LTE_LMAC_MAJOR					560		
#define LOGID_LTE_LMAC_TX_MAJOR					561		
#define LOGID_LTE_LMAC_RX_MAJOR					562		
#define LOGID_LTE_LMAC_POWER_MAJOR				563	
#define LOGID_LTE_LMAC_HO_MEAS_MAJOR			564
#define LOGID_LTE_LMAC_RACH_MAJOR				565	
#define LOGID_LTE_LMAC_RLM_MAJOR				566	

#define LOGID_LTE_PDCP_TX_MINOR					571
#define LOGID_LTE_PDCP_RX_MINOR					572
#define LOGID_LTE_RLC_TX_MINOR					573
#define LOGID_LTE_RLC_RX_MINOR					574
#define LOGID_LTE_MAC_TX_MINOR					575
#define LOGID_LTE_MAC_RX_MINOR					576
#define LOGID_LTE_MAC_RA_MINOR					577
#define LOGID_LTE_MAC_HH_MINOR					578
#define LOGID_LTE_NAM_MINOR						579
#define LOGID_LTE_PLM_MINOR						580
#define LOGID_LTE_ADM_MINOR						581
#define LOGID_LTE_TAM_MINOR						582
#define LOGID_LTE_SCM_MINOR						583
#define LOGID_LTE_ESM_MINOR						584
#define LOGID_LTE_TCM_MINOR						585
#define LOGID_LTE_ULA_MINOR						586
#define LOGID_LTE_NAS_MINOR						587
#define LOGID_LTE_LMAC_MINOR					588
//#define LOGID_LTE_MINOR_END						598

/* Log ids 601 - 650 are reserved for LTE SCC */
//#define LOGID_LTE_SCC_GROUP1					601
#define LOGID_LTE_SCC_CAPI_LIB					601
#define LOGID_LTE_SCC_RSA						602
#define LOGID_LTE_SCC_IMS_SUE_APP				603
#define LOGID_LTE_SCC_IMS_SAC_APP				604
#define LOGID_LTE_SCC_IMS_SMS_APP				605
#define LOGID_LTE_SCC_IMS_VOLTE_APP				606
#define LOGID_LTE_SCC_IMS_SUE_ENGINE			607
#define LOGID_LTE_SCC_IMS_SAC_ENGINE			608
#define LOGID_LTE_SCC_IMS_SMS_ENGINE			609
#define LOGID_LTE_SCC_IMS_VOLTE_ENGINE			610
#define LOGID_LTE_SCC_IMS_PDN_MGR				611
#define LOGID_LTE_SCC_IMS_GENERAL				612

#define LOGID_LTE_SCC_KERNEL_GENERAL			649
//#define LOGID_LTE_SCC_GROUP50					650

#define	LOGID_RTOS								666

// This logging ID is used when it is needed to log only one kind of messages
// No logging code should use this ID unconditionally, because it would void above purpose.
// The logging code using this ID should be turned on/off by, e.g., an AT command (default off)
// Hui Luo, 6/18/09
#define	LOGID_EXCLUSIVE							777

#define	LOGID_PROFILING							888

#define P_log_chart                             999

/// last logging id that would be used by Broadcom platform
/// the numbers after this are all for MMI/application use
#define LAST_RESERVED_ID						1023

#define	PSEUDO_LOGID_OUTPUT						0xFFFF
#define	PSEUDO_LOGID_CUSTOMER					0xFFFE
#define	PSEUDO_LOGID_CP							0xFFFD
#define	PSEUDO_LOGID_AP							0xFFFC
#define	PSEUDO_LOGID_LOGCTRL					0xFFFB
	
#include "log_sig_code.h"

typedef struct
{
    char title[32]; // chart name
    char x_axis_name[16]; // x axis name, such as "TimeLine".
    char y_axis_name[16];
    char x_unit[8]; // x axis unit name, such as "ms".
    char y_unit[8];
	UInt16 array_length; // specify the number of entries in the array.
	UInt16 display_style; // 0: line; 1: dots only; 2: bars on X axis; 3: bars on Y axis
	UInt32 background_color; // 00RRGGBB
	UInt32 line_color; // 00RRGGBB
	UInt8 x_value_visible; // determine whether to show x value for each point. boolean - 0:false 1:true
	UInt8 y_value_visible; // determine whether to show y value for each point. boolean - 0:false 1:true
	UInt8 x_array_unit_size; // specify the number of bytes per array entry (1, 2, or 4 are supported, little-endian only; 0 means no x array in dereference, x array is derived from array_length, x_min, and x_max)
	UInt8 y_array_unit_size; // specify the number of bytes per array entry (1, 2, or 4 are supported, little-endian only)
	Int32 x_min; // minimum value of x scale, if both x_min and x_max are zero, auto-scaling will be used.
	Int32 x_max; // maximum value of x scale, if both x_min and x_max are zero, auto-scaling will be used.
	Int32 y_min; // minimum value of y scale, if both y_min and y_max are zero, auto-scaling will be used.
	Int32 y_max; // maximum value of y scale, if both y_min and y_max are zero, auto-scaling will be used.
} T_CHART_X_Y_HEADER;

typedef T_CHART_X_Y_HEADER CHART_X_Y;

typedef struct
{
    char title[32]; // chart name
    char x_axis_name[16]; // x axis name, such as "TimeLine".
    char y_axis_name[16];
    char x_unit[8]; // x axis unit name, such as "ms".
    char y_unit[8];
	UInt16 array_length; // specify the number of entries in the array.
	UInt16 display_style; // 0: line; 1: dots only; 2: bars on X axis; 3: bars on Y axis
	UInt32 background_color; // 00RRGGBB
	UInt32 line1_color; // 00RRGGBB
	UInt32 line2_color; // 00RRGGBB
	UInt8 x_value_visible; // determine whether to show x value for each point. boolean - 0:false 1:true
	UInt8 y1_value_visible; // determine whether to show y value for each point. boolean - 0:false 1:true
	UInt8 y2_value_visible; // determine whether to show y value for each point. boolean - 0:false 1:true
	UInt8 x_array_unit_size; // specify the number of bytes per array entry (1, 2, or 4 are supported, little-endian only; 0 means no x array in dereference, x array is derived from array_length, x_min, and x_max)
	UInt8 y1_array_unit_size; // specify the number of bytes per array entry (1, 2, or 4 are supported, little-endian only)
	UInt8 y2_array_unit_size; // specify the number of bytes per array entry (1, 2, or 4 are supported, little-endian only)
	Int32 x_min; // minimum value of x scale, if both x_min and x_max are zero, auto-scaling will be used.
	Int32 x_max; // maximum value of x scale, if both x_min and x_max are zero, auto-scaling will be used.
	Int32 y1_min; // minimum value of y scale, if both y_min and y_max are zero, auto-scaling will be used.
	Int32 y1_max; // maximum value of y scale, if both y_min and y_max are zero, auto-scaling will be used.
	Int32 y2_min; // minimum value of y scale, if both y_min and y_max are zero, auto-scaling will be used.
	Int32 y2_max; // maximum value of y scale, if both y_min and y_max are zero, auto-scaling will be used.
} T_CHART_X_2Y_HEADER;

typedef T_CHART_X_2Y_HEADER CHART_X_2Y;

typedef struct
{
    char title[32]; // chart name
    char x_axis_name[16]; // x axis name, such as "TimeLine".
    char y_axis_name[16];
    char x_unit[8]; // x axis unit name, such as "ms".
    char y_unit[8];
	UInt16 array_length; // specify the number of entries in the array.
	UInt16 display_style; // 0: line; 1: dots only; 2: bars on X axis; 3: bars on Y axis
	UInt32 background_color; // 00RRGGBB
	UInt32 line1_color; // 00RRGGBB
	UInt32 line2_color; // 00RRGGBB
	UInt32 line3_color; // 00RRGGBB
	UInt8 x_value_visible; // determine whether to show x value for each point. boolean - 0:false 1:true
	UInt8 y1_value_visible; // determine whether to show y value for each point. boolean - 0:false 1:true
	UInt8 y2_value_visible; // determine whether to show y value for each point. boolean - 0:false 1:true
	UInt8 y3_value_visible; // determine whether to show y value for each point. boolean - 0:false 1:true
	UInt8 x_array_unit_size; // specify the number of bytes per array entry (1, 2, or 4 are supported, little-endian only; 0 means no x array in dereference, x array is derived from array_length, x_min, and x_max)
	UInt8 y1_array_unit_size; // specify the number of bytes per array entry (1, 2, or 4 are supported, little-endian only)
	UInt8 y2_array_unit_size; // specify the number of bytes per array entry (1, 2, or 4 are supported, little-endian only)
	UInt8 y3_array_unit_size; // specify the number of bytes per array entry (1, 2, or 4 are supported, little-endian only)
	Int32 x_min; // minimum value of x scale, if both x_min and x_max are zero, auto-scaling will be used.
	Int32 x_max; // maximum value of x scale, if both x_min and x_max are zero, auto-scaling will be used.
	Int32 y1_min; // minimum value of y scale, if both y_min and y_max are zero, auto-scaling will be used.
	Int32 y1_max; // maximum value of y scale, if both y_min and y_max are zero, auto-scaling will be used.
	Int32 y2_min; // minimum value of y scale, if both y_min and y_max are zero, auto-scaling will be used.
	Int32 y2_max; // maximum value of y scale, if both y_min and y_max are zero, auto-scaling will be used.
	Int32 y3_min; // minimum value of y scale, if both y_min and y_max are zero, auto-scaling will be used.
	Int32 y3_max; // maximum value of y scale, if both y_min and y_max are zero, auto-scaling will be used.
} T_CHART_X_3Y_HEADER;

typedef T_CHART_X_3Y_HEADER CHART_X_3Y;

typedef struct
{
    char title[32]; // chart name
    char x_axis_name[16]; // x axis name, such as "TimeLine".
    char y_axis_name[16];
    char x_unit[8]; // x axis unit name, such as "ms".
    char y_unit[8];
	UInt16 array_length; // specify the number of entries in the array.
	UInt16 display_style; // 0: line; 1: dots only; 2: bars on X axis; 3: bars on Y axis
	UInt32 background_color; // 00RRGGBB
	UInt32 line1_color; // 00RRGGBB
	UInt32 line2_color; // 00RRGGBB
	UInt32 line3_color; // 00RRGGBB
	UInt32 line4_color; // 00RRGGBB
	UInt8 x_value_visible; // determine whether to show x value for each point. boolean - 0:false 1:true
	UInt8 y1_value_visible; // determine whether to show y value for each point. boolean - 0:false 1:true
	UInt8 y2_value_visible; // determine whether to show y value for each point. boolean - 0:false 1:true
	UInt8 y3_value_visible; // determine whether to show y value for each point. boolean - 0:false 1:true
	UInt8 y4_value_visible; // determine whether to show y value for each point. boolean - 0:false 1:true
	UInt8 x_array_unit_size; // specify the number of bytes per array entry (1, 2, or 4 are supported, little-endian only; 0 means no x array in dereference, x array is derived from array_length, x_min, and x_max)
	UInt8 y1_array_unit_size; // specify the number of bytes per array entry (1, 2, or 4 are supported, little-endian only)
	UInt8 y2_array_unit_size; // specify the number of bytes per array entry (1, 2, or 4 are supported, little-endian only)
	UInt8 y3_array_unit_size; // specify the number of bytes per array entry (1, 2, or 4 are supported, little-endian only)
	UInt8 y4_array_unit_size; // specify the number of bytes per array entry (1, 2, or 4 are supported, little-endian only)
	Int32 x_min; // minimum value of x scale, if both x_min and x_max are zero, auto-scaling will be used.
	Int32 x_max; // maximum value of x scale, if both x_min and x_max are zero, auto-scaling will be used.
	Int32 y1_min; // minimum value of y scale, if both y_min and y_max are zero, auto-scaling will be used.
	Int32 y1_max; // maximum value of y scale, if both y_min and y_max are zero, auto-scaling will be used.
	Int32 y2_min; // minimum value of y scale, if both y_min and y_max are zero, auto-scaling will be used.
	Int32 y2_max; // maximum value of y scale, if both y_min and y_max are zero, auto-scaling will be used.
	Int32 y3_min; // minimum value of y scale, if both y_min and y_max are zero, auto-scaling will be used.
	Int32 y3_max; // maximum value of y scale, if both y_min and y_max are zero, auto-scaling will be used.
	Int32 y4_min; // minimum value of y scale, if both y_min and y_max are zero, auto-scaling will be used.
	Int32 y4_max; // maximum value of y scale, if both y_min and y_max are zero, auto-scaling will be used.
} T_CHART_X_4Y_HEADER;

typedef T_CHART_X_4Y_HEADER CHART_X_4Y;

typedef struct
{
    char title[32]; // chart name
    char x_axis_name[16]; // x axis name, such as "TimeLine".
    char y_axis_name[16];
    char x_unit[8]; // x axis unit name, such as "ms".
    char y_unit[8];
	UInt16 array_length; // specify the number of entries in the array.
	UInt16 display_style; // 0: line; 1: dots only; 2: bars on X axis; 3: bars on Y axis
	UInt32 background_color; // 00RRGGBB
	UInt32 line1_color; // 00RRGGBB
	UInt32 line2_color; // 00RRGGBB
	UInt32 line3_color; // 00RRGGBB
	UInt32 line4_color; // 00RRGGBB
	UInt32 line5_color; // 00RRGGBB
	UInt8 x_value_visible; // determine whether to show x value for each point. boolean - 0:false 1:true
	UInt8 y1_value_visible; // determine whether to show y value for each point. boolean - 0:false 1:true
	UInt8 y2_value_visible; // determine whether to show y value for each point. boolean - 0:false 1:true
	UInt8 y3_value_visible; // determine whether to show y value for each point. boolean - 0:false 1:true
	UInt8 y4_value_visible; // determine whether to show y value for each point. boolean - 0:false 1:true
	UInt8 y5_value_visible; // determine whether to show y value for each point. boolean - 0:false 1:true
	UInt8 x_array_unit_size; // specify the number of bytes per array entry (1, 2, or 4 are supported, little-endian only; 0 means no x array in dereference, x array is derived from array_length, x_min, and x_max)
	UInt8 y1_array_unit_size; // specify the number of bytes per array entry (1, 2, or 4 are supported, little-endian only)
	UInt8 y2_array_unit_size; // specify the number of bytes per array entry (1, 2, or 4 are supported, little-endian only)
	UInt8 y3_array_unit_size; // specify the number of bytes per array entry (1, 2, or 4 are supported, little-endian only)
	UInt8 y4_array_unit_size; // specify the number of bytes per array entry (1, 2, or 4 are supported, little-endian only)
	UInt8 y5_array_unit_size; // specify the number of bytes per array entry (1, 2, or 4 are supported, little-endian only)
	Int32 x_min; // minimum value of x scale, if both x_min and x_max are zero, auto-scaling will be used.
	Int32 x_max; // maximum value of x scale, if both x_min and x_max are zero, auto-scaling will be used.
	Int32 y1_min; // minimum value of y scale, if both y_min and y_max are zero, auto-scaling will be used.
	Int32 y1_max; // maximum value of y scale, if both y_min and y_max are zero, auto-scaling will be used.
	Int32 y2_min; // minimum value of y scale, if both y_min and y_max are zero, auto-scaling will be used.
	Int32 y2_max; // maximum value of y scale, if both y_min and y_max are zero, auto-scaling will be used.
	Int32 y3_min; // minimum value of y scale, if both y_min and y_max are zero, auto-scaling will be used.
	Int32 y3_max; // maximum value of y scale, if both y_min and y_max are zero, auto-scaling will be used.
	Int32 y4_min; // minimum value of y scale, if both y_min and y_max are zero, auto-scaling will be used.
	Int32 y4_max; // maximum value of y scale, if both y_min and y_max are zero, auto-scaling will be used.
	Int32 y5_min; // minimum value of y scale, if both y_min and y_max are zero, auto-scaling will be used.
	Int32 y5_max; // maximum value of y scale, if both y_min and y_max are zero, auto-scaling will be used.
} T_CHART_X_5Y_HEADER;

typedef T_CHART_X_5Y_HEADER CHART_X_5Y;

typedef struct
{
    char title[32]; // chart name
    char x_axis_name[16]; // x axis name, such as "TimeLine".
    char y_axis_name[16];
    char x_unit[8]; // x axis unit name, such as "ms".
    char y_unit[8];
	UInt16 array_length; // specify the number of entries in the array.
	UInt16 display_style; // 0: line; 1: dots only; 2: bars on X axis; 3: bars on Y axis
	UInt32 background_color; // 00RRGGBB
	UInt32 line1_color; // 00RRGGBB
	UInt32 line2_color; // 00RRGGBB
	UInt32 line3_color; // 00RRGGBB
	UInt32 line4_color; // 00RRGGBB
	UInt32 line5_color; // 00RRGGBB
	UInt32 line6_color; // 00RRGGBB
	UInt8 x_value_visible; // determine whether to show x value for each point. boolean - 0:false 1:true
	UInt8 y1_value_visible; // determine whether to show y value for each point. boolean - 0:false 1:true
	UInt8 y2_value_visible; // determine whether to show y value for each point. boolean - 0:false 1:true
	UInt8 y3_value_visible; // determine whether to show y value for each point. boolean - 0:false 1:true
	UInt8 y4_value_visible; // determine whether to show y value for each point. boolean - 0:false 1:true
	UInt8 y5_value_visible; // determine whether to show y value for each point. boolean - 0:false 1:true
	UInt8 y6_value_visible; // determine whether to show y value for each point. boolean - 0:false 1:true
	UInt8 x_array_unit_size; // specify the number of bytes per array entry (1, 2, or 4 are supported, little-endian only; 0 means no x array in dereference, x array is derived from array_length, x_min, and x_max)
	UInt8 y1_array_unit_size; // specify the number of bytes per array entry (1, 2, or 4 are supported, little-endian only)
	UInt8 y2_array_unit_size; // specify the number of bytes per array entry (1, 2, or 4 are supported, little-endian only)
	UInt8 y3_array_unit_size; // specify the number of bytes per array entry (1, 2, or 4 are supported, little-endian only)
	UInt8 y4_array_unit_size; // specify the number of bytes per array entry (1, 2, or 4 are supported, little-endian only)
	UInt8 y5_array_unit_size; // specify the number of bytes per array entry (1, 2, or 4 are supported, little-endian only)
	UInt8 y6_array_unit_size; // specify the number of bytes per array entry (1, 2, or 4 are supported, little-endian only)
	Int32 x_min; // minimum value of x scale, if both x_min and x_max are zero, auto-scaling will be used.
	Int32 x_max; // maximum value of x scale, if both x_min and x_max are zero, auto-scaling will be used.
	Int32 y1_min; // minimum value of y scale, if both y_min and y_max are zero, auto-scaling will be used.
	Int32 y1_max; // maximum value of y scale, if both y_min and y_max are zero, auto-scaling will be used.
	Int32 y2_min; // minimum value of y scale, if both y_min and y_max are zero, auto-scaling will be used.
	Int32 y2_max; // maximum value of y scale, if both y_min and y_max are zero, auto-scaling will be used.
	Int32 y3_min; // minimum value of y scale, if both y_min and y_max are zero, auto-scaling will be used.
	Int32 y3_max; // maximum value of y scale, if both y_min and y_max are zero, auto-scaling will be used.
	Int32 y4_min; // minimum value of y scale, if both y_min and y_max are zero, auto-scaling will be used.
	Int32 y4_max; // maximum value of y scale, if both y_min and y_max are zero, auto-scaling will be used.
	Int32 y5_min; // minimum value of y scale, if both y_min and y_max are zero, auto-scaling will be used.
	Int32 y5_max; // maximum value of y scale, if both y_min and y_max are zero, auto-scaling will be used.
	Int32 y6_min; // minimum value of y scale, if both y_min and y_max are zero, auto-scaling will be used.
	Int32 y6_max; // maximum value of y scale, if both y_min and y_max are zero, auto-scaling will be used.
} T_CHART_X_6Y_HEADER;

typedef T_CHART_X_6Y_HEADER CHART_X_6Y;

typedef struct
{
    char title[32]; // chart name
    char x_axis_name[16]; // x axis name, such as "TimeLine".
    char y_axis_name[16];
    char x_unit[8]; // x axis unit name, such as "ms".
    char y_unit[8];
	UInt16 array_length; // specify the number of entries in the array.
	UInt16 display_style; // 0: line; 1: dots only; 2: bars on X axis; 3: bars on Y axis
	UInt32 background_color; // 00RRGGBB
	UInt32 line1_color; // 00RRGGBB
	UInt32 line2_color; // 00RRGGBB
	UInt32 line3_color; // 00RRGGBB
	UInt32 line4_color; // 00RRGGBB
	UInt32 line5_color; // 00RRGGBB
	UInt32 line6_color; // 00RRGGBB
	UInt32 line7_color; // 00RRGGBB
	UInt8 x_value_visible; // determine whether to show x value for each point. boolean - 0:false 1:true
	UInt8 y1_value_visible; // determine whether to show y value for each point. boolean - 0:false 1:true
	UInt8 y2_value_visible; // determine whether to show y value for each point. boolean - 0:false 1:true
	UInt8 y3_value_visible; // determine whether to show y value for each point. boolean - 0:false 1:true
	UInt8 y4_value_visible; // determine whether to show y value for each point. boolean - 0:false 1:true
	UInt8 y5_value_visible; // determine whether to show y value for each point. boolean - 0:false 1:true
	UInt8 y6_value_visible; // determine whether to show y value for each point. boolean - 0:false 1:true
	UInt8 y7_value_visible; // determine whether to show y value for each point. boolean - 0:false 1:true
	UInt8 x_array_unit_size; // specify the number of bytes per array entry (1, 2, or 4 are supported, little-endian only; 0 means no x array in dereference, x array is derived from array_length, x_min, and x_max)
	UInt8 y1_array_unit_size; // specify the number of bytes per array entry (1, 2, or 4 are supported, little-endian only)
	UInt8 y2_array_unit_size; // specify the number of bytes per array entry (1, 2, or 4 are supported, little-endian only)
	UInt8 y3_array_unit_size; // specify the number of bytes per array entry (1, 2, or 4 are supported, little-endian only)
	UInt8 y4_array_unit_size; // specify the number of bytes per array entry (1, 2, or 4 are supported, little-endian only)
	UInt8 y5_array_unit_size; // specify the number of bytes per array entry (1, 2, or 4 are supported, little-endian only)
	UInt8 y6_array_unit_size; // specify the number of bytes per array entry (1, 2, or 4 are supported, little-endian only)
	UInt8 y7_array_unit_size; // specify the number of bytes per array entry (1, 2, or 4 are supported, little-endian only)
	Int32 x_min; // minimum value of x scale, if both x_min and x_max are zero, auto-scaling will be used.
	Int32 x_max; // maximum value of x scale, if both x_min and x_max are zero, auto-scaling will be used.
	Int32 y1_min; // minimum value of y scale, if both y_min and y_max are zero, auto-scaling will be used.
	Int32 y1_max; // maximum value of y scale, if both y_min and y_max are zero, auto-scaling will be used.
	Int32 y2_min; // minimum value of y scale, if both y_min and y_max are zero, auto-scaling will be used.
	Int32 y2_max; // maximum value of y scale, if both y_min and y_max are zero, auto-scaling will be used.
	Int32 y3_min; // minimum value of y scale, if both y_min and y_max are zero, auto-scaling will be used.
	Int32 y3_max; // maximum value of y scale, if both y_min and y_max are zero, auto-scaling will be used.
	Int32 y4_min; // minimum value of y scale, if both y_min and y_max are zero, auto-scaling will be used.
	Int32 y4_max; // maximum value of y scale, if both y_min and y_max are zero, auto-scaling will be used.
	Int32 y5_min; // minimum value of y scale, if both y_min and y_max are zero, auto-scaling will be used.
	Int32 y5_max; // maximum value of y scale, if both y_min and y_max are zero, auto-scaling will be used.
	Int32 y6_min; // minimum value of y scale, if both y_min and y_max are zero, auto-scaling will be used.
	Int32 y6_max; // maximum value of y scale, if both y_min and y_max are zero, auto-scaling will be used.
	Int32 y7_min; // minimum value of y scale, if both y_min and y_max are zero, auto-scaling will be used.
	Int32 y7_max; // maximum value of y scale, if both y_min and y_max are zero, auto-scaling will be used.
} T_CHART_X_7Y_HEADER;

typedef T_CHART_X_7Y_HEADER CHART_X_7Y;

typedef struct
{
    char title[32]; // chart name
    char x_axis_name[16]; // x axis name, such as "TimeLine".
    char y_axis_name[16];
    char x_unit[8]; // x axis unit name, such as "ms".
    char y_unit[8];
	UInt16 array_length; // specify the number of entries in the array.
	UInt16 display_style; // 0: line; 1: dots only; 2: bars on X axis; 3: bars on Y axis
	UInt32 background_color; // 00RRGGBB
	UInt32 line1_color; // 00RRGGBB
	UInt32 line2_color; // 00RRGGBB
	UInt32 line3_color; // 00RRGGBB
	UInt32 line4_color; // 00RRGGBB
	UInt32 line5_color; // 00RRGGBB
	UInt32 line6_color; // 00RRGGBB
	UInt32 line7_color; // 00RRGGBB
	UInt32 line8_color; // 00RRGGBB
	UInt8 x_value_visible; // determine whether to show x value for each point. boolean - 0:false 1:true
	UInt8 y1_value_visible; // determine whether to show y value for each point. boolean - 0:false 1:true
	UInt8 y2_value_visible; // determine whether to show y value for each point. boolean - 0:false 1:true
	UInt8 y3_value_visible; // determine whether to show y value for each point. boolean - 0:false 1:true
	UInt8 y4_value_visible; // determine whether to show y value for each point. boolean - 0:false 1:true
	UInt8 y5_value_visible; // determine whether to show y value for each point. boolean - 0:false 1:true
	UInt8 y6_value_visible; // determine whether to show y value for each point. boolean - 0:false 1:true
	UInt8 y7_value_visible; // determine whether to show y value for each point. boolean - 0:false 1:true
	UInt8 y8_value_visible; // determine whether to show y value for each point. boolean - 0:false 1:true
	UInt8 x_array_unit_size; // specify the number of bytes per array entry (1, 2, or 4 are supported, little-endian only; 0 means no x array in dereference, x array is derived from array_length, x_min, and x_max)
	UInt8 y1_array_unit_size; // specify the number of bytes per array entry (1, 2, or 4 are supported, little-endian only)
	UInt8 y2_array_unit_size; // specify the number of bytes per array entry (1, 2, or 4 are supported, little-endian only)
	UInt8 y3_array_unit_size; // specify the number of bytes per array entry (1, 2, or 4 are supported, little-endian only)
	UInt8 y4_array_unit_size; // specify the number of bytes per array entry (1, 2, or 4 are supported, little-endian only)
	UInt8 y5_array_unit_size; // specify the number of bytes per array entry (1, 2, or 4 are supported, little-endian only)
	UInt8 y6_array_unit_size; // specify the number of bytes per array entry (1, 2, or 4 are supported, little-endian only)
	UInt8 y7_array_unit_size; // specify the number of bytes per array entry (1, 2, or 4 are supported, little-endian only)
	UInt8 y8_array_unit_size; // specify the number of bytes per array entry (1, 2, or 4 are supported, little-endian only)
	Int32 x_min; // minimum value of x scale, if both x_min and x_max are zero, auto-scaling will be used.
	Int32 x_max; // maximum value of x scale, if both x_min and x_max are zero, auto-scaling will be used.
	Int32 y1_min; // minimum value of y scale, if both y_min and y_max are zero, auto-scaling will be used.
	Int32 y1_max; // maximum value of y scale, if both y_min and y_max are zero, auto-scaling will be used.
	Int32 y2_min; // minimum value of y scale, if both y_min and y_max are zero, auto-scaling will be used.
	Int32 y2_max; // maximum value of y scale, if both y_min and y_max are zero, auto-scaling will be used.
	Int32 y3_min; // minimum value of y scale, if both y_min and y_max are zero, auto-scaling will be used.
	Int32 y3_max; // maximum value of y scale, if both y_min and y_max are zero, auto-scaling will be used.
	Int32 y4_min; // minimum value of y scale, if both y_min and y_max are zero, auto-scaling will be used.
	Int32 y4_max; // maximum value of y scale, if both y_min and y_max are zero, auto-scaling will be used.
	Int32 y5_min; // minimum value of y scale, if both y_min and y_max are zero, auto-scaling will be used.
	Int32 y5_max; // maximum value of y scale, if both y_min and y_max are zero, auto-scaling will be used.
	Int32 y6_min; // minimum value of y scale, if both y_min and y_max are zero, auto-scaling will be used.
	Int32 y6_max; // maximum value of y scale, if both y_min and y_max are zero, auto-scaling will be used.
	Int32 y7_min; // minimum value of y scale, if both y_min and y_max are zero, auto-scaling will be used.
	Int32 y7_max; // maximum value of y scale, if both y_min and y_max are zero, auto-scaling will be used.
	Int32 y8_min; // minimum value of y scale, if both y_min and y_max are zero, auto-scaling will be used.
	Int32 y8_max; // maximum value of y scale, if both y_min and y_max are zero, auto-scaling will be used.
} T_CHART_X_8Y_HEADER;

typedef T_CHART_X_8Y_HEADER CHART_X_8Y;

//#ifdef WIN32
//#else
typedef struct
{
	UInt8* byte_array;
	UInt32 size;
} log_link_list_t;
//#endif //WIN32

//***************************************************************************************
/**
    Function to enable /disable logging for specified modules
	@param		logID (in) Module log ID
	@param		isEnable (in) Boolean value to turn ON/OFF logging
	@return		Boolean  TRUE/FALSE indicating if the operation was successful
**/	

Boolean Log_EnableLogging(UInt16 logID, Boolean isEnable);

//***************************************************************************************
/**
    Function to register a callback function that is called by Log_EnableRange()
	@param		f (in) the callback function pointer
	@return		-1 registration failed; other = successful registration function order
**/	

typedef	void(*T_FUNC_LOG_CTRL_CALLBACK)(UInt16, UInt16, Boolean);

int Log_RegisterCallbackToLogControl(T_FUNC_LOG_CTRL_CALLBACK f);

//***************************************************************************************
/**
    Function to enable /disable logging for a specified range of modules
	@param		fromLogID (in) Starting Module log ID
	@param		toLogId (in) ending value of module range
	@param		isEnable (in) Boolean value to turn ON/OFF logging
	@return		Boolean  TRUE/FALSE indicating if the operation was successful
**/	

Boolean Log_EnableRange(UInt16 fromLogID, UInt16 toLogId, Boolean isEnable);

//***************************************************************************************
/**
    Function to enable /disable default logging
**/	

void Log_EnableDefaultRange(void);

//***************************************************************************************
/**
    Function to enable /disable logging for specified group of inter-task messages
	@param		msgID (in) inter-task message group ID (0-255)
	@param		isEnabled (in) Boolean value to turn ON/OFF logging of such inter-task message group
**/	

void Log_EnableTaskMsgLogging(UInt16 msgID, Boolean isEnabled);

//***************************************************************************************
/**
    Function to enable /disable logging for a specified range of inter-task message groups
	@param		fromMsgID (in) Starting inter-task message group ID
	@param		toMsgId (in) Ending inter-task message group ID
	@param		isEnabled (in) Boolean value to turn ON/OFF logging of the inter-task message groups
**/	

void Log_EnableTaskMsgRange(UInt16 fromMsgID, UInt16 toMsgId, Boolean isEnabled);

//***************************************************************************************
/**
    Function to enable /disable stack ASCII logging based on sub logging ID
	@param		subID (in) stack task IDs serve as sub logging IDs (0-255)
	@param		isEnabled (in) Boolean value to turn ON/OFF such sub logging ID
**/	

void Log_EnableStackLogging(UInt16 subID, Boolean isEnabled);

//***************************************************************************************
/**
    Function to enable /disable stack ASCII logging based on sub logging ID range
	@param		fromSubID (in) Starting stack task ID
	@param		toSubId (in) Ending stack task ID
	@param		isEnabled (in) Boolean value to turn ON/OFF such sub logging ID range
**/	

void Log_EnableStackRange(UInt16 fromSubID, UInt16 toSubId, Boolean isEnabled);

//***************************************************************************************
/**
    Function to enable a logging group by name
	@param		group_name (in) group name. if it ends with "_detail", this API enables
				both "group" and "group_detail"; otherwise, this API only enables "group".
**/	

void Log_EnableGroup(char *group_name);

//***************************************************************************************
/**
    Function to disable a logging group by name
	@param		group_name (in) group name. if it ends with "_detail", this API disables
				"group_detail" only; otherwise, this API disables both "group" and
				"group_detail".
**/	

void Log_DisableGroup(char *group_name);

//***************************************************************************************
/**
    Function to set logging to be limited (L3 only) or unlimited
	@param		isLimited (in) Boolean value to turn ON/OFF the limitation
	@return		Boolean  TRUE/FALSE indicating if the operation was successful
**/	

Boolean Log_LimitedLogging(Boolean isLimited);

//***************************************************************************************
/**
    Function to check if logging is enabled for a specified ID
	@param		logID (in) ID of the module to be checked
	@return		Boolean  TRUE if logging is enabled
**/	

Boolean Log_IsLoggingEnable(UInt16 logID);

//***************************************************************************************
/**
    Function to check if logging is limited
	@return		Boolean  TRUE if logging is limited
**/	

Boolean Log_IsLoggingLimited(void);

//***************************************************************************************
/**
    Function to check if logging is allowed
	@return		Boolean  TRUE if logging is allowed
**/	

Boolean Log_IsLoggingAllowed(void);

//***************************************************************************************
/**
    Function to check if logging of an inter-task message group is allowed
	@return		Boolean  TRUE if logging is allowed
**/	

Boolean  Log_IsTaskMsgLoggingEnabled(UInt16 msgID);

//***************************************************************************************
/**
    Function to check if logging of stack ASCII sub logging ID is allowed
	@return		Boolean  TRUE if logging is allowed
**/	

Boolean  Log_IsStackLoggingEnabled(UInt16 subID);

//***************************************************************************************
/**
    Function to log a specific string on a specific module
	@param		logID (in) ID of the module to be logged
	@param		*dbgString (in) Pointer to the debug string
	@note
	This function can be used when strings need to be traced
	during debugging operation. The debug string passed in will be buffered into a
	string of up to MAX_CLIENT_STRING_LEN bytes.
**/	

void	Log_DebugOutputString(UInt16 logID, char* dbgString);

//***************************************************************************************
/**
    Function to log a multi-line string in a multi-line format on a specific module
	@param		logID (in) ID of the module to be logged
	@param		*dbgString (in) Pointer to the debug string
	@note
	This function can be used when strings need to be traced
	during debugging operation. The debug string passed in will be buffered into a
	string. Up to MAX_MULTILINE_NUM lines will be logged.
**/	

void	Log_DebugOutputMultiLine(UInt16 logID, char* dbgString);

//***************************************************************************************
/**
    Function to log a specific string and an integer value on a specific module
	@param		logID (in) ID of the module to be logged
	@param		*dbgString (in) Pointer to the debug string
	@param		dbgValue (in) 32bit integer value to be logged
	@note
	This function can be used when strings and integral values need to be traced
	during debugging operation. The debug string passed in will be buffered into a
	string of up to MAX_CLIENT_STRING_LEN bytes.
**/	

void	Log_DebugOutputValue(UInt16 logID, char* dbgString, UInt32 dbgValue);

//***************************************************************************************
/**
    Function to log a specific string and two integer values on a specific module
	@param		logID (in) ID of the module to be logged
	@param		*dbgString (in) Pointer to the debug string
	@param		v1, v2 (in) 32bit integer values to be logged
	@note
	This function can be used when strings and integral values need to be traced
	during debugging operation. The debug string passed in will be buffered into a
	string of up to MAX_CLIENT_STRING_LEN bytes.
**/	

void	Log_DebugOutputValue2(UInt16 logID, char* dbgString, UInt32 v1, UInt32 v2);

//***************************************************************************************
/**
    Function to log a specific string and three integer values on a specific module
	@param		logID (in) ID of the module to be logged
	@param		*dbgString (in) Pointer to the debug string
	@param		v1, v2, v3 (in) 32bit integer values to be logged
	@note
	This function can be used when strings and integral values need to be traced
	during debugging operation. The debug string passed in will be buffered into a
	string of up to MAX_CLIENT_STRING_LEN bytes.
**/	

void	Log_DebugOutputValue3(UInt16 logID, char* dbgString, UInt32 v1, UInt32 v2, UInt32 v3);

//***************************************************************************************
/**
    Function to log a specific string and four integer values on a specific module
	@param		logID (in) ID of the module to be logged
	@param		*dbgString (in) Pointer to the debug string
	@param		v1, v2, v3, v4 (in) 32bit integer values to be logged
	@note
	This function can be used when strings and integral values need to be traced
	during debugging operation. The debug string passed in will be buffered into a
	string of up to MAX_CLIENT_STRING_LEN bytes.
**/	

void	Log_DebugOutputValue4(UInt16 logID, char* dbgString, UInt32 v1, UInt32 v2, UInt32 v3, UInt32 v4);

//***************************************************************************************
/**
    Function to log a specific string and five integer values on a specific module
	@param		logID (in) ID of the module to be logged
	@param		*dbgString (in) Pointer to the debug string
	@param		v1, v2, v3, v4, v5 (in) 32bit integer values to be logged
	@note
	This function can be used when strings and integral values need to be traced
	during debugging operation. The debug string passed in will be buffered into a
	string of up to MAX_CLIENT_STRING_LEN bytes.
**/	

void	Log_DebugOutputValue5(UInt16 logID, char* dbgString, UInt32 v1, UInt32 v2, UInt32 v3, UInt32 v4, UInt32 v5);

//***************************************************************************************
/**
    Function to log a specific string and six integer values on a specific module
	@param		logID (in) ID of the module to be logged
	@param		*dbgString (in) Pointer to the debug string
	@param		v1, v2, v3, v4, v5, v6 (in) 32bit integer values to be logged
	@note
	This function can be used when strings and integral values need to be traced
	during debugging operation. The debug string passed in will be buffered into a
	string of up to MAX_CLIENT_STRING_LEN bytes.
**/	

void	Log_DebugOutputValue6(UInt16 logID, char* dbgString, UInt32 v1, UInt32 v2, UInt32 v3, UInt32 v4, UInt32 v5, UInt32 v6);

//***************************************************************************************
/**
    Function to log a specific string and an integer value on a specific module
	@param		logID (in) ID of the module to be logged
	@param		*dbgString (in) Pointer to the debug string
	@param		v1, v2, v3, v4, v5, v6, v7 (in) 32bit integer values to be logged
	@note
	This function can be used when strings and integral values need to be traced
	during debugging operation. The debug string passed in will be buffered into a
	string of up to MAX_CLIENT_STRING_LEN bytes.
**/	

void	Log_DebugOutputValue7(UInt16 logID, char* dbgString, UInt32 v1, UInt32 v2, UInt32 v3, UInt32 v4, UInt32 v5, UInt32 v6, UInt32 v7);

//***************************************************************************************
/**
    Function to log a specific string and an integer value on a specific module
	@param		logID (in) ID of the module to be logged
	@param		*dbgString (in) Pointer to the debug string
	@param		array (in) integer array to be logged
	@param		unit_size (in) 1 = uint8, 2 = uint16, 4 = uint32
	@param		nof_units (in) array size, <= 255
	@note
	This function can be used when strings and integral values need to be traced
	during debugging operation. The debug string passed in will be buffered into a
	string of up to MAX_CLIENT_STRING_LEN bytes.
**/	

void	Log_DebugOutputArray(UInt16 logID, char* dbgString, void* array, UInt32 unit_size, UInt32 nof_units);

//***************************************************************************************
/**
    Function to log a specific string as does printf()
	@param		logID (in) ID of the module to be logged
	@param		*fmt (in) a format string as printf()
	...			(in) a variable number of arguments as printf()
	@note
	This "printed" string will be buffered into a string of up to MAX_CLIENT_STRING_LEN bytes.
**/	

int	Log_DebugPrintf(UInt16 logID, char* fmt, ...);

//***************************************************************************************
/**
    Function to log a binary logging message
	@param		log_id (in) logic id controlling whether this binary logging is generated
	@param		sig_code (in) binary logging code
	@param		*ptr (in) starting address of the binary logging content
	@param		*ptr_size (in) size of the binary logging content, in number of bytes
	@note
**/	

void Log_BinaryLogging(UInt16 log_id, UInt32 sig_code, void *ptr, UInt32 ptr_size);
void Log_BinaryLoggingUncompressed(UInt16 log_id, UInt32 sig_code, void *ptr, UInt32 ptr_size);

//***************************************************************************************
/**
    Function to log a binary logging message with a pointer dereference
	@param		log_id (in) logic id controlling whether this binary logging is generated
	@param		sig_code (in) binary logging code
	@param		*ptr (in) starting address of the binary logging content
	@param		*ptr_size (in) size of the binary logging content, in number of bytes
	@param		*ptr2 (in) starting address of the pointer dereference content
	@param		*ptr2_size (in) size of the pointer dereference content, in number of bytes
	@note
**/	

void Log_BinaryLoggingWithDeref(UInt16 log_id, UInt32 sig_code, void *ptr, UInt32 ptr_size, void *ptr2, UInt32 ptr2_size);
void Log_BinaryLoggingUncompressedWithDeref(UInt16 log_id, UInt32 sig_code, void *ptr, UInt32 ptr_size, void *ptr2, UInt32 ptr2_size);

//***************************************************************************************
/**
    Function to log a binary logging message described by a link list
	@param		log_id (in) logic id controlling whether this binary logging is generated
	@param		sig_code (in) binary logging code
	@param		*link_list (in) starting address of a link list
	@param		list_size_size (in) size of the link list, in number of list items
	@note
	This function is provided to assist Log_BinaryLogging() in case the binary logging is
	assembled from different memories, where a link list can avoid unnecessary memcpy.
**/	

void Log_BinaryBlocks(UInt16 log_id, UInt32 sig_code, log_link_list_t* link_list, UInt32 list_size_size);
void Log_BinaryBlocksUncompressed(UInt16 log_id, UInt32 sig_code, log_link_list_t* link_list, UInt32 list_size_size);

//***************************************************************************************
/**
    Function to log a binary logging message described by a link list with a dereference link list
	@param		log_id (in) logic id controlling whether this binary logging is generated
	@param		sig_code (in) binary logging code
	@param		*link_list (in) starting address of a link list
	@param		list_size (in) size of the link list, in number of list items
	@param		*link_list2 (in) starting address of a dereference link list
	@param		list2_size (in) size of the dereference link list, in number of list items
	@note
	This function is provided to assist Log_BinaryLogging() in case the binary logging is
	assembled from different memories, where a link list can avoid unnecessary memcpy.
**/	

void Log_BinaryBlocksWithDerefBlocks(UInt16 log_id, UInt32 sig_code, log_link_list_t* link_list, UInt32 list_size, log_link_list_t *link_list2, UInt32 list2_size);
void Log_BinaryBlocksUncompressedWithDerefBlocks(UInt16 log_id, UInt32 sig_code, log_link_list_t* link_list, UInt32 list_size, log_link_list_t *link_list2, UInt32 list2_size);

//***************************************************************************************
/**
    Function to log a binary signal
	@param		sig_code (in) binary signal code, the high 16 bits are logging ID/receiver
	@param		*ptr (in) starting address of the binary signal content
	@param		*ptr_size (in) size of the binary signal content, in number of bytes
	@param		state (in) receiving task's state information (optional)
	@param		sender (in) ID of sending task
	@note
	If a customer wants to use this API to log their own binary signals, the value of high
	16 bits (i.e., logging ID for the binary signal) should be within the range of 192-223.
	In addition, the customer must provide a formatted text file (see example below) to
	define their binary signals. Such signal definitions should keep good backward
	compatibility. The recommendations are:\n
	-# if a new signal for a receiving task is added, it should be appended to the end of
	the Signals list of the receiving task; 
	-# never delete a signal code, even obsolete; 
	-# if a new receiving task is added, it should be appended to the end of the Process 
	list; and 
	-# never delete a receiving task, even obsolete.

	\verbatim
	Process
		192 receiving_task1_name
		States
			1	task1_state1_name
			2	task1_state2_name
		End States
		Signals
			1	task1_signal1_name
			2	task1_signal2_name
		End Signals
	End Process
	Process
		193 receiving_task2_name
		States
			1	task2_state1_name
			2	task2_state2_name
		End States
		Signals
			1	task2_signal1_name
			2	task2_signal2_name
		End Signals
	End Process
	\endverbatim
**/	

void Log_DebugSignal(UInt32 sig_code, void *ptr, UInt32 ptr_size, UInt16 state, UInt16 sender);
void Log_DebugSignalUncompressed(UInt32 sig_code, void *ptr, UInt32 ptr_size, UInt16 state, UInt16 sender);

//***************************************************************************************
/**
    Function to log a binary signal described by a link list
	@param		sig_code (in) binary signal code, the high 16 bits are logging ID/receiver
	@param		*link_list (in) starting address of a link list
	@param		list_size_size (in) size of the link list, in number of list items
	@param		state (in) receiving task's state information (optional)
	@param		sender (in) ID of sending task
	@note
	This function is provided to assist Log_DebugSignal() in case the binary signal is
	assembled from different memories, where a link list can avoid unnecessary memory.
**/	

void Log_DebugLinkList(UInt32 sig_code, log_link_list_t* link_list, UInt32 list_size_size, UInt16 state, UInt16 sender);
void Log_DebugLinkListUncompressed(UInt32 sig_code, log_link_list_t* link_list, UInt32 list_size_size, UInt16 state, UInt16 sender);

//***************************************************************************************

/**
	Definitions of the size of the log value to be logged
**/	
#define		LLG_LOG_TAG_TYPE_1BIT_VALUE			1
#define		LLG_LOG_TAG_TYPE_8BIT_VALUE			2
#define		LLG_LOG_TAG_TYPE_16BIT_VALUE		4
#define		LLG_LOG_TAG_TYPE_32BIT_VALUE		8

#define		LLG_LOG_TAG_TYPE_EXT_1BIT_VALUE		0x31
#define		LLG_LOG_TAG_TYPE_EXT_8BIT_VALUE		0x32
#define		LLG_LOG_TAG_TYPE_EXT_16BIT_VALUE	0x34
#define		LLG_LOG_TAG_TYPE_EXT_32BIT_VALUE	0x38
#define		LLG_LOG_TAG_TYPE_EXT_8BIT_ARRAY		0x33
#define		LLG_LOG_TAG_TYPE_EXT_16BIT_ARRAY	0x35
#define		LLG_LOG_TAG_TYPE_EXT_32BIT_ARRAY	0x39
#define		LLG_LOG_TAG_TYPE_EXT_ASCIIZ			0x37

#define		LLG_LOG_TAG_TIMESTAMP				0	// reserved tag ID for 32bit time stamp as first LLG log in a grouped binary logging

/**
    Function to log a low-level logging message in a binary signal
	@note
	This group of functions are provided to assist Log_DebugSignal() in case it is needed
	to group many low-level, simple "Tag = Value/Array" logging messages inside a binary
	signal in order to reduce MTT framing overhead.

	In the group, a "Tag=Value/Array" logging message is packed as one of the following formats
@code 
		TagType(4 bit) + TagID(11-12 bit) + Value(size determined by TagType)
		TagType(4 bit) + TagTypeExt(4 bit) + TagID(15-16 bit) + Value/Array(size determined by TagTypeExt)
@endcode

	In details, the following TagType and TagTypeExt are supported
@code 
		0001 + TagID(11 bit) + 1-bit Value							// carried out by Log_Grouped1bitValue()
		0010 + TagID(12 bit) + 8-bit Value							// carried out by Log_Grouped8bitValue()
		0100 + TagID(12 bit) + 16-bit Value							// carried out by Log_Grouped16bitValue()
		1000 + TagID(12 bit) + 32-bit Value							// carried out by Log_Grouped32bitValue()
		0011 + 0001 + TagID(15 bit) + 1-bit Value					// carried out by Log_Grouped1bitValue()
		0011 + 0010 + TagID(16 bit) + 8-bit Value					// carried out by Log_Grouped8bitValue()
		0011 + 0100 + TagID(16 bit) + 16-bit Value					// carried out by Log_Grouped16bitValue()
		0011 + 1000 + TagID(16 bit) + 32-bit Value					// carried out by Log_Grouped32bitValue()
		0011 + 0011 + TagID(16 bit) + Length(8 bit) + 8-bit Array	// carried out by Log_Grouped8bitArray()
		0011 + 0101 + TagID(16 bit) + Length(8 bit) + 16-bit Array	// carried out by Log_Grouped16bitArray()
		0011 + 1001 + TagID(16 bit) + Length(8 bit) + 32-bit Array	// carried out by Log_Grouped32bitArray()
		0011 + 0111 + TagID(16 bit) + ASCIIZ string					// carried out by Log_GroupedString()
@endcode

	The accumulated logging messages are sent to logging circular buffer using Log_BinaryLogging() when any of
	the following conditions is met<BR>
	(1)	The accumulated size of "Tag=Value" logging messages exceeds 1kB<BR>
	(2) The time threshold is no longer met (default 0, meaning time stamps must be identical, i.e., within 1ms)<BR>
	(3) By Log_FlushGroup().<BR>
**/	

void Log_Grouped1bitValue	(UInt16 log_id, UInt32 sig_code, UInt32 tag_id, UInt8 value);
void Log_Grouped8bitValue	(UInt16 log_id, UInt32 sig_code, UInt32 tag_id, UInt8 value);
void Log_Grouped16bitValue	(UInt16 log_id, UInt32 sig_code, UInt32 tag_id, UInt16 value);
void Log_Grouped32bitValue	(UInt16 log_id, UInt32 sig_code, UInt32 tag_id, UInt32 value);
void Log_Grouped8bitArray	(UInt16 log_id, UInt32 sig_code, UInt32 tag_id, UInt8* array, UInt8 size);
void Log_Grouped16bitArray	(UInt16 log_id, UInt32 sig_code, UInt32 tag_id, UInt16* array, UInt8 size);
void Log_Grouped32bitArray	(UInt16 log_id, UInt32 sig_code, UInt32 tag_id, UInt32* array, UInt8 size);
void Log_GroupedString		(UInt16 log_id, UInt32 sig_code, UInt32 tag_id, char* asciiz);

void Log_FlushGroup(UInt32 sig_code);
void Log_SetGroupTimeThreshold(UInt32 sig_code, UInt32 time_threshold);
UInt32 Log_GetGroupTimeThreshold(UInt32 sig_code);
void Log_SetGroupSizeThreshold(UInt32 sig_code, UInt32 size_threshold);
UInt32 Log_GetGroupSizeThreshold(UInt32 sig_code);


//***************************************************************************************
/**
    Function to register/deregister a logging buffer so that LOG task can push it to
	a logging port as a binary logging message using a specified sig_code
	@param		p (in) logging buffer of T_REGISTERED_LOG_BUFFER type.

	These logging APIs can be used for ARM to support DSP logging. When DSP is initialized,
	Log_RegisterLogBuffer(dsp_logging_buffer) should be called to register dsp_logging_buffer
	in shared memory that is accessible by ARM and DSP. DSP can elect either accumulates
	low-level grouped DSP logging in this buffer directly (logging could be slow due to
	access to shared memory) or DMA accumulated DSP logging from DSP internal memory
	(logging can be much fast) to this buffer (if DMA is used, "ready" field in the header
	structure should be set to 1 after DMA finishes).

	When this buffer is not being used, or when it is not ready to be sent to a logging
	port, "ready" should be zero. When it is ready to be sent to a logging port, "ready" is
	set to 1, and LOG task running on ARM will be responsible for sending it to a logging
	port as a binary logging message using "sig_code" specified in the header strcuture.
	After it is sent out, "ready" is set to zero again for DSP to re-use this buffer.
**/	

#define	LOG_BUFFER_SIGNATURE	0x4642474C	//"LGBF" in little endian

typedef struct
{
	unsigned int signature;	// must be LOG_BUFFER_SIGNATURE
	unsigned int ready;
	void *ptr;
	unsigned int size;
	unsigned int sig_code;
} T_REGISTERED_LOG_BUFFER;

int Log_RegisterLogBuffer(T_REGISTERED_LOG_BUFFER *p);
int Log_DeregisterLogBuffer(T_REGISTERED_LOG_BUFFER *p);


//***************************************************************************************
/**
    Function to select specified logging format
	@param		logFormat (in) Format can be ASCII, MOBILE ANALYZER, or BINARY
**/	

void Log_SelectLoggingFormat(UInt16 logFormat);

//***************************************************************************************
/**
    Function to return the current logging format
	@return		Format can be ASCII, MOBILE ANALYZER, or BINARY
**/	

UInt16	Log_GetLoggingFormat(void);

//***************************************************************************************
/**
    Function to get the text name of a log ID
	@param		logID (in) ID
	@return		text string of the name
**/	

const char* Log_GetLogIdName(UInt16 logID);

//***************************************************************************************
/**
    Function to output logging statistics in log file
**/	

void Log_OutputStatistics(void);


#ifdef CNEON_COMMON
const char* Log_GetLogIdName(UInt16 logID);

//***************************************************************************************
/**
    Function to output logging statistics in log file
**/	

void Log_OutputStatistics(void);
#endif
/** @} */

#endif


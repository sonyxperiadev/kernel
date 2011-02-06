/*******************************************************************************************
Copyright 2010 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement 
governing use of this software, this software is licensed to you under the 
terms of the GNU General Public License version 2, available at 
http://www.gnu.org/copyleft/gpl.html (the "GPL"). 

Notwithstanding the above, under no circumstances may you combine this software 
in any way with any other Broadcom software provided under a license other than 
the GPL, without Broadcom's express prior written consent.
*******************************************************************************************/

#ifndef _INC_CONSTS_H_
#define _INC_CONSTS_H_

//#include "types.h"

#ifndef TRUE
#define TRUE			( (Boolean)1 )
#endif

#ifndef FALSE
#define FALSE			( (Boolean)0 )
#endif

#define NOT_AVAIL		0xff

#ifdef NO_LA_TESTPOINT
#define _TP_OUTPUT(x, y) {}
#else

#define LA_RTOS			0x00000001
#define	LA_PHY			0x00000002
#define LA_MP			0x00000004
#define LA_DATA			0x00000008
#define LA_ECDC			0x00000010
#define LA_STACK		0x00000020
#define LA_GENERAL		0x00000080
#define LA_IRAT			0x00000040

extern	UInt32			isTestPointEnabled;
#define _TP_OUTPUT(mask, x)	if(isTestPointEnabled & mask) { x ; }
#endif

#ifndef WIN32

#define TP_BASE_ADDR	0x04000000

#define SPARE_TP		(*(volatile UInt16 *)(TP_BASE_ADDR+0xfffff0))
#define TP2				(*(volatile UInt16 *)(TP_BASE_ADDR+0xfffff2))
#define TP4				(*(volatile UInt16 *)(TP_BASE_ADDR+0xfffff4))
#define TP6				(*(volatile UInt16 *)(TP_BASE_ADDR+0xfffff6))
#define TP8				(*(volatile UInt16 *)(TP_BASE_ADDR+0xfffff8))
#define TPa				(*(volatile UInt16 *)(TP_BASE_ADDR+0xfffffa))
#define TPc				(*(volatile UInt16 *)(TP_BASE_ADDR+0xfffffc))
#define TPe				(*(volatile UInt16 *)(TP_BASE_ADDR+0xfffffe))
#define SPEED_TP		(*(volatile UInt16 *)(TP_BASE_ADDR+0xffff8e))
#define OAKMSG_TP		(*(volatile UInt16 *)(TP_BASE_ADDR+0xffff8c))
#define FRAME_INT_TP	(*(volatile UInt16 *)(TP_BASE_ADDR+0xffff8a))
#define DATA_TP			(*(volatile UInt16 *)(TP_BASE_ADDR+0xffff88))
#define TPP0 			(*(volatile UInt32 *)(TP_BASE_ADDR+0xffffe0))
#define TPP4 			(*(volatile UInt32 *)(TP_BASE_ADDR+0xffffe4))
#define TPP8 			(*(volatile UInt32 *)(TP_BASE_ADDR+0xffffe8))
#define TPPc 			(*(volatile UInt32 *)(TP_BASE_ADDR+0xffffec))

#define FRAME_TP		(*(volatile UInt16 *)(TP_BASE_ADDR+0xffff50)) // to 0x4ffff70
#define FRAME_INDEX_TP	(*(volatile UInt16 *)(TP_BASE_ADDR+0xffffb0))
#define CMD_TP			(*(volatile UInt16 *)(TP_BASE_ADDR+0xffffc0))

#define TX_DATA_INDEX_TP (*(volatile UInt16 *)(TP_BASE_ADDR+0xfff700))
#define TX_DATA_MODE_TP	(*(volatile UInt16 *)(TP_BASE_ADDR+0xfff702))
#define TX_DATA_TP		(*(volatile UInt16 *)(TP_BASE_ADDR+0xfff000))
#define TX_FACCH_TP		(*(volatile UInt16 *)(TP_BASE_ADDR+0xfff100))
#define TX_SACCH_TP		(*(volatile UInt16 *)(TP_BASE_ADDR+0xfff200))

#define RX_DATA_INDEX_TP (*(volatile UInt16 *)(TP_BASE_ADDR+0xfff708))
#define RX_DATA_TP		(*(volatile UInt16 *)(TP_BASE_ADDR+0xfff080))
#define RX_FACCH_TP		(*(volatile UInt16 *)(TP_BASE_ADDR+0xfff180))
#define RX_SACCH_TP		(*(volatile UInt16 *)(TP_BASE_ADDR+0xfff280))
#define RX_BFI			(*(volatile UInt16 *)(TP_BASE_ADDR+0xfff500))

#define TBF_IF_MON0		(*(volatile UInt16 *)(TP_BASE_ADDR+0xfff600))
#define TBF_IF_MON1		(*(volatile UInt16 *)(TP_BASE_ADDR+0xfff602))
#define TBF_IF_MON2		(*(volatile UInt16 *)(TP_BASE_ADDR+0xfff604))
#define TBF_IF_MON3		(*(volatile UInt16 *)(TP_BASE_ADDR+0xfff606))


// High layer related test points
#define DTT_ACTION				(*(volatile UInt16 *)(TP_BASE_ADDR+0xffff00))
#define DTT_WRITE_FRAMES 		(*(volatile UInt16 *)(TP_BASE_ADDR+0xffff02))
#define DTT_READ_FRAMES  		(*(volatile UInt16 *)(TP_BASE_ADDR+0xffff04))
#define DTA_READ_INDEX   		(*(volatile UInt16 *)(TP_BASE_ADDR+0xffff06))
#define DTA_WRITE_INDEX   		(*(volatile UInt16 *)(TP_BASE_ADDR+0xffff08))
#define L1DS_WRITE_INDEX   		(*(volatile UInt16 *)(TP_BASE_ADDR+0xffff0a))
#define L1DS_READ_INDEX   		(*(volatile UInt16 *)(TP_BASE_ADDR+0xffff0c))
#define DTT_READ_SUBNUM			(*(volatile UInt16 *)(TP_BASE_ADDR+0xffff0e))

#define TP_DTN_ACTION			(*(volatile UInt16 *)(TP_BASE_ADDR+0xffff10))

#define TP_L1S_ACTION			(*(volatile UInt16 *)(TP_BASE_ADDR+0xffff12))

#define TP_RRM_ACTION			(*(volatile UInt16 *)(TP_BASE_ADDR+0xffff14))

#define TP_DLL_ACTION			(*(volatile UInt16 *)(TP_BASE_ADDR+0xffff16))

#define DTT_READ_BYTCNT			(*(volatile UInt16 *)(TP_BASE_ADDR+0xffff18))
#define DTT_READ_L1S2INT		(*(volatile UInt16 *)(TP_BASE_ADDR+0xffff1a))
#define DTT_READ_INT2DTA		(*(volatile UInt16 *)(TP_BASE_ADDR+0xffff1c))
#define DTT_READ_INT2DTA2		(*(volatile UInt16 *)(TP_BASE_ADDR+0xffff1e))

#define TP_DRL_ACTION			(*(volatile UInt16 *)(TP_BASE_ADDR+0xffff20))

#define TP_DCH_ACTION			(*(volatile UInt16 *)(TP_BASE_ADDR+0xffff30))

#define ATC_ACTION				(*(volatile UInt16 *)(TP_BASE_ADDR+0xffff40))

#define TP_HEAP_ALLOC_SIZE		(*(volatile UInt16 *)(TP_BASE_ADDR+0x000000))

#define TP_ASSERT				(*(volatile UInt16 *)(TP_BASE_ADDR+0x888888))

#define TP_OAK_RXQUAL_FULL		(*(volatile UInt16 *)(TP_BASE_ADDR+0x100000))
#define TP_OAK_RXQUAL_SUB		(*(volatile UInt16 *)(TP_BASE_ADDR+0x100002))
#define TP_OAK_RXLEV_FULL		(*(volatile UInt16 *)(TP_BASE_ADDR+0x100004))
#define TP_OAK_RXLEV_SUB		(*(volatile UInt16 *)(TP_BASE_ADDR+0x100006))

#define TP_READ_BUFF			(*(volatile UInt16 *)(TP_BASE_ADDR+0x1ff000))
#define TP_RX_BUFF_BFI			(*(volatile UInt16 *)(TP_BASE_ADDR+0x1ff040))
#define TP_RX_BUFF_CODE			(*(volatile UInt16 *)(TP_BASE_ADDR+0x1ff042))
#define TP_RX_BUFF_OTD			(*(volatile UInt16 *)(TP_BASE_ADDR+0x1ff044))
#define TP_RX_BUFF_INDEX		(*(volatile UInt16 *)(TP_BASE_ADDR+0x1ff046))
#define TP_RX_BUFF_ADJ			(*(volatile UInt16 *)(TP_BASE_ADDR+0x1ff048))

#define TP_WRITE_BUFF			(*(volatile UInt16 *)(TP_BASE_ADDR+0x1ff080))
#define TP_TX_BUFF_INDEX		(*(volatile UInt16 *)(TP_BASE_ADDR+0x1ff0c0))
#define TP_TX_BUFF_CODE			(*(volatile UInt16 *)(TP_BASE_ADDR+0x1ff0c2))

#define TP_MAC_GET_BLOCK_TX_SLOT (*(volatile UInt16 *)(TP_BASE_ADDR+0x1ff050))
#define TP_MAC_GET_BLOCK_FN 	(*(volatile UInt32 *)(TP_BASE_ADDR+0x1ff054))
#define TP_MAC_GET_BLOCK_TBF_ID (*(volatile UInt16 *)(TP_BASE_ADDR+0x1ff058))

#define TP_MAC_ACK_NU_SENT 		(*(volatile UInt16 *)(TP_BASE_ADDR+0x1ff05a))
#define TP_MAC_ACK_FN		 	(*(volatile UInt32 *)(TP_BASE_ADDR+0x1ff05c))

#define TP_START_PLOT_FRAME		(*(volatile UInt16 *)(TP_BASE_ADDR+0x1ff060))

#define TP_FRAME_STATE			(*(volatile UInt16 *)(TP_BASE_ADDR+0x110000))
#define TP_SWEEP_SIZE			(*(volatile UInt16 *)(TP_BASE_ADDR+0x120000))
#define TP_SWEEP_CNT			(*(volatile UInt16 *)(TP_BASE_ADDR+0x120002))
#define TP_L1_RX_MSG            (*(volatile UInt16 *)(TP_BASE_ADDR+0x130000))

#define TP_CELLDATA_ID			(*(volatile UInt16 *)(TP_BASE_ADDR+0x200000))
#define TP_CELLDATA_VALID		(*(volatile UInt16 *)(TP_BASE_ADDR+0x200002))
#define TP_CELLDATA_SYNC		(*(volatile UInt16 *)(TP_BASE_ADDR+0x200004))
#define TP_CELLDATA_ARFCN		(*(volatile UInt16 *)(TP_BASE_ADDR+0x200006))
#define TP_CELLDATA_RXLEV		(*(volatile UInt16 *)(TP_BASE_ADDR+0x200008))

#define TP_SORT_INDEX			(*(volatile UInt16 *)(TP_BASE_ADDR+0x300000))
#define TP_SORT_ARFCN			(*(volatile UInt16 *)(TP_BASE_ADDR+0x300002))
#define TP_SORT_RXLEV			(*(volatile UInt16 *)(TP_BASE_ADDR+0x300004))

#define TP_MON_ARFCN			(*(volatile UInt16 *)(TP_BASE_ADDR+0x400000))
#define TP_MON_RXLEV			(*(volatile UInt16 *)(TP_BASE_ADDR+0x400002))

#define TP_RX_SLOTS_PATT		(*(volatile UInt16 *)(TP_BASE_ADDR+0x500000))
#define TP_TX_SLOTS_PATT		(*(volatile UInt16 *)(TP_BASE_ADDR+0x500002))
#define TP_FIRST_SLOT			(*(volatile UInt16 *)(TP_BASE_ADDR+0x500004))
#define TP_TCH_MODE 			(*(volatile UInt16 *)(TP_BASE_ADDR+0x500006))
#define TP_CHAN_CFG_IS_HSCSD	(*(volatile UInt16 *)(TP_BASE_ADDR+0x500008))

// Test Points for ECDC 0x4600000..0x4600fff
#define TP_ECDC_ENTER			(*(volatile UInt16 *)(TP_BASE_ADDR+0x600000))
#define TP_ECDC_MSR_EVT			(*(volatile UInt16 *)(TP_BASE_ADDR+0x600002))
#define TP_ECDC_MSR_RD			(*(volatile UInt16 *)(TP_BASE_ADDR+0x600004))
#define TP_ECDC_RX_EVT			(*(volatile UInt16 *)(TP_BASE_ADDR+0x600006))
#define TP_ECDC_RPT_DTR			(*(volatile UInt16 *)(TP_BASE_ADDR+0x600008))
#define TP_ECDC_FC				(*(volatile UInt16 *)(TP_BASE_ADDR+0x60000a))
#define TP_ECDC_RTS				(*(volatile UInt16 *)(TP_BASE_ADDR+0x60000c))
#define TP_ECDC_DTR				(*(volatile UInt16 *)(TP_BASE_ADDR+0x60000e))
#define TP_ECDC_T1_ENTER		(*(volatile UInt16 *)(TP_BASE_ADDR+0x600010))
#define TP_ECDC_T1_EXIT			(*(volatile UInt16 *)(TP_BASE_ADDR+0x600012))
#define TP_ECDC_MPX_RPT_EVT		(*(volatile UInt16 *)(TP_BASE_ADDR+0x600014))
#define TP_ECDC_V24_RPT_LSI		(*(volatile UInt16 *)(TP_BASE_ADDR+0x600016))
#define TP_ECDC_V24_EVT_PRC		(*(volatile UInt16 *)(TP_BASE_ADDR+0x600018))

#define TP_ECDC_DROPPED			(*(volatile UInt32 *)(TP_BASE_ADDR+0x60001c))
#define TP_ECDC_SENT			(*(volatile UInt32 *)(TP_BASE_ADDR+0x600020))

#define TP_ECDC_V24_UPD_DCE_FC	(*(volatile UInt16 *)(TP_BASE_ADDR+0x600030))
#define TP_ECDC_V24_RPT_REM_FC	(*(volatile UInt16 *)(TP_BASE_ADDR+0x600032))
#define TP_ECDC_UPD_DCE_FC		(*(volatile UInt16 *)(TP_BASE_ADDR+0x600034))
#define TP_ECDC_RPT_REM_FC		(*(volatile UInt16 *)(TP_BASE_ADDR+0x600036))
#define TP_ECDC_UPD_DCE_SZ		(*(volatile UInt16 *)(TP_BASE_ADDR+0x600038))

#define TP_ECDC_BYTES_READ		(*(volatile UInt32 *)(TP_BASE_ADDR+0x600100))
#define TP_ECDC_DTE_RD_DATA		(*(volatile UInt8  *)(TP_BASE_ADDR+0x600104))
#define TP_ECDC_BYTES_WRITTEN	(*(volatile UInt32 *)(TP_BASE_ADDR+0x600180))
#define TP_ECDC_DTE_WR_DATA		(*(volatile UInt8  *)(TP_BASE_ADDR+0x600184))

#define TP_ECDC_TX_DATA			( (volatile UInt16 *)(TP_BASE_ADDR+0x600300))	// Data sent to Stack (up to 256 bytes per read)
#define TP_ECDC_RX_DATA			( (volatile UInt16 *)(TP_BASE_ADDR+0x600400))	// Data received from Stack (up to 256 bytes per write)

#define TP_ECDC_EXIT			(*(volatile UInt16 *)(TP_BASE_ADDR+0x600ffe))

#define TP_SIO_RX_CHAR			(*(volatile UInt8  *)(TP_BASE_ADDR+0x601000))
#define TP_SIO_TX_CHAR			(*(volatile UInt8  *)(TP_BASE_ADDR+0x601001))
#define TP_SIO_BRK_CK0			(*(volatile UInt16 *)(TP_BASE_ADDR+0x601010))
#define TP_SIO_BRK_CK1			(*(volatile UInt16 *)(TP_BASE_ADDR+0x601012))
#define TP_SIO_BRK_CK2			(*(volatile UInt16 *)(TP_BASE_ADDR+0x601014))
#define TP_SIO_BRK_CK3			(*(volatile UInt16 *)(TP_BASE_ADDR+0x601016))
#define TP_SIO_BRK_CK4			(*(volatile UInt16 *)(TP_BASE_ADDR+0x601018))
#define TP_SIO_BRK_CK5			(*(volatile UInt16 *)(TP_BASE_ADDR+0x60101a))
#define TP_SIO_BRK_CK6			(*(volatile UInt16 *)(TP_BASE_ADDR+0x60101c))
#define TP_SIO_BRK_CK7			(*(volatile UInt16 *)(TP_BASE_ADDR+0x60101e))
#define TP_SIO_MSR_MON			(*(volatile UInt16 *)(TP_BASE_ADDR+0x601020))
#define TP_SIO_IS_TXEN			(*(volatile UInt16 *)(TP_BASE_ADDR+0x601022))

#define TP_TX_96_SLOT_NO		(*(volatile UInt16 *)(TP_BASE_ADDR+0x602000))
#define TP_TX_96_NT_FRAME		(*(volatile UInt16 *)(TP_BASE_ADDR+0x602002))
#define TP_RX_96_SLOT_NO		(*(volatile UInt16 *)(TP_BASE_ADDR+0x602080))
#define TP_RX_96_NT_FRAME		(*(volatile UInt16 *)(TP_BASE_ADDR+0x602082))

#define TP_TX_144_SLOT_NO		(*(volatile UInt16 *)(TP_BASE_ADDR+0x602100))
#define TP_TX_144_NT_FRAME		(*(volatile UInt16 *)(TP_BASE_ADDR+0x602102))
#define TP_RX_144_SLOT_NO		(*(volatile UInt16 *)(TP_BASE_ADDR+0x602180))
#define TP_RX_144_NT_FRAME		(*(volatile UInt16 *)(TP_BASE_ADDR+0x602182))

#define TP_SERIAL_TX_SIZE		(*(volatile UInt16 *)(TP_BASE_ADDR+0x680000))

#define TP_FLW_SIO_RX_CNT		(*(volatile UInt32 *)(TP_BASE_ADDR+0x688800))
#define TP_FLW_SIO_TX_CNT		(*(volatile UInt32 *)(TP_BASE_ADDR+0x688880))

#define TP_FLW_MPXRX_RX_CNT		(*(volatile UInt32 *)(TP_BASE_ADDR+0x688804))
#define TP_FLW_MPX_TX_CNT		(*(volatile UInt32 *)(TP_BASE_ADDR+0x688884))

//
// Not really a test point, but rather, a placeholder for test point in
// __16__rt_stkovf_split_big in rt_16.s so it doesn't get used unwittingly.
// Rt_16.s reads memmap.inc and not this file so there's no common include
// file.  Having this here should at least raise a flag...
//
#define	TP_STACK_OVERFLOW		(*(volatile UInt32 *)(TP_BASE_ADDR+0x6ffffc))

#define TP_RXPM_SIO				(*(volatile UInt16 *)(TP_BASE_ADDR+0x700000))
#define TP_RXPM_RXDATA			(*(volatile UInt16 *)(TP_BASE_ADDR+0x700002))
#define TP_RXPM_MPX				(*(volatile UInt16 *)(TP_BASE_ADDR+0x700004))
#define TP_RXPM_V24				(*(volatile UInt16 *)(TP_BASE_ADDR+0x700006))
#define TP_RXPM_ATC				(*(volatile UInt16 *)(TP_BASE_ADDR+0x700008))
#define TP_RXPM_ECDC			(*(volatile UInt16 *)(TP_BASE_ADDR+0x70000a))
#define TP_RXPM_PPP				(*(volatile UInt16 *)(TP_BASE_ADDR+0x70000c))
#define TP_RXPM_DTT				(*(volatile UInt16 *)(TP_BASE_ADDR+0x70000e))

#define TP_TXPM_UART			(*(volatile UInt16 *)(TP_BASE_ADDR+0x700010))
#define TP_TXPM_SIO				(*(volatile UInt16 *)(TP_BASE_ADDR+0x700012))
#define TP_TXPM_MPX				(*(volatile UInt16 *)(TP_BASE_ADDR+0x700014))
#define TP_TXPM_V24				(*(volatile UInt16 *)(TP_BASE_ADDR+0x700016))
#define TP_TXPM_ATC				(*(volatile UInt16 *)(TP_BASE_ADDR+0x700018))
#define TP_TXPM_ECDC			(*(volatile UInt16 *)(TP_BASE_ADDR+0x70001a))
#define TP_TXPM_PPP				(*(volatile UInt16 *)(TP_BASE_ADDR+0x70001c))
#define TP_TXPM_DTT				(*(volatile UInt16 *)(TP_BASE_ADDR+0x70001e))

#define TP_I2S						(*(volatile UInt16 *)(TP_BASE_ADDR+0x700030))

#define TP_HEDGE_START		(*(volatile UInt16 *)(TP_BASE_ADDR+0x800000))
#define TP_HEDGE_END			(*(volatile UInt16 *)(TP_BASE_ADDR+0x80FFFF))
#else

// Dummy Testpoint Array

extern UInt16 dummy_buff[1024];

#define SPARE_TP		(*(volatile UInt16 *)dummy_buff)
#define TP2				(*(volatile UInt16 *)dummy_buff)
#define TP4				(*(volatile UInt16 *)dummy_buff)
#define TP6				(*(volatile UInt16 *)dummy_buff)
#define TP8				(*(volatile UInt16 *)dummy_buff)
#define TPa				(*(volatile UInt16 *)dummy_buff)
#define TPc				(*(volatile UInt16 *)dummy_buff)
#define TPe				(*(volatile UInt16 *)dummy_buff)
#define SPEED_TP		(*(volatile UInt16 *)dummy_buff)
#define OAKMSG_TP		(*(volatile UInt16 *)dummy_buff)
#define FRAME_INT_TP	(*(volatile UInt16 *)dummy_buff)
#define DATA_TP			(*(volatile UInt16 *)dummy_buff)
#define TPP0 			(*(volatile UInt16 *)dummy_buff)
#define TPP4 			(*(volatile UInt16 *)dummy_buff)
#define TPP8 			(*(volatile UInt16 *)dummy_buff)
#define TPPc 			(*(volatile UInt16 *)dummy_buff)

#define FRAME_TP		(*(volatile UInt16 *)dummy_buff)
#define FRAME_INDEX_TP	(*(volatile UInt16 *)dummy_buff)
#define CMD_TP			(*(volatile UInt16 *)dummy_buff)
#define TX_DATA_INDEX_TP (*(volatile UInt16 *)dummy_buff)
#define TX_DATA_MODE_TP (*(volatile UInt16 *)dummy_buff)
#define TX_DATA_TP		(*(volatile UInt16 *)dummy_buff)
#define TX_FACCH_TP		(*(volatile UInt16 *)dummy_buff)
#define TX_SACCH_TP		(*(volatile UInt16 *)dummy_buff)
#define RX_DATA_INDEX_TP (*(volatile UInt16 *)dummy_buff)
#define RX_DATA_TP		(*(volatile UInt16 *)dummy_buff)
#define RX_FACCH_TP		(*(volatile UInt16 *)dummy_buff)
#define RX_SACCH_TP		(*(volatile UInt16 *)dummy_buff)
#define TBF_IF_MON0		(*(volatile UInt16 *)dummy_buff)
#define TBF_IF_MON1		(*(volatile UInt16 *)dummy_buff)
#define TBF_IF_MON2		(*(volatile UInt16 *)dummy_buff)
#define TBF_IF_MON3		(*(volatile UInt16 *)dummy_buff)

// High layer related test points
#define DTT_ACTION			(*(volatile UInt16 *)dummy_buff)
#define DTT_WRITE_FRAMES	(*(volatile UInt16 *)dummy_buff)
#define DTT_READ_FRAMES 	(*(volatile UInt16 *)dummy_buff)
#define DTA_READ_INDEX  	(*(volatile UInt16 *)dummy_buff)
#define DTA_WRITE_INDEX 	(*(volatile UInt16 *)dummy_buff)
#define L1DS_WRITE_INDEX	(*(volatile UInt16 *)dummy_buff)
#define L1DS_READ_INDEX 	(*(volatile UInt16 *)dummy_buff)

#define TP_DTN_ACTION			(*(volatile UInt16 *)dummy_buff)

#define TP_DRL_ACTION			(*(volatile UInt16 *)dummy_buff)

#define TP_DCH_ACTION			(*(volatile UInt16 *)dummy_buff)

#define TP_HEAP_ALLOC_SIZE		(*(volatile UInt16 *)dummy_buff)
#define TP_ASSERT			(*(volatile UInt16 *)dummy_buff)

#define TP_L1S_ACTION			(*(volatile UInt16 *)dummy_buff)
#define TP_OAK_RXQUAL_FULL		(*(volatile UInt16 *)dummy_buff)
#define TP_OAK_RXQUAL_SUB		(*(volatile UInt16 *)dummy_buff)
#define TP_OAK_RXLEV_FULL		(*(volatile UInt16 *)dummy_buff)
#define TP_OAK_RXLEV_SUB		(*(volatile UInt16 *)dummy_buff)

#define TP_FRAME_STATE			(*(volatile UInt16 *)dummy_buff)
#define TP_SWEEP_SIZE			(*(volatile UInt16 *)dummy_buff)
#define TP_SWEEP_CNT			(*(volatile UInt16 *)dummy_buff)
#define TP_L1_RX_MSG            (*(volatile UInt16 *)dummy_buff)

#define TP_READ_BUFF			(*(volatile UInt16 *)dummy_buff)
#define TP_RX_BUFF_BFI			(*(volatile UInt16 *)dummy_buff)
#define TP_RX_BUFF_CODE			(*(volatile UInt16 *)dummy_buff)
#define TP_RX_BUFF_OTD			(*(volatile UInt16 *)dummy_buff)
#define TP_RX_BUFF_INDEX		(*(volatile UInt16 *)dummy_buff)
#define TP_RX_BUFF_ADJ			(*(volatile UInt16 *)dummy_buff)

#define TP_WRITE_BUFF			(*(volatile UInt16 *)dummy_buff)
#define TP_TX_BUFF_INDEX		(*(volatile UInt16 *)dummy_buff)
#define TP_TX_BUFF_CODE			(*(volatile UInt16 *)dummy_buff)

#define TP_MAC_GET_BLOCK_TX_SLOT (*(volatile UInt16 *)dummy_buff)
#define TP_MAC_GET_BLOCK_FN 	(*(volatile UInt16 *)dummy_buff)
#define TP_MAC_GET_BLOCK_TBF_ID (*(volatile UInt16 *)dummy_buff)

#define TP_MAC_ACK_NU_SENT 		(*(volatile UInt16 *)dummy_buff)
#define TP_MAC_ACK_FN		 	(*(volatile UInt16 *)dummy_buff)

#define TP_START_PLOT_FRAME		(*(volatile UInt16 *)dummy_buff)

#define TP_CELLDATA_ID			(*(volatile UInt16 *)dummy_buff)
#define TP_CELLDATA_VALID		(*(volatile UInt16 *)dummy_buff)
#define TP_CELLDATA_SYNC		(*(volatile UInt16 *)dummy_buff)
#define TP_CELLDATA_ARFCN		(*(volatile UInt16 *)dummy_buff)
#define TP_CELLDATA_RXLEV		(*(volatile UInt16 *)dummy_buff)

#define TP_SORT_INDEX			(*(volatile UInt16 *)dummy_buff)
#define TP_SORT_ARFCN			(*(volatile UInt16 *)dummy_buff)
#define TP_SORT_RXLEV			(*(volatile UInt16 *)dummy_buff)

#define TP_MON_ARFCN			(*(volatile UInt16 *)dummy_buff)
#define TP_MON_RXLEV			(*(volatile UInt16 *)dummy_buff)

#define TP_RX_SLOTS_PATT		(*(volatile UInt16 *)dummy_buff)
#define TP_TX_SLOTS_PATT		(*(volatile UInt16 *)dummy_buff)
#define TP_FIRST_SLOT			(*(volatile UInt16 *)dummy_buff)
#define TP_TCH_MODE 			(*(volatile UInt16 *)dummy_buff)
#define TP_CHAN_CFG_IS_HSCSD	(*(volatile UInt16 *)dummy_buff)

#define TP_ECDC_ENTER			(*(volatile UInt16 *)dummy_buff)
#define TP_ECDC_MSR_EVT			(*(volatile UInt16 *)dummy_buff)
#define TP_ECDC_MSR_RD			(*(volatile UInt16 *)dummy_buff)
#define TP_ECDC_RX_EVT			(*(volatile UInt16 *)dummy_buff)
#define TP_ECDC_RPT_DTR			(*(volatile UInt16 *)dummy_buff)
#define TP_ECDC_FC				(*(volatile UInt16 *)dummy_buff)
#define TP_ECDC_RTS				(*(volatile UInt16 *)dummy_buff)
#define TP_ECDC_DTR				(*(volatile UInt16 *)dummy_buff)
#define TP_ECDC_T1_ENTER		(*(volatile UInt16 *)dummy_buff)
#define TP_ECDC_T1_EXIT			(*(volatile UInt16 *)dummy_buff)
#define TP_ECDC_MPX_RPT_EVT		(*(volatile UInt16 *)dummy_buff)
#define TP_ECDC_V24_RPT_LSI		(*(volatile UInt16 *)dummy_buff)
#define TP_ECDC_V24_EVT_PRC		(*(volatile UInt16 *)dummy_buff)
#define TP_ECDC_DROPPED			(*(volatile UInt16 *)dummy_buff)
#define TP_ECDC_SENT			(*(volatile UInt16 *)dummy_buff)
#define TP_ECDC_BYTES_READ		(*(volatile UInt16 *)dummy_buff)
#define TP_ECDC_DTE_RD_DATA		(*(volatile UInt8  *)dummy_buff)
#define TP_ECDC_BYTES_WRITTEN	(*(volatile UInt16 *)dummy_buff)
#define TP_ECDC_DTE_WR_DATA		(*(volatile UInt8  *)dummy_buff)
#define TP_ECDC_V24_UPD_DCE_FC	(*(volatile UInt16 *)dummy_buff)
#define TP_ECDC_V24_RPT_REM_FC	(*(volatile UInt16 *)dummy_buff)
#define TP_ECDC_UPD_DCE_FC		(*(volatile UInt16 *)dummy_buff)
#define TP_ECDC_RPT_REM_FC		(*(volatile UInt16 *)dummy_buff)
#define TP_ECDC_UPD_DCE_SZ		(*(volatile UInt16 *)dummy_buff)
#define TP_ECDC_TX_DATA			(*(volatile UInt16 *)dummy_buff)
#define TP_ECDC_RX_DATA			(*(volatile UInt16 *)dummy_buff)
#define TP_ECDC_EXIT			(*(volatile UInt16 *)dummy_buff)

#define TP_SIO_RX_CHAR			(*(volatile UInt16 *)dummy_buff)
#define TP_SIO_TX_CHAR			(*(volatile UInt16 *)dummy_buff)
#define TP_SIO_BRK_CK0			(*(volatile UInt16 *)dummy_buff)
#define TP_SIO_BRK_CK1			(*(volatile UInt16 *)dummy_buff)
#define TP_SIO_BRK_CK2			(*(volatile UInt16 *)dummy_buff)
#define TP_SIO_BRK_CK3			(*(volatile UInt16 *)dummy_buff)
#define TP_SIO_BRK_CK4			(*(volatile UInt16 *)dummy_buff)
#define TP_SIO_BRK_CK5			(*(volatile UInt16 *)dummy_buff)
#define TP_SIO_BRK_CK6			(*(volatile UInt16 *)dummy_buff)
#define TP_SIO_BRK_CK7			(*(volatile UInt16 *)dummy_buff)

#define TP_TX_96_SLOT_NO		(*(volatile UInt16 *)dummy_buff)
#define TP_RX_96_SLOT_NO		(*(volatile UInt16 *)dummy_buff)
#define TP_TX_96_NT_FRAME		(*(volatile UInt16 *)dummy_buff)
#define TP_RX_96_NT_FRAME		(*(volatile UInt16 *)dummy_buff)

#define TP_TX_144_SLOT_NO		(*(volatile UInt16 *)dummy_buff)
#define TP_RX_144_SLOT_NO		(*(volatile UInt16 *)dummy_buff)
#define TP_TX_144_NT_FRAME		(*(volatile UInt16 *)dummy_buff)
#define TP_RX_144_NT_FRAME		(*(volatile UInt16 *)dummy_buff)

#define TP_SERIAL_TX_SIZE		(*(volatile UInt16 *)dummy_buff)

#define TP_FLW_SIO_RX_CNT		(*(volatile UInt16 *)dummy_buff)
#define TP_FLW_SIO_TX_CNT		(*(volatile UInt16 *)dummy_buff)

#define TP_FLW_MPXRX_RX_CNT		(*(volatile UInt16 *)dummy_buff)
#define TP_FLW_MPX_TX_CNT		(*(volatile UInt16 *)dummy_buff)

#define	TP_STACK_OVERFLOW		(*(volatile UInt16 *)dummy_buff)

#endif	// end if ifdef WIN32


#ifndef NULL
#ifdef __cplusplus
#define NULL          0
#else
#define NULL ((void *)0)
#endif
#endif

#endif


//Below defines conflict with arch/arm/mach-bcm116x/include/mach/bits.h:
#if 0
#define	BIT00			0
#define	BIT01			1
#define	BIT02			2
#define	BIT03			3
#define	BIT04			4
#define	BIT05			5
#define	BIT06			6
#define	BIT07			7
#define	BIT08			8
#define	BIT09			9
#define	BIT10			10
#define	BIT11			11
#define	BIT12			12
#define	BIT13			13
#define	BIT14			14
#define	BIT15			15
#define	BIT16			16
#define	BIT17			17
#define	BIT18			18
#define	BIT19			19
#define	BIT20			20
#define	BIT21			21
#define	BIT22			22
#define	BIT23			23
#define	BIT24			24
#define	BIT25			25
#define	BIT26			26
#define	BIT27			27
#define	BIT28			28
#define	BIT29			29
#define	BIT30			30
#define	BIT31			31
#endif

#define	BIT00_MASK		(1 << 0)
#define	BIT01_MASK		(1 << 1)
#define	BIT02_MASK		(1 << 2)
#define	BIT03_MASK		(1 << 3)
#define	BIT04_MASK		(1 << 4)
#define	BIT05_MASK		(1 << 5)
#define	BIT06_MASK		(1 << 6)
#define	BIT07_MASK		(1 << 7)
#define	BIT08_MASK		(1 << 8)
#define	BIT09_MASK		(1 << 9)
#define	BIT10_MASK		(1 << 10)
#define	BIT11_MASK		(1 << 11)
#define	BIT12_MASK		(1 << 12)
#define	BIT13_MASK		(1 << 13)
#define	BIT14_MASK		(1 << 14)
#define	BIT15_MASK		(1 << 15)
#define	BIT16_MASK		(1 << 16)
#define	BIT17_MASK		(1 << 17)
#define	BIT18_MASK		(1 << 18)
#define	BIT19_MASK		(1 << 19)
#define	BIT20_MASK		(1 << 20)
#define	BIT21_MASK		(1 << 21)
#define	BIT22_MASK		(1 << 22)
#define	BIT23_MASK		(1 << 23)
#define	BIT24_MASK		(1 << 24)
#define	BIT25_MASK		(1 << 25)
#define	BIT26_MASK		(1 << 26)
#define	BIT27_MASK		(1 << 27)
#define	BIT28_MASK		(1 << 28)
#define	BIT29_MASK		(1 << 29)
#define	BIT30_MASK		(1 << 30)
#define	BIT31_MASK		(1 << 31)


//_shared_tx_buff: D:82A0	-> 6000540( arm address)
//typedef struct
//{
//	volatile UInt16 octets;
//	volatile UInt16 tx_option;
//	volatile UInt16 msg[ CONTENTS_SIZE ];	// Two octets per word
//} TxBuf_t;
//size = 60 bytes
//
//#define CONTENTS_SIZE		28		// # of octets in multislot message
//#define ENTRY_TXBUF_FACCH		1
//#define ENTRY_TXBUF_SACCH		2
//#define ENTRY_TXBUF_SDCCH		3
//tx sdcch buffer address = 0x540 + 60 *3 = 0x5f4
//tx FACCH buffer address = 0x540 + 60 = 0x57c
//tx DATA buffer address = 0x540 + 60 * 4 = 0x630

//_shared_rx_buff: D:8080, in file D:\hscsd\grip\common\shared.c
//
//#define ENTRY_RXBUF_FACCH		1
//#define ENTRY_RXBUF_SACCH		6
//#define ENTRY_RXBUF_SDCCH		3
//
//typedef struct
//{
//	volatile UInt16 bfi;
//	volatile UInt16 index;
//	volatile UInt16 otd;
//	volatile UInt16 octets;
//	volatile UInt16 rxlev;
//	volatile UInt16 coding;
//	volatile UInt16 msg[ CONTENTS_SIZE ];	// Two octets per word
//} RxBuf_t;
// rx sdcch buffer address = 0x100 + 68 * 3 = 0x60001cc
//
// rx FACCH buffer address = 0x100 + 68 = 0x6000144
// rx DATA buffer address = 0x100 + 68 * 4  = 0x6000210	( actual data starts from


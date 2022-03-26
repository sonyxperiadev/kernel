/*
 * Copyright (c) 2019, The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef _RX_RXPCU_CLASSIFICATION_OVERVIEW_H_
#define _RX_RXPCU_CLASSIFICATION_OVERVIEW_H_
#if !defined(__ASSEMBLER__)
#endif


// ################ START SUMMARY #################
//
//	Dword	Fields
//	0	filter_pass_mpdus[0], filter_pass_mpdus_fcs_ok[1], monitor_direct_mpdus[2], monitor_direct_mpdus_fcs_ok[3], monitor_other_mpdus[4], monitor_other_mpdus_fcs_ok[5], phyrx_abort_received[6], reserved_0[15:7], phy_ppdu_id[31:16]
//
// ################ END SUMMARY #################

#define NUM_OF_DWORDS_RX_RXPCU_CLASSIFICATION_OVERVIEW 1

struct rx_rxpcu_classification_overview {
             uint32_t filter_pass_mpdus               :  1, //[0]
                      filter_pass_mpdus_fcs_ok        :  1, //[1]
                      monitor_direct_mpdus            :  1, //[2]
                      monitor_direct_mpdus_fcs_ok     :  1, //[3]
                      monitor_other_mpdus             :  1, //[4]
                      monitor_other_mpdus_fcs_ok      :  1, //[5]
                      phyrx_abort_received            :  1, //[6]
                      reserved_0                      :  9, //[15:7]
                      phy_ppdu_id                     : 16; //[31:16]
};

/*

filter_pass_mpdus
			
			When set, at least one Filter Pass MPDU has been
			received. FCS might or might not have been passing.
			
			
			
			For MU UL, in  TLVs RX_PPDU_END and
			RX_PPDU_END_STATUS_DONE, this field is the OR of all the
			users.
			
			<legal all>

filter_pass_mpdus_fcs_ok
			
			When set, at least one Filter Pass MPDU has been
			received that has a correct FCS.
			
			
			
			For MU UL, in  TLVs RX_PPDU_END and
			RX_PPDU_END_STATUS_DONE, this field is the OR of all the
			users.
			
			
			
			<legal all>

monitor_direct_mpdus
			
			When set, at least one Monitor Direct MPDU has been
			received. FCS might or might not have been passing
			
			
			
			For MU UL, in  TLVs RX_PPDU_END and
			RX_PPDU_END_STATUS_DONE, this field is the OR of all the
			users.
			
			<legal all>

monitor_direct_mpdus_fcs_ok
			
			When set, at least one Monitor Direct MPDU has been
			received that has a correct FCS.
			
			
			
			For MU UL, in  TLVs RX_PPDU_END and
			RX_PPDU_END_STATUS_DONE, this field is the OR of all the
			users.
			
			
			
			<legal all>

monitor_other_mpdus
			
			When set, at least one Monitor Direct MPDU has been
			received. FCS might or might not have been passing.
			
			
			
			For MU UL, in  TLVs RX_PPDU_END and
			RX_PPDU_END_STATUS_DONE, this field is the OR of all the
			users.
			
			<legal all>

monitor_other_mpdus_fcs_ok
			
			When set, at least one Monitor Direct MPDU has been
			received that has a correct FCS.
			
			
			
			For MU UL, in  TLVs RX_PPDU_END and
			RX_PPDU_END_STATUS_DONE, this field is the OR of all the
			users.
			
			<legal all>

phyrx_abort_received
			
			When set, PPDU reception was aborted by the PHY
			
			<legal all>

reserved_0
			
			<legal 0>

phy_ppdu_id
			
			A ppdu counter value that PHY increments for every PPDU
			received. The counter value wraps around  
			
			<legal all>
*/


/* Description		RX_RXPCU_CLASSIFICATION_OVERVIEW_0_FILTER_PASS_MPDUS
			
			When set, at least one Filter Pass MPDU has been
			received. FCS might or might not have been passing.
			
			
			
			For MU UL, in  TLVs RX_PPDU_END and
			RX_PPDU_END_STATUS_DONE, this field is the OR of all the
			users.
			
			<legal all>
*/
#define RX_RXPCU_CLASSIFICATION_OVERVIEW_0_FILTER_PASS_MPDUS_OFFSET  0x00000000
#define RX_RXPCU_CLASSIFICATION_OVERVIEW_0_FILTER_PASS_MPDUS_LSB     0
#define RX_RXPCU_CLASSIFICATION_OVERVIEW_0_FILTER_PASS_MPDUS_MASK    0x00000001

/* Description		RX_RXPCU_CLASSIFICATION_OVERVIEW_0_FILTER_PASS_MPDUS_FCS_OK
			
			When set, at least one Filter Pass MPDU has been
			received that has a correct FCS.
			
			
			
			For MU UL, in  TLVs RX_PPDU_END and
			RX_PPDU_END_STATUS_DONE, this field is the OR of all the
			users.
			
			
			
			<legal all>
*/
#define RX_RXPCU_CLASSIFICATION_OVERVIEW_0_FILTER_PASS_MPDUS_FCS_OK_OFFSET 0x00000000
#define RX_RXPCU_CLASSIFICATION_OVERVIEW_0_FILTER_PASS_MPDUS_FCS_OK_LSB 1
#define RX_RXPCU_CLASSIFICATION_OVERVIEW_0_FILTER_PASS_MPDUS_FCS_OK_MASK 0x00000002

/* Description		RX_RXPCU_CLASSIFICATION_OVERVIEW_0_MONITOR_DIRECT_MPDUS
			
			When set, at least one Monitor Direct MPDU has been
			received. FCS might or might not have been passing
			
			
			
			For MU UL, in  TLVs RX_PPDU_END and
			RX_PPDU_END_STATUS_DONE, this field is the OR of all the
			users.
			
			<legal all>
*/
#define RX_RXPCU_CLASSIFICATION_OVERVIEW_0_MONITOR_DIRECT_MPDUS_OFFSET 0x00000000
#define RX_RXPCU_CLASSIFICATION_OVERVIEW_0_MONITOR_DIRECT_MPDUS_LSB  2
#define RX_RXPCU_CLASSIFICATION_OVERVIEW_0_MONITOR_DIRECT_MPDUS_MASK 0x00000004

/* Description		RX_RXPCU_CLASSIFICATION_OVERVIEW_0_MONITOR_DIRECT_MPDUS_FCS_OK
			
			When set, at least one Monitor Direct MPDU has been
			received that has a correct FCS.
			
			
			
			For MU UL, in  TLVs RX_PPDU_END and
			RX_PPDU_END_STATUS_DONE, this field is the OR of all the
			users.
			
			
			
			<legal all>
*/
#define RX_RXPCU_CLASSIFICATION_OVERVIEW_0_MONITOR_DIRECT_MPDUS_FCS_OK_OFFSET 0x00000000
#define RX_RXPCU_CLASSIFICATION_OVERVIEW_0_MONITOR_DIRECT_MPDUS_FCS_OK_LSB 3
#define RX_RXPCU_CLASSIFICATION_OVERVIEW_0_MONITOR_DIRECT_MPDUS_FCS_OK_MASK 0x00000008

/* Description		RX_RXPCU_CLASSIFICATION_OVERVIEW_0_MONITOR_OTHER_MPDUS
			
			When set, at least one Monitor Direct MPDU has been
			received. FCS might or might not have been passing.
			
			
			
			For MU UL, in  TLVs RX_PPDU_END and
			RX_PPDU_END_STATUS_DONE, this field is the OR of all the
			users.
			
			<legal all>
*/
#define RX_RXPCU_CLASSIFICATION_OVERVIEW_0_MONITOR_OTHER_MPDUS_OFFSET 0x00000000
#define RX_RXPCU_CLASSIFICATION_OVERVIEW_0_MONITOR_OTHER_MPDUS_LSB   4
#define RX_RXPCU_CLASSIFICATION_OVERVIEW_0_MONITOR_OTHER_MPDUS_MASK  0x00000010

/* Description		RX_RXPCU_CLASSIFICATION_OVERVIEW_0_MONITOR_OTHER_MPDUS_FCS_OK
			
			When set, at least one Monitor Direct MPDU has been
			received that has a correct FCS.
			
			
			
			For MU UL, in  TLVs RX_PPDU_END and
			RX_PPDU_END_STATUS_DONE, this field is the OR of all the
			users.
			
			<legal all>
*/
#define RX_RXPCU_CLASSIFICATION_OVERVIEW_0_MONITOR_OTHER_MPDUS_FCS_OK_OFFSET 0x00000000
#define RX_RXPCU_CLASSIFICATION_OVERVIEW_0_MONITOR_OTHER_MPDUS_FCS_OK_LSB 5
#define RX_RXPCU_CLASSIFICATION_OVERVIEW_0_MONITOR_OTHER_MPDUS_FCS_OK_MASK 0x00000020

/* Description		RX_RXPCU_CLASSIFICATION_OVERVIEW_0_PHYRX_ABORT_RECEIVED
			
			When set, PPDU reception was aborted by the PHY
			
			<legal all>
*/
#define RX_RXPCU_CLASSIFICATION_OVERVIEW_0_PHYRX_ABORT_RECEIVED_OFFSET 0x00000000
#define RX_RXPCU_CLASSIFICATION_OVERVIEW_0_PHYRX_ABORT_RECEIVED_LSB  6
#define RX_RXPCU_CLASSIFICATION_OVERVIEW_0_PHYRX_ABORT_RECEIVED_MASK 0x00000040

/* Description		RX_RXPCU_CLASSIFICATION_OVERVIEW_0_RESERVED_0
			
			<legal 0>
*/
#define RX_RXPCU_CLASSIFICATION_OVERVIEW_0_RESERVED_0_OFFSET         0x00000000
#define RX_RXPCU_CLASSIFICATION_OVERVIEW_0_RESERVED_0_LSB            7
#define RX_RXPCU_CLASSIFICATION_OVERVIEW_0_RESERVED_0_MASK           0x0000ff80

/* Description		RX_RXPCU_CLASSIFICATION_OVERVIEW_0_PHY_PPDU_ID
			
			A ppdu counter value that PHY increments for every PPDU
			received. The counter value wraps around  
			
			<legal all>
*/
#define RX_RXPCU_CLASSIFICATION_OVERVIEW_0_PHY_PPDU_ID_OFFSET        0x00000000
#define RX_RXPCU_CLASSIFICATION_OVERVIEW_0_PHY_PPDU_ID_LSB           16
#define RX_RXPCU_CLASSIFICATION_OVERVIEW_0_PHY_PPDU_ID_MASK          0xffff0000


#endif // _RX_RXPCU_CLASSIFICATION_OVERVIEW_H_

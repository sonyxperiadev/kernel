/*
 * Copyright (c) 2018 The Linux Foundation. All rights reserved.
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

#ifndef _RX_PPDU_END_USER_STATS_EXT_H_
#define _RX_PPDU_END_USER_STATS_EXT_H_
#if !defined(__ASSEMBLER__)
#endif

#include "rx_rxpcu_classification_overview.h"

// ################ START SUMMARY #################
//
//	Dword	Fields
//	0	struct rx_rxpcu_classification_overview rxpcu_classification_details;
//	1	fcs_ok_bitmap_95_64[31:0]
//	2	fcs_ok_bitmap_127_96[31:0]
//	3	fcs_ok_bitmap_159_128[31:0]
//	4	fcs_ok_bitmap_191_160[31:0]
//	5	fcs_ok_bitmap_223_192[31:0]
//	6	fcs_ok_bitmap_255_224[31:0]
//
// ################ END SUMMARY #################

#define NUM_OF_DWORDS_RX_PPDU_END_USER_STATS_EXT 7

struct rx_ppdu_end_user_stats_ext {
    struct            rx_rxpcu_classification_overview                       rxpcu_classification_details;
             uint32_t fcs_ok_bitmap_95_64             : 32; //[31:0]
             uint32_t fcs_ok_bitmap_127_96            : 32; //[31:0]
             uint32_t fcs_ok_bitmap_159_128           : 32; //[31:0]
             uint32_t fcs_ok_bitmap_191_160           : 32; //[31:0]
             uint32_t fcs_ok_bitmap_223_192           : 32; //[31:0]
             uint32_t fcs_ok_bitmap_255_224           : 32; //[31:0]
};

/*

struct rx_rxpcu_classification_overview rxpcu_classification_details
			
			Details related to what RXPCU classification types of
			MPDUs have been received

fcs_ok_bitmap_95_64
			
			Bitmap indicates in order of received MPDUs, which MPDUs
			had an passing FCS or had an error.
			
			1: FCS OK
			
			0: FCS error
			
			<legal all>

fcs_ok_bitmap_127_96
			
			Bitmap indicates in order of received MPDUs, which MPDUs
			had an passing FCS or had an error.
			
			1: FCS OK
			
			0: FCS error
			
			<legal all>

fcs_ok_bitmap_159_128
			
			Bitmap indicates in order of received MPDUs, which MPDUs
			had an passing FCS or had an error.
			
			1: FCS OK
			
			0: FCS error
			
			<legal all>

fcs_ok_bitmap_191_160
			
			Bitmap indicates in order of received MPDUs, which MPDUs
			had an passing FCS or had an error.
			
			1: FCS OK
			
			0: FCS error
			
			<legal all>

fcs_ok_bitmap_223_192
			
			Bitmap indicates in order of received MPDUs, which MPDUs
			had an passing FCS or had an error.
			
			1: FCS OK
			
			0: FCS error
			
			<legal all>

fcs_ok_bitmap_255_224
			
			Bitmap indicates in order of received MPDUs, which MPDUs
			had an passing FCS or had an error.
			
			1: FCS OK
			
			0: FCS error
			
			<legal all>
*/

#define RX_PPDU_END_USER_STATS_EXT_0_RX_RXPCU_CLASSIFICATION_OVERVIEW_RXPCU_CLASSIFICATION_DETAILS_OFFSET 0x00000000
#define RX_PPDU_END_USER_STATS_EXT_0_RX_RXPCU_CLASSIFICATION_OVERVIEW_RXPCU_CLASSIFICATION_DETAILS_LSB 0
#define RX_PPDU_END_USER_STATS_EXT_0_RX_RXPCU_CLASSIFICATION_OVERVIEW_RXPCU_CLASSIFICATION_DETAILS_MASK 0xffffffff

/* Description		RX_PPDU_END_USER_STATS_EXT_1_FCS_OK_BITMAP_95_64
			
			Bitmap indicates in order of received MPDUs, which MPDUs
			had an passing FCS or had an error.
			
			1: FCS OK
			
			0: FCS error
			
			<legal all>
*/
#define RX_PPDU_END_USER_STATS_EXT_1_FCS_OK_BITMAP_95_64_OFFSET      0x00000004
#define RX_PPDU_END_USER_STATS_EXT_1_FCS_OK_BITMAP_95_64_LSB         0
#define RX_PPDU_END_USER_STATS_EXT_1_FCS_OK_BITMAP_95_64_MASK        0xffffffff

/* Description		RX_PPDU_END_USER_STATS_EXT_2_FCS_OK_BITMAP_127_96
			
			Bitmap indicates in order of received MPDUs, which MPDUs
			had an passing FCS or had an error.
			
			1: FCS OK
			
			0: FCS error
			
			<legal all>
*/
#define RX_PPDU_END_USER_STATS_EXT_2_FCS_OK_BITMAP_127_96_OFFSET     0x00000008
#define RX_PPDU_END_USER_STATS_EXT_2_FCS_OK_BITMAP_127_96_LSB        0
#define RX_PPDU_END_USER_STATS_EXT_2_FCS_OK_BITMAP_127_96_MASK       0xffffffff

/* Description		RX_PPDU_END_USER_STATS_EXT_3_FCS_OK_BITMAP_159_128
			
			Bitmap indicates in order of received MPDUs, which MPDUs
			had an passing FCS or had an error.
			
			1: FCS OK
			
			0: FCS error
			
			<legal all>
*/
#define RX_PPDU_END_USER_STATS_EXT_3_FCS_OK_BITMAP_159_128_OFFSET    0x0000000c
#define RX_PPDU_END_USER_STATS_EXT_3_FCS_OK_BITMAP_159_128_LSB       0
#define RX_PPDU_END_USER_STATS_EXT_3_FCS_OK_BITMAP_159_128_MASK      0xffffffff

/* Description		RX_PPDU_END_USER_STATS_EXT_4_FCS_OK_BITMAP_191_160
			
			Bitmap indicates in order of received MPDUs, which MPDUs
			had an passing FCS or had an error.
			
			1: FCS OK
			
			0: FCS error
			
			<legal all>
*/
#define RX_PPDU_END_USER_STATS_EXT_4_FCS_OK_BITMAP_191_160_OFFSET    0x00000010
#define RX_PPDU_END_USER_STATS_EXT_4_FCS_OK_BITMAP_191_160_LSB       0
#define RX_PPDU_END_USER_STATS_EXT_4_FCS_OK_BITMAP_191_160_MASK      0xffffffff

/* Description		RX_PPDU_END_USER_STATS_EXT_5_FCS_OK_BITMAP_223_192
			
			Bitmap indicates in order of received MPDUs, which MPDUs
			had an passing FCS or had an error.
			
			1: FCS OK
			
			0: FCS error
			
			<legal all>
*/
#define RX_PPDU_END_USER_STATS_EXT_5_FCS_OK_BITMAP_223_192_OFFSET    0x00000014
#define RX_PPDU_END_USER_STATS_EXT_5_FCS_OK_BITMAP_223_192_LSB       0
#define RX_PPDU_END_USER_STATS_EXT_5_FCS_OK_BITMAP_223_192_MASK      0xffffffff

/* Description		RX_PPDU_END_USER_STATS_EXT_6_FCS_OK_BITMAP_255_224
			
			Bitmap indicates in order of received MPDUs, which MPDUs
			had an passing FCS or had an error.
			
			1: FCS OK
			
			0: FCS error
			
			<legal all>
*/
#define RX_PPDU_END_USER_STATS_EXT_6_FCS_OK_BITMAP_255_224_OFFSET    0x00000018
#define RX_PPDU_END_USER_STATS_EXT_6_FCS_OK_BITMAP_255_224_LSB       0
#define RX_PPDU_END_USER_STATS_EXT_6_FCS_OK_BITMAP_255_224_MASK      0xffffffff


#endif // _RX_PPDU_END_USER_STATS_EXT_H_

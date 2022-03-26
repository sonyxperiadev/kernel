/*
 * Copyright (c) 2016-2017 The Linux Foundation. All rights reserved.
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

#ifndef _REO_FLUSH_TIMEOUT_LIST_H_
#define _REO_FLUSH_TIMEOUT_LIST_H_
#if !defined(__ASSEMBLER__)
#endif

#include "uniform_reo_cmd_header.h"

// ################ START SUMMARY #################
//
//	Dword	Fields
//	0	struct uniform_reo_cmd_header cmd_header;
//	1	ac_timout_list[1:0], reserved_1[31:2]
//	2	minimum_release_desc_count[15:0], minimum_forward_buf_count[31:16]
//	3	reserved_3a[31:0]
//	4	reserved_4a[31:0]
//	5	reserved_5a[31:0]
//	6	reserved_6a[31:0]
//	7	reserved_7a[31:0]
//	8	reserved_8a[31:0]
//
// ################ END SUMMARY #################

#define NUM_OF_DWORDS_REO_FLUSH_TIMEOUT_LIST 9

struct reo_flush_timeout_list {
    struct            uniform_reo_cmd_header                       cmd_header;
             uint32_t ac_timout_list                  :  2, //[1:0]
                      reserved_1                      : 30; //[31:2]
             uint32_t minimum_release_desc_count      : 16, //[15:0]
                      minimum_forward_buf_count       : 16; //[31:16]
             uint32_t reserved_3a                     : 32; //[31:0]
             uint32_t reserved_4a                     : 32; //[31:0]
             uint32_t reserved_5a                     : 32; //[31:0]
             uint32_t reserved_6a                     : 32; //[31:0]
             uint32_t reserved_7a                     : 32; //[31:0]
             uint32_t reserved_8a                     : 32; //[31:0]
};

/*

struct uniform_reo_cmd_header cmd_header
			
			Consumer: REO
			
			Producer: SW
			
			
			
			Details for command execution tracking purposes.

ac_timout_list
			
			Consumer: REO
			
			Producer: SW
			
			
			
			The AC_timeout list to be used for this command
			
			<legal all>

reserved_1
			
			<legal 0>

minimum_release_desc_count
			
			Consumer: REO
			
			Producer: SW
			
			
			
			The minimum number of link descriptors requested to be
			released. If set to 0, only buffer release counts seems to
			be important... When set to very high value, likely the
			entire timeout list will be exhausted before this count is
			reached or maybe this count will not get reached. REO
			however will stop here as it can not do anything else.
			
			
			
			When both this field and field Minimum_forward_buf_count
			are > 0, REO needs to meet both requirements. When both
			entries are 0 (which should be a programming error), REO
			does not need to do anything.
			
			
			
			Note that this includes counts of MPDU link Desc as well
			as MSDU link Desc. Where the count of MSDU link Desc is not
			known to REO it's approximated by deriving from MSDU count
			
			<legal all>

minimum_forward_buf_count
			
			Consumer: REO
			
			Producer: SW
			
			
			
			The minimum number of buffer descriptors requested to be
			passed on to the REO destination rings. 
			
			
			
			If set to 0, only descriptor release counts seems to be
			important... 
			
			
			
			When set to very high value, likely the entire timeout
			list will be exhausted before this count is reached or maybe
			this count will not get reached. REO however will stop here
			as it can not do anything else.
			
			
			
			Note that REO does not know the exact buffer count. This
			can be approximated by using the MSDU_COUNT
			
			<legal all>

reserved_3a
			
			<legal 0>

reserved_4a
			
			<legal 0>

reserved_5a
			
			<legal 0>

reserved_6a
			
			<legal 0>

reserved_7a
			
			<legal 0>

reserved_8a
			
			<legal 0>
*/

#define REO_FLUSH_TIMEOUT_LIST_0_UNIFORM_REO_CMD_HEADER_CMD_HEADER_OFFSET 0x00000000
#define REO_FLUSH_TIMEOUT_LIST_0_UNIFORM_REO_CMD_HEADER_CMD_HEADER_LSB 0
#define REO_FLUSH_TIMEOUT_LIST_0_UNIFORM_REO_CMD_HEADER_CMD_HEADER_MASK 0xffffffff

/* Description		REO_FLUSH_TIMEOUT_LIST_1_AC_TIMOUT_LIST
			
			Consumer: REO
			
			Producer: SW
			
			
			
			The AC_timeout list to be used for this command
			
			<legal all>
*/
#define REO_FLUSH_TIMEOUT_LIST_1_AC_TIMOUT_LIST_OFFSET               0x00000004
#define REO_FLUSH_TIMEOUT_LIST_1_AC_TIMOUT_LIST_LSB                  0
#define REO_FLUSH_TIMEOUT_LIST_1_AC_TIMOUT_LIST_MASK                 0x00000003

/* Description		REO_FLUSH_TIMEOUT_LIST_1_RESERVED_1
			
			<legal 0>
*/
#define REO_FLUSH_TIMEOUT_LIST_1_RESERVED_1_OFFSET                   0x00000004
#define REO_FLUSH_TIMEOUT_LIST_1_RESERVED_1_LSB                      2
#define REO_FLUSH_TIMEOUT_LIST_1_RESERVED_1_MASK                     0xfffffffc

/* Description		REO_FLUSH_TIMEOUT_LIST_2_MINIMUM_RELEASE_DESC_COUNT
			
			Consumer: REO
			
			Producer: SW
			
			
			
			The minimum number of link descriptors requested to be
			released. If set to 0, only buffer release counts seems to
			be important... When set to very high value, likely the
			entire timeout list will be exhausted before this count is
			reached or maybe this count will not get reached. REO
			however will stop here as it can not do anything else.
			
			
			
			When both this field and field Minimum_forward_buf_count
			are > 0, REO needs to meet both requirements. When both
			entries are 0 (which should be a programming error), REO
			does not need to do anything.
			
			
			
			Note that this includes counts of MPDU link Desc as well
			as MSDU link Desc. Where the count of MSDU link Desc is not
			known to REO it's approximated by deriving from MSDU count
			
			<legal all>
*/
#define REO_FLUSH_TIMEOUT_LIST_2_MINIMUM_RELEASE_DESC_COUNT_OFFSET   0x00000008
#define REO_FLUSH_TIMEOUT_LIST_2_MINIMUM_RELEASE_DESC_COUNT_LSB      0
#define REO_FLUSH_TIMEOUT_LIST_2_MINIMUM_RELEASE_DESC_COUNT_MASK     0x0000ffff

/* Description		REO_FLUSH_TIMEOUT_LIST_2_MINIMUM_FORWARD_BUF_COUNT
			
			Consumer: REO
			
			Producer: SW
			
			
			
			The minimum number of buffer descriptors requested to be
			passed on to the REO destination rings. 
			
			
			
			If set to 0, only descriptor release counts seems to be
			important... 
			
			
			
			When set to very high value, likely the entire timeout
			list will be exhausted before this count is reached or maybe
			this count will not get reached. REO however will stop here
			as it can not do anything else.
			
			
			
			Note that REO does not know the exact buffer count. This
			can be approximated by using the MSDU_COUNT
			
			<legal all>
*/
#define REO_FLUSH_TIMEOUT_LIST_2_MINIMUM_FORWARD_BUF_COUNT_OFFSET    0x00000008
#define REO_FLUSH_TIMEOUT_LIST_2_MINIMUM_FORWARD_BUF_COUNT_LSB       16
#define REO_FLUSH_TIMEOUT_LIST_2_MINIMUM_FORWARD_BUF_COUNT_MASK      0xffff0000

/* Description		REO_FLUSH_TIMEOUT_LIST_3_RESERVED_3A
			
			<legal 0>
*/
#define REO_FLUSH_TIMEOUT_LIST_3_RESERVED_3A_OFFSET                  0x0000000c
#define REO_FLUSH_TIMEOUT_LIST_3_RESERVED_3A_LSB                     0
#define REO_FLUSH_TIMEOUT_LIST_3_RESERVED_3A_MASK                    0xffffffff

/* Description		REO_FLUSH_TIMEOUT_LIST_4_RESERVED_4A
			
			<legal 0>
*/
#define REO_FLUSH_TIMEOUT_LIST_4_RESERVED_4A_OFFSET                  0x00000010
#define REO_FLUSH_TIMEOUT_LIST_4_RESERVED_4A_LSB                     0
#define REO_FLUSH_TIMEOUT_LIST_4_RESERVED_4A_MASK                    0xffffffff

/* Description		REO_FLUSH_TIMEOUT_LIST_5_RESERVED_5A
			
			<legal 0>
*/
#define REO_FLUSH_TIMEOUT_LIST_5_RESERVED_5A_OFFSET                  0x00000014
#define REO_FLUSH_TIMEOUT_LIST_5_RESERVED_5A_LSB                     0
#define REO_FLUSH_TIMEOUT_LIST_5_RESERVED_5A_MASK                    0xffffffff

/* Description		REO_FLUSH_TIMEOUT_LIST_6_RESERVED_6A
			
			<legal 0>
*/
#define REO_FLUSH_TIMEOUT_LIST_6_RESERVED_6A_OFFSET                  0x00000018
#define REO_FLUSH_TIMEOUT_LIST_6_RESERVED_6A_LSB                     0
#define REO_FLUSH_TIMEOUT_LIST_6_RESERVED_6A_MASK                    0xffffffff

/* Description		REO_FLUSH_TIMEOUT_LIST_7_RESERVED_7A
			
			<legal 0>
*/
#define REO_FLUSH_TIMEOUT_LIST_7_RESERVED_7A_OFFSET                  0x0000001c
#define REO_FLUSH_TIMEOUT_LIST_7_RESERVED_7A_LSB                     0
#define REO_FLUSH_TIMEOUT_LIST_7_RESERVED_7A_MASK                    0xffffffff

/* Description		REO_FLUSH_TIMEOUT_LIST_8_RESERVED_8A
			
			<legal 0>
*/
#define REO_FLUSH_TIMEOUT_LIST_8_RESERVED_8A_OFFSET                  0x00000020
#define REO_FLUSH_TIMEOUT_LIST_8_RESERVED_8A_LSB                     0
#define REO_FLUSH_TIMEOUT_LIST_8_RESERVED_8A_MASK                    0xffffffff


#endif // _REO_FLUSH_TIMEOUT_LIST_H_

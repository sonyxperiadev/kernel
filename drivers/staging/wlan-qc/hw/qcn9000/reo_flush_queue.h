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

#ifndef _REO_FLUSH_QUEUE_H_
#define _REO_FLUSH_QUEUE_H_
#if !defined(__ASSEMBLER__)
#endif

#include "uniform_reo_cmd_header.h"

// ################ START SUMMARY #################
//
//	Dword	Fields
//	0	struct uniform_reo_cmd_header cmd_header;
//	1	flush_desc_addr_31_0[31:0]
//	2	flush_desc_addr_39_32[7:0], block_desc_addr_usage_after_flush[8], block_resource_index[10:9], invalidate_queue_and_flush[11], reserved_2a[31:12]
//	3	reserved_3a[31:0]
//	4	reserved_4a[31:0]
//	5	reserved_5a[31:0]
//	6	reserved_6a[31:0]
//	7	reserved_7a[31:0]
//	8	reserved_8a[31:0]
//
// ################ END SUMMARY #################

#define NUM_OF_DWORDS_REO_FLUSH_QUEUE 9

struct reo_flush_queue {
    struct            uniform_reo_cmd_header                       cmd_header;
             uint32_t flush_desc_addr_31_0            : 32; //[31:0]
             uint32_t flush_desc_addr_39_32           :  8, //[7:0]
                      block_desc_addr_usage_after_flush:  1, //[8]
                      block_resource_index            :  2, //[10:9]
                      invalidate_queue_and_flush      :  1, //[11]
                      reserved_2a                     : 20; //[31:12]
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

flush_desc_addr_31_0
			
			Consumer: REO
			
			Producer: SW
			
			
			
			Address (lower 32 bits) of the descriptor to flush
			
			<legal all>

flush_desc_addr_39_32
			
			Consumer: REO
			
			Producer: SW
			
			
			
			Address (upper 8 bits) of the descriptor to flush
			
			<legal all>

block_desc_addr_usage_after_flush
			
			When set, REO shall not re-fetch this address till SW
			explicitly unblocked this address
			
			
			
			If the blocking resource was already used, this command
			shall fail and an error is reported
			
			
			
			<legal all>

block_resource_index
			
			Field only valid when 'Block_desc_addr_usage_after_flush
			' is set.
			
			
			
			Indicates which of the four blocking resources in REO
			will be assigned for managing the blocking of this address.
			
			<legal all>

invalidate_queue_and_flush
			
			When set, after the queue has been completely flushed,
			invalidate the queue by clearing VLD and flush the queue
			descriptor from the cache.
			
			
			
			<legal all>

reserved_2a
			
			<legal 0>

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


 /* EXTERNAL REFERENCE : struct uniform_reo_cmd_header cmd_header */ 


/* Description		REO_FLUSH_QUEUE_0_CMD_HEADER_REO_CMD_NUMBER
			
			Consumer: REO/SW/DEBUG
			
			Producer: SW 
			
			
			
			This number can be used by SW to track, identify and
			link the created commands with the command statusses
			
			
			
			
			
			<legal all> 
*/
#define REO_FLUSH_QUEUE_0_CMD_HEADER_REO_CMD_NUMBER_OFFSET           0x00000000
#define REO_FLUSH_QUEUE_0_CMD_HEADER_REO_CMD_NUMBER_LSB              0
#define REO_FLUSH_QUEUE_0_CMD_HEADER_REO_CMD_NUMBER_MASK             0x0000ffff

/* Description		REO_FLUSH_QUEUE_0_CMD_HEADER_REO_STATUS_REQUIRED
			
			Consumer: REO
			
			Producer: SW 
			
			
			
			<enum 0 NoStatus> REO does not need to generate a status
			TLV for the execution of this command
			
			<enum 1 StatusRequired> REO shall generate a status TLV
			for the execution of this command
			
			
			
			<legal all>
*/
#define REO_FLUSH_QUEUE_0_CMD_HEADER_REO_STATUS_REQUIRED_OFFSET      0x00000000
#define REO_FLUSH_QUEUE_0_CMD_HEADER_REO_STATUS_REQUIRED_LSB         16
#define REO_FLUSH_QUEUE_0_CMD_HEADER_REO_STATUS_REQUIRED_MASK        0x00010000

/* Description		REO_FLUSH_QUEUE_0_CMD_HEADER_RESERVED_0A
			
			<legal 0>
*/
#define REO_FLUSH_QUEUE_0_CMD_HEADER_RESERVED_0A_OFFSET              0x00000000
#define REO_FLUSH_QUEUE_0_CMD_HEADER_RESERVED_0A_LSB                 17
#define REO_FLUSH_QUEUE_0_CMD_HEADER_RESERVED_0A_MASK                0xfffe0000

/* Description		REO_FLUSH_QUEUE_1_FLUSH_DESC_ADDR_31_0
			
			Consumer: REO
			
			Producer: SW
			
			
			
			Address (lower 32 bits) of the descriptor to flush
			
			<legal all>
*/
#define REO_FLUSH_QUEUE_1_FLUSH_DESC_ADDR_31_0_OFFSET                0x00000004
#define REO_FLUSH_QUEUE_1_FLUSH_DESC_ADDR_31_0_LSB                   0
#define REO_FLUSH_QUEUE_1_FLUSH_DESC_ADDR_31_0_MASK                  0xffffffff

/* Description		REO_FLUSH_QUEUE_2_FLUSH_DESC_ADDR_39_32
			
			Consumer: REO
			
			Producer: SW
			
			
			
			Address (upper 8 bits) of the descriptor to flush
			
			<legal all>
*/
#define REO_FLUSH_QUEUE_2_FLUSH_DESC_ADDR_39_32_OFFSET               0x00000008
#define REO_FLUSH_QUEUE_2_FLUSH_DESC_ADDR_39_32_LSB                  0
#define REO_FLUSH_QUEUE_2_FLUSH_DESC_ADDR_39_32_MASK                 0x000000ff

/* Description		REO_FLUSH_QUEUE_2_BLOCK_DESC_ADDR_USAGE_AFTER_FLUSH
			
			When set, REO shall not re-fetch this address till SW
			explicitly unblocked this address
			
			
			
			If the blocking resource was already used, this command
			shall fail and an error is reported
			
			
			
			<legal all>
*/
#define REO_FLUSH_QUEUE_2_BLOCK_DESC_ADDR_USAGE_AFTER_FLUSH_OFFSET   0x00000008
#define REO_FLUSH_QUEUE_2_BLOCK_DESC_ADDR_USAGE_AFTER_FLUSH_LSB      8
#define REO_FLUSH_QUEUE_2_BLOCK_DESC_ADDR_USAGE_AFTER_FLUSH_MASK     0x00000100

/* Description		REO_FLUSH_QUEUE_2_BLOCK_RESOURCE_INDEX
			
			Field only valid when 'Block_desc_addr_usage_after_flush
			' is set.
			
			
			
			Indicates which of the four blocking resources in REO
			will be assigned for managing the blocking of this address.
			
			<legal all>
*/
#define REO_FLUSH_QUEUE_2_BLOCK_RESOURCE_INDEX_OFFSET                0x00000008
#define REO_FLUSH_QUEUE_2_BLOCK_RESOURCE_INDEX_LSB                   9
#define REO_FLUSH_QUEUE_2_BLOCK_RESOURCE_INDEX_MASK                  0x00000600

/* Description		REO_FLUSH_QUEUE_2_INVALIDATE_QUEUE_AND_FLUSH
			
			When set, after the queue has been completely flushed,
			invalidate the queue by clearing VLD and flush the queue
			descriptor from the cache.
			
			
			
			<legal all>
*/
#define REO_FLUSH_QUEUE_2_INVALIDATE_QUEUE_AND_FLUSH_OFFSET          0x00000008
#define REO_FLUSH_QUEUE_2_INVALIDATE_QUEUE_AND_FLUSH_LSB             11
#define REO_FLUSH_QUEUE_2_INVALIDATE_QUEUE_AND_FLUSH_MASK            0x00000800

/* Description		REO_FLUSH_QUEUE_2_RESERVED_2A
			
			<legal 0>
*/
#define REO_FLUSH_QUEUE_2_RESERVED_2A_OFFSET                         0x00000008
#define REO_FLUSH_QUEUE_2_RESERVED_2A_LSB                            12
#define REO_FLUSH_QUEUE_2_RESERVED_2A_MASK                           0xfffff000

/* Description		REO_FLUSH_QUEUE_3_RESERVED_3A
			
			<legal 0>
*/
#define REO_FLUSH_QUEUE_3_RESERVED_3A_OFFSET                         0x0000000c
#define REO_FLUSH_QUEUE_3_RESERVED_3A_LSB                            0
#define REO_FLUSH_QUEUE_3_RESERVED_3A_MASK                           0xffffffff

/* Description		REO_FLUSH_QUEUE_4_RESERVED_4A
			
			<legal 0>
*/
#define REO_FLUSH_QUEUE_4_RESERVED_4A_OFFSET                         0x00000010
#define REO_FLUSH_QUEUE_4_RESERVED_4A_LSB                            0
#define REO_FLUSH_QUEUE_4_RESERVED_4A_MASK                           0xffffffff

/* Description		REO_FLUSH_QUEUE_5_RESERVED_5A
			
			<legal 0>
*/
#define REO_FLUSH_QUEUE_5_RESERVED_5A_OFFSET                         0x00000014
#define REO_FLUSH_QUEUE_5_RESERVED_5A_LSB                            0
#define REO_FLUSH_QUEUE_5_RESERVED_5A_MASK                           0xffffffff

/* Description		REO_FLUSH_QUEUE_6_RESERVED_6A
			
			<legal 0>
*/
#define REO_FLUSH_QUEUE_6_RESERVED_6A_OFFSET                         0x00000018
#define REO_FLUSH_QUEUE_6_RESERVED_6A_LSB                            0
#define REO_FLUSH_QUEUE_6_RESERVED_6A_MASK                           0xffffffff

/* Description		REO_FLUSH_QUEUE_7_RESERVED_7A
			
			<legal 0>
*/
#define REO_FLUSH_QUEUE_7_RESERVED_7A_OFFSET                         0x0000001c
#define REO_FLUSH_QUEUE_7_RESERVED_7A_LSB                            0
#define REO_FLUSH_QUEUE_7_RESERVED_7A_MASK                           0xffffffff

/* Description		REO_FLUSH_QUEUE_8_RESERVED_8A
			
			<legal 0>
*/
#define REO_FLUSH_QUEUE_8_RESERVED_8A_OFFSET                         0x00000020
#define REO_FLUSH_QUEUE_8_RESERVED_8A_LSB                            0
#define REO_FLUSH_QUEUE_8_RESERVED_8A_MASK                           0xffffffff


#endif // _REO_FLUSH_QUEUE_H_

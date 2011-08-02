/*****************************************************************************
* Copyright 2011 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a
* license other than the GPL, without Broadcom's express prior written
* consent.
*****************************************************************************/

#ifndef __VC_SM_DEFS_H__INCLUDED__

// FourCC code used for VCHI connection
#define VC_SM_SERVER_NAME MAKE_FOURCC("SMEM")

// Maximum message length
#define VC_SM_MAX_MSG_LEN (sizeof( VC_SM_MSG_UNION_T ) + \
                           sizeof( VC_SM_MSG_HDR_T ))

// Resource name maximum size
#define VC_SM_RESOURCE_NAME 32

// All message types supported for HOST->VC direction
typedef enum
{
   VC_SM_MSG_TYPE_ALLOC,               // Allocate shared memory block
   VC_SM_MSG_TYPE_LOCK,                // Lock allocated shared memory block
   VC_SM_MSG_TYPE_UNLOCK,              // Unlock allocated shared memory block
   VC_SM_MSG_TYPE_FREE,                // Free shared memory block
   VC_SM_MSG_TYPE_RESIZE,              // Resize a shared memory block
   VC_SM_MSG_TYPE_WALK_ALLOC,          // Walk the allocated shared memory block(s)
   VC_SM_MSG_TYPE_MAX

} VC_SM_MSG_TYPE;

// Type of memory to be allocated
typedef enum
{
   VC_SM_ALLOC_CACHED,        // cached memory
   VC_SM_ALLOC_NON_CACHED,    // non cached memory

} VC_SM_ALLOC_TYPE_T;

// Message header for all messages in HOST->VC direction
typedef struct
{
   int32_t type;     // Message type (VC_SM_MSG_TYPE)
   uint8_t body[0];  // Pointer to message body (if exists)

} VC_SM_MSG_HDR_T;

// Request to allocate memory (HOST->VC)
typedef struct
{
   VC_SM_ALLOC_TYPE_T type; // type of memory to allocate
   uint32_t base_unit;      // byte amount of data to allocate per unit
   uint32_t num_unit;       // number of unit to allocate
   uint32_t alignement;     // alignement to be applied on allocation
   char     name[VC_SM_RESOURCE_NAME]; // resource name (for easier tracking on vc side)

} VC_SM_ALLOC_T;

// Result of a requested memory allocation (VC->HOST)
typedef struct
{
   uint32_t res_handle;    // Resource handle
   void    *res_mem;       // Pointer to resource buffer
   uint32_t res_base_size; // Resource base size (bytes)
   uint32_t res_num;       // Resource number

} VC_SM_ALLOC_RESULT_T;

// Request to free a previously allocated memory (HOST->VC)
typedef struct
{
   uint32_t res_handle;    // Resource handle (returned from alloc)
   void     *res_mem;      // Resource buffer (returned from alloc)

} VC_SM_FREE_T;

// Request to lock a previously allocated memory (HOST->VC)
typedef struct
{
   uint32_t res_handle;    // Resource handle (returned from alloc)
   void     *res_mem;      // Resource buffer (returned from alloc)

} VC_SM_LOCK_UNLOCK_T;

// Request to resize a previously allocated memory (HOST->VC)
typedef struct
{
   uint32_t res_handle;    // Resource handle (returned from alloc)
   void     *res_mem;      // Resource buffer (returned from alloc)
   uint32_t res_new_size;  // Resource *new* size requested (bytes)

} VC_SM_RESIZE_T;

// Result of a requested memory lock (VC->HOST)
typedef struct
{
   uint32_t res_handle;    // Resource handle
   void    *res_mem;       // Pointer to resource buffer
   void    *res_old_mem;   // Pointer to former resource buffer if the memory
                           // was reallocated

} VC_SM_LOCK_RESULT_T;

// Generic result for a request (VC->HOST)
typedef struct
{
   int32_t success;  // Success value

} VC_SM_RESULT_T;

// Union of ALL messages
typedef union
{
   VC_SM_ALLOC_T         alloc;
   VC_SM_ALLOC_RESULT_T  alloc_result;
   VC_SM_FREE_T          free;
   VC_SM_RESULT_T        result;
} VC_SM_MSG_UNION_T;

#endif /* __VC_SM_DEFS_H__INCLUDED__ */


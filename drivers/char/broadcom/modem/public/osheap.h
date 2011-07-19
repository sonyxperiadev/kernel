/****************************************************************************
*
*     Copyright (c) 2004-2009 Broadcom Corporation
*           All Rights Reserved
*
*     No portions of this material may be reproduced in any form without the
*     written permission of:
*
*           Broadcom Corporation
*           16215 Alton Parkway
*           P.O. Box 57013
*           Irvine, California 92619-7013
*
*     All information contained in this document is Broadcom Corporation
*     company private, proprietary, and trade secret.
*
****************************************************************************/
/**
*
*   @file   osheap.h
*
*   @brief  This file contains the interface to the HEAP functionality.
*
****************************************************************************/
/**

*   @defgroup   RTOSHEAPGroup   Dynamic Memory Services
*   @ingroup    RTOSGroup
*
*   @brief      This group defines the interfaces to the dynamic memory system.
*
*	The Heap API provides interfaces to allocate and deallocate memory.  The
	platforms dynamic memory system consists of partitions of fixed size blocks 
	and a Heap.  The number of partitions and the size of blocks within a
	partition are configurable via the include file osheap_config.h.  When an 
	allocation request is made the Heap API uses a best fit strategy to satisfy 
	the request.  It will select the partition whose block size most closely matches 
	the allocation request and if no partition is available it will allocate memory 
	from the Heap rather than a partition.  The useage of partitions is very efficient 
	and eliminates the problem of fragementing the Heap.  
*
****************************************************************************/

#ifndef _INC_OSHEAP_H_
#define _INC_OSHEAP_H_

//#include "mobcom_types.h"
//#include "consts.h"
#include "ostypes.h"

#ifdef	__cplusplus
extern "C"
{
#endif

#define	OSPARTITIONPOOL_DEBUG_ID					0x504D4447	//PMDG
#define	OSMEMORYPOOL_DEBUG_ID						0x444D4447	//DMDG

typedef struct {
    void             *pm_next_available;     /* Next available memory  */
                                             /*   partition            */
    void             *pm_partition_pool;     /* Partition pool pointer */
    UInt32            pm_dbg_time;           /* Last alloc/free time   */
    void             *pm_dbg_alloc_task;     /* Last alloc task TCB    */
    char             *pm_dbg_alloc_file;     /* Last alloc file name   */
    UInt16            pm_dbg_alloc_line;     /* Last alloc line number */
    UInt16            pm_dbg_free_line;      /* Last free line number  */
    char             *pm_dbg_free_file;      /* Last free file name    */
    void             *pm_dbg_free_task;      /* Last free task TCB     */
    UInt32            size;                  /* Requested size         */
}  OSHEAP_DEBUG_DATA;	// first 8 words are identical to PM_HEADER. Hui Luo, 8/25/08

typedef enum
{
	OSHEAP_OP_ALLOC,
	OSHEAP_OP_FREE
} OSHEAP_OP_TYPE;

#define	PARTITION_OVERFLOW_FLAG	0x4F56


typedef void* Heap_t;


/**
 * @addtogroup RTOSHEAPGroup
 * @{
 */
typedef void (OSHEAP_HOOK_FNC) (OSHEAP_OP_TYPE opType, OSHEAP_DEBUG_DATA *debugData); ///< Callback function to be invoked for each OSHEAP alloc/dealloc


/**
	This function registers a callback function to be invoked for each OSHEAP alloc/dealloc
	through partition pool. After a callback function is registered, the debug data for 
	the particular parition pool element is returned to the callback function. 

	@param		hookFunc (in) callback function. 	
	@return		The original callback function (NULL if no callback is ever registered). 
**/
OSHEAP_HOOK_FNC *OSHEAP_RegisterHookFunc(OSHEAP_HOOK_FNC *hookFunc);


/**
	This function returns the debug data of a particular partition pool element. 

	@param		poolNum (in) 0-based pool index
	@param		bufNum (in) 0-based index of the partition pool element
	@param		bufDebugData (out) pointer to the debug data buffer
	@return		None
**/
void OSHEAP_GetBufStatus(UInt16 poolNum, UInt32 bufNum, OSHEAP_DEBUG_DATA *bufDebugData);


/**
	This function returns the "size of element", "number of elements allocated" and "total
	number of elements" parameters of a particular partition pool. 

	@param		poolNum (in) 0-based pool index
	@param		poolBufSize (out) pointer to "size of element" variable
	@param		numAllocated (out) pointer to "number of elements allocated" variable
	@param		numTotal (out) pointer to "total number of elements" variable
	@return		None
**/
void OSHEAP_GetPoolStatus(UInt16 poolNum, UInt32 *poolBufSize, UInt32 *numAllocated, UInt32 *numTotal);


//******************************************************************************
// Global Macros
//******************************************************************************

//******************************************************************************
// Handy macros to allocate and deallocate memory
//******************************************************************************

/// The C++ operators new and delete map to the RTOS Heap allocation and 
/// deallocation functions respectively.
#ifndef __cplusplus
#define new(type)					((type *)OSHEAP_Alloc(sizeof(type)))
#define delete(ptr)					(OSHEAP_Delete(ptr))
#endif	// _cplusplus

/// Allocate memory and assert if there is a failure.
#define ALLOC_ASSERT(ptr, type)		(ptr) = ((type *)OSHEAP_Alloc(sizeof(type))); assert( (ptr) != NULL )


//******************************************************************************
// Global Function Prototypes
//******************************************************************************

/** 
	Initializes the Application and System Heap.  The include file osheap_config.h
	specifies the configuration and sizes of the partitions and heap.
	@param suspend_mode	(in) Suspend mode in which the caller will be suspended if allocation fails
**/

void OSHEAP_Init( 						// Creates and initializes the heap
	OSSuspend_t	suspend_mode
	);

/**
	Dumps out partition and heap statistics to the logging port.
**/

void OSHEAP_DumpHeapSize(void);

/// Macro used to allocate memory from the system heap; this function will always return immediately 
/// no matter the allocation is successful or not.  This macro wraps the HEAP allocation function
/// (no suspend) so that the file name and line number where the allocation occurs can be record.

#define OSHEAP_Alloc(x)	dbHEAP_Alloc( x, __FILE__, __LINENUM__ )

/// Macro used to allocate memory from the system heap; the caller will be suspended until the specified
/// timeout if the allocation fails.  This macro wraps the HEAP allocation function (suspend possible)
/// so that the file name and line number where the allocation occurs can be record.

#ifndef WIN32
#define OSHEAP_AllocSuspend(x, t)	dbHEAP_AllocSuspend( x, t, __FILE__, __LINENUM__ )
#endif

/// Macro used to deallocate memory from the system heap.  The macro wraps the HEAP deallocation function
/// so that the file name and line number where the deallocation occurs can be record.

#define OSHEAP_Delete(x)	dbHEAP_Delete( x, __FILE__, __LINENUM__ )

/// Macro used to allocate memory from the system heap; this function will always return immediately 
/// no matter the allocation is successful or not.  This macro wraps the HEAP allocation function
/// (no suspend) so that the file name and line number where the allocation occurs can be record.

#define OSHEAP_AllocPhys(x, y)	dbHEAP_AllocPhys( x, y, __FILE__, __LINENUM__ )

/// Macro used to deallocate memory from the system heap.  The macro wraps the HEAP deallocation function
/// so that the file name and line number where the deallocation occurs can be record.

#define OSHEAP_DeletePhys(x)	dbHEAP_DeletePhys( x, __FILE__, __LINENUM__ )

/**
	Heap Allocation Function (no suspend)
	@param size		(in) Number of bytes to allocate
	@param *file	(in) Source File name
	@param line		(in) Line number in source file where allocation occurs
	@return void*	A pointer to the allocated block
**/

void *dbHEAP_Alloc( 
   UInt32   size, 		// Number of bytes to allocate
   char		*file,
   UInt32	line
);

#ifndef WIN32
/**
	Heap Allocation Function (suspend possible)
	@param size			(in) Number of bytes to allocate
	@param timeout		(in) Timeout value for suspension if allocation fails
	@param *file		(in) Source File name
	@param line			(in) Line number in source file where allocation occurs
	@return void*		A pointer to the allocated block
**/

void *dbHEAP_AllocSuspend( 
   UInt32   	size,
   Ticks_t		timeout,
   char			*file,
   UInt32		line
);
#endif // #ifndef WIN32

/**
	Heap dellocation Function
	@param *ptr		(in) Pointer to memory block to deallocate.
	@param *file	(in) Source File name
	@param line		(in) Line number in source file where deallocation occurs
**/

void dbHEAP_Delete(
   void      *ptr,		// Pointer to block to deallocate
   char		*file,
   UInt32	line
);

/**
	Heap Allocation Function (to get physically contiguous non-cacheable memory)
	@param size			(in) Number of bytes to allocate
	@param *phys_addr	(out) Physical address
	@param *file		(in) Source File name
	@param line			(in) Line number in source file where allocation occurs
	@return void*		A pointer to the allocated block
**/

void *dbHEAP_AllocPhys( 
   UInt32   size, 			// Number of bytes to allocate
   UInt32	*phys_addr,		// Physical address
   char		*file,
   UInt32	line
);

/**
	Heap dellocation Function (to free the memory allocated by OSHEAP_AllocPhys())
	@param *ptr		(in) Pointer to memory block to deallocate.
	@param *file	(in) Source File name
	@param line		(in) Line number in source file where deallocation occurs
**/

void dbHEAP_DeletePhys(
   void		*ptr,		// Pointer to block to deallocate
   char		*file,
   UInt32	line
);

#ifndef WIN32
/**
	Heap suspend mode change function; this function can only be used when there is no task being 
	suspended due to memory allocation failure, otherwise an error will be returned.
	@param suspend_mode	(in) Suspend mode in which the caller will be suspended if allocation fails
	@return OSStatus_t	(out) Success or fail 
**/

OSStatus_t OSHEAP_ChangeSuspendMode(
   OSSuspend_t	suspend_mode
);
#endif // #ifndef WIN32

/**
	Return the number of blocks of the specified size that is left on the heap.
	@param block_size		(in) Block size
	@return UInt32	Number of blocks of the specified size that is left on the heap
**/

UInt32 OSHEAP_GetNumBlocks(
	UInt32	block_size						   
);

/**
	Checks if the specified pointer points to the heap or not.
	@param p		(in) Pointer to be checked
	@return Boolean	True if the specific pointer points inside the heap memory space, false otherwise
**/

Boolean OSHEAP_PoolCheck(
	void *p
);

//******************************************************************************
// The following set of routines for used with Tracing (debugging)
// Do not use for other reason.
//******************************************************************************

// Macros to handle legacy code.

#define HEAP_AllocTraceMem	OSHEAP_AllocTraceMem
#define HEAP_DeleteTraceMem	OSHEAP_DeleteTraceMem

/**
	Initialize heap that will be used for trace memory.
	@param HeapStart	(in) Pointer to start of trace memory
	@param HeapSize		(in) Number of bytes to allocate for trace memory
**/

void OSHEAP_InitTraceMem( 				// Initialize trace heap memory
	void   *HeapStart,   				// Pointer to start of heap
	UInt32 HeapSize						// Number of bytes in heap
	);

/**
	Allocate memory from the trace heap.
	@param size	(in) Number of bytes to allocate
	@return void*	Pointer to allocated block
**/

#ifdef HISTORY_LOGGING
#define	OSHEAP_AllocTraceMem(size)	OSHEAP_AllocTraceMemDebug(size, __FILE__, __LINENUM__)
void *OSHEAP_AllocTraceMemDebug(		// Allocates N bytes from the heap
	UInt32 size,	 					// Number of bytes to allocate
	char *file,							// name of file from which OSHEAP_AllocTraceMem() is called
	UInt32 line							// line number of above
	);
#else // #ifdef HISTORY_LOGGING
void *OSHEAP_AllocTraceMem(				// Allocates N bytes from the heap
	UInt32 size	 						// Number of bytes to allocate
	);
#endif // #ifdef HISTORY_LOGGING

/** 
	Deallocate memory allocated from the trace heap.
	@param	ptr	(in) Pointer to memory block to deallocate
**/

#ifdef HISTORY_LOGGING
#define	OSHEAP_DeleteTraceMem(ptr)	OSHEAP_DeleteTraceMemDebug(ptr, __FILE__, __LINENUM__)
void OSHEAP_DeleteTraceMemDebug(		// Deallocates the block pointed to by ptr
	void *ptr,							// Pointer to block to deallocate
	char *file,							// name of file from which OSHEAP_DeleteTraceMem() is called
	UInt32 line							// line number of above
	);
#else // #ifdef HISTORY_LOGGING
void OSHEAP_DeleteTraceMem(				// Deallocates the block pointed to by ptr
	void *ptr							// Pointer to block to deallocate
	);
#endif // #ifdef HISTORY_LOGGING

/** @} */




/* dedicated memory pool API */
extern Heap_t uncached_mem_heap;
extern Heap_t phy_to_sl1_mem_heap;
extern Heap_t cphy_to_sl1_mem_heap;
void OSHEAP_InitDedicatedMem(Heap_t , char* , char* , UInt32 , UInt32 , Boolean );
void OSHEAP_DeleteDedicatedMem(Heap_t );
#ifdef HISTORY_LOGGING
#define OSHEAP_AllocDedicatedMem(h, s) OSHEAP_AllocDedicatedMemDebug(h, s, __FILE__, __LINENUM__)
void* OSHEAP_AllocDedicatedMemDebug(Heap_t , UInt32 , char *, UInt32 );
#define OSHEAP_FreeDedicatedMem(h, p) OSHEAP_FreeDedicatedMemDebug(h, p, __FILE__, __LINENUM__)
void OSHEAP_FreeDedicatedMemDebug(Heap_t ,	void *,	char *,	UInt32 );
#else
void* OSHEAP_AllocDedicatedMem(Heap_t , UInt32 );
void OSHEAP_FreeDedicatedMem(Heap_t ,	void * );
#endif
UInt32 OSHEAP_GetAvailableDedicatedMem(Heap_t );


/* uncached memory pool API */
#define OSHEAP_InitUncachedMem() \
	OSHEAP_InitDedicatedMem(uncached_mem_heap, "UNCACHED", (char* )uncached_pool_space, \
	OSHEAP_UNCACHED_MEMPOOL_SIZE, 8, TRUE)
#define OSHEAP_AllocUncachedMem(size) \
	OSHEAP_AllocDedicatedMem(uncached_mem_heap, size)
#define OSHEAP_DeleteUncachedMem(ptr) \
	OSHEAP_FreeDedicatedMem(uncached_mem_heap, ptr)



char* OSHEAP_GetMemProfile(void);

void OSHEAP_FastDumpHeapSize(void);



#ifdef	__cplusplus
}
#endif

#endif


/***************************************************************************
 *     Copyright (c) 2003-2008, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *  CHAL kernel abstration interface.
 *
 * Revision History:
 *
 * $brcm_Log:  $
 * 
 ***************************************************************************/
#ifndef CHAL_OSABS_H__
#define CHAL_OSABS_H__

#ifdef IGNORE_CHAL_OSABS
#else

#include <mach/csp/chal_std.h>
#include <mach/csp/bcmdbg.h>
#include <mach/csp/bcmerr.h>


#ifdef __cplusplus
extern "C" {
#endif

/*====================== Module Overview ===========================

---++++ *Summary:*

The Kernel Interface is the set of operating system and standard C library functions
required by the architecture which is portable across all software
and hardware platforms which Broadcom uses.


---++++ *Requirements:*

*Portable* - every function must work on every platform in the same well-defined way.
Some error conditions cannot or should not be standardized, therefore we also include
usage rules (see chal_enter_critical_section for an example.)
Every platform implementation must be regularly checked with compliance tests.

*Safe* - Because of the inherent dangers of multitasking,
each kernel interface implementation must ensure there are no race conditions within the
implementation.
The API must also be simple and explicit in order to help developers avoid race
conditions in their code.

*Minimal* - The number of functions should be just enough to allow us to implement the Magnum
architecture. Platform-dependent code should make use of platform-specific functions.


---++++ *Files:*

The kernel interface is divided into three sections:

*bkni.h* is the single-task kernel interface. It is used by code which follows the single-task
model of execution specified in ThreadSafe.

*chal_multi.h* is the multi-tasking kernel interface. It is used by code which follows 
the multi-task model of execution specified in ThreadSafe. chal_multi.h also includes
bkni.h.

*chal_metrics.h* is the metrics interface. It must not be called by code, but
provides system monitoring and debugging to platform-specific code.


<nopublish>
---++++ *Summary of Changes:*

New naming convention.

Replaced structures with opaque handles.

Init and Uninit functions.

Replaced semaphores with mutexes. While a semaphore can be configured as a mutex, it can
easily be misconfigured and introduce race conditions. The combination of a mutex and an event
is simple and sufficient.

Removed sprintf and vsprintf because they introduce possible buffer overruns. snprintf
and vsnprintf are sufficient.

Memory map functionality (MmuMap and UnmmuMap) was moved to the MemoryManager.

Removed debug macros (DBGMSG) and data types (WORD, HANDLE, etc.).
</nopublish>


---++++ *Interrupt Safety:*

The following functions can be called from an ISR or a critical section:

	* chal_memset, chal_memcpy, chal_memcmp, chal_memchr, chal_memmove
	* chal_printf, chal_vprintf
	* chal_delay
	* chal_set_event
	* chal_wait_for_event, but only with a timeout of zero.

All other functions are not callable from either an ISR or critical section.
*****************************************************************************/

/***************************************************************************
Summary:
	Initialize the kernel interface before use.

Description:
	The system must call chal_osabs_init() before making any other kernel interface call.
	code cannot call chal_osabs_init().

	The chal_osabs_init call should reset the metrics interface. See chal_Metrics_Reset.

Returns:
	BCM_SUCCESS - The kernel interface successfully initialized.
	BCM_ERROR - Initialization failed.
****************************************************************************/
BCM_ERR_CODE chal_osabs_init(void);


/***************************************************************************
Summary:
	Uninitialize the kernel interface after use.

Description:
	Cleans up the kernel interface. No kernel interface calls can be made after this,
	except chal_osabs_init().
	code cannot call chal_osabs_cleanup().

Returns:
	<none>
****************************************************************************/
void chal_osabs_cleanup(void);


/***************************************************************************
Summary:
	Set byte array to a value.

Description:
	Copies the value of ch (converted to an unsigned char) into each of the first n
	characters of the memory pointed to by mem.

	Can be called from an interrupt-context.

Input:
	mem - memory to be set
	ch - 8 bit value to be copied into memory
	n - number of bytes to be copied into memory

Returns:
 	The value of mem
****************************************************************************/
void *chal_memset(void *mem, int ch, size_t n);


/***************************************************************************
Summary:
	Copy non-overlapping memory.

Description:
	Copies n characters from the object pointed to by src into the object pointed
	to by dest. 
	
	If copying takes place between objects that overlap, the
	behavior is undefined. Use chal_memmove instead.

	Can be called from an interrupt-context.

Input:
	dest - the destination byte array
	src - the source byte array
	n - number of bytes to copy

Returns:
	The value of dest
****************************************************************************/
void *chal_memcpy(void *dest, const void *src, size_t n);


/***************************************************************************
Summary:
	Compare two blocks of memory.

Description:
	Compares the first n characters of the object pointed to by s1 to the first n
	characters of the object pointed to by s2.

	Can be called from an interrupt-context.

Returns:
	An integer greater than, equal to, or less than zero, accordingly as the object
	pointed to by s1 is greater than, equal to, or less than the object pointed to by s2.
****************************************************************************/
int chal_memcmp(
	const void *s1, 	/* byte array to be compared */
	const void *s2,		/* byte array to be compared */
	size_t n			/* maximum number of bytes to be compared */
	);


/***************************************************************************
Summary:
	Find a byte in a block of memory.

Description:
	Locates the first occurrence of ch (converted to an unsigned char) in the initial n
	characters (each interpreted as unsigned char) of the object pointed to by mem.

	Can be called from an interrupt-context.

Input:
	mem - byte array to be searched
	ch - 8 bit value to be searched for
	n - maximum number of bytes to be searched

Returns:
	A pointer to the located character, or a null pointer if the character does not
	occur in the object.
****************************************************************************/
void *chal_memchr(const void *mem, int ch, size_t n);


/***************************************************************************
Summary:
	Copy potentially overlapping memory.

Description:
	Copies n characters from the object pointed to by src into the object pointed
	to by dest. Copying takes place as if the n characters from the object pointed
	to by src are first copied into a temporary array of n characters that does
	not overlap the objects pointed to by dest and src, and then the n characters
	from the temporary array are copied into the object pointed to by dest.
	
	If the memory does not overlap, chal_memcpy is preferred.

	Can be called from an interrupt-context.

Returns:
	The value of dest
****************************************************************************/
void *chal_memmove(
	void *dest, 		/* destination byte array */
	const void *src, 	/* source byte array */
	size_t n			/* number of bytes to copy */
	);


/***************************************************************************
Summary:
	Print characters to the console.

Description:
	Although printing to the console is very important for development, it cannot
	and should not be guaranteed to actually print in all contexts.
	It is valid for the system developer to eliminate all chal_printf output in
	release builds or if the context demands it (e.g. interrupt context).

	You should use chal_printf instead of
	DebugInterface when you explicity want to print information to a console
	regardless of debug state (e.g. BXPT_PrintStatus, BPSI_PrintPsiInformation).
	chal_printf is also used by the generic DebugInterface implementation.

	We only guarantee a subset of ANSI C format specifiers. These include:

	* %d  - int in decimal form
	* %u  - unsigned int in decimal form
	* %ld - long in decimal form
	* %lu - unsigned long in decimal form
	* %x  - unsigned int in lowercase hex form
	* %lx - unsigned long in lowercase hex form
	* %X  - unsigned int in uppercase hex form
	* %lX - unsigned long in uppercase hex form
	* %c  - an int argument converted to unsigned char
	* %s  - string
	* \n  - newline
	* \t  - tab
	* %%  - % character
	* %03d - Zero padding of integers, where '3' and 'd' are only examples. This can be applied to any of the preceding numeric format specifiers (not %c or %s).
	* Pass-through of non-control characters.

	Beyond these, we do not guarantee the output format.

	For chal_printf and chal_vprintf, other ANSI C format specifiers
	may be used, and platforms should try to make sure that any combination of formats
	and parameters will not cause a system crash.

	When calling chal_Snprint and chal_vsnprintf, code must only use the
	guaranteed format specifiers if the results must always be the same on all platforms.

	chal_printf can be called from an interrupt-context.

Returns:
 	>=0 is success. It is the number of characters transmitted.
	<0 is failure, either in encoding or in outputing.
****************************************************************************/
int chal_printf(
	const char *fmt, /* format string */
	...					/* variable arguments */
	);


/***************************************************************************
Summary:
	Print characters to a null-terminated string.

Description:
	See chal_printf for a description of the format specifiers supported.

	Can be called from an interrupt-context.

Returns:
	If the output is not truncated, it returns the number of characters printed, not
	including the trailing null byte.

	If the output is truncated, it should try to return the number
	of characters that would have been printed had the size of memory been large
	enough. However, this result is not required and no code should
	depend on this result.
****************************************************************************/
int chal_snprintf(
	char *s, 			/* destination string */
	size_t n,			/* size of memory that can be used. It should include
							space for the trailing null byte. */
	const char *fmt, 	/* format string */
	...					/* variable arguments */
	);


/***************************************************************************
Summary:
	Print characters to the console using a variable argument list.

Description:
	Equivalent to chal_printf, with the variable argument list replaced by the va_list
	parameter. va_list must initialized by the va_start macro (and possibly
	subsequent va_arg calls). The chal_vprintf function does not invoke the va_end macro.

	The value of the va_list parameter may be modified and so it is indeterminate upon return.

	See chal_printf for a description of the format specifiers supported.

	Can be called from an interrupt-context.

Input:
	fmt - See chal_printf
	va_list - See StandardTypes and stdarg.h

Returns:
 	>=0 is success. It is the number of characters transmitted.
	<0 is failure, either in encoding or in outputing.
****************************************************************************/
int chal_vprintf(const char *fmt, va_list ap);


/***************************************************************************
Summary:
	Print characters to a null-terminated string using a variable argument list.

Description:
	See chal_printf for a description of the format specifiers supported.
	See chal_vprintf for a description of the va_list parameter.

	Can be called from an interrupt-context.

Input:
	s - memory to print into
	n - size of memory that can be used. It should include space for the trailing null byte.
	fmt - See chal_printf
	va_list - See StandardTypes and stdarg.h

Returns:
	If the output is not truncated, it returns the number of characters printed, not
	including the trailing null byte.

	If the output is truncated, it should try to return the number
	of characters that would have been printed had the size of memory been large
	enough. However, this result is not required and no code should
	depend on this result.
****************************************************************************/
int chal_vsnprintf(char *s, size_t n, const char *fmt, va_list ap);


/***************************************************************************
Summary:
	Allocate system memory.

Description:
	Allocates space for an object whose size is specified by size and whose
	value is indeterminate.

	System memory is usually managed by an operating system. It differs
	from memory managed by a MemoryManager in that it is not
	guaranteed to be physically continuous and you cannot request alignment.

	Passing a size of 0 is not allowed and leads to undefined behavior.

	The caller is responsible to also call chal_free to free the memory
	when done. Memory that is not explicitly freed
	may or may not remain allocated beyond the life-cycle of a particular
	application.

Returns:
	NULL - Memory could not be allocated
	Non-NULL - Memory was allocated
****************************************************************************/
void *chal_malloc(
	size_t size		/* Number of bytes to allocate */
	);


/***************************************************************************
Summary:
	Dellocate system memory.

Description:
	Causes the memory pointed to by mem to be deallocated, that is, made available for
	further allocation.

	The following scenarios are not allowed and lead to undefined behavior:
	
	* Passing a pointer which was not returned by an earlier chal_malloc call
	* Passing a pointer which was already freed
	* Passing NULL

Returns:
 	<none>
****************************************************************************/
void chal_free(
	void *mem			/* Pointer to memory allocated by chal_malloc */
	);


/***************************************************************************
Summary:
	Cause the application or system to abort immediately if possible.
	On platforms that support it, the system state should be captured.

Description:
	This is called from a failed BCM_ASSERT() (see DebugInterface).

	Can be called from an interrupt-context.

	There is no required behavior for this function. It can be completely
	empty. There is no need to print an error message from inside chal_fail
	because the DebugInterface will have printed something before calling
	it.

See Also:
	DebugInterface, BCM_ASSERT
****************************************************************************/
void chal_fail(void);


/***************************************************************************
Summary:
	Busy sleep.

Description:
	chal_delay is a busy sleep which guarantees you will delay for at least the specified
	number of microseconds. It does not call the scheduler, therefore the Delay is able to be
	less than the system clock time. This consumes CPU time, so it should be used for only
	short sleeps and only when chal_sleep cannot be used.

	Be aware that on a preemptive system, any task can be interrupted and the scheduler can
	run, and so there is no guarantee of maximum delay time. If you have maximum time
	constraints, you should be using an interrupt.

	Can be called from an interrupt-context.

Input:
	microsec - minimum number of microseconds to delay

Returns:
	<none>
****************************************************************************/
void chal_delay(unsigned int microsec);


/***************************************************************************
Summary:
	Yield the current task to the scheduler.

Description:
	chal_sleep is a sheduler sleep which guarantees you will delay for at least the
	specified number of milliseconds. It puts the process to sleep and allows the scheduler
	to run. The minimum sleep time is dependent on the system clock time. If you need
	a minimum time which is less that the system clock time, you'll need to use chal_delay.

	Actual sleep time is dependent on the scheduler but will be at least as long as
	the value specified by the millisec parameter.

	A sleep value of 0 should cause the scheduler to execute. This may or may not result in
	any delay.

	chal_sleep cannot be called from an interrupt context. Use chal_delay instead.

Returns:
 	BCM_SUCCESS - The system slept for at least the specified number of milliseconds.
 	BCM_ERROR - The sleep was interrupted before the specified time.
****************************************************************************/
BCM_ERR_CODE chal_sleep(
	unsigned int millisec	/* minimum number of milliseconds to sleep */
	);


/***************************************************************************
Summary:
	Create a critical section which protects against concurrent execution by
	another critical section or ISR (interrupt service routine).
 
Description:
	A critical section is defined as a block of code between a chal_enter_critical_section
	call and a chal_exit_critical_section call or any code inside a ISR (distinguished with an _isr suffix). 
	Be aware that a critical section may not mean the same thing as an operating system's critical
	section. Typically, operating system critical sections mean that interrupts are disabled and
	no context switch is possible.
	A critical section may or may not mean this, depending on the implementation of KNI.
	
	critical sections cannot be preempted by other critical sections. 
	This includes both interrupts and context switching. But it only applies to code, not
	to code outside of Magnum. While there are many ways these rules can be implemented in a system, the recommended 
	approach is as following:
	* If your system executes Magnum-isr code in interrupt context, critical sections must be implemented by disabling interrupts. 
	* If your system executes Magnum-isr code in task context, critical sections must be implemented with a global mutex.
	
	Be aware that on task-context only systems, critical sections may not prevent the scheduler
	from time-slicing and executing non-critical section code concurrently with the
	critical section.
	
	Critical sections are also used to protect concurrent access to a register shared
	with another module. Ideally, there are no registers shared between disconnected
	software modules, but sometimes this is unavoidable.
	
	code cannot nest critical sections. Calling chal_enter_critical_section from
	inside a critical section is not allowed and leads to undefined behavior.
	Possible results include deadlock or undetected race conditions.
	
	A platform implementation might chose to allow chal_enter_critical_section to nest
	because of platform-specific considerations.
 
See Also:
	chal_exit_critical_section, InterruptSafe rules
****************************************************************************/
void chal_enter_critical_section(void);


/***************************************************************************
Summary:
	Leave a critical section.

Description:
	Calling chal_exit_critical_section when you are not in a critical section
	is not allowed and leads to undefined behavior.

See Also:
	chal_enter_critical_section, InterruptSafe rules
****************************************************************************/
void chal_exit_critical_section(void);


/* WARNING: TODO: need to implement these macros */
#define BCM_ENTER_CRITICAL_SECTION chal_enter_critical_section
#define BCM_EXIT_CRITICAL_SECTION  chal_exit_critical_section

#ifdef __cplusplus
}
#endif

#endif /* #ifdef IGNORE_CHAL_OSABS */
#endif /* CHAL_OSABS_H__ */

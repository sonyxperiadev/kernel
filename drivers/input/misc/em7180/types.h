////////////////////////////////////////////////////////////////////////////////
///
/// \file   types.h
///
/// \authors   Evan Lojewski
///
/// \brief  Standard datatypes
///
////////////////////////////////////////////////////////////////////////////////
///
////////////////////////////////////////////////////////////////////////////////
///
/// \copyright (C) EM Microelectronic US Inc.
///
/// \copyright Disclosure to third parties or reproduction in any form
///   whatsoever, without prior written consent, is strictly forbidden
///
////////////////////////////////////////////////////////////////////////////////

#ifndef TYPES_H
#define TYPES_H

#include "portable.h"

#if !defined(__KERNEL__)

	#include <stdint.h>
	typedef int8_t     s8;                                       /**< Signed 8 bit integer. */
	typedef uint8_t    u8;                                       /**< Unsigned 8 bit integer. */
	typedef int16_t    s16;                                      /**< Signed 16 bit integer. */
	typedef uint16_t   u16;                                      /**< Unsigned 16 bit integer. */
	typedef int32_t    s32;                                      /**< Signed 32 bit integer. */
	typedef uint32_t   u32;                                      /**< Unsigned 32 bit integer. */
	typedef int64_t    s64;                                      /**< Signed 64 bit integer. */
	typedef uint64_t   u64;                                      /**< Unsigned 64 bit integer. */

	#if !defined(__cplusplus)
	typedef char         bool;                                           /**< Boolean. */
	#endif

#else

	#include <linux/types.h>

#endif /* __KERNEL__ */

#ifndef TRUE
	#define TRUE         1     /**< True */
#endif /* !TRUE */

#ifndef FALSE
	#define FALSE        0     /**< False */
#endif /* !FALSE */

#ifndef NULL
	#define NULL         ((void*)0)  /**< Null pointer */
#endif /* NULL */


/**
 * \brief I2C transfer status values
 **/
   typedef enum TransferStatus
   {
      TS_I2C_IDLE,                                                   /**< no active transfer is underway */
      TS_I2C_COMPLETE,                                               /**< transfer is now complete */
      TS_I2C_IN_PROGRESS,                                            /**< transfer is active */
      TS_I2C_ERROR                                                   /**< transfer failed */
   } TransferStatus;

/**
 * \brief I2C_HANDLE structure needs to be defined by the host 
 *        OS; for some, it could be just the 7 bit slave
 *        address; for others, it could be a combination of I2C
 *        bus number and slave address, or low level driver file
 *        handle plus slave address, or some other combination
 */
//struct I2C_HANDLE;
/** \brief a typedef wrapping the I2C_HANDLE structure */
   typedef void *I2C_HANDLE_T;
//typedef struct I2C_HANDLE *I2C_HANDLE_T;

/**
 * \brief IRQ_HANDLE structure needs to be defined by the host 
 * OS; for some, it could be just the GPIO pin number that 
 * generated the interrupt; for others, an OS reference to an 
 * interrupt object 
 */
//struct IRQ_HANDLE;
/** \brief a typedef wrapping the IRQ_HANDLE structure */
   typedef void *IRQ_HANDLE_T;
//typedef struct IRQ_HANDLE *IRQ_HANDLE_T;

/**
 * \brief FILE_HANDLE structure needs to be defined by the host 
 * OS; for some, it could be just contain a memory pointer and 
 * offset to the hard-coded contents of the only supported file; 
 * for others, it could be a FILE * returned by fopen(), or 
 * a low level file handle returned by open() 
 */
//struct FILE_HANDLE;
/** \brief a typedef wrapping the FILE_HANDLE structure */
   typedef void *FILE_HANDLE_T;
//typedef struct FILE_HANDLE *FILE_HANDLE_T;

/**
 * \brief transfer completion callback type 
 * \param handle - abstract handle to Sentral via I2C 
 * \param complete - status of transfer 
 * \param len_transferred - the number of bytes read or written
 * \param user_param - the same value specified by the caller to 
 *                   the original i2c_read_start() or
 *                   i2c_write_start() functions
 * \return bool indicating not sure what yet 
 **/
   typedef bool (*I2C_CALLBACK)(I2C_HANDLE_T handle, TransferStatus complete, u16 len_transferred, void *user_param);

/**
 * \brief IRQ callback type 
 * \param handle - the host-specific handle to the IRQ that 
 *               generated the callback
 * \param os_param - a host-specific additional value 
 * \param user_param - the value specified on the call to 
 *                   irq_register()
 */
   typedef bool (*IRQ_CALLBACK)(IRQ_HANDLE_T handle, u32 os_param, void *user_param);


#endif /* TYPES_H */

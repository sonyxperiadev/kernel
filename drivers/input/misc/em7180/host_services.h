/**
 * \file    host_services.h 
 *  
 * \authors     Pete Skeggs 
 *
 * \brief      Platform-independent services required by 
 *         generic Sentral host driver.
 *
 * \copyright  (C) 2013-2014 EM Microelectronic
 *
 */

/** \defgroup Host_Services Platform-independent services
 *  
 *  \brief This defines Platform-independent services required by the Sentral
 *  Generic Host Driver; it is up to the user to write these functions to
 *  connect the Generic Host Driver to the actual services available on the
 *  platform they are targetting.
 *  @{ */

#ifndef _HOST_SERVICES_H_
#define _HOST_SERVICES_H_

#include "types.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * \brief I2C master services 
 **/

/*struct i2c_ops 
{
   status_t (*write)(struct device *dev, uint8_t addr, const void *buf, size_t len);
   status_t (*read)(struct device *dev, uint8_t addr, void *buf, size_t len);
   status_t (*write_reg)(struct device *dev, uint8_t addr, uint8_t reg, uint8_t value);
   status_t (*read_reg)(struct device *dev, uint8_t addr, uint8_t reg, void *value);
};*/

/**
 * \brief initialize the specified I2C handle for use
 * \param handle -- the I2C handle to init
 * \return bool false if handle invalid or device not reachable
 */
#ifndef __KERNEL__
   extern bool i2c_init(I2C_HANDLE_T handle);
#else
   static inline bool i2c_init(I2C_HANDLE_T handle) { return true; }
#endif

/**
 * \brief deinitialize the specified I2C handle; release any 
 *        resources associated with it
 * \param handle -- the I2C handle to deinit
 * \return bool false if handle invalid or device was not 
 *         initialized
 */
#ifndef __KERNEL__
	extern bool i2c_deinit(I2C_HANDLE_T handle);
#else
	static inline bool i2c_deinit(I2C_HANDLE_T handle) { return true; }
#endif

/**
 * \brief register with the host a function to be called when 
 *        the interrupt occurs
 * \param handle - an abstract representation of the I2C in 
 *               question
 * \param callback - the function to call
 * \param user_param - value the caller wishes to be passed to 
 *                   the callback when it occurs later
 * \return bool - false if parameters are invalid
 */
#ifndef __KERNEL__
   extern bool i2c_register(I2C_HANDLE_T handle, I2C_CALLBACK callback, void *user_param);
#else
   static inline bool i2c_register(I2C_HANDLE_T handle, I2C_CALLBACK callback, void *user_param) { return true; }
#endif

/** 
 * \brief remove callback and disable the interrupt 
 * \param handle - the I2C in question
 * \return bool - false if the specified IRQ was never registered
 */
#ifndef __KERNEL__
   extern bool i2c_deregister(I2C_HANDLE_T handle);
#else
   static inline bool i2c_deregister(I2C_HANDLE_T handle)  { return true; }
#endif

/**
 * \brief ask the I2C handler how large of a read it can perform
 * \param handle - the I2C bus/device in question 
 * \return u16 - number of bytes; needs to be multiple of 4
 */
#ifndef __KERNEL__
   extern u16 i2c_get_max_read_length(I2C_HANDLE_T handle);
#else
   static inline u16 i2c_get_max_read_length(I2C_HANDLE_T handle) { return 32; }
#endif

/**
 * \brief ask the I2C handler how large of a write it can perform
 * \param handle - the I2C bus/device in question 
 * \return u16 - number of bytes; needs to be multiple of 4 
 */
#ifndef __KERNEL__
   extern u16 i2c_get_max_write_length(I2C_HANDLE_T handle);
#else
   static inline u16 i2c_get_max_write_length(I2C_HANDLE_T handle) { return 32; }
#endif

/**
 * \brief request the start of a non-blocking I2C read
 * \param handle - the device to read from
 * \param reg_addr - which register address to write, then 
 *                 perform an I2C RESTART, then read
 * \param buffer - where to store the data read
 * \param len - the number of bytes required
 * \return bool - false if parameters are invalid
 */
extern bool i2c_read_start(I2C_HANDLE_T handle, u8 reg_addr, u8 *buffer, u16 len);


/**
 * \brief request the start of a non-blocking I2C write
 * \param handle - the device to write to
 * \param reg_addr - the register address to write first
 * \param buffer - the data to write after that
 * \param len - the number of bytes to write
 * \return bool - false if parameters are invalid
 */
extern bool i2c_write_start(I2C_HANDLE_T handle, u8 reg_addr, u8 *buffer, u16 len);

/**
 * \brief request the start of a non-blocking I2C write / read
 * \param handle - the device to write to
 * \param wbuffer - the data to write after that
 * \param wlen - the number of bytes to write
 * \param rbuffer - the data to read after that
 * \param rlen - the number of bytes to read
 * \return bool - false if parameters are invalid
 */
extern bool i2c_write_read_start(I2C_HANDLE_T handle, u8 *wbuffer, u16 wlen, u8 *rbuffer, u16 rlen);


/**
 * \brief request the start of a non-blocking I2C read
 * \param handle - the device to read from
 * \param reg_addr - which register address to write, then 
 *                 perform an I2C RESTART, then read
 * \param buffer - where to store the data read
 * \param len - the number of bytes required
 * \return bool - false if parameters are invalid
 */
   extern bool i2c_blocking_read(I2C_HANDLE_T handle, u8 reg_addr, u8 *buffer, u16 len);

/**
 * \brief request the start of a non-blocking I2C write
 * \param handle - the device to write to
 * \param reg_addr - the register address to write first
 * \param buffer - the data to write after that
 * \param len - the number of bytes to write
 * \return bool - false if parameters are invalid
 */
   extern bool i2c_blocking_write(I2C_HANDLE_T handle, u8 reg_addr, u8 *buffer, u16 len);

/**
 * \brief request the start of a non-blocking I2C write
 * \param handle - the device to write to
 * \param wbuffer - the data to write after that
 * \param wlen - the number of bytes to write
 * \param rbuffer - the data to read after that
 * \param rlen - the number of bytes to read
 * \return bool - false if parameters are invalid
 */
   extern bool i2c_blocking_write_read(I2C_HANDLE_T handle, u8 *wbuffer, u16 wlen, u8 *rbuffer, u16 rlen);


/**
 * \brief determine the status of a pending I2C transfer
 * \param handle - the device in question
 * \param complete - pointer to a structure owned by the caller 
 *                 that will receive the status
 * \param len_transferred - pointer to a 16 bit unsigned integer 
 *                        owned by the caller that will receive
 *                        the number of bytes transferred
 * \return bool - false if parameters are invalid
 */
#ifndef __KERNEL__
   extern bool i2c_check_status(I2C_HANDLE_T handle, TransferStatus *complete, u16 *len_transferred);
#else
   static inline bool i2c_check_status(I2C_HANDLE_T handle, TransferStatus *complete, u16 *len_transferred) { return true; }
#endif

/**
 * \brief IRQ services
 */

/**
 * \brief register with the host a function to be called when 
 *        the interrupt occurs
 * \param handle - an abstract representation of the IRQ in 
 *               question
 * \param callback - the function to call
 * \param user_param - value the caller wishes to be passed to 
 *                   the callback when it occurs later
 * \return bool - false if parameters are invalid
 */
#ifndef __KERNEL__
   extern bool irq_register(IRQ_HANDLE_T handle, IRQ_CALLBACK callback, void *user_param);
#else
   static inline  bool irq_register(IRQ_HANDLE_T handle, IRQ_CALLBACK callback, void *user_param) { return true; }
#endif

/**
 * \brief let the host OS know that we've processed the IRQ (may 
 *        be no-op in some systems)
 * \param handle - the IRQ being acknowledged
 */
#ifndef __KERNEL__
   extern void irq_acknowledge(IRQ_HANDLE_T handle);                 /**< some hosts might need to be told to clear the interrupt condition before returning from the IRQ callback */
#else
   static inline void irq_acknowledge(IRQ_HANDLE_T handle)  { }
#endif

/** 
 * \brief remove callback and disable the interrupt 
 * \param handle - the IRQ in question
 * \return bool - false if the specified IRQ was never registered
 */
#ifndef __KERNEL__
   extern bool irq_deregister(IRQ_HANDLE_T handle);
#else
   static inline bool irq_deregister(IRQ_HANDLE_T handle) { return true; }
#endif


/**
 * \brief this is used to help simulate host interrupts on systems not capable 
 *        of generating one
 * \param handle - the IRQ 
 * \return bool - set if IRQ is asserted 
 */
#ifndef __KERNEL__
   extern bool irq_check(IRQ_HANDLE_T handle);
#else
   static inline bool irq_check(IRQ_HANDLE_T handle) { return true; }	
#endif

/** 
 * \brief for platforms that actually provide an interrupt service that 
 *        guarantees detection of the host interrupt, such as embedded hosts,
 *        return TRUE; for platforms which simulate one by slow polling, and
 *        thus cannot guarantee they will detect short interrupt assertions,
 *        return FALSE
 */
#ifndef __KERNEL__
   extern bool irq_is_reliable(IRQ_HANDLE_T handle);
#else
   static inline bool irq_is_reliable(IRQ_HANDLE_T handle) { return true; }
#endif


/**
 * \brief Time services
 **/

/**
 * \brief return the number of milliseconds since reset
 * \return milliseconds
 */
   extern /*inline*/ u32 time_ms(void);

#ifndef __KERNEL__
	/**
	 * \brief wait the specified number of milliseconds
	 * \param num_ms 
	 */
	   extern void time_delay_ms(u32 num_ms);

	/**
	 * \brief wait the specified number of microseconds
	 * \param num_us 
	 */
	   extern void time_delay_us(u32 num_us);
#else
	#include <linux/delay.h>
	#define time_delay_ms(ms)		mdelay(ms)
	#define time_delay_us(us)		udelay(us)
#endif

/**
 * \brief Debug services 
 */

/**
 * \brief log levels
 **/
   typedef enum LOG_LEVEL
   {
      LL_NONE = 0,      //< will not be logged
      LL_ERROR = 1,     //< will be logged with "ERROR: " up front
      LL_WARN = 2,      //< will be logged with "WARNING: " up front
      LL_INFO = 3,      //< will be logged as is
      LL_DEBUG = 4,     //< will be logged with "DEBUG: " up front
      LL_INSANE = 5,    //< will be logged as is
      LL_NUM_LEVELS = 6
   } LOG_LEVEL_T;


/** 
 * /brief log a message 
 * The implementor is free to implement this anyway they want, including 
 * discarding all log output in a deeply embedded application that does not have 
 * loggging facilities or needs.  It may be helpful to send this logging data to 
 * a platform-specific debugger API, such as via semihosting in ARM 
 * environments, which allows printf() to log to the debugger console when 
 * debugging but discards it otherwise. 
 * \param level - the priority 
 * \param fmt - printf format string 
 */
#ifndef __KERNEL__

   extern void do_log(LOG_LEVEL_T level, const char *fmt, ...);


	#ifndef _MSC_VER
	//__attribute__((format (printf, 1, 2)))
	#endif

   /**
	* \brief macros to log at various logging levels
	*/
   #define error_log(...) do_log(LL_ERROR, __VA_ARGS__)     /**< log failures that should not occur */
   #define warn_log(...) do_log(LL_WARN, __VA_ARGS__)       /**< log benign failures */
   #define info_log(...) do_log(LL_INFO, __VA_ARGS__)       /**< log general info useful to know what is happening */
   #define debug_log(...) do_log(LL_DEBUG, __VA_ARGS__)     /**< log information only useful to debug the system */
   #define insane_log(...) do_log(LL_INSANE, __VA_ARGS__)   /**< log detail not normally ever required; voluminous! */

#else
	#include <linux/printk.h>
	#define error_log(...) printk(KERN_ERR __VA_ARGS__)     /**< log failures that should not occur */
	#define warn_log(...) printk(KERN_WARNING __VA_ARGS__)       /**< log benign failures */
	#define info_log(...) printk(KERN_INFO  __VA_ARGS__)       /**< log general info useful to know what is happening */
	#define debug_log(...) printk(KERN_DEBUG __VA_ARGS__)     /**< log information only useful to debug the system */
	#define insane_log(...) printk(KERN_DEBUG __VA_ARGS__)   /**< log detail not normally ever required; voluminous! */

#endif /* __KERNEL__ */

#ifdef __cplusplus
}
#endif

#endif

/** @}*/


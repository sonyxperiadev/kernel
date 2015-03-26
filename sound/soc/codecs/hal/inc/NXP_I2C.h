#ifndef NXP_I2C_H
#define NXP_I2C_H

/* Version */
#define TFA98XX_HAL_REV_MAJOR (2)
#define TFA98XX_HAL_REV_MINOR (4)
#define TFA98XX_HAL_REV_STR "2.4"

#ifdef __cplusplus
extern "C" {
#endif

/* A glue layer.
 * The NXP components will use the functions defined in this API to do
   the actual calls to I2C
 * Make sure you implement this to use your I2C access functions (which are
   SoC and OS dependent)
 */

enum NXP_I2C_Error {
	NXP_I2C_UnassignedErrorCode,
	NXP_I2C_Ok,
	NXP_I2C_NoAck,
	NXP_I2C_SclStuckAtOne,
	NXP_I2C_SdaStuckAtOne,
	NXP_I2C_SclStuckAtZero,
	NXP_I2C_SdaStuckAtZero,
	NXP_I2C_TimeOut,
	NXP_I2C_ArbLost,
	NXP_I2C_NoInit,
	NXP_I2C_Disabled,
	NXP_I2C_UnsupportedValue,
	NXP_I2C_UnsupportedType,
	NXP_I2C_NoInterfaceFound,
	NXP_I2C_NoPortnumber,
	NXP_I2C_BufferOverRun,
	NXP_I2C_ErrorMaxValue
};

/* The maximum I2C message size allowed for read and write buffers, incl
   the slave address */
#define NXP_I2C_MAX_SIZE 254
/* The maximum I2C burst size, transaction will be split into smaller chunks */
/*#define NXP_I2C_MAX_BURST 32*/ /*if defined then NXP_I2C_Write() will enable
   this, note that the read has not been done yet */

/* Execute an I2C write transaction
   @sla = slave address
   @num_write_bytes = size of data[]
   @data[] = byte array of data to write
*/
enum NXP_I2C_Error NXP_I2C_Write(unsigned char sla,
				int num_write_bytes,
				const unsigned char data[]);

/* Execute a write, followed by I2C restart and a read of num_read_bytes bytes.
   The read_buffer must be big enough to contain num_read_bytes.
   @sla = slave address
   @num_write_bytes = size of data[]
   @write_data[] = byte array of data to write
   @num_read_bytes = size of read_buffer[] and number of bytes to read
   @read_buffer[] = byte array to receive the read data
*/
enum NXP_I2C_Error NXP_I2C_WriteRead(unsigned char sla,
				int num_write_bytes,
				const unsigned char write_data[],
				int num_read_bytes,
				unsigned char read_buffer[]);

/* Returns the number of bytes that can be transfered in one transaction */
int NXP_I2C_BufferSize(void);

/* when enabled, the low level I2C transactions are displayed using printf */
enum NXP_I2C_Error NXP_I2C_EnableLogging(int bEnable);
#ifdef WIN32
enum NXP_I2C_Error init_I2C(void);
#else
enum NXP_I2C_Error i2cExecute(int num_write_bytes, unsigned char *data);
enum NXP_I2C_Error i2cExecuteRS(int NrOfWriteBytes,
	const unsigned char *WriteData, int NrOfReadBytes,
	unsigned char *ReadData);

#endif
#ifdef __cplusplus
}
#endif

#endif /* NXP_I2C_H */

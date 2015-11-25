#ifdef __KERNEL__
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#else
#include <assert.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#endif

#include "../inc/Tfa98xx_internals.h"
#include "../inc/Tfa98xx_Registers.h"
#include "../../hal/inc/NXP_I2C.h"

#include "../inc/initTfa9890.h"

#ifdef __KERNEL__
#define _ASSERT(e) do { if (!(e)) pr_err("PrintAssert:%s (%s:%d)\n",\
		__func__, __FILE__, __LINE__); } while (0)
#else
#define _ASSERT(e)  assert(e)
#endif

#define OPTIMIZED_RPC

/* 4 possible I2C addresses
 */
#define MAX_HANDLES 4
static struct Tfa98xx_handle_private handlesLocal[MAX_HANDLES];

/**
 * Return the maximum nr of devices (SC39786)
 */
int Tfa98xx_MaxDevices(void)
{
	return MAX_HANDLES;
}

int handle_is_open(Tfa98xx_handle_t h)
{
	int retval = 0;

	if ((h >= 0) && (h < MAX_HANDLES))
		retval = handlesLocal[h].in_use != 0;

	return retval;
}

/* translate a I2C driver error into an error for Tfa9887 API */
static enum Tfa98xx_Error classify_i2c_error(enum NXP_I2C_Error i2c_error)
{
	switch (i2c_error) {
	case NXP_I2C_Ok:
		return Tfa98xx_Error_Ok;
	case NXP_I2C_NoAck:
	case NXP_I2C_ArbLost:
	case NXP_I2C_TimeOut:
		return Tfa98xx_Error_I2C_NonFatal;
	default:
		return Tfa98xx_Error_I2C_Fatal;
	}
}

/* the patch contains a header with the following
 * IC revision register: 1 byte, 0xFF means don't care
 * XMEM address to check: 2 bytes, big endian, 0xFFFF means don't care
 * XMEM value to expect: 3 bytes, big endian
 */
static enum Tfa98xx_Error
checkICROMversion(Tfa98xx_handle_t handle, const unsigned char patchheader[])
{
	enum Tfa98xx_Error error = Tfa98xx_Error_Ok;
	unsigned short checkrev;
	unsigned short checkaddress;
	int checkvalue;
	int value = 0;
	int status;
	checkrev = patchheader[0];
	if ((checkrev != 0xFF) && (checkrev != handlesLocal[handle].rev))
		return Tfa98xx_Error_Not_Supported;

	checkaddress = (patchheader[1] << 8) + patchheader[2];
	checkvalue =
	    (patchheader[3] << 16) + (patchheader[4] << 8) + patchheader[5];
	if (checkaddress != 0xFFFF) {
		/* before reading XMEM, check if we can access the DSP */
		error = Tfa98xx_DspSystemStable(handle, &status);
		if (error == Tfa98xx_Error_Ok) {
			if (!status) {
				/* DSP subsys not running */
				error = Tfa98xx_Error_DSP_not_running;
			}
		}
		/* read register to check the correct ROM version */
		if (error == Tfa98xx_Error_Ok) {
			error =
			Tfa98xx_DspReadMem(handle, checkaddress, 1, &value);
		}
		if (error == Tfa98xx_Error_Ok) {
			if (value != checkvalue)
				error = Tfa98xx_Error_Not_Supported;
		}
	}
	return error;
}

enum Tfa98xx_Error
Tfa98xx_DspGetSwFeatureBits(Tfa98xx_handle_t handle, int features[2])
{
	enum Tfa98xx_Error error = Tfa98xx_Error_Ok;
	unsigned char bytes[3 * 2];

	error =
	    Tfa98xx_DspGetParam(handle, MODULE_FRAMEWORK,
				FW_PARAM_GET_FEATURE_BITS, sizeof(bytes),
				bytes);
	if (error != Tfa98xx_Error_Ok) {
		/* old ROM code may respond with Tfa98xx_Error_RpcParamId */
		return error;
	}
	Tfa98xx_ConvertBytes2Data(sizeof(bytes), bytes, features);

	return error;
}

enum Tfa98xx_Error
Tfa98xx_Open(unsigned char slave_address, Tfa98xx_handle_t *pHandle)
{
	enum Tfa98xx_Error error = Tfa98xx_Error_OutOfHandles;
	unsigned short rev;
	int i;
	_ASSERT(pHandle != (Tfa98xx_handle_t *) 0);
	*pHandle = -1;
	/* find free handle */
	for (i = 0; i < MAX_HANDLES; ++i) {
		if (!handlesLocal[i].in_use) {
			switch (slave_address) {
			case TFA98XX_GENERIC_SLAVE_ADDRESS:
			case 0x68:
			case 0x6A:
			case 0x6C:
			case 0x6E:
				handlesLocal[i].in_use = 1;
				handlesLocal[i].slave_address = slave_address;
				handlesLocal[i].supportDrc = supportNotSet;
				handlesLocal[i].supportFramework =
				    supportNotSet;
				error =
				    Tfa98xx_ReadRegister16(i,
							TFA98XX_REVISIONNUMBER,
							&rev);
				if (Tfa98xx_Error_Ok != error) {
					handlesLocal[i].in_use = 0;
					return error;
				}
				handlesLocal[i].rev = rev;
				*pHandle = i;
				error = Tfa98xx_Error_Ok;
				break;
			default:
				/* wrong slave address */
				error = Tfa98xx_Error_Bad_Parameter;
			}
			break;	/* handle is assigned, exit the for-loop */
		}
	}
	return error;
}

enum Tfa98xx_Error Tfa98xx_Close(Tfa98xx_handle_t handle)
{
	if (handle_is_open(handle)) {
		handlesLocal[handle].in_use = 0;
		return Tfa98xx_Error_Ok;
	} else {
		return Tfa98xx_Error_NotOpen;
	}
}

/* Tfa98xx_DspConfigParameterCount
 * Yields the number of parameters to be used in Tfa98xx_DspWriteConfig()
 */
enum Tfa98xx_Error Tfa98xx_DspConfigParameterCount(Tfa98xx_handle_t handle,
						int *pParamCount)
{
#define dateLen (13)
	enum Tfa98xx_Error error;
	char date[dateLen + 1];
	char buffer[dateLen * 3 + 2 + 6];
	char month[4] = "";
	int day = 0;
	int year = 0;
	int i;

	if (!handle_is_open(handle))
		return Tfa98xx_Error_NotOpen;

	memset(buffer, 0, sizeof(buffer));
	/* Get the release date */
	error =
	    Tfa98xx_DspGetParam(handle, 1, 0xFF, dateLen * 3,
				(unsigned char *)buffer);
	if (error != Tfa98xx_Error_Ok)
		return error;

	/* create date as ASCII string */
	for (i = 0; i < dateLen; i++)
		date[i] = buffer[i * 3 + 2 + 6];
	date[dateLen - 2] = '\0';

	/* split into date parts */
	if (sscanf(date, "%3s %d %d", month, &day, &year) == 3) {
		/* Anything after 2012/12/19 has 67 words, else 55 */
		if (year > 2012)
			*pParamCount = 67;
		else if (year < 2012)
			*pParamCount = 55;
		else if (day <= 19)		/* Year 2012 */
			*pParamCount = 55;	/* anything <= 19 */
		else if ((month[0] | 0x20) != 'd')
			*pParamCount = 55;	/* month < december */
		else
			*pParamCount = 67;	/* month = 12 and day > 19 */
	} else {
		/* scanf failed parsing the tag */
		error = Tfa98xx_Error_Other;
	}
	/* *pParamCount is only assigned if error == Tfa98xx_Error_Ok */
	return error;
}

/* Tfa98xx_DspReset
 *  set or clear DSP reset signal
 */
/* the wrapper for DspReset, for tfa9890 */
enum Tfa98xx_Error Tfa98xx_DspReset(Tfa98xx_handle_t handle, int state)
{

	return Tfa9890_DspReset(handle, state);	/* tfa9890 function */
}

/* Tfa98xx_DspSystemStable
 *  return: *ready = 1 when clocks are stable to allow DSP subsystem access
 */
enum Tfa98xx_Error Tfa98xx_DspSystemStable(Tfa98xx_handle_t handle, int *ready)
{
	return Tfa9890_DspSystemStable(handle, ready);	/* tfa9890 function */
}

/* return the device revision id
 */
unsigned short Tfa98xx_GetDeviceRevision(Tfa98xx_handle_t handle)
{
	/* local function. Caller must make sure handle is valid */
	return handlesLocal[handle].rev;
}

enum Tfa98xx_Error Tfa98xx_Init(Tfa98xx_handle_t handle)
{
	enum Tfa98xx_Error error;
	unsigned short regRead = 0;

	if (!handle_is_open(handle))
		return Tfa98xx_Error_NotOpen;

	/* reset all i2C registers to default */
	error = Tfa98xx_WriteRegister16(handle, TFA98XX_SYS_CTRL,
					TFA98XX_SYS_CTRL_I2CR_MSK);

	/* some other registers must be set for optimal amplifier behaviour
	 * This is implemented in a file specific for the type number
	 */
	error = Tfa9890_specific(handle);
	error = Tfa98xx_WriteRegister16(handle, 0x40, 0x5a6b);
	error = Tfa98xx_ReadRegister16(handle, 0x59, &regRead);

	regRead |= 0x3;

	error = Tfa98xx_WriteRegister16(handle, 0x59, regRead);
	error = Tfa98xx_WriteRegister16(handle, 0x40, 0x0000);

	return error;
}

static enum Tfa98xx_Error
processPatchFile(Tfa98xx_handle_t handle, int length,
		 const unsigned char *bytes)
{
	unsigned short size;
	int index;
	enum NXP_I2C_Error i2c_error = NXP_I2C_Ok;
	/* expect following format in patchBytes:
	 * 2 bytes length of I2C transaction in little endian, then the bytes,
	 * excluding the slave address which is added from the handle
	 * This repeats for the whole file
	 */
	index = 0;
	while (index < length) {
		/* extract little endian length */
		size = bytes[index] + bytes[index + 1] * 256;
		index += 2;
		if ((index + size) > length) {
			/* outside the buffer, error in the input data */
			return Tfa98xx_Error_Bad_Parameter;
		}
		if (size > NXP_I2C_BufferSize()) {
			/* too big, must fit buffer */
			return Tfa98xx_Error_Bad_Parameter;
		}
		i2c_error =
		    NXP_I2C_Write(handlesLocal[handle].slave_address, size,
				  bytes + index);
		if (i2c_error != NXP_I2C_Ok)
			break;
		index += size;
	}
	return classify_i2c_error(i2c_error);
}

/*
 * write a 16 bit subaddress
 */
enum Tfa98xx_Error
Tfa98xx_WriteRegister16(Tfa98xx_handle_t handle,
			unsigned char subaddress, unsigned short value)
{
	enum NXP_I2C_Error i2c_error;
	int bytes2write = 3;	/* subaddress and 2 bytes of the value */
	unsigned char write_data[3];
	if (!handle_is_open(handle))
		return Tfa98xx_Error_NotOpen;

	write_data[0] = subaddress;
	write_data[1] = (value >> 8) & 0xFF;
	write_data[2] = value & 0xFF;
	i2c_error =
	    NXP_I2C_Write(handlesLocal[handle].slave_address, bytes2write,
			  write_data);
	return classify_i2c_error(i2c_error);
}

enum Tfa98xx_Error
Tfa98xx_DspSupportFramework(Tfa98xx_handle_t handle, int *pbSupportFramework)
{
	int featureBits[2] = { 0, 0 };
	enum Tfa98xx_Error error = Tfa98xx_Error_Ok;

	_ASSERT(pbSupportFramework != 0);

	if (!handle_is_open(handle))
		return Tfa98xx_Error_NotOpen;

	if (handlesLocal[handle].supportFramework != supportNotSet) {
		*pbSupportFramework = handlesLocal[handle].supportFramework;
	} else {
		error = Tfa98xx_DspGetSwFeatureBits(handle, featureBits);
		if (error == Tfa98xx_Error_Ok) {
			*pbSupportFramework = 1;
			handlesLocal[handle].supportFramework = supportYes;
		} else if (error == Tfa98xx_Error_RpcParamId) {
			*pbSupportFramework = 0;
			handlesLocal[handle].supportFramework = supportNo;
			error = Tfa98xx_Error_Ok;
		}
	}

	/* *pbSupportFramework only changed when error == Tfa98xx_Error_Ok */
	return error;
}

/*
 * Write all the bytes specified by num_bytes and data
 */
enum Tfa98xx_Error
Tfa98xx_WriteData(Tfa98xx_handle_t handle,
		  unsigned char subaddress, int num_bytes,
		  const unsigned char data[])
{
	enum NXP_I2C_Error i2c_error;
	/* subaddress followed by data */
	int bytes2write = num_bytes + 1;

	unsigned char write_data[MAX_PARAM_SIZE];
	if (!handle_is_open(handle))
		return Tfa98xx_Error_NotOpen;
	if (bytes2write > MAX_PARAM_SIZE)
		return Tfa98xx_Error_Bad_Parameter;
	if (bytes2write > NXP_I2C_BufferSize())
		return Tfa98xx_Error_Bad_Parameter;
	write_data[0] = subaddress;
	memcpy(write_data + 1, data, num_bytes);
	i2c_error =
	    NXP_I2C_Write(handlesLocal[handle].slave_address, bytes2write,
			  write_data);
	return classify_i2c_error(i2c_error);
}

enum Tfa98xx_Error
Tfa98xx_ReadRegister16(Tfa98xx_handle_t handle,
		       unsigned char subaddress, unsigned short *pValue)
{
	enum NXP_I2C_Error i2c_error;
	const int bytes2write = 1;	/* subaddress */
	/* 2 bytes that will contain the data of the register */
	const int bytes2read = 2;
	unsigned char write_data[1];
	unsigned char read_buffer[2];
	_ASSERT(pValue != (unsigned short *)0);
	if (!handle_is_open(handle))
		return Tfa98xx_Error_NotOpen;
	write_data[0] = subaddress;
	read_buffer[0] = read_buffer[1] = 0;
	i2c_error =
	    NXP_I2C_WriteRead(handlesLocal[handle].slave_address, bytes2write,
			      write_data, bytes2read, read_buffer);
	if (classify_i2c_error(i2c_error) != Tfa98xx_Error_Ok) {
		return classify_i2c_error(i2c_error);
	} else {
		*pValue = (read_buffer[0] << 8) + read_buffer[1];
		return Tfa98xx_Error_Ok;
	}
}

enum Tfa98xx_Error
Tfa98xx_ReadData(Tfa98xx_handle_t handle,
		 unsigned char subaddress, int num_bytes, unsigned char data[])
{
	enum NXP_I2C_Error i2c_error;
	const int bytes2write = 1;	/* subaddress */
	unsigned char write_data[1];
	if (!handle_is_open(handle))
		return Tfa98xx_Error_NotOpen;
	if (num_bytes > NXP_I2C_BufferSize())
		return Tfa98xx_Error_Bad_Parameter;
	write_data[0] = subaddress;
	i2c_error =
	    NXP_I2C_WriteRead(handlesLocal[handle].slave_address, bytes2write,
			      write_data, num_bytes, data);
	if (classify_i2c_error(i2c_error) != Tfa98xx_Error_Ok)
		return classify_i2c_error(i2c_error);
	else
		return Tfa98xx_Error_Ok;
}

enum Tfa98xx_Error Tfa98xx_Powerdown(Tfa98xx_handle_t handle, int powerdown)
{
	enum Tfa98xx_Error error;
	unsigned short value;
	if (!handle_is_open(handle))
		return Tfa98xx_Error_NotOpen;
	/* read the SystemControl register, modify the bit and write again */
	error = Tfa98xx_ReadRegister16(handle, TFA98XX_SYS_CTRL, &value);
	if (error != Tfa98xx_Error_Ok)
		return error;
	switch (powerdown) {
	case 1:
		value |= TFA98XX_SYS_CTRL_PWDN_MSK;
		break;
	case 0:
		value &= ~(TFA98XX_SYS_CTRL_PWDN_MSK);
		break;
	default:
		return Tfa98xx_Error_Bad_Parameter;
	}
	error = Tfa98xx_WriteRegister16(handle, TFA98XX_SYS_CTRL, value);
	return error;
}

enum Tfa98xx_Error Tfa98xx_SetConfigured(Tfa98xx_handle_t handle)
{
	enum Tfa98xx_Error error;
	unsigned short value;
	if (!handle_is_open(handle))
		return Tfa98xx_Error_NotOpen;
	/* read the SystemControl register, modify the bit and write again */
	error = Tfa98xx_ReadRegister16(handle, TFA98XX_SYS_CTRL, &value);
	if (error != Tfa98xx_Error_Ok)
		return error;

	value |= TFA98XX_SYS_CTRL_SBSL_MSK;
	error = Tfa98xx_WriteRegister16(handle, TFA98XX_SYS_CTRL, value);
	return error;
}

enum Tfa98xx_Error
Tfa98xx_SelectAmplifierInput(Tfa98xx_handle_t handle,
			     enum Tfa98xx_AmpInputSel input_sel)
{
	enum Tfa98xx_Error error;
	unsigned short value;
	if (!handle_is_open(handle))
		return Tfa98xx_Error_NotOpen;
	/* read the SystemControl register, modify the bit and write again */
	error = Tfa98xx_ReadRegister16(handle, TFA98XX_I2SREG, &value);
	if (error == Tfa98xx_Error_Ok) {
		/* clear the 2 bits first */
		value &= ~(0x3 << TFA98XX_I2SREG_CHSA_POS);
		switch (input_sel) {
		case Tfa98xx_AmpInputSel_I2SLeft:
			value |= (0x0 << TFA98XX_I2SREG_CHSA_POS);
			break;
		case Tfa98xx_AmpInputSel_I2SRight:
			value |= (0x1 << TFA98XX_I2SREG_CHSA_POS);
			break;
		case Tfa98xx_AmpInputSel_DSP:
			value |= (0x2 << TFA98XX_I2SREG_CHSA_POS);
			break;
		default:
			error = Tfa98xx_Error_Bad_Parameter;
		}
	}
	if (error == Tfa98xx_Error_Ok)
		error = Tfa98xx_WriteRegister16(handle, TFA98XX_I2SREG, value);

	return error;
}

enum Tfa98xx_Error
Tfa98xx_SelectI2SOutputLeft(Tfa98xx_handle_t handle,
			    enum Tfa98xx_OutputSel output_sel)
{
	enum Tfa98xx_Error error;
	unsigned short value;
	if (!handle_is_open(handle))
		return Tfa98xx_Error_NotOpen;
	/* read the I2S SEL register, modify the bit and write again */
	error = Tfa98xx_ReadRegister16(handle, TFA98XX_I2S_SEL_REG, &value);
	if (error == Tfa98xx_Error_Ok) {
		/* clear the 3 bits first */
		value &= ~(0x7 << TFA98XX_I2S_SEL_REG_DOLS_POS);
		switch (output_sel) {
		case Tfa98xx_I2SOutputSel_CurrentSense:
		case Tfa98xx_I2SOutputSel_DSP_Gain:
		case Tfa98xx_I2SOutputSel_DSP_AEC:
		case Tfa98xx_I2SOutputSel_Amp:
		case Tfa98xx_I2SOutputSel_DataI3R:
		case Tfa98xx_I2SOutputSel_DataI3L:
		case Tfa98xx_I2SOutputSel_DcdcFFwdCur:
			/* enum definition matches the HW registers */
			value |= (output_sel << TFA98XX_I2S_SEL_REG_DOLS_POS);
			break;
		default:
			error = Tfa98xx_Error_Bad_Parameter;
		}
	}
	if (error == Tfa98xx_Error_Ok) {
		error = Tfa98xx_WriteRegister16(
			handle, TFA98XX_I2S_SEL_REG, value);
	}
	return error;
}

enum Tfa98xx_Error
Tfa98xx_SelectI2SOutputRight(Tfa98xx_handle_t handle,
			     enum Tfa98xx_OutputSel output_sel)
{
	enum Tfa98xx_Error error;
	unsigned short value;
	if (!handle_is_open(handle))
		return Tfa98xx_Error_NotOpen;
	/* read the I2S SEL register, modify the bit and write again */
	error = Tfa98xx_ReadRegister16(handle, TFA98XX_I2S_SEL_REG, &value);
	if (error == Tfa98xx_Error_Ok) {
		/* clear the 3 bits first */
		value &= ~(0x7 << TFA98XX_I2S_SEL_REG_DORS_POS);
		switch (output_sel) {
		case Tfa98xx_I2SOutputSel_CurrentSense:
		case Tfa98xx_I2SOutputSel_DSP_Gain:
		case Tfa98xx_I2SOutputSel_DSP_AEC:
		case Tfa98xx_I2SOutputSel_Amp:
		case Tfa98xx_I2SOutputSel_DataI3R:
		case Tfa98xx_I2SOutputSel_DataI3L:
		case Tfa98xx_I2SOutputSel_DcdcFFwdCur:
			/* enum definition matches the HW registers */
			value |= (output_sel << TFA98XX_I2S_SEL_REG_DORS_POS);
			break;
		default:
			error = Tfa98xx_Error_Bad_Parameter;
		}
	}
	if (error == Tfa98xx_Error_Ok) {
		error =
		    Tfa98xx_WriteRegister16(handle, TFA98XX_I2S_SEL_REG,
						value);
	}
	return error;
}

enum Tfa98xx_Error
Tfa98xx_SelectStereoGainChannel(Tfa98xx_handle_t handle,
				enum Tfa98xx_StereoGainSel gain_sel)
{
	enum Tfa98xx_Error error;
	unsigned short value;
	if (!handle_is_open(handle))
		return Tfa98xx_Error_NotOpen;
	/* read the I2S Control register, modify the bit and write again */
	error = Tfa98xx_ReadRegister16(handle, TFA98XX_I2SREG, &value);
	if (error == Tfa98xx_Error_Ok) {
		/* clear the bits first */
		value &= ~(0x1 << TFA98XX_I2SREG_CHS3_POS);
		switch (gain_sel) {
		case Tfa98xx_StereoGainSel_Left:
		case Tfa98xx_StereoGainSel_Right:
			/* enum definition matches the HW registers */
			value |= (gain_sel << TFA98XX_I2SREG_CHS3_POS);
			break;
		default:
			error = Tfa98xx_Error_Bad_Parameter;
		}
	}
	if (error == Tfa98xx_Error_Ok)
		error = Tfa98xx_WriteRegister16(handle, TFA98XX_I2SREG, value);

	return error;
}

enum Tfa98xx_Error
Tfa98xx_SelectChannel(Tfa98xx_handle_t handle, enum Tfa98xx_Channel channel)
{
	enum Tfa98xx_Error error;
	unsigned short value;
	if (!handle_is_open(handle))
		return Tfa98xx_Error_NotOpen;
	/* read the SystemControl register, modify the bit and write again */
	error = Tfa98xx_ReadRegister16(handle, TFA98XX_I2SREG, &value);
	if (error == Tfa98xx_Error_Ok) {
		/* clear the 2 bits first */
		value &= ~(0x3 << TFA98XX_I2SREG_CHS12_POS);
		switch (channel) {
		case Tfa98xx_Channel_L:
			value |= (0x1 << TFA98XX_I2SREG_CHS12_POS);
			break;
		case Tfa98xx_Channel_R:
			value |= (0x2 << TFA98XX_I2SREG_CHS12_POS);
			break;
		case Tfa98xx_Channel_L_R:
			value |= (0x3 << TFA98XX_I2SREG_CHS12_POS);
			break;
		case Tfa98xx_Channel_Stereo:
			/* real stereo on 1 DSP not yet supported */
			error = Tfa98xx_Error_Not_Supported;
			break;
		default:
			error = Tfa98xx_Error_Bad_Parameter;
		}
	}
	if (error == Tfa98xx_Error_Ok)
		error = Tfa98xx_WriteRegister16(handle, TFA98XX_I2SREG, value);

	return error;
}

enum Tfa98xx_Error Tfa98xx_SetSampleRate(Tfa98xx_handle_t handle, int rate)
{
	enum Tfa98xx_Error error;
	unsigned short value;
	if (!handle_is_open(handle))
		return Tfa98xx_Error_NotOpen;
	/* read the SystemControl register, modify the bit and write again */
	error = Tfa98xx_ReadRegister16(handle, TFA98XX_I2SREG, &value);
	if (error == Tfa98xx_Error_Ok) {
		/* clear the 4 bits first */
		value &= (~(0xF << TFA98XX_I2SREG_I2SSR_POS));
		switch (rate) {
		case 48000:
			value |= TFA98XX_I2SCTRL_RATE_48000;
			break;
		case 44100:
			value |= TFA98XX_I2SCTRL_RATE_44100;
			break;
		case 32000:
			value |= TFA98XX_I2SCTRL_RATE_32000;
			break;
		case 24000:
			value |= TFA98XX_I2SCTRL_RATE_24000;
			break;
		case 22050:
			value |= TFA98XX_I2SCTRL_RATE_22050;
			break;
		case 16000:
			value |= TFA98XX_I2SCTRL_RATE_16000;
			break;
		case 12000:
			value |= TFA98XX_I2SCTRL_RATE_12000;
			break;
		case 11025:
			value |= TFA98XX_I2SCTRL_RATE_11025;
			break;
		case 8000:
			value |= TFA98XX_I2SCTRL_RATE_08000;
			break;
		default:
			error = Tfa98xx_Error_Bad_Parameter;
		}
	}
	if (error == Tfa98xx_Error_Ok)
		error = Tfa98xx_WriteRegister16(handle, TFA98XX_I2SREG, value);

	return error;
}

enum Tfa98xx_Error Tfa98xx_GetSampleRate(Tfa98xx_handle_t handle, int *pRate)
{
	enum Tfa98xx_Error error;
	unsigned short value;
	if (!handle_is_open(handle))
		return Tfa98xx_Error_NotOpen;
	_ASSERT(pRate != 0);
	/* read the SystemControl register, modify the bit and write again */
	error = Tfa98xx_ReadRegister16(handle, TFA98XX_I2SREG, &value);
	if (error == Tfa98xx_Error_Ok) {
		/* clear the 4 bits first */
		value = value & (0xF << TFA98XX_I2SREG_I2SSR_POS);
		switch (value) {
		case TFA98XX_I2SCTRL_RATE_48000:
			*pRate = 48000;
			break;
		case TFA98XX_I2SCTRL_RATE_44100:
			*pRate = 44100;
			break;
		case TFA98XX_I2SCTRL_RATE_32000:
			*pRate = 32000;
			break;
		case TFA98XX_I2SCTRL_RATE_24000:
			*pRate = 24000;
			break;
		case TFA98XX_I2SCTRL_RATE_22050:
			*pRate = 22050;
			break;
		case TFA98XX_I2SCTRL_RATE_16000:
			*pRate = 16000;
			break;
		case TFA98XX_I2SCTRL_RATE_12000:
			*pRate = 12000;
			break;
		case TFA98XX_I2SCTRL_RATE_11025:
			*pRate = 11025;
			break;
		case TFA98XX_I2SCTRL_RATE_08000:
			*pRate = 8000;
			break;
		default:
			/* cannot happen, only 9 cases possible */
			_ASSERT(0);
		}
	}
	return error;
}


enum Tfa98xx_Error
Tfa98xx_DspWriteSpeakerParameters(Tfa98xx_handle_t handle,
				  int length,
				  const unsigned char *pSpeakerBytes)
{
	enum Tfa98xx_Error error;
	if (pSpeakerBytes != 0) {
		/* by design: keep the data opaque and no
		 * interpreting/calculation */
		/* Use long WaitResult retry count */
		error = Tfa98xx_DspSetParamVarWait(
					handle,
					MODULE_SPEAKERBOOST,
					SB_PARAM_SET_LSMODEL, length,
					pSpeakerBytes,
					TFA98XX_WAITRESULT_NTRIES_LONG);
	} else {
		error = Tfa98xx_Error_Bad_Parameter;
	}

	return error;
}

enum Tfa98xx_Error
Tfa98xx_DspWriteSpeakerParametersMultiple(int handle_cnt,
					  Tfa98xx_handle_t
					  handles[],
					  int length,
					  const unsigned char *pSpeakerBytes)
{
	enum Tfa98xx_Error error;
	if (pSpeakerBytes != 0) {
		/* by design: keep the data opaque and no
		 * interpreting/calculation */
		/* Use long WaitResult retry count */
		error = Tfa98xx_DspSetParamMultipleVarWait(
					handle_cnt, handles,
					MODULE_SPEAKERBOOST,
					SB_PARAM_SET_LSMODEL,
					length,
					pSpeakerBytes,
					TFA98XX_WAITRESULT_NTRIES_LONG);
	} else {
		error = Tfa98xx_Error_Bad_Parameter;
	}

	return error;
}

enum Tfa98xx_Error
Tfa98xx_DspReadSpeakerParameters(Tfa98xx_handle_t handle,
				 int length, unsigned char *pSpeakerBytes)
{
	return Tfa98xx_DspReadSpkrParams(handle,
					 SB_PARAM_GET_LSMODEL, length,
					 pSpeakerBytes);
}

enum Tfa98xx_Error
Tfa98xx_DspReadExcursionModel(Tfa98xx_handle_t handle,
			      int length, unsigned char *pSpeakerBytes)
{
	return Tfa98xx_DspReadSpkrParams(handle,
					 SB_PARAM_GET_XMODEL, length,
					 pSpeakerBytes);
}

enum Tfa98xx_Error
Tfa98xx_DspReadSpkrParams(Tfa98xx_handle_t handle,
			  unsigned char paramId,
			  int length, unsigned char *pSpeakerBytes)
{
	enum Tfa98xx_Error error;
	if (!handle_is_open(handle))
		return Tfa98xx_Error_NotOpen;
	if (pSpeakerBytes != 0) {
		error =
		    Tfa98xx_DspGetParam(handle, MODULE_SPEAKERBOOST,
					paramId, length, pSpeakerBytes);
	} else {
		error = Tfa98xx_Error_Bad_Parameter;
	}
	return error;
}

enum Tfa98xx_Error Tfa98xx_DspSupporttCoef(Tfa98xx_handle_t handle,
					int *pbSupporttCoef)
{
	int featureBits[2];
	unsigned short featureBits1;
	enum Tfa98xx_Error error = Tfa98xx_Error_Ok;

	_ASSERT(pbSupporttCoef != 0);

	error = Tfa98xx_DspGetSwFeatureBits(handle, featureBits);
	if (error == Tfa98xx_Error_Ok) {
		/* easy case: new API available => not the 87 */
		/* bit=0 means tCoef expected */
		*pbSupporttCoef = (featureBits[0] & FEATURE1_TCOEF) == 0;
	} else if (error == Tfa98xx_Error_RpcParamId) {
		/* feature bits available in MTP directly */
		error =
		    Tfa98xx_ReadRegister16(handle, TFA98XX_MTP + 6,
					   &featureBits1);
		if (error == Tfa98xx_Error_Ok) {
			/* bit=0 means tCoef expected */
			*pbSupporttCoef = (featureBits1 & FEATURE1_TCOEF) == 0;
		}

	}
	/* else some other error, return transparantly */

	/* pbSupporttCoef only changed when error == Tfa98xx_Error_Ok */
	return error;
}


enum Tfa98xx_Error
Tfa98xx_DspWriteConfig(Tfa98xx_handle_t handle, int length,
		       const unsigned char *pConfigBytes)
{
	enum Tfa98xx_Error error = Tfa98xx_Error_Ok;
	error =
	    Tfa98xx_DspSetParam(handle, MODULE_SPEAKERBOOST,
				SB_PARAM_SET_CONFIG, length, pConfigBytes);


	return error;
}

enum Tfa98xx_Error
Tfa98xx_DspWriteConfigMultiple(int handle_cnt,
			       Tfa98xx_handle_t handles[],
			       int length, const unsigned char *pConfigBytes)
{
	enum Tfa98xx_Error error = Tfa98xx_Error_Ok;
	error =
	    Tfa98xx_DspSetParamMultiple(handle_cnt, handles,
					MODULE_SPEAKERBOOST,
					SB_PARAM_SET_CONFIG, length,
					pConfigBytes);

	return error;
}

enum Tfa98xx_Error
Tfa98xx_DspWritePreset(Tfa98xx_handle_t handle, int length,
		       const unsigned char *pPresetBytes)
{
	enum Tfa98xx_Error error = Tfa98xx_Error_Ok;
	if (pPresetBytes != 0) {
		/* by design: keep the data opaque and no
		 * interpreting/calculation */
		error =
		    Tfa98xx_DspSetParam(handle, MODULE_SPEAKERBOOST,
					SB_PARAM_SET_PRESET, length,
					pPresetBytes);
	} else {
		error = Tfa98xx_Error_Bad_Parameter;
	}
	return error;
}

enum Tfa98xx_Error Tfa98xx_DspReadConfig(Tfa98xx_handle_t handle, int length,
				      unsigned char *pConfigBytes)
{
	enum Tfa98xx_Error error = Tfa98xx_Error_Ok;

	if (pConfigBytes != 0) {
		/* Here one can keep it simple by reading only the first
		 * length bytes from DSP memory */
		error = Tfa98xx_DspGetParam(handle, MODULE_SPEAKERBOOST,
					    SB_PARAM_GET_CONFIG_PRESET, length,
					    pConfigBytes);
	} else {
		error = Tfa98xx_Error_Bad_Parameter;
	}
	return error;
}

enum Tfa98xx_Error Tfa98xx_DspReadPreset(Tfa98xx_handle_t handle, int length,
				      unsigned char *pPresetBytes)
{
	enum Tfa98xx_Error error = Tfa98xx_Error_Ok;
	/* Here one cannot keep it simple as in Tfa98xx_DspReadConfig(), */
	/* since we are interested by the LAST length bytes from DSP memory */
	unsigned char temp[TFA98XX_CONFIG_LENGTH + TFA98XX_PRESET_LENGTH];

	if (pPresetBytes != 0) {
		error = Tfa98xx_DspGetParam(handle, MODULE_SPEAKERBOOST,
					    SB_PARAM_GET_CONFIG_PRESET,
					    (TFA98XX_CONFIG_LENGTH +
					     TFA98XX_PRESET_LENGTH), temp);
		if (error == Tfa98xx_Error_Ok) {
			int i;
			for (i = 0; i < length; i++) {
				pPresetBytes[i] =
				    temp[TFA98XX_CONFIG_LENGTH + i];
			}
		}
	} else {
		error = Tfa98xx_Error_Bad_Parameter;
	}

	return error;
}

enum Tfa98xx_Error
Tfa98xx_DspWritePresetMultiple(int handle_cnt,
			       Tfa98xx_handle_t handles[],
			       int length, const unsigned char *pPresetBytes)
{
	enum Tfa98xx_Error error = Tfa98xx_Error_Ok;
	if (pPresetBytes != 0) {
		/* by design: keep the data opaque and no
		 * interpreting/calculation */
		error =
		    Tfa98xx_DspSetParamMultiple(handle_cnt, handles,
						MODULE_SPEAKERBOOST,
						SB_PARAM_SET_PRESET, length,
						pPresetBytes);
	} else
		error = Tfa98xx_Error_Bad_Parameter;

	return error;
}

enum Tfa98xx_Error Tfa98xx_SetVolume(Tfa98xx_handle_t handle, FIXEDPT voldB)
{
	enum Tfa98xx_Error error = Tfa98xx_Error_Ok;
	unsigned short value;
	if (!handle_is_open(handle))
		return Tfa98xx_Error_NotOpen;
#ifdef __KERNEL__
	if (voldB <= TO_FIXED(0)) {
#else
	if (voldB <= (0)) {
#endif
		/* 0x00 -> 0.0 dB
		 * 0x01 -> -0.5 dB
		 * ...
		 * 0xFE -> -127dB
		 * 0xFF -> muted
		 */
		error =
		    Tfa98xx_ReadRegister16(handle, TFA98XX_AUDIO_CTR, &value);
	} else {
		error = Tfa98xx_Error_Bad_Parameter;
	}
	if (error == Tfa98xx_Error_Ok) {
		int volume_value;
#ifdef __KERNEL__
		volume_value = TO_INT(voldB * -2);
#else
		volume_value = (int)(voldB / (-0.5f));
#endif
		if (volume_value > 255)
			volume_value = 255;
		/* volume value is in the top 8 bits of the register */
		value = (value & 0x00FF) | (unsigned short)(volume_value << 8);
		error =
		    Tfa98xx_WriteRegister16(handle, TFA98XX_AUDIO_CTR, value);
	}
	return error;
}

enum Tfa98xx_Error Tfa98xx_GetVolume(Tfa98xx_handle_t handle, FIXEDPT *pVoldB)
{
	enum Tfa98xx_Error error;
	unsigned short value;
	if (!handle_is_open(handle))
		return Tfa98xx_Error_NotOpen;
	_ASSERT(pVoldB != 0);
	error = Tfa98xx_ReadRegister16(handle, TFA98XX_AUDIO_CTR, &value);
	if (error == Tfa98xx_Error_Ok) {
		value >>= 8;
#ifdef __KERNEL__
		*pVoldB = TO_FIXED(value) / -2;
#else
		*pVoldB = (-0.5f) * value;
#endif
	}
	return error;
}

enum Tfa98xx_Error
Tfa98xx_SetMute(Tfa98xx_handle_t handle, enum Tfa98xx_Mute mute)
{
	enum Tfa98xx_Error error;
	unsigned short audioctrl_value;
	unsigned short sysctrl_value;
	if (!handle_is_open(handle))
		return Tfa98xx_Error_NotOpen;
	error =
	    Tfa98xx_ReadRegister16(
		handle, TFA98XX_AUDIO_CTR, &audioctrl_value);
	if (error != Tfa98xx_Error_Ok)
		return error;
	error =
	    Tfa98xx_ReadRegister16(handle, TFA98XX_SYS_CTRL, &sysctrl_value);
	if (error != Tfa98xx_Error_Ok)
		return error;
	switch (mute) {
	case Tfa98xx_Mute_Off:
		/* previous state can be digital or amplifier mute,
		 * clear the cf_mute and set the enbl_amplifier bits
		 *
		 * To reduce PLOP at power on it is needed to switch the
		 * amplifier on with the DCDC in follower mode
		 * (enbl_boost = 0 ?).
		 * This workaround is also needed when toggling the
		 * powerdown bit!
		 */
		audioctrl_value &= ~(TFA98XX_AUDIO_CTR_CFSM_MSK);
		sysctrl_value |= (TFA98XX_SYS_CTRL_AMPE_MSK |
				  TFA98XX_SYS_CTRL_DCA_MSK);
		break;
	case Tfa98xx_Mute_Digital:
		/* expect the amplifier to run */
		/* set the cf_mute bit */
		audioctrl_value |= TFA98XX_AUDIO_CTR_CFSM_MSK;
		/* set the enbl_amplifier bit */
		sysctrl_value |= (TFA98XX_SYS_CTRL_AMPE_MSK);
		/* clear active mode */
		sysctrl_value &= ~(TFA98XX_SYS_CTRL_DCA_MSK);
		break;
	case Tfa98xx_Mute_Amplifier:
		/* clear the cf_mute bit */
		audioctrl_value &= ~TFA98XX_AUDIO_CTR_CFSM_MSK;
		/* clear the enbl_amplifier bit and active mode */
		sysctrl_value &=
		    ~(TFA98XX_SYS_CTRL_AMPE_MSK | TFA98XX_SYS_CTRL_DCA_MSK);
		break;
	default:
		error = Tfa98xx_Error_Bad_Parameter;
	}
	if (error != Tfa98xx_Error_Ok)
		return error;
	error =
	    Tfa98xx_WriteRegister16(handle, TFA98XX_AUDIO_CTR,
					audioctrl_value);
	if (error != Tfa98xx_Error_Ok)
		return error;
	error =
	    Tfa98xx_WriteRegister16(handle, TFA98XX_SYS_CTRL, sysctrl_value);
	return error;
}

enum Tfa98xx_Error
Tfa98xx_GetMute(Tfa98xx_handle_t handle, enum Tfa98xx_Mute *pMute)
{
	enum Tfa98xx_Error error;
	unsigned short audioctrl_value;
	unsigned short sysctrl_value;
	if (!handle_is_open(handle))
		return Tfa98xx_Error_NotOpen;
	_ASSERT(pMute != 0);
	error =
	    Tfa98xx_ReadRegister16(
		handle, TFA98XX_AUDIO_CTR, &audioctrl_value);
	if (error != Tfa98xx_Error_Ok)
		return error;
	error =
	    Tfa98xx_ReadRegister16(handle, TFA98XX_SYS_CTRL, &sysctrl_value);
	if (error != Tfa98xx_Error_Ok)
		return error;
	if (sysctrl_value & TFA98XX_SYS_CTRL_AMPE_MSK) {
		/* amplifier is enabled */
		if (audioctrl_value & (TFA98XX_AUDIO_CTR_CFSM_MSK))
			*pMute = Tfa98xx_Mute_Digital;
		else
			*pMute = Tfa98xx_Mute_Off;
	} else {
		/* amplifier disabled. */
		*pMute = Tfa98xx_Mute_Amplifier;
	}
	return Tfa98xx_Error_Ok;
}

void Tfa98xx_ConvertData2Bytes(int num_data, const int24 data[],
			       unsigned char bytes[])
{
	int i;			/* index for data */
	int k;			/* index for bytes */
	int d;
	/* note: cannot just take the lowest 3 bytes from the 32 bit
	 * integer, because also need to take care of clipping any
	 * value > 2&23 */
	for (i = 0, k = 0; i < num_data; ++i, k += 3) {
		if (data[i] >= 0)
			d = MIN(data[i], (1 << 23) - 1);
		else {
			/* 2's complement */
			d = (1 << 24) - MIN(-data[i], 1 << 23);
		}
		_ASSERT(d >= 0);
		_ASSERT(d < (1 << 24));	/* max 24 bits in use */
		bytes[k] = (d >> 16) & 0xFF;	/* MSB */
		bytes[k + 1] = (d >> 8) & 0xFF;
		bytes[k + 2] = (d) & 0xFF;	/* LSB */
	}
}

/*
 * Floating point calculations must be done in user-space
 */
static enum Tfa98xx_Error
calcBiquadCoeff(FIXEDPT b0, FIXEDPT b1, FIXEDPT b2,
		FIXEDPT a1, FIXEDPT a2,
		unsigned char bytes[BIQUAD_COEFF_SIZE * 3])
{
	FIXEDPT max_coeff;
	FIXEDPT mask;
	int headroom;
	int24 coeff_buffer[BIQUAD_COEFF_SIZE];
	/* find max value in coeff to define a scaler */
#ifdef __KERNEL__
	max_coeff = abs(b0);
	if (abs(b1) > max_coeff)
		max_coeff = abs(b1);
	if (abs(b2) > max_coeff)
		max_coeff = abs(b2);
	if (abs(a1) > max_coeff)
		max_coeff = abs(a1);
	if (abs(a2) > max_coeff)
		max_coeff = abs(a2);
	/* round up to power of 2 */
	mask = 0x0040000000000000;
	for (headroom = 23; headroom > 0; headroom--) {
		if (max_coeff & mask)
			break;
		mask >>= 1;
	}
#else
	max_coeff = (float)fabs(b0);
	if (fabs(b1) > max_coeff)
		max_coeff = (float)fabs(b1);
	if (fabs(b2) > max_coeff)
		max_coeff = (float)fabs(b2);
	if (fabs(a1) > max_coeff)
		max_coeff = (float)fabs(a1);
	if (fabs(a2) > max_coeff)
		max_coeff = (float)fabs(a2);
	/* round up to power of 2 */
	headroom = (int)ceil(log(max_coeff + pow(2.0, -23)) / log(2.0));
#endif
	/* some sanity checks on headroom */
	if (headroom > 8)
		return Tfa98xx_Error_Bad_Parameter;
	if (headroom < 0)
		headroom = 0;
	/* set in correct order and format for the DSP */
	coeff_buffer[0] = headroom;
#ifdef __KERNEL__
	coeff_buffer[1] = (int24) TO_INT(-a2 * (1 << (23 - headroom)));
	coeff_buffer[2] = (int24) TO_INT(-a1 * (1 << (23 - headroom)));
	coeff_buffer[3] = (int24) TO_INT(b2 * (1 << (23 - headroom)));
	coeff_buffer[4] = (int24) TO_INT(b1 * (1 << (23 - headroom)));
	coeff_buffer[5] = (int24) TO_INT(b0 * (1 << (23 - headroom)));
#else
	coeff_buffer[1] = (int24) (-a2 * pow(2.0, 23 - headroom));
	coeff_buffer[2] = (int24) (-a1 * pow(2.0, 23 - headroom));
	coeff_buffer[3] = (int24) (b2 * pow(2.0, 23 - headroom));
	coeff_buffer[4] = (int24) (b1 * pow(2.0, 23 - headroom));
	coeff_buffer[5] = (int24) (b0 * pow(2.0, 23 - headroom));
#endif

	/* convert to fixed point and then bytes suitable for
	 * transmission over I2C */
	Tfa98xx_ConvertData2Bytes(BIQUAD_COEFF_SIZE, coeff_buffer, bytes);
	return Tfa98xx_Error_Ok;
}

enum Tfa98xx_Error
Tfa98xx_DspBiquad_SetCoeff(Tfa98xx_handle_t handle,
			   int biquad_index, FIXEDPT b0,
			   FIXEDPT b1, FIXEDPT b2, FIXEDPT a1, FIXEDPT a2)
{
	enum Tfa98xx_Error error;
	unsigned char bytes[BIQUAD_COEFF_SIZE * 3];
	if (!handle_is_open(handle))
		return Tfa98xx_Error_NotOpen;
	if (biquad_index > TFA98XX_BIQUAD_NUM)
		return Tfa98xx_Error_Bad_Parameter;
	if (biquad_index < 1)
		return Tfa98xx_Error_Bad_Parameter;
	error = calcBiquadCoeff(b0, b1, b2, a1, a2, bytes);
	if (error == Tfa98xx_Error_Ok) {
		error =
		    Tfa98xx_DspSetParam(handle, MODULE_BIQUADFILTERBANK,
					(unsigned char)biquad_index,
					(BIQUAD_COEFF_SIZE * 3), bytes);
	}
	return error;
}

enum Tfa98xx_Error
Tfa98xx_DspBiquad_SetCoeffMultiple(int handle_cnt,
				   Tfa98xx_handle_t handles[],
				   int biquad_index, FIXEDPT b0,
				   FIXEDPT b1, FIXEDPT b2, FIXEDPT a1,
				   FIXEDPT a2)
{
	enum Tfa98xx_Error error;
	unsigned char bytes[BIQUAD_COEFF_SIZE * 3];
	if (biquad_index > TFA98XX_BIQUAD_NUM)
		return Tfa98xx_Error_Bad_Parameter;
	if (biquad_index < 1)
		return Tfa98xx_Error_Bad_Parameter;
	error = calcBiquadCoeff(b0, b1, b2, a1, a2, bytes);
	if (error == Tfa98xx_Error_Ok) {
		error =
		    Tfa98xx_DspSetParamMultiple(handle_cnt, handles,
					MODULE_BIQUADFILTERBANK,
					(unsigned char)biquad_index,
					(BIQUAD_COEFF_SIZE * 3), bytes);
	}
	return error;
}

enum Tfa98xx_Error
Tfa98xx_DspBiquad_SetAllCoeff(Tfa98xx_handle_t handle,
			      FIXEDPT b[TFA98XX_BIQUAD_NUM * 5])
{
	enum Tfa98xx_Error error = Tfa98xx_Error_Ok;
	int i;
	unsigned char bytes[BIQUAD_COEFF_SIZE * TFA98XX_BIQUAD_NUM * 3];
	for (i = 0; (i < TFA98XX_BIQUAD_NUM) && (error == Tfa98xx_Error_Ok);
	     ++i) {
		error =
		    calcBiquadCoeff(b[i * 5], b[i * 5 + 1], b[i * 5 + 2],
				    b[i * 5 + 3], b[i * 5 + 4],
				    bytes + i * BIQUAD_COEFF_SIZE * 3);
	}
	if (error == Tfa98xx_Error_Ok) {
		error =
		    Tfa98xx_DspSetParam(handle, MODULE_BIQUADFILTERBANK,
					0 /* program all at once */ ,
					(unsigned char)(BIQUAD_COEFF_SIZE *
						TFA98XX_BIQUAD_NUM * 3),
					bytes);
	}
	return error;
}

enum Tfa98xx_Error
Tfa98xx_DspBiquad_SetAllCoeffMultiple(int handle_cnt,
				      Tfa98xx_handle_t
				      handles[],
				      FIXEDPT b[TFA98XX_BIQUAD_NUM * 5])
{
	enum Tfa98xx_Error error = Tfa98xx_Error_Ok;
	int i;
	unsigned char bytes[BIQUAD_COEFF_SIZE * TFA98XX_BIQUAD_NUM * 3];
	for (i = 0; (i < TFA98XX_BIQUAD_NUM) && (error == Tfa98xx_Error_Ok);
	     ++i) {
		error =
		    calcBiquadCoeff(b[i * 5], b[i * 5 + 1], b[i * 5 + 2],
				    b[i * 5 + 3], b[i * 5 + 4],
				    bytes + i * BIQUAD_COEFF_SIZE * 3);
	}
	if (error == Tfa98xx_Error_Ok) {
		error =
		    Tfa98xx_DspSetParamMultiple(handle_cnt, handles,
						MODULE_BIQUADFILTERBANK,
						0 /* program all at once */ ,
						(unsigned
						 char)(BIQUAD_COEFF_SIZE *
						       TFA98XX_BIQUAD_NUM * 3),
						bytes);
	}
	return error;
}

enum Tfa98xx_Error
Tfa98xx_DspBiquad_SetCoeffBytes(Tfa98xx_handle_t handle,
				int biquad_index,
				const unsigned char *pBiquadBytes)
{
	enum Tfa98xx_Error error = Tfa98xx_Error_Ok;
	if (!handle_is_open(handle))
		return Tfa98xx_Error_NotOpen;
	if (biquad_index > TFA98XX_BIQUAD_NUM)
		return Tfa98xx_Error_Bad_Parameter;
	if (biquad_index < 1)
		return Tfa98xx_Error_Bad_Parameter;
	if (pBiquadBytes == 0)
		return Tfa98xx_Error_Bad_Parameter;

	if (error == Tfa98xx_Error_Ok) {
		error = Tfa98xx_DspSetParam(handle, MODULE_BIQUADFILTERBANK,
					    (unsigned char)biquad_index,
					    (BIQUAD_COEFF_SIZE * 3),
					    pBiquadBytes);
	}
	return error;
}

enum Tfa98xx_Error
Tfa98xx_DspBiquad_SetCoeffMultipleBytes(int handle_cnt,
					Tfa98xx_handle_t handles[],
					int biquad_index,
					const unsigned char *pBiquadBytes)
{
	enum Tfa98xx_Error error;
	if (biquad_index > TFA98XX_BIQUAD_NUM)
		return Tfa98xx_Error_Bad_Parameter;
	if (biquad_index < 1)
		return Tfa98xx_Error_Bad_Parameter;
	if (pBiquadBytes == 0)
		return Tfa98xx_Error_Bad_Parameter;
	error =
	    Tfa98xx_DspSetParamMultiple(handle_cnt, handles,
					MODULE_BIQUADFILTERBANK,
					(unsigned char)biquad_index,
					(BIQUAD_COEFF_SIZE * 3),
					pBiquadBytes);
	return error;
}

enum Tfa98xx_Error
Tfa98xx_DspBiquad_Disable(Tfa98xx_handle_t handle, int biquad_index)
{
	enum Tfa98xx_Error error = Tfa98xx_Error_Ok;
	int24 coeff_buffer[BIQUAD_COEFF_SIZE];
	unsigned char bytes[BIQUAD_COEFF_SIZE * 3];
	if (biquad_index > TFA98XX_BIQUAD_NUM)
		return Tfa98xx_Error_Bad_Parameter;
	if (biquad_index < 1)
		return Tfa98xx_Error_Bad_Parameter;
	/* set in correct order and format for the DSP */
	coeff_buffer[0] = (int24) - 8388608;	/* -1.0f */
	coeff_buffer[1] = 0;
	coeff_buffer[2] = 0;
	coeff_buffer[3] = 0;
	coeff_buffer[4] = 0;
	coeff_buffer[5] = 0;
	/* convert to fixed point and then bytes suitable for
	 * transmaission over I2C */
	Tfa98xx_ConvertData2Bytes(BIQUAD_COEFF_SIZE, coeff_buffer, bytes);
	error =
	    Tfa98xx_DspSetParam(handle, MODULE_BIQUADFILTERBANK,
				(unsigned char)biquad_index,
				(unsigned char)BIQUAD_COEFF_SIZE * 3, bytes);
	return error;
}

enum Tfa98xx_Error
Tfa98xx_DspBiquad_DisableMultiple(int handle_cnt,
				  Tfa98xx_handle_t handles[], int biquad_index)
{
	enum Tfa98xx_Error error = Tfa98xx_Error_Ok;
	int24 coeff_buffer[BIQUAD_COEFF_SIZE];
	unsigned char bytes[BIQUAD_COEFF_SIZE * 3];
	if (biquad_index > TFA98XX_BIQUAD_NUM)
		return Tfa98xx_Error_Bad_Parameter;
	if (biquad_index < 1)
		return Tfa98xx_Error_Bad_Parameter;

	/* set in correct order and format for the DSP */
	coeff_buffer[0] = (int24) - 8388608;	/* -1.0f */
	coeff_buffer[1] = 0;
	coeff_buffer[2] = 0;
	coeff_buffer[3] = 0;
	coeff_buffer[4] = 0;
	coeff_buffer[5] = 0;
	/* convert to fixed point and then bytes suitable for
	 * transmaission over I2C */
	Tfa98xx_ConvertData2Bytes(BIQUAD_COEFF_SIZE, coeff_buffer, bytes);
	error =
	    Tfa98xx_DspSetParamMultiple(handle_cnt, handles,
					MODULE_BIQUADFILTERBANK,
					(unsigned char)biquad_index,
					(unsigned char)BIQUAD_COEFF_SIZE * 3,
					bytes);
	return error;
}

#define PATCH_HEADER_LENGTH 6
enum Tfa98xx_Error
Tfa98xx_DspPatch(Tfa98xx_handle_t handle, int patchLength,
		 const unsigned char *patchBytes)
{
	enum Tfa98xx_Error error;
	if (!handle_is_open(handle))
		return Tfa98xx_Error_NotOpen;
	if (patchLength < PATCH_HEADER_LENGTH)
		return Tfa98xx_Error_Bad_Parameter;
	error = checkICROMversion(handle, patchBytes);
	if (Tfa98xx_Error_Ok != error)
		return error;
	error =
	    processPatchFile(handle, patchLength - PATCH_HEADER_LENGTH,
			     patchBytes + PATCH_HEADER_LENGTH);
	return error;
}

/* read the return code for the RPC call */
static enum Tfa98xx_Error
checkRpcStatus(Tfa98xx_handle_t handle, int *pRpcStatus)
{
	enum Tfa98xx_Error error = Tfa98xx_Error_Ok;
	/* the value to sent to the * CF_CONTROLS register: cf_req=00000000,
	 * cf_int=0, cf_aif=0, cf_dmem=XMEM=01, cf_rst_dsp=0 */
	unsigned short cf_ctrl = 0x0002;
	/* memory address to be accessed (0: Status, 1: ID, 2: parameters) */
	unsigned short cf_mad = 0x0000;
	unsigned char mem[3];	/* for the status read from DSP memory */
	if (!handle_is_open(handle))
		return Tfa98xx_Error_NotOpen;
	if (pRpcStatus == 0)
		return Tfa98xx_Error_Bad_Parameter;
#ifdef OPTIMIZED_RPC
	{
		/* minimize the number of I2C transactions by making use
		 * of the autoincrement in I2C */
		unsigned char buffer[4];
		/* first the data for CF_CONTROLS */
		buffer[0] = (unsigned char)((cf_ctrl >> 8) & 0xFF);
		buffer[1] = (unsigned char)(cf_ctrl & 0xFF);
		/* write the contents of CF_MAD which is the subaddress
		 * following CF_CONTROLS */
		buffer[2] = (unsigned char)((cf_mad >> 8) & 0xFF);
		buffer[3] = (unsigned char)(cf_mad & 0xFF);
		error =
		    Tfa98xx_WriteData(handle, TFA98XX_CF_CONTROLS,
				      sizeof(buffer), buffer);
	}
#else /* OPTIMIZED_RPC */
	/* 1) write DMEM=XMEM to the DSP XMEM */
	if (error == Tfa98xx_Error_Ok) {
		error =
		    Tfa98xx_WriteRegister16(handle, TFA98XX_CF_CONTROLS,
					    cf_ctrl);
	}
	if (error == Tfa98xx_Error_Ok) {
		/* write the address in XMEM where to read */
		error = Tfa98xx_WriteRegister16(
			handle, TFA98XX_CF_MAD, cf_mad);
	}
#endif /* OPTIMIZED_RPC */
	if (error == Tfa98xx_Error_Ok) {
		/* read 1 word (24 bit) from XMEM */
		error =
		    Tfa98xx_ReadData(handle, TFA98XX_CF_MEM,
				     3 /*sizeof(mem) */ , mem);
	}
	if (error == Tfa98xx_Error_Ok)
		*pRpcStatus = (mem[0] << 16) | (mem[1] << 8) | mem[2];
	return error;
}

/* check that num_byte matches the memory type selected */
static enum Tfa98xx_Error
check_size(enum Tfa98xx_DMEM which_mem, int num_bytes)
{
	enum Tfa98xx_Error error = Tfa98xx_Error_Ok;
	int modulo_size = 1;
	switch (which_mem) {
	case Tfa98xx_DMEM_PMEM:
		/* 32 bit PMEM */
		modulo_size = 4;
		break;
	case Tfa98xx_DMEM_XMEM:
	case Tfa98xx_DMEM_YMEM:
	case Tfa98xx_DMEM_IOMEM:
		/* 24 bit MEM */
		modulo_size = 3;
		break;
	default:
		error = Tfa98xx_Error_Bad_Parameter;
	}
	if (error == Tfa98xx_Error_Ok) {
		if ((num_bytes % modulo_size) != 0)
			error = Tfa98xx_Error_Bad_Parameter;

	}
	return error;
}

static enum Tfa98xx_Error
writeParameter(Tfa98xx_handle_t handle,
	       unsigned char module_id,
	       unsigned char param_id,
	       int num_bytes, const unsigned char data[])
{
	enum Tfa98xx_Error error;
	/* the value to be sent to the CF_CONTROLS register: cf_req=00000000,
	 * cf_int=0, cf_aif=0, cf_dmem=XMEM=01, cf_rst_dsp=0 */
	unsigned short cf_ctrl = 0x0002;
	/* memory address to be accessed (0 : Status, 1 : ID, 2 : parameters)*/
	unsigned short cf_mad = 0x0001;
	error = check_size(Tfa98xx_DMEM_XMEM, num_bytes);
	if (error == Tfa98xx_Error_Ok) {
		if ((num_bytes <= 0) || (num_bytes > MAX_PARAM_SIZE))
			error = Tfa98xx_Error_Bad_Parameter;
	}
#ifdef OPTIMIZED_RPC
	{
		/* minimize the number of I2C transactions by making use of
		 * the autoincrement in I2C */
		unsigned char buffer[7];
		/* first the data for CF_CONTROLS */
		buffer[0] = (unsigned char)((cf_ctrl >> 8) & 0xFF);
		buffer[1] = (unsigned char)(cf_ctrl & 0xFF);
		/* write the contents of CF_MAD which is the subaddress
		 * following CF_CONTROLS */
		buffer[2] = (unsigned char)((cf_mad >> 8) & 0xFF);
		buffer[3] = (unsigned char)(cf_mad & 0xFF);
		/* write the module and RPC id into CF_MEM, which
		 * follows CF_MAD */
		buffer[4] = 0;
		buffer[5] = module_id + 128;
		buffer[6] = param_id;
		error =
		    Tfa98xx_WriteData(handle, TFA98XX_CF_CONTROLS,
				      sizeof(buffer), buffer);
	}
#else /*
       */
	if (error == Tfa98xx_Error_Ok) {
		error =
		    Tfa98xx_WriteRegister16(handle, TFA98XX_CF_CONTROLS,
					    cf_ctrl);
	}
	if (error == Tfa98xx_Error_Ok) {
		error = Tfa98xx_WriteRegister16(handle, TFA98XX_CF_MAD,
							cf_mad);
	}
	if (error == Tfa98xx_Error_Ok) {
		unsigned char id[3];
		id[0] = 0;
		id[1] = module_id + 128;
		id[2] = param_id;
		error = Tfa98xx_WriteData(handle, TFA98XX_CF_MEM, 3, id);
	}
#endif /* OPTIMIZED_RPC */
	if (error == Tfa98xx_Error_Ok) {
		int offset = 0;
		int chunk_size =
			ROUND_DOWN(NXP_I2C_BufferSize(), 3);
			/* XMEM word size */
		int remaining_bytes = num_bytes;
		/* due to autoincrement in cf_ctrl, next write will happen at
		 * the next address */
		while ((error == Tfa98xx_Error_Ok) && (remaining_bytes > 0)) {
			if (remaining_bytes < chunk_size)
				chunk_size = remaining_bytes;
			/* else chunk_size remains at initialize value above */
			error =
			    Tfa98xx_WriteData(handle, TFA98XX_CF_MEM,
					      chunk_size, data + offset);
			remaining_bytes -= chunk_size;
			offset += chunk_size;
		}
	}
	return error;
}

static enum Tfa98xx_Error executeParam(Tfa98xx_handle_t handle)
{
	enum Tfa98xx_Error error;
	/* the value to be sent to the CF_CONTROLS register: cf_req=00000000,
	 * cf_int=0, cf_aif=0, cf_dmem=XMEM=01, cf_rst_dsp=0 */
	unsigned short cf_ctrl = 0x0002;
	cf_ctrl |= (1 << 8) | (1 << 4);	/* set the cf_req1 and cf_int bit */
	error = Tfa98xx_WriteRegister16(handle, TFA98XX_CF_CONTROLS, cf_ctrl);
	return error;
}

static enum Tfa98xx_Error
waitResult(Tfa98xx_handle_t handle, int waitRetryCount)
{
	enum Tfa98xx_Error error;
	unsigned short cf_status; /* the contents of the CF_STATUS register */
	int tries = 0;
	do {
		error =
		    Tfa98xx_ReadRegister16(handle, TFA98XX_CF_STATUS,
					   &cf_status);
		tries++;
	}
	/* don't wait forever, DSP is pretty quick to respond (< 1ms) */
	while ((error == Tfa98xx_Error_Ok) && ((cf_status & 0x0100) == 0)
			&& (tries < waitRetryCount))
		;
	if (tries >= waitRetryCount) {
		/* something wrong with communication with DSP */
		error = Tfa98xx_Error_DSP_not_running;
	}
	return error;
}

/* Execute RPC protocol to write something to the DSP */
enum Tfa98xx_Error
Tfa98xx_DspSetParamVarWait(Tfa98xx_handle_t handle,
			   unsigned char module_id,
			   unsigned char param_id, int num_bytes,
			   const unsigned char data[], int waitRetryCount)
{
	enum Tfa98xx_Error error;
	int rpcStatus = STATUS_OK;
	if (!handle_is_open(handle))
		return Tfa98xx_Error_NotOpen;
	/* 1) write the id and data to the DSP XMEM */
	error = writeParameter(handle, module_id, param_id, num_bytes, data);
	/* 2) wake up the DSP and let it process the data */
	if (error == Tfa98xx_Error_Ok)
		error = executeParam(handle);
	/* check the result when addressed an IC uniquely */
	if (handlesLocal[handle].slave_address !=
				TFA98XX_GENERIC_SLAVE_ADDRESS) {
		/* 3) wait for the ack */
		if (error == Tfa98xx_Error_Ok)
			error = waitResult(handle, waitRetryCount);
		/* 4) check the RPC return value */
		if (error == Tfa98xx_Error_Ok)
			error = checkRpcStatus(handle, &rpcStatus);
		if (error == Tfa98xx_Error_Ok) {
			if (rpcStatus != STATUS_OK) {
				/* DSP RPC call returned an error */
				error =
				    (enum Tfa98xx_Error) (rpcStatus +
						       Tfa98xx_Error_RpcBase);
			}
		}
	}
	return error;
}

/* Execute RPC protocol to write something to the DSP */
enum Tfa98xx_Error
Tfa98xx_DspSetParam(Tfa98xx_handle_t handle,
		    unsigned char module_id,
		    unsigned char param_id, int num_bytes,
		    const unsigned char data[])
{
	/* Use small WaitResult retry count */
	return Tfa98xx_DspSetParamVarWait(handle, module_id, param_id,
					  num_bytes, data,
					  TFA98XX_WAITRESULT_NTRIES);
}

/* Execute RPC protocol to write something to all the DSPs interleaved,
 * stop at the first error. optimized to minimize the latency between the
 * execution point on the various DSPs */
enum Tfa98xx_Error
Tfa98xx_DspSetParamMultipleVarWait(int handle_cnt,
				   Tfa98xx_handle_t handles[],
				   unsigned char module_id,
				   unsigned char param_id,
				   int num_bytes, const unsigned char data[],
				   int waitRetryCount)
{
	enum Tfa98xx_Error error = Tfa98xx_Error_Ok;
	int i;
	int rpcStatus = STATUS_OK;
	for (i = 0; i < handle_cnt; ++i) {
		if (!handle_is_open(handles[i]))
			return Tfa98xx_Error_NotOpen;
	}
	/* from here onward, any error will fall through without executing the
	 * following for loops */
	/* 1) write the id and data to the DSP XMEM */
	for (i = 0; (i < handle_cnt) && (error == Tfa98xx_Error_Ok); ++i)
		error =
		    writeParameter(handles[i], module_id, param_id, num_bytes,
				   data);
	/* 2) wake up the DSP and let it process the data */
	for (i = 0; (i < handle_cnt) && (error == Tfa98xx_Error_Ok); ++i)
		error = executeParam(handles[i]);
	/* 3) wait for the ack */
	for (i = 0; (i < handle_cnt) && (error == Tfa98xx_Error_Ok); ++i)
		error = waitResult(handles[i], waitRetryCount);
	/* 4) check the RPC return value */
	for (i = 0; (i < handle_cnt) && (error == Tfa98xx_Error_Ok); ++i) {
		error = checkRpcStatus(handles[i], &rpcStatus);
		if (rpcStatus != STATUS_OK) {
			/* DSP RPC call returned an error */
			error =
			    (enum Tfa98xx_Error) (rpcStatus +
					       Tfa98xx_Error_RpcBase);
			/* stop at first error */
			return error;
		}
	}
	return error;
}

/* Execute RPC protocol to write something to all the DSPs interleaved,
 * stop at the first error
 * optimized to minimize the latency between the execution point on the
 * various DSPs.
 * Uses small retry count.
 */
enum Tfa98xx_Error
Tfa98xx_DspSetParamMultiple(int handle_cnt,
			    Tfa98xx_handle_t handles[],
			    unsigned char module_id,
			    unsigned char param_id,
			    int num_bytes, const unsigned char data[])
{
	return Tfa98xx_DspSetParamMultipleVarWait(handle_cnt, handles,
						  module_id, param_id,
						  num_bytes, data,
						  TFA98XX_WAITRESULT_NTRIES);
}

/* Execute RPC protocol to read something from the DSP */
enum Tfa98xx_Error
Tfa98xx_DspGetParam(Tfa98xx_handle_t handle,
		    unsigned char module_id,
		    unsigned char param_id,
		    int num_bytes, unsigned char data[])
{
	enum Tfa98xx_Error error;
	/* the value to be sent to the CF_CONTROLS register: cf_req=00000000,
	 * cf_int=0, cf_aif=0, cf_dmem=XMEM=01, cf_rst_dsp=0 */
	unsigned short cf_ctrl = 0x0002;
	/* memory address to be accessed (0 : Status, 1 : ID, 2 : parameters)*/
	unsigned short cf_mad = 0x0001;
	/* the contents of the CF_STATUS register */
	unsigned short cf_status;
	int rpcStatus = STATUS_OK;
	if (!handle_is_open(handle))
		return Tfa98xx_Error_NotOpen;
	if (handlesLocal[handle].slave_address ==
				TFA98XX_GENERIC_SLAVE_ADDRESS) {
		/* cannot read */
		return Tfa98xx_Error_Bad_Parameter;
	}
	error = check_size(Tfa98xx_DMEM_XMEM, num_bytes);
	if (error == Tfa98xx_Error_Ok) {
		if ((num_bytes <= 0) || (num_bytes > MAX_PARAM_SIZE))
			error = Tfa98xx_Error_Bad_Parameter;
		}
#ifdef OPTIMIZED_RPC
	{
		/* minimize the number of I2C transactions by making use of
		 * the autoincrement in I2C */
		unsigned char buffer[7];
		/* first the data for CF_CONTROLS */
		buffer[0] = (unsigned char)((cf_ctrl >> 8) & 0xFF);
		buffer[1] = (unsigned char)(cf_ctrl & 0xFF);
		/* write the contents of CF_MAD which is the subaddress
		 * following CF_CONTROLS */
		buffer[2] = (unsigned char)((cf_mad >> 8) & 0xFF);
		buffer[3] = (unsigned char)(cf_mad & 0xFF);
		/* write the module and RPC id into CF_MEM,
		 * which follows CF_MAD */
		buffer[4] = 0;
		buffer[5] = module_id + 128;
		buffer[6] = param_id;
		error =
		    Tfa98xx_WriteData(handle, TFA98XX_CF_CONTROLS,
				      sizeof(buffer), buffer);
	}
#else /* OPTIMIZED_RPC */
	/* 1) write the id and data to the DSP XMEM */
	if (error == Tfa98xx_Error_Ok) {
		error =
		    Tfa98xx_WriteRegister16(handle, TFA98XX_CF_CONTROLS,
					    cf_ctrl);
	}
	if (error == Tfa98xx_Error_Ok)
		error = Tfa98xx_WriteRegister16(handle, TFA98XX_CF_MAD, cf_ma);

	if (error == Tfa98xx_Error_Ok) {
		unsigned char id[3];
		id[0] = 0;
		id[1] = module_id + 128;
		id[2] = param_id;
		/* only try MEM once, if error, need to resend mad as well */
		error = Tfa98xx_WriteData(handle, TFA98XX_CF_MEM, 3, id);
	}
#endif /* OPTIMIZED_RPC */
	/* 2) wake up the DSP and let it process the data */
	if (error == Tfa98xx_Error_Ok) {
		/* set the cf_req1 and cf_int bit */
		cf_ctrl |= (1 << 8) | (1 << 4);
		error =
		    Tfa98xx_WriteRegister16(handle, TFA98XX_CF_CONTROLS,
					    cf_ctrl);
	}
	/* 3) wait for the ack */
	if (error == Tfa98xx_Error_Ok) {
		int tries = 0;
		do {
			error =
			    Tfa98xx_ReadRegister16(handle, TFA98XX_CF_STATUS,
						   &cf_status);
			tries++;
		}

		/* don't wait forever, DSP is pretty quick to respond (< 1ms)*/
		while ((error == Tfa98xx_Error_Ok)
			&& ((cf_status & 0x0100) == 0)
			&& (tries < TFA98XX_WAITRESULT_NTRIES))
			;

		if (tries >= TFA98XX_WAITRESULT_NTRIES) {
			/* something wrong with communication with DSP */
			return Tfa98xx_Error_DSP_not_running;
		}
	}
	/* 4) check the RPC return value */
	if (error == Tfa98xx_Error_Ok)
		error = checkRpcStatus(handle, &rpcStatus);
	if (error == Tfa98xx_Error_Ok) {
		if (rpcStatus != STATUS_OK) {
			/* DSP RPC call returned an error */
			error =
			    (enum Tfa98xx_Error) (rpcStatus +
					       Tfa98xx_Error_RpcBase);
		}
	}
	/* 5) read the resulting data */
	if (error == Tfa98xx_Error_Ok) {
		/* memory address to be accessed (0: Status,
		 * 1: ID, 2: parameters) */
		cf_mad = 0x0002;
		error = Tfa98xx_WriteRegister16(
			handle, TFA98XX_CF_MAD, cf_mad);
	}
	if (error == Tfa98xx_Error_Ok) {
		int offset = 0;
		int chunk_size =
		    ROUND_DOWN(NXP_I2C_BufferSize(), 3 /* XMEM word size */);
		int remaining_bytes = num_bytes;
		/* due to autoincrement in cf_ctrl, next write will happen at
		 * the next address */
		while ((error == Tfa98xx_Error_Ok) && (remaining_bytes > 0)) {
			if (remaining_bytes < NXP_I2C_BufferSize())
				chunk_size = remaining_bytes;

			/* else chunk_size remains at initialize value above */
			error = Tfa98xx_ReadData(
				handle, TFA98XX_CF_MEM, chunk_size,
				data + offset);
			remaining_bytes -= chunk_size;
			offset += chunk_size;
		}
	}
	return error;
}

/* convert DSP memory bytes to signed 24 bit integers
   data contains "num_bytes/3" elements
   bytes contains "num_bytes" elements */
void Tfa98xx_ConvertBytes2Data(int num_bytes, const unsigned char bytes[],
			       int24 data[])
{
	int i;			/* index for data */
	int k;			/* index for bytes */
	int d;
	int num_data = num_bytes / 3;
	_ASSERT((num_bytes % 3) == 0);
	for (i = 0, k = 0; i < num_data; ++i, k += 3) {
		d = (bytes[k] << 16) | (bytes[k + 1] << 8) | (bytes[k + 2]);
		_ASSERT(d >= 0);
		_ASSERT(d < (1 << 24));	/* max 24 bits in use */
		if (bytes[k] & 0x80)	/* sign bit was set */
			d = -((1 << 24) - d);

		data[i] = d;
	}
}

enum Tfa98xx_Error
Tfa98xx_DspGetStateInfo(Tfa98xx_handle_t handle,
			struct Tfa98xx_StateInfo *pInfo)
{
	enum Tfa98xx_Error error = Tfa98xx_Error_Ok;
	int24 data[FW_STATE_MAX_SIZE];	/* allocate worst case */
	unsigned char bytes[FW_STATE_MAX_SIZE * 3];
	/* contains the actual amount of parameters transferred,
	 * depends on IC and ROM code version */
	int stateSize = FW_STATE_SIZE;
	int i;
	int bSupportFramework = 0;


	_ASSERT(pInfo != 0);

	/* init to default value to have sane values even when
	 * some features aren't supported */
	for (i = 0; i < FW_STATE_MAX_SIZE; i++)
		data[i] = 0;


	error = Tfa98xx_DspSupportFramework(handle, &bSupportFramework);
	if (error != Tfa98xx_Error_Ok)
		return error;

	if (bSupportFramework) {
		error =
		    Tfa98xx_DspGetParam(handle, MODULE_FRAMEWORK,
					FW_PARAM_GET_STATE, 3 * stateSize,
					bytes);
	} else {
		/* old ROM code, ask SpeakerBoost and only do first portion */
		stateSize = 8;
		error =
		    Tfa98xx_DspGetParam(handle, MODULE_SPEAKERBOOST,
					SB_PARAM_GET_STATE, 3 * stateSize,
					bytes);
	}
	Tfa98xx_ConvertBytes2Data(3 * stateSize, bytes, data);

#ifdef __KERNEL__
	/* /2^23*2^(SPKRBST_AGCGAIN_EXP) */
	pInfo->agcGain = TO_FIXED(data[0]) / (1 << (23 - SPKRBST_AGCGAIN_EXP));
	/* /2^23*2^(SPKRBST_LIMGAIN_EXP) */
	pInfo->limGain = TO_FIXED(data[1]) / (1 << (23 - SPKRBST_LIMGAIN_EXP));
	/* /2^23*2^(SPKRBST_HEADROOM)    */
	pInfo->sMax = TO_FIXED(data[2]) / (1 << (23 - SPKRBST_HEADROOM));
	/* /2^23*2^(def.SPKRBST_TEMPERATURE_EXP) */
	pInfo->T = data[3] / (1 << (23 - SPKRBST_TEMPERATURE_EXP));
	pInfo->statusFlag = data[4];
	/* /2^23*2^(SPKRBST_HEADROOM) */
	pInfo->X1 = TO_FIXED(data[5]) / (1 << (23 - SPKRBST_HEADROOM));
	/* /2^23*2^(SPKRBST_HEADROOM)        */
	pInfo->X2 = TO_FIXED(data[6]) / (1 << (23 - SPKRBST_HEADROOM));
	/* /2^23*2^(SPKRBST_TEMPERATURE_EXP) */
	pInfo->Re = TO_FIXED(data[7]) / (1 << (23 - SPKRBST_TEMPERATURE_EXP));
	pInfo->shortOnMips = data[8];
#else
	/* /2^23*2^(SPKRBST_AGCGAIN_EXP) */
	pInfo->agcGain = (FIXEDPT)data[0] / (1 << (23 - SPKRBST_AGCGAIN_EXP));
	/* /2^23*2^(SPKRBST_LIMGAIN_EXP) */
	pInfo->limGain = (FIXEDPT)data[1] / (1 << (23 - SPKRBST_LIMGAIN_EXP));
	/* /2^23*2^(SPKRBST_HEADROOM)    */
	pInfo->sMax = (FIXEDPT)data[2] / (1 << (23 - SPKRBST_HEADROOM));
	/* /2^23*2^(def.SPKRBST_TEMPERATURE_EXP) */
	pInfo->T = data[3] / (1 << (23 - SPKRBST_TEMPERATURE_EXP));
	pInfo->statusFlag = data[4];
	/* /2^23*2^(SPKRBST_HEADROOM)        */
	pInfo->X1 = (FIXEDPT)data[5] / (1 << (23 - SPKRBST_HEADROOM));
	/* /2^23*2^(SPKRBST_HEADROOM)        */
	pInfo->X2 = (FIXEDPT)data[6] / (1 << (23 - SPKRBST_HEADROOM));
	/* /2^23*2^(SPKRBST_TEMPERATURE_EXP) */
	pInfo->Re = (FIXEDPT)data[7] / (1 << (23 - SPKRBST_TEMPERATURE_EXP));

#endif

	return error;
}

enum Tfa98xx_Error
Tfa98xx_DspGetCalibrationImpedance(Tfa98xx_handle_t handle, FIXEDPT *pRe25)
{
	enum Tfa98xx_Error error = Tfa98xx_Error_Ok;
	unsigned char bytes[3];
	int24 data[1];
	int calibrateDone;

	_ASSERT(pRe25 != 0);
	*pRe25 = 0.0f;		/* default 0.0 */
	error = Tfa98xx_DspReadMem(handle, 231, 1, &calibrateDone);
	if (error == Tfa98xx_Error_Ok) {
		if (!calibrateDone) {
			/* return the default */
			return error;
		}
		error = Tfa98xx_DspGetParam(handle, MODULE_SPEAKERBOOST,
					    SB_PARAM_GET_RE0, 3, bytes);
	}
	if (error == Tfa98xx_Error_Ok) {
		Tfa98xx_ConvertBytes2Data(3, bytes, data);
		/* /2^23*2^(def.SPKRBST_TEMPERATURE_EXP) */
#ifdef __KERNEL__
		*pRe25 = TO_FIXED(data[0]) / (1 << (23 -
						SPKRBST_TEMPERATURE_EXP));
#else
		*pRe25 = (FIXEDPT) data[0] / (1 << (23 -
						SPKRBST_TEMPERATURE_EXP));
#endif
	}
	return error;
}

enum Tfa98xx_Error
Tfa98xx_DspReadMem(Tfa98xx_handle_t handle,
		   unsigned short start_offset, int num_words, int *pValues)
{
	enum Tfa98xx_Error error = Tfa98xx_Error_Ok;
	unsigned short cf_ctrl;	/* to sent to the CF_CONTROLS register */
	unsigned char bytes[MAX_PARAM_SIZE];
	int burst_size;		/* number of words per burst size */
	int bytes_per_word = 3;
	int num_bytes;
	int *p;
	/* first set DMEM and AIF, leaving other bits intact */
	error = Tfa98xx_ReadRegister16(handle, TFA98XX_CF_CONTROLS, &cf_ctrl);
	if (error != Tfa98xx_Error_Ok)
		return error;

	cf_ctrl &= ~0x000E;	/* clear AIF & DMEM */
	/* set DMEM, leave AIF cleared for autoincrement */
	cf_ctrl |= (Tfa98xx_DMEM_XMEM << 1);
	error = Tfa98xx_WriteRegister16(handle, TFA98XX_CF_CONTROLS, cf_ctrl);
	if (error != Tfa98xx_Error_Ok)
		return error;

	error = Tfa98xx_WriteRegister16(handle, TFA98XX_CF_MAD, start_offset);
	if (error != Tfa98xx_Error_Ok)
		return error;

	num_bytes = num_words * bytes_per_word;
	p = pValues;
	for (; num_bytes > 0;) {
		burst_size = ROUND_DOWN(NXP_I2C_BufferSize(), bytes_per_word);
		if (num_bytes < burst_size)
			burst_size = num_bytes;

		_ASSERT(burst_size <= sizeof(bytes));
		error =
		    Tfa98xx_ReadData(
			handle, TFA98XX_CF_MEM, burst_size, bytes);
		if (error != Tfa98xx_Error_Ok)
			return error;

		Tfa98xx_ConvertBytes2Data(burst_size, bytes, p);
		num_bytes -= burst_size;
		p += burst_size / bytes_per_word;
	}
	return Tfa98xx_Error_Ok;
}

enum Tfa98xx_Error
Tfa98xx_DspWriteMem(Tfa98xx_handle_t handle, unsigned short address, int value)
{
	enum Tfa98xx_Error error = Tfa98xx_Error_Ok;
	unsigned short cf_ctrl;	/* to send to the CF_CONTROLS register */
	unsigned char bytes[3];
	/* first set DMEM and AIF, leaving other bits intact */
	error = Tfa98xx_ReadRegister16(handle, TFA98XX_CF_CONTROLS, &cf_ctrl);
	if (error != Tfa98xx_Error_Ok)
		return error;

	cf_ctrl &= ~0x000E;	/* clear AIF & DMEM */
	/* set DMEM, leave AIF cleared for autoincrement */
	cf_ctrl |= (Tfa98xx_DMEM_XMEM << 1);
	error = Tfa98xx_WriteRegister16(handle, TFA98XX_CF_CONTROLS, cf_ctrl);
	if (error != Tfa98xx_Error_Ok)
		return error;
	error = Tfa98xx_WriteRegister16(handle, TFA98XX_CF_MAD, address);
	if (error != Tfa98xx_Error_Ok)
		return error;

	Tfa98xx_ConvertData2Bytes(1, &value, bytes);
	error = Tfa98xx_WriteData(handle, TFA98XX_CF_MEM, 3, bytes);
	if (error != Tfa98xx_Error_Ok)
		return error;

	return Tfa98xx_Error_Ok;
}

/* Execute generic RPC protocol that has both input and output parameters */
enum Tfa98xx_Error
Tfa98xx_DspExecuteRpc(Tfa98xx_handle_t handle,
		      unsigned char module_id,
		      unsigned char param_id, int num_inbytes,
		      unsigned char indata[], int num_outbytes,
		      unsigned char outdata[])
{
	enum Tfa98xx_Error error;
	int rpcStatus = STATUS_OK;
	int i;

	if (!handle_is_open(handle))
		return Tfa98xx_Error_NotOpen;

	/* 1) write the id and data to the DSP XMEM */
	error =
	    writeParameter(handle, module_id, param_id, num_inbytes, indata);

	/* 2) wake up the DSP and let it process the data */
	if (error == Tfa98xx_Error_Ok)
		error = executeParam(handle);

	/* 3) wait for the ack, but not too long */
	if (error == Tfa98xx_Error_Ok)
		error = waitResult(handle, TFA98XX_WAITRESULT_NTRIES);

	/* 4) check the RPC return value */
	if (error == Tfa98xx_Error_Ok)
		error = checkRpcStatus(handle, &rpcStatus);

	if (error == Tfa98xx_Error_Ok) {
		if (rpcStatus != STATUS_OK) {
			/* DSP RPC call returned an error */
			error =
			    (enum Tfa98xx_Error) (rpcStatus +
					       Tfa98xx_Error_RpcBase);
		}
	}

	/* 5) read the resulting data */
	error =
	    Tfa98xx_WriteRegister16(handle, TFA98XX_CF_MAD,
				    2 /*start_offset */);
	if (error != Tfa98xx_Error_Ok)
		return error;

	/* read in chunks, limited by max I2C length */
	for (i = 0; i < num_outbytes;) {
		int burst_size =
		    ROUND_DOWN(NXP_I2C_BufferSize(), 3);  /*bytes_per_word */
		if ((num_outbytes - i) < burst_size)
			burst_size = num_outbytes - i;

		error =
		    Tfa98xx_ReadData(handle, TFA98XX_CF_MEM, burst_size,
				     outdata + i);
		if (error != Tfa98xx_Error_Ok)
			return error;

		i += burst_size;
	}

	return error;
}

enum Tfa98xx_Error
Tfa98xx_DspReadMemory(Tfa98xx_handle_t handle, enum Tfa98xx_DMEM which_mem,
		      unsigned short start_offset, int num_words, int *pValues)
{
	enum Tfa98xx_Error error = Tfa98xx_Error_Ok;

	int input[3];
	unsigned char input_bytes[3 * 3];
	unsigned char output_bytes[80 * 3];

	/* want to read all in 1 RPC call, so limit the max size */
	if (num_words > 80)
		return Tfa98xx_Error_Bad_Parameter;

	input[0] = which_mem;
	input[1] = start_offset;
	input[2] = num_words;
	Tfa98xx_ConvertData2Bytes(3, input, input_bytes);

	error =
	    Tfa98xx_DspExecuteRpc(handle, 0 /* moduleId */ , 5 /* paramId */ ,
				  sizeof(input_bytes), input_bytes,
				  num_words * 3, output_bytes);

	Tfa98xx_ConvertBytes2Data(num_words * 3, output_bytes, pValues);

	return error;
}

enum Tfa98xx_Error Tfa98xx_DspWriteMemory(Tfa98xx_handle_t handle,
				       enum Tfa98xx_DMEM which_mem,
				       unsigned short start_offset,
				       int num_words, int *pValues)
{
	int output[3];
	unsigned char output_bytes[83 * 3];
	int num_bytes;

	/* want to read all in 1 RPC call, so limit the max size */
	if (num_words > 80)
		return Tfa98xx_Error_Bad_Parameter;

	output[0] = which_mem;
	output[1] = start_offset;
	output[2] = num_words;
	Tfa98xx_ConvertData2Bytes(3, output, output_bytes);
	Tfa98xx_ConvertData2Bytes(num_words, pValues, output_bytes + 3 * 3);

	num_bytes = 3 * (num_words + 3);

	return Tfa98xx_DspSetParam(handle, 0 /* framework */ , 4 /* param */ ,
				   num_bytes, output_bytes);
}


#ifdef __KERNEL__
#include <linux/kernel.h>
#define _ASSERT(e) do { if (!(e)) pr_err("PrintAssert:%s (%s:%d)\n",\
		__func__, __FILE__, __LINE__); } while (0)
#else
#include <assert.h>
#define _ASSERT(e)  assert(e)
#endif
#include "../inc/Tfa98xx.h"
#include "../inc/Tfa98xx_Registers.h"

enum Tfa98xx_Error Tfa9890_specific(Tfa98xx_handle_t handle)
{
	enum Tfa98xx_Error error = Tfa98xx_Error_Ok;

	if (!handle_is_open(handle))
		return Tfa98xx_Error_NotOpen;

	/* all i2C registers are already set to default for N1C2 */

	return error;
}

/*
 * Tfa9890_DspSystemStable will compensate for the wrong behavior of CLKS
 * to determine if the DSP subsystem is ready for patch and config loading.
 *
 * A MTP calibration register is checked for non-zero.
 *
 * Note: This only works after i2c reset as this will clear the MTP contents.
 * When we are configured then the DSP communication will synchronize access.
 *
 */
enum Tfa98xx_Error Tfa9890_DspSystemStable(Tfa98xx_handle_t handle, int *ready)
{
	enum Tfa98xx_Error error;
	unsigned short status, mtp0;
	int tries;

	/* check the contents of the STATUS register */
	error = Tfa98xx_ReadRegister16(handle, TFA98XX_STATUSREG, &status);
	if (error)
		goto errorExit;

	/* if AMPS is set then we were already configured and running
	 *   no need to check further
	 */
	*ready = (status & TFA98XX_STATUSREG_AMPS_MSK)
	    == (TFA98XX_STATUSREG_AMPS_MSK);
	if (*ready)		/* if  ready go back */
		return error;	/* will be Tfa98xx_Error_Ok */

	/* check AREFS and CLKS: not ready if either is clear */
	*ready =
	    (status &
	     (TFA98XX_STATUSREG_AREFS_MSK | TFA98XX_STATUSREG_CLKS_MSK))
	    == (TFA98XX_STATUSREG_AREFS_MSK | TFA98XX_STATUSREG_CLKS_MSK);
	if (!*ready)		/* if not ready go back */
		return error;	/* will be Tfa98xx_Error_Ok */

	/* check MTPB
	 *   mtpbusy will be active when the subsys copies MTP to I2C
	 *   2 times retry avoids catching this short mtpbusy active period
	 */
	for (tries = 2; tries > 0; tries--) {
		error =
		    Tfa98xx_ReadRegister16(handle, TFA98XX_STATUSREG, &status);
		if (error)
			goto errorExit;
		/* check the contents of the STATUS register */
		*ready = (status & TFA98XX_STATUSREG_MTPB_MSK) == 0;
		if (*ready)	/* if ready go on */
			break;
	}
	if (tries == 0)		/* ready will be 0 if retries exausted */
		return Tfa98xx_Error_Ok;

	/* check the contents of  MTP register for non-zero,
	 *  this indicates that the subsys is ready  */
	error = Tfa98xx_ReadRegister16(handle, 0x84, &mtp0);
	if (error)
		goto errorExit;

	*ready = (mtp0 != 0);	/* The MTP register written? */

	return error;

errorExit:
	*ready = 0;
	_ASSERT(error);		/* an error here can be considered
				   to be fatal */
	return error;
}

/*
 * Tfa9890_DspReset will deal with clock gating control in order
 * to reset the DSP for warm state restart
 * The CurrentSense4 register is not in the datasheet, define local
 */
#define TFA98XX_CURRENTSENSE4_CTRL_CLKGATECFOFF (1<<2)
#define TFA98XX_CURRENTSENSE4 0x49
enum Tfa98xx_Error Tfa9890_DspReset(Tfa98xx_handle_t handle, int state)
{
	enum Tfa98xx_Error error;
	unsigned short value49, value;

	/* for TFA9890 temporarily disable clock gating
	   when dsp reset is used */
	error = Tfa98xx_ReadRegister16(
		handle, TFA98XX_CURRENTSENSE4, &value49);
	if (error)
		return error;

	if (Tfa98xx_Error_Ok == error) {
		/* clock gating off */
		value = value49 | TFA98XX_CURRENTSENSE4_CTRL_CLKGATECFOFF;
		error =
		    Tfa98xx_WriteRegister16(handle, TFA98XX_CURRENTSENSE4,
					    value);
		if (error)
			return error;
	}

	error = Tfa98xx_ReadRegister16(handle, TFA98XX_CF_CONTROLS, &value);
	if (error)
		return error;

	/* set requested the DSP reset signal state */
	value = state ? (value | TFA98XX_CF_CONTROLS_RST_MSK) :
	    (value & ~TFA98XX_CF_CONTROLS_RST_MSK);

	error = Tfa98xx_WriteRegister16(handle, TFA98XX_CF_CONTROLS, value);

	/* clock gating restore */
	error = Tfa98xx_WriteRegister16(handle, 0x49, value49);
	return error;
}

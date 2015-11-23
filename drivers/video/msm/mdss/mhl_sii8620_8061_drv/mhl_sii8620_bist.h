/* vendor/semc/hardware/mhl/mhl_sii8620_8061_drv/mhl_sii8620_bist.h
 *
 * Copyright (C) 2014 Sony Mobile Communications inc.
 * Copyright (C) 2014 Silicon Image Inc.
 *
 */

#ifndef MHL_SII8620_BIST_H_
#define MHL_SII8620_BIST_H_

/* init/exit */
#ifdef MHL_BIST
void mhl_bist_initilize(struct device *pdev);
void mhl_bist_release(void);
#else
void mhl_bist_initilize(struct device *pdev){}
void mhl_bist_release(void){}
#endif

#endif /* MHL_SII8620_BIST_H_ */

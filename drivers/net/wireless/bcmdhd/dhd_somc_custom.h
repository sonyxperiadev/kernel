/*
 * Copyright (C) 2014 Sony Mobile Communications Inc.
 * All rights, including trade secret rights, reserved.
 */

#ifndef __DHD_SOMC_CUSTOM_H__
#define __DHD_SOMC_CUSTOM_H__

extern int somc_txpower_calibrate(char *nvram, int nvram_len);
extern int somc_update_qtxpower(char *buf, char band, int chain);

#endif /* __DHD_SOMC_CUSTOM_H__ */

/*
* Copyright (C) 2012 Sony Mobile Communications AB.
* All rights, including trade secret rights, reserved.
*
*@ file drivers/platform/msm/forcecrash.h
*
*
*/


#ifndef __POWERKEY_FORCECRASH_H_
#define __POWERKEY_FORCECRASH_H_


void qpnp_powerkey_forcecrash_timer_setup(bool);
void qpnp_powerkey_forcecrash_exit(struct spmi_device *);
int qpnp_powerkey_forcecrash_init(struct spmi_device *, u16);

#endif

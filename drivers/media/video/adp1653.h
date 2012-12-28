/*
 * adp1653 flash driver
 *
 * Copyright (C) 2011 Texas Instruments Incorporated - http://www.ti.com/
 *
 * This program is free software; you can redistribute it and/or
 *modify it under the terms of the GNU General Public License as
 *published by the Free Software Foundation version 2.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any
 *kind, whether express or implied; without even the implied warranty
 *of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */


extern int adp1653_set_ind_led(int iled);
extern int adp1653_set_torch_flash(int hpled);
extern int adp1653_sw_strobe(int on);
extern int adp1653_set_timer(int timer_on, int timer_val);
extern int adp1653_gpio_toggle(bool en);
extern int adp1653_clear_all(void);
extern int adp1653_gpio_strobe(int on);


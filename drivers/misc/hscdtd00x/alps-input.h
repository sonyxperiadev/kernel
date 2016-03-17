/* drivers/misc/hscdtd00x/alps-input.h
 *
 * Input device driver for alps sensor
 *
 * Copyright (C) 2011-2012 ALPS ELECTRIC CO., LTD. All Rights Reserved.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef ___ALPS_INPUT_H_INCLUDED
#define ___ALPS_INPUT_H_INCLUDED

extern int accsns_get_acceleration_data(int *xyz);
extern int hscdtd_get_magnetic_field_data(int *xyz);
extern void hscdtd_activate(int flgatm, int flg, int dtime);
extern void accsns_activate(int flgatm, int flg, int dtime);
extern int hscdtd_self_test_a(void);
extern int hscdtd_self_test_b(void);
extern int hscdtd_get_hardware_data(int *xyz);

#endif

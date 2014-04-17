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


extern int a3907_lens_set_position(int target_position);
extern int a3907_lens_set_position_fine(int target_position);
extern int a3907_lens_get_position(int *current_position,
		int *time_to_destination);
extern int a3907_enable(bool enable);


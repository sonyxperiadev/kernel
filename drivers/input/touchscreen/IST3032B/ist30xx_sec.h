/*
 *  Copyright (C) 2010, Imagis Technology Co. Ltd. All Rights Reserved.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 */

#ifndef __IST30XX_SEC_H__
#define __IST30XX_SEC_H__


#define SEC_CMD_STR_LEN			32
#define SEC_CMD_RESULT_STR_LEN		512
#define SEC_CMD_PARAM_NUM		8

#include "ist30xx_tsp.h"

struct sec_factory {
    struct list_head	cmd_list_head;
	unsigned char       cmd_state;
	char                cmd[SEC_CMD_STR_LEN];
	int                 cmd_param[SEC_CMD_PARAM_NUM];
	char                cmd_result[SEC_CMD_RESULT_STR_LEN];
	struct              mutex cmd_lock;
	bool                cmd_is_running;
};

#endif  // __IST30XX_SEC_H__

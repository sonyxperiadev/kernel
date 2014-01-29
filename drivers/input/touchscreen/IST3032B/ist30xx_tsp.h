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

#ifndef __IST30XX_TSP_H__
#define __IST30XX_TSP_H__

#define IST30XX_DD_VERSION  (2)

#define IST30XX_MULTIPLE_TSP    (0)

#define TSP_CHIP_VENDOR     ("IMAGIS")
#define TSP_CHIP_NAME       ("IST3032B")

#define IST30XXB_PARSE_TSPTYPE(k)   ((k >> 1) & 0xF)

#define TSP_TYPE_UNKNOWN    (0xF0)
#define TSP_TYPE_ALPS       (0xF)
#define TSP_TYPE_EELY       (0xE)
#define TSP_TYPE_TOP        (0xD)
#define TSP_TYPE_DIGITECH   (0xC)
#define TSP_TYPE_ILJIN      (0xB)
#define TSP_TYPE_SYNOPEX    (0xA)
#define TSP_TYPE_SMAC       (0x9)
#define TSP_TYPE_TAEYANG    (0x8)
#define TSP_TYPE_TOVIS      (0x7)
#define TSP_TYPE_ELK        (0x6)

#define FLAG_NODE_Y         (0)
#define FLAG_NODE_X         (1)

#define NODE_TX_NUM         (18)
#define NODE_RX_NUM         (13)
#define NODE_TOTAL_NUM      (TSP_TX_NUM * TSP_RX_NUM)

#define TSP_TX_NUM          (18)
#define TSP_RX_NUM          (12)
#define TSP_TOTAL_NUM       (TSP_TX_NUM * TSP_RX_NUM)

#define TSP_THRESHOLD       (30)
#define TKEY_THRESHOLD      (350)

#endif  // __IST30XX_TSP_H__

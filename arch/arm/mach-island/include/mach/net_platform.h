/*****************************************************************************
* Copyright 2006 - 2008 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a
* license other than the GPL, without Broadcom's express prior written
* consent.
*****************************************************************************/

#ifndef _NET_PLATFORM_H
#define _NET_PLATFORM_H

/*
 * Board dependent configuration for the Ethernet driver
 */
struct island_net_hw_cfg
{
   /*
    * The PHYs are external to the chip, so the connections are dependent on
    * the hardware layout.  The following module arguments can be used to
    * define:
    * -The MDIO PHY address
    * -GPIO pin connected to reset input of the PHY
    */
   int addrPhy0;
   int addrPhy1;

   int gpioPhy0;
   int gpioPhy1;
};

#endif /* _NET_PLATFORM_H */

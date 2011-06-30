#*****************************************************************************
# Copyright 2003 - 2008 Broadcom Corporation.  All rights reserved.
#
# Unless you and Broadcom execute a separate written software license
# agreement governing use of this software, this software is licensed to you
# under the terms of the GNU General Public License version 2, available at
# http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
#
# Notwithstanding the above, under no circumstances may you combine this
# software in any way with any other Broadcom software provided under a
# license other than the GPL, without Broadcom's express prior written
# consent.
#*****************************************************************************
obj-$(CONFIG_FB_BRCM_RHEA) += csl_dsi.o csl_smi.o csl_dma_vc4lite.o csl_tectl_vc4lite.o
# obj-$(CONFIG_BCM_STM) += csl_trace.o
# we have moved SW configuration for STM trace to stm.c so commenting out

# Turn off over zealous gcc warnings
EXTRA_CFLAGS += -D_RHEA_ -Wno-format -Wno-missing-braces

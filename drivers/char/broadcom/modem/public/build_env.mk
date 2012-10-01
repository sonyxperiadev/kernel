#/****************************************************************************
#*
#*	Copyright (c) 1999-2008 Broadcom Corporation
#*
#*   Unless you and Broadcom execute a separate written software license
#*   agreement governing use of this software, this software is licensed to you
#*   under the terms of the GNU General Public License version 2, available
#*    at http://www.gnu.org/licenses/old-licenses/gpl-2.0.html (the "GPL").
#*
#*   Notwithstanding the above, under no circumstances may you combine this
#*   software in any way with any other Broadcom software provided under a
#*   license other than the GPL, without Broadcom's express prior written
#*   consent.
#*
#****************************************************************************/

CHIP=bcm21553

BRCM_PUBLIC_DIR := $(BRCM_ROOTDIR)/public
BRCM_INCLUDES += -I$(BRCM_ROOTDIR)/public -I$(BRCM_ROOTDIR)/public/kernel
BRCM_INCLUDES += -I$(BRCM_PUBLIC_DIR)/soc/public

INCLUDE_XSCRIPT := false

# including these AT specific variables here because they are environment/platform variables
TOOL_PATH = $(BRCM_ROOTDIR)/at/atc/tool
AT_INTERMEDIATE_DIR := $(BRCM_ROOTDIR)/at/atc/tool

BRCM_CFLAGS := -W -Wall -Wno-unused -Winit-self -Wpointer-arith \
			-Werror=return-type -Werror=non-virtual-dtor -Werror=address \
			-Werror=sequence-point -g -Wstrict-aliasing=2 -finline-functions \
			-fno-inline-functions-called-once -fgcse-after-reload \
			-frerun-cse-after-loop -frename-registers -Os -fomit-frame-pointer \
			-fno-strict-aliasing -finline-limit=64 -D__LINENUM__=__LINE__ -MD
# gcc flags for building host files
BRCM_HOST_CFLAGS = $(BRCM_CFLAGS)
# gcc flags for building target files
BRCM_TARGET_CFLAGS := $(BRCM_CFLAGS)
BRCM_TARGET_CFLAGS += -march=armv5te -mtune=xscale -D__ARM_ARCH_5__ \
					-D__ARM_ARCH_5T__ -D__ARM_ARCH_5E__ -D__ARM_ARCH_5TE__ \
					-mthumb -mthumb-interwork -fmessage-length=0
BRCM_TARGET_CFLAGS += -DFUSE_APPS_MODEM -DHAL_INCLUDED -DSTACK_wedge
BRCM_TARGET_CFLAGS += -DBRCM_LINUX_ONLY_BUILD

CC := /projects/mob_tools/CodeSourcery/Sourcery_G++_Lite/bin/arm-none-linux-gnueabi-gcc
CXX := /projects/mob_tools/CodeSourcery/Sourcery_G++_Lite/bin/arm-none-linux-gnueabi-g++
AR := /projects/mob_tools/CodeSourcery/Sourcery_G++_Lite/arm-none-linux-gnueabi/bin/ar

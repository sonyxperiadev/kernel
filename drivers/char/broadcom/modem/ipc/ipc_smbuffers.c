/*
	©2007 Broadcom Corporation

	Unless you and Broadcom execute a separate written software license
	agreement governing use of this software, this software is licensed to you
	under the terms of the GNU General Public License version 2, available
	at http://www.gnu.org/licenses/old-licenses/gpl-2.0.html (the "GPL").


   Notwithstanding the above, under no circumstances may you combine this
   software in any way with any other Broadcom software provided under a license
   other than the GPL, without Broadcom's express prior written consent.
*/

// File to create IPC shared memory buffers
// attribute qualifier to force int variables into .bss

#include "ipc_smbuffers.h"

__attribute__((zero_init)) volatile unsigned char SmBuffer [IPC_SmSize];



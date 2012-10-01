/*******************************************************************************
*  Copyright 2003 - 2011 Broadcom Corporation.  All rights reserved.
*
*  Unless you and Broadcom execute a separate written software license
*  agreement governing use of this software, this software is licensed to you
*  under the terms of the GNU General Public License version 2, available at
*  http://www.gnu.org/copyleft/gpl.html (the "GPL").
*
*  Notwithstanding the above, under no circumstances may you combine this
*  software in any way with any other Broadcom software provided under a
*  license other than the GPL, without Broadcom's express prior written
*  consent.
*
*******************************************************************************/

/*******************************************************************************
 *      SKELETON_DRIVER==1 will build a skeleton driver for debugging
 *      the sysparms interface.  The skeleton driver maps to COMMS sysparm
 *      shared memory but does not implement specific sysparm accessor APIs.
 ******************************************************************************/
#define SKELETON_DRIVER 0

#include <linux/io.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/module.h>
#include <linux/interrupt.h>

#include <linux/broadcom/mobcom_types.h>
#include <linux/broadcom/bcmtypes.h>
#include <plat/kona_reset_reason.h>

#include <mach/comms/platform_mconfig.h>
#include <linux/broadcom/bcm_fuse_sysparm_CIB.h>

static int sysparm_init(void);

static UInt8 *sys_data_dep;
static SysparmIndex_t *sysparm_index;
volatile static UInt32 *sysparm_ready_ind_ptr;
volatile static UInt32 *total_index_ptr;

#if !SKELETON_DRIVER
static SysAudioParm_t __iomem *audio_parm_table;
#if defined(USE_NEW_AUDIO_MM_PARAM)
static SysMultimediaAudioParm_t __iomem *mmaudio_parm_table;
#define MMAUDIO_PARM_TABLE_ENTRIES (AUDIO_APP_MM_NUMBER*AUDIO_MODE_NUMBER)
#define MM_AUDIO_PARM_NAME "ind_mmaudio_parm"
#else
#define MM_AUDIO_PARM_NAME "mmaudio_parm"
#endif
static SysIndMultimediaAudioParm_t __iomem *ind_mmaudio_parm_table;
#endif

static int fuse_sysparm_initialised;

/*******************************************************************************
 *
 * Function Name: SYSPARM_GetNameFromPA
 *
 * Description:
 *
 *
 * Notes:
 *
 ******************************************************************************/
static int SYSPARM_GetNameFromPA(char *nameaddr, char *str)
{
	char *name_ptr;
	int name_len;

	/*printk(KERN_INFO"[sysparm]: SYSPARM_GetNameFromPA:
	 * nameaddr:0x%08lX\n",(UInt32)nameaddr);
	 */
	name_ptr = (char *)ioremap_nocache(
			(UInt32)nameaddr, MAX_SYSPARM_NAME_SIZE);
	if (!name_ptr) {
		pr_err
		("[sysparm]: SYSPARM_GetNameFromPA: nameaddr ioremap failed\n");
		return 0;
	}

	for (name_len = 0; name_len < MAX_SYSPARM_NAME_SIZE; name_len++) {
		if (name_ptr[name_len] == '\0')
			break;
	}

	if (name_len == 0 || name_len == MAX_SYSPARM_NAME_SIZE) {
		pr_err
		("[sysparm]: SYSPARM_GetNameFromPA: name_len is invalid:%d\n",
			name_len);
		iounmap(name_ptr);
		return 0;
	}

	strcpy(str, name_ptr);

	iounmap(name_ptr);
	return 1;
}

/*******************************************************************************
 *
 * Function Name: SYSPARM_GePAtByIndex
 * Params:  name: sysparm name (case sensitive, must match SysCalData_t fields)
 *          size: specifies the size of the sysparm in number of bytes
 *          flag: specifies required flag value
 * Return:  ptr  = pointer to the sysparm physical adress if it is found
 *          NULL = sysparm not found, or size/flag mismatch
 *
 ******************************************************************************/
static UInt32 SYSPARM_GePAtByIndex(char *name, unsigned int size,
				   unsigned int flag)
{
	int i;
	char sysparm_name[MAX_SYSPARM_NAME_SIZE];

	for (i = 0; i < *total_index_ptr; i++) {
		memset(sysparm_name, 0, MAX_SYSPARM_NAME_SIZE);

		if (!SYSPARM_GetNameFromPA(sysparm_index[i].name,
					sysparm_name)) {
			pr_err("[sysparm]: SYSPARM_GePAtByIndex: i:%d\n", i);
			return 0;
		}

		if (!strcmp(sysparm_name, name)) {
			if (size != sysparm_index[i].size) {
				pr_err("[sysparm]: SYSPARM_GePAtByIndex: sysparm:"
						" \"%s\" size:%u mismatch input size:%u\n",
					name, sysparm_index[i].size, size);
				return 0;
			}

			if (flag != sysparm_index[i].flag) {
				pr_err("[sysparm]: SYSPARM_GePAtByIndex:"
						" sysparm: \"%s\" flag:%u mismatch"
						" input flag:%u\n",
					name, sysparm_index[i].flag, flag);
				return 0;
			}

			return (UInt32)sysparm_index[i].ptr;
		}
	}

	return 0;
}

/*******************************************************************************
 *
 * Function Name: SYSPARM_SetByIndex
 * Params:  name: sysparm name (case sensitive, must match SysCalData_t fields)
 *      ptr:  pointer to the sysparm to be set
 *      size: specifies the size of the sysparm in number of bytes
 *      flag: specifies required flag value
 * Return:  1 = sysparm is set successfully
 *      0 = sysparm not found, or size/flag mismatch
 *
 ******************************************************************************/
#if 0
static int SYSPARM_SetByIndex(void *ptr, char *name, unsigned int size,
			      unsigned int flag)
{
	UInt32 i, mask;

	for (i = 0; i < *total_index_ptr; i++) {
		if (!strcmp(sysparm_index[i].name, name)) {
			if (size != sysparm_index[i].size)
				return 0;
			if (flag != sysparm_index[i].flag)
				return 0;
			local_irq_save(mask);
			memcpy(sysparm_index[i].ptr, ptr, size);
			local_irq_restore(mask);
			return 1;
		}
	}
	return 0;
}
#endif

/*******************************************************************************
 * Function Name: SYSPARM_GetParamU32ByName
 *
 * Description:  Generic function to read value of 32-bit parameters from
 *               sysparm. Returns 0 on success, negative on error. The
 *               arguments are:
 *
 *               name - name of the parameter.
 *               parm - 32-bit value of the parameter is returned via this
 *                      address (memory must be allocated by caller).
 *
 * Notes:
 ******************************************************************************/
int SYSPARM_GetParmU32ByName(char *name, unsigned int *parm)
{
	UInt32 parm_addr;
	UInt32 *parm_ptr;

	if (name == NULL || parm == NULL) {
		pr_err("%s: invalid arguments\n", __func__);
		return -EINVAL;
	}

	if (!fuse_sysparm_initialised) {
	    if (sysparm_init()) {
		pr_err("%s: Fuse sysparm is not yet initialised\n", __func__);
		return -EIO;
	    }
	}

	parm_addr = SYSPARM_GePAtByIndex(name, sizeof(*parm_ptr), 1);
	if (!parm_addr)
		return -ENOENT;

	parm_ptr = (UInt32 *)ioremap_nocache(parm_addr, sizeof(UInt32));
	if (!parm_ptr) {
		pr_err("%s: ioremap failed\n", __func__);
		return -EIO;
	}

	*parm = ioread32(parm_ptr);

	iounmap(parm_ptr);

	return 0;
}

#if !SKELETON_DRIVER
/*******************************************************************************
 *
 * Function Name: APSYSPARM_GetAudioParmAccessPtr
 *
 * Description:   Get access pointer to root of audio sysparm structure
 *
 * Notes:     This is only applicable to audio tuning parameters.
 *
 ******************************************************************************/
SysAudioParm_t __iomem *APSYSPARM_GetAudioParmAccessPtr(void)
{
	UInt32 audio_parm_addr;

	if (!fuse_sysparm_initialised) {
		if (sysparm_init()) {
			pr_err("[sysparm]: sysparm is not yet initialised\n");
			return 0;
		}
	}

	if (!audio_parm_table) {
		int parm_table_size =
			AUDIO_MODE_NUMBER_VOICE*sizeof(SysAudioParm_t);
		audio_parm_addr = SYSPARM_GePAtByIndex("audio_parm",
							parm_table_size,
							1);
		if (!audio_parm_addr) {
			pr_err("[sysparm]: Get audio_parm PA failed\n");
			return 0;
		}

		audio_parm_table
			= ioremap_nocache(audio_parm_addr, parm_table_size);

		if (!audio_parm_table)
			pr_err(
			"[sysparm]: audio_parm_addr ioremap failed\n");
	}


	return audio_parm_table;
}

/*******************************************************************************
 *
 * Function Name: APSYSPARM_GetMultimediaAudioParmAccessPtr
 *
 * Description:   Get access pointer to root of MM audio sysparm structure
 *
 * Notes:     This is only applicable to audio tuning parameters.
 *
 ******************************************************************************/
#if defined(USE_NEW_AUDIO_MM_PARAM)
SysIndMultimediaAudioParm_t __iomem
	*APSYSPARM_GetIndMultimediaAudioParmAccessPtr(void)
#else
SysIndMultimediaAudioParm_t __iomem
	*APSYSPARM_GetMultimediaAudioParmAccessPtr(void)
#endif
{
	UInt32 ind_mmaudio_parm_addr;

	if (!fuse_sysparm_initialised) {
		if (sysparm_init()) {
			pr_err("[sysparm]: Fuse sysparm is not yet initialised\n");
			return 0;
		}
	}

	if (!ind_mmaudio_parm_table) {
		int parm_table_size =
			AUDIO_MODE_NUMBER*sizeof(SysIndMultimediaAudioParm_t);

		/* NOTE: parm name (mmaudio_parm) will need to change
		 * to "ind_mmaudio_parm" once USE_NEW_AUDIO_MM_PARAM
		 * is enabled
		 */
		ind_mmaudio_parm_addr
			= SYSPARM_GePAtByIndex(MM_AUDIO_PARM_NAME,
						parm_table_size,
						1);
		if (!ind_mmaudio_parm_addr) {
			pr_err("[sysparm]: Get mmaudio_parm PA failed\n");
			return 0;
		}

		ind_mmaudio_parm_table
			= ioremap_nocache(ind_mmaudio_parm_addr,
					parm_table_size);

		if (!ind_mmaudio_parm_table)
			pr_err("[sysparm]: mmaudio_parm_addr ioremap failed\n");
	}

	return ind_mmaudio_parm_table;

}

#if defined(USE_NEW_AUDIO_MM_PARAM)
/******************************************************************************

 Function Name:	APSYSPARM_GetMultimediaAudioParmAccessPtr

 Description:		Return the multimedia audio parm ptr

 Notes:

******************************************************************************/
SysMultimediaAudioParm_t __iomem
	*APSYSPARM_GetMultimediaAudioParmAccessPtr(void)
{
	UInt32 mmaudio_parm_addr;

	if (!fuse_sysparm_initialised) {
		if (sysparm_init()) {
			pr_err("[sysparm]: Fuse sysparm is not yet initialised\n");
			return 0;
		}
	}

	if (!mmaudio_parm_table) {
		int parm_table_size = sizeof(SysMultimediaAudioParm_t)*
					MMAUDIO_PARM_TABLE_ENTRIES;
		mmaudio_parm_addr = SYSPARM_GePAtByIndex("mmaudio_parm",
							parm_table_size,
							1);
		if (!mmaudio_parm_addr) {
			pr_err("[sysparm]: Get mmaudio_parm PA failed\n");
			return 0;
		}

		mmaudio_parm_table
			= ioremap_nocache(mmaudio_parm_addr, parm_table_size);

		if (!mmaudio_parm_table)
			pr_err(
			"[sysparm]: mmaudio_parm_addr ioremap failed\n");
	}


	return mmaudio_parm_table;
}
#endif

/******************************************************************************
 *
 * Function Name: APSYSPARM_RefreshAudioParmAccessPtr
 *
 * Description:   Reload audio sysparm structure from CP
 *
 * Notes:     This is only applicable to audio tuning parameters.
 *
 ******************************************************************************/
int APSYSPARM_RefreshAudioParm(unsigned int addr)
{
	/* Note: no longer needed now that we're not caching
	 * audio sysparms locally; can be removed once this
	 * call is removed from audio driver
	 */
	pr_info("APSYSPARM_RefreshAudioParm: 0x%x\n", addr);
	return 0;
}
#endif /* !SKELETON_DRIVER */

/*******************************************************************************
 *
 * Function Name: sysparm_init
 *
 * Description:   Start system parameter driver initialise
 *
 *
 * Notes:
 *
 ******************************************************************************/
static int sysparm_init(void)
{
	int sysparm_ready_count = 0;

	if (is_ap_only_boot()) {
		pr_info("[sysparm]: AP only boot, return error\n");
		return 1;
	}

	pr_info
	    ("[sysparm]: sysparm driver start (PARM_DEP_RAM_ADDR=%x PARM_DEP_SIZE=%x)\n",
	     (int)PARM_DEP_RAM_ADDR, (int)PARM_DEP_SIZE);

	if (fuse_sysparm_initialised) {
		pr_info("[sysparm]: init already done\n");
		return 0;
	}

	sys_data_dep = ioremap_nocache(PARM_DEP_RAM_ADDR, PARM_DEP_SIZE);
	if (!sys_data_dep) {
		pr_err("[sysparm]: PARM_DEP_RAM_ADDR ioremap failed\n");
		BUG();
	}

	sysparm_ready_ind_ptr = (UInt32 *)(sys_data_dep + 0x4000);

	while (SYSPARM_INDEX_READY_INDICATOR != *sysparm_ready_ind_ptr) {
		if (sysparm_ready_count >= 5) {
			pr_err("[sysparm]: sys_data_dep+0x0: %x %x %x %x\n",
			       (int)(*(unsigned long *)(sys_data_dep + 0x0)),
			       (int)(*(unsigned long *)(sys_data_dep + 0x4)),
			       (int)(*(unsigned long *)(sys_data_dep + 0x8)),
			       (int)(*(unsigned long *)(sys_data_dep + 0xc)));

			pr_err("[sysparm]: sysparm_ind_ready_ind: %x\n",
			       (int)(*sysparm_ready_ind_ptr));

			pr_err
			   ("[sysparm]: timeout waiting for ready indicator\n");

			BUG();
		}

		msleep(25);
		sysparm_ready_count++;
	}

	total_index_ptr =
	    (UInt32 *)((UInt32)sysparm_ready_ind_ptr +
		       sizeof(sysparm_ready_ind_ptr));
	sysparm_index =
	    (SysparmIndex_t *) ((UInt32)total_index_ptr +
				sizeof(total_index_ptr));

	pr_info("[sysparm]: sysparm_init ok\n");
	fuse_sysparm_initialised = 1;

	return 0;
}

/*******************************************************************************
 *
 * Function Name: sysparm_module_init
 *
 * Description:  Function to satisfy the condition of module_init
 *
 *
 * Notes:
 *
 ******************************************************************************/
static int __init sysparm_module_init(void)
{
#if SKELETON_DRIVER
	sysparm_init();
#endif /* SKELETON_DRIVER */
	return 0;
}

/*******************************************************************************
 *
 * Function Name: sysparm_module_exit
 *
 * Description:   Unload system parameter driver
 *
 *
 * Notes:
 *
 ******************************************************************************/
static void __exit sysparm_module_exit(void)
{
	iounmap(sys_data_dep);

	return;
}

module_init(sysparm_module_init);
module_exit(sysparm_module_exit);

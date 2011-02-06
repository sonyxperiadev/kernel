/*******************************************************************************************
Copyright 2010 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement
governing use of this software, this software is licensed to you under the
terms of the GNU General Public License version 2, available at
http://www.gnu.org/copyleft/gpl.html (the "GPL").

Notwithstanding the above, under no circumstances may you combine this software
in any way with any other Broadcom software provided under a license other than
the GPL, without Broadcom's express prior written consent.
*******************************************************************************************/

//***************************************************************************
/**
*
*   @file   xscript_kernel.c
*
*   @brief  This driver is used to make kernel API calls for xScript
*			adapters.
*
*
****************************************************************************/
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/fcntl.h>
#include <linux/ioctl.h>
#include <linux/types.h>
#include <linux/unistd.h>
#include <linux/errno.h>
#include <linux/wait.h>
#include <linux/poll.h>

#include <linux/broadcom/xscript/xsapi_clk_def.h>
#include "xscript_kernel.h"

#include <linux/broadcom/bcm_major.h>
#include <linux/broadcom/csl_types.h>
#include <linux/clk.h>
#include <mach/clkmgr.h>

// Dynamically determine the major number
#ifndef BCM_XSCRIPT_MAJOR
#define BCM_XSCRIPT_MAJOR 0
#endif

/// BCM_XSCRIPT_MAJOR major number, which may be dynamically allocated.
static int sXScriptMajorNum = BCM_XSCRIPT_MAJOR;

/**
 *  module data
 */
typedef struct
{
    struct class*   mDriverClass ;          ///< driver class

}   XSCRIPT_KERNEL_Module_t ;

/**
 *  module status
 */
static XSCRIPT_KERNEL_Module_t sModule = {0};

/**
 *  private data for each session, right now, only support one client, so not needed for now.
 *  can be modified if need to support multiple clients later on.
 */
typedef struct
{
    struct file *mUserfile;                         ///< user file handle (for open, close and ioctl calls)             ///< log enable/disable status bits
}   XSCRIPT_KERNEL_PrivData_t ;

/// Maps xScript clock IDs to kernel clock strings.
typedef struct
{
    XS_BcmClk_t fClkID;     ///< Clock enumeration ID.
    const char* fClkName;   ///< Clock name string.

} XS_ClkMap;

/// Mapping of xScript clock IDs to kernel clock strings.
static const XS_ClkMap sClkMap[] =
{
    { XS_BCM_CLK_MAIN_PLL_STR_ID, BCM_CLK_MAIN_PLL_STR_ID },
    { XS_BCM_CLK_ARM11_STR_ID, BCM_CLK_ARM11_STR_ID },
    { XS_BCM_CLK_CAMERA_STR_ID, BCM_CLK_CAMERA_STR_ID },
    { XS_BCM_CLK_I2S_INT_STR_ID, BCM_CLK_I2S_INT_STR_ID },
    { XS_BCM_CLK_I2S_EXT_STR_ID, BCM_CLK_I2S_EXT_STR_ID },
    { XS_BCM_CLK_DAM_STR_ID, BCM_CLK_DAM_STR_ID },
    { XS_BCM_CLK_PDP_STR_ID, BCM_CLK_PDP_STR_ID },
    { XS_BCM_CLK_SDIO1_STR_ID, BCM_CLK_SDIO1_STR_ID },
    { XS_BCM_CLK_SDIO2_STR_ID, BCM_CLK_SDIO2_STR_ID },
#ifdef CONFIG_ARCH_BCM2153
    { XS_BCM_CLK_SM_STR_ID, BCM_CLK_SM_STR_ID },
#endif  //#ifdef CONFIG_ARCH_BCM2153
    { XS_BCM_CLK_SPI0_STR_ID, BCM_CLK_SPI0_STR_ID },
    { XS_BCM_CLK_UARTA_STR_ID, BCM_CLK_UARTA_STR_ID },
    { XS_BCM_CLK_UARTB_STR_ID, BCM_CLK_UARTB_STR_ID },
    { XS_BCM_CLK_GP_STR_ID, BCM_CLK_GP_STR_ID },
#if defined(CONFIG_ARCH_BCM2153) || defined(CONFIG_ARCH_BCM21553)
    { XS_BCM_CLK_MSPRO_STR_ID, BCM_CLK_MSPRO_STR_ID },
#endif  //#if defined(CONFIG_ARCH_BCM2153) || defined(CONFIG_ARCH_BCM21553)
    { XS_BCM_CLK_I2C1_STR_ID, BCM_CLK_I2C1_STR_ID },
    { XS_BCM_CLK_I2C2_STR_ID, BCM_CLK_I2C2_STR_ID },
#if defined(CONFIG_ARCH_BCM2153) || defined(CONFIG_ARCH_BCM2157)
    { XS_BCM_CLK_GE_STR_ID, BCM_CLK_GE_STR_ID },
#endif  //#if defined(CONFIG_ARCH_BCM2153) || defined(CONFIG_ARCH_BCM2157)
    { XS_BCM_CLK_USB_STR_ID, BCM_CLK_USB_STR_ID },

#if defined(CONFIG_ARCH_BCM2157) || defined(CONFIG_ARCH_BCM21553)
    { XS_BCM_CLK_PWM_STR_ID, BCM_CLK_PWM_STR_ID },
    { XS_BCM_CLK_VCODEC_STR_ID, BCM_CLK_VCODEC_STR_ID },
#endif  //#if defined(CONFIG_ARCH_BCM2157) || defined(CONFIG_ARCH_BCM21553)
    { XS_BCM_CLK_DMAC_STR_ID, BCM_CLK_DMAC_STR_ID },
#if defined(CONFIG_ARCH_BCM2157) || defined(CONFIG_ARCH_BCM21553)
    { XS_BCM_CLK_RNG_STR_ID, BCM_CLK_RNG_STR_ID },
    { XS_BCM_CLK_LCD_STR_ID, BCM_CLK_LCD_STR_ID },
    { XS_BCM_CLK_MPHI_STR_ID, BCM_CLK_MPHI_STR_ID },
#endif  //#if defined(CONFIG_ARCH_BCM2157) || defined(CONFIG_ARCH_BCM21553)
#if defined(CONFIG_ARCH_BCM2157)
    { XS_BCM_CLK_CMI_STR_ID, BCM_CLK_CMI_STR_ID },
#endif  //#if defined(CONFIG_ARCH_BCM2157)

#ifdef CONFIG_ARCH_BCM21553
    { XS_BCM_CLK_CAMERARX_STR_ID, BCM_CLK_CAMERARX_STR_ID },
    { XS_BCM_CLK_CAMERASYS_STR_ID, BCM_CLK_CAMERASYS_STR_ID },
    { XS_BCM_CLK_DPE_STR_ID, BCM_CLK_DPE_STR_ID },
    { XS_BCM_CLK_SDIO3_STR_ID, BCM_CLK_SDIO3_STR_ID },
    { XS_BCM_CLK_SDRAM_STR_ID, BCM_CLK_SDRAM_STR_ID },
    { XS_BCM_CLK_NVSRAM_STR_ID, BCM_CLK_NVSRAM_STR_ID },
    { XS_BCM_CLK_SPI1_STR_ID, BCM_CLK_SPI1_STR_ID },
    { XS_BCM_CLK_UARTC_STR_ID, BCM_CLK_UARTC_STR_ID },
    { XS_BCM_CLK_MIPIDSI_STR_ID, BCM_CLK_MIPIDSI_STR_ID },
#endif  //#ifdef CONFIG_ARCH_BCM21553

    { XS_BCM_CLK_TOTAL, NULL }
};

//local function protos

//forward declarations used in 'struct file_operations'
static int XSCRIPT_KERNEL_Open(struct inode *inode, struct file *filp) ;
static int XSCRIPT_KERNEL_Ioctl(struct inode *inode, struct file *filp, unsigned int cmd, UInt32 arg) ;
static int XSCRIPT_KERNEL_Release(struct inode *inode, struct file *filp) ;
static void Cleanup(void);
static const char* GetClkNameFromID(XS_BcmClk_t inClkID);

/**
 *  file ops
 */
static struct file_operations sFileOperations =
{
    .owner      = THIS_MODULE,
    .open       = XSCRIPT_KERNEL_Open,
    .read       = NULL,
    .write      = NULL,
    .ioctl      = XSCRIPT_KERNEL_Ioctl,
    .poll       = NULL,
    .mmap       = NULL,
    .release    = XSCRIPT_KERNEL_Release,
};


//#define XSCRIPT_KERNEL_TRACE_ON
#ifdef XSCRIPT_KERNEL_TRACE_ON
#define XSCRIPT_KERNEL_TRACE(str) printk str
#else
#define XSCRIPT_KERNEL_TRACE(str) {}
#endif


//======================================File operations==================================================
//***************************************************************************
/**
 *  Called by Linux I/O system to handle open() call.
 *  @param  (in)    not used
 *  @param  (io)    file pointer
 *  @return int     0 if success, -1 if error
 *  @note
 *      API is defined by struct file_operations 'open' member.
 */
static int XSCRIPT_KERNEL_Open(struct inode *inode, struct file *filp)
{
    XSCRIPT_KERNEL_PrivData_t *priv;

    XSCRIPT_KERNEL_TRACE(( "XSCRIPT_KERNEL_Open\n") ) ;

    priv = kmalloc(sizeof(XSCRIPT_KERNEL_PrivData_t), GFP_KERNEL);

    if (!priv)
    {
        XSCRIPT_KERNEL_TRACE(( "XSCRIPT_KERNEL_Open: ENOMEM\n") ) ;
        return -ENOMEM;
    }

    priv->mUserfile = filp;
    filp->private_data = priv;

    return 0;
}

//***************************************************************************
/**
 *  Called by Linux I/O system to handle ioctl() call.
 *  @param  inode (in)    not used
 *  @param  filp (in)    not used
 *  @param  cmd (in)    ioctl command (see note)
 *  @param  arg (in)    ioctl argument (see note)
 *  @return int     0 if success, -1 if error
 *  @note
 *      API is defined by struct file_operations 'ioctl' member.
 *
 *      cmd is
 *
 *          XSCRIPT_KERNEL_CMD_CLK_GET		  - get a clk struct
 *          XSCRIPT_KERNEL_CMD_CLK_GET_RATE    - get the clock rate
 *          XSCRIPT_KERNEL_CMD_CLK_SET_RATE    - set the clock rate
 *
 */
static int XSCRIPT_KERNEL_Ioctl( struct inode *inode, struct file *filp, unsigned int cmd, UInt32 arg )
{
    int retVal = 0;

    XSCRIPT_KERNEL_TRACE(( "XSCRIPT_KERNEL_Ioctl\n" )) ;

    switch( cmd )
    {
        case XSCRIPT_KERNEL_CMD_CLK_GET:
            {
                XS_KERNEL_ClkGet_t* clkArg = (XS_KERNEL_ClkGet_t*)arg;
				struct clk * pClk = NULL;

                if ( clkArg == NULL )
                {
                    retVal = -1;
                }
				else
				{
                    // Convert the clock ID to a string.
                    const char* clkName = GetClkNameFromID(clkArg->fId);
                    if (clkName != NULL)
                    {
                        pClk = clk_get(clkArg->fDev, clkName);
                        XSCRIPT_KERNEL_TRACE(( "XSCRIPT_KERNEL_Ioctl() - clk_get: 0x%x\n", pClk ));
                        if (copy_to_user(&(clkArg->fOutClk), &pClk, sizeof(struct clk*)) != 0)
                        {
                            XSCRIPT_KERNEL_TRACE(( "XSCRIPT_KERNEL_Ioctl() - copy_to_user() had error\n" ));
                            retVal = -1;
                        }
                    }
                    else
                    {
                        retVal = -1;
                    }
				}

                break;
            }

        case XSCRIPT_KERNEL_CMD_CLK_GET_RATE:
            {
                XS_KERNEL_ClkGetRate_t* clkArg = (XS_KERNEL_ClkGetRate_t*)arg;
				unsigned long clkRate = 0;

                if ( clkArg == NULL )
                {
                    retVal = -1;
                }
				else
				{
					clkRate = clk_get_rate(clkArg->fClk);
					XSCRIPT_KERNEL_TRACE(( "XSCRIPT_KERNEL_Ioctl() - clk_get_rate: %ld Hz\n", clkRate ));
					if (copy_to_user(&(clkArg->fOutRate), &clkRate, sizeof(unsigned long)) != 0)
					{
						XSCRIPT_KERNEL_TRACE(( "XSCRIPT_KERNEL_Ioctl() - copy_to_user() had error\n" ));
						retVal = -1;
					}
				}

                break;
            }

        case XSCRIPT_KERNEL_CMD_CLK_SET_RATE:
            {
                XS_KERNEL_ClkSetRate_t* clkArg = (XS_KERNEL_ClkSetRate_t*)arg;

                if ( clkArg == NULL )
                {
                    retVal = -1;
                }
				else
				{
					int errorCode = clk_set_rate(clkArg->fClk, clkArg->fRate);
					XSCRIPT_KERNEL_TRACE(( "XSCRIPT_KERNEL_Ioctl() - clk_get_rate: %d\n", errorCode ));
					if (copy_to_user(&(clkArg->fOutErrno), &errorCode, sizeof(int)) != 0)
					{
						XSCRIPT_KERNEL_TRACE(( "XSCRIPT_KERNEL_Ioctl() - copy_to_user() had error\n" ));
						retVal = -1;
					}
				}

                break;
            }

        default:
            retVal = -1 ;
            break ;
    }

    return retVal;
}

//***************************************************************************
/**
 *  Called by Linux I/O system to handle release() call.
 *  @param  (in)    not used
 *  @param  (in)    not used
 *  @return int     0 if success, -1 if error
 *  @note
 *      API is defined by struct file_operations 'release' member.
 */
static int XSCRIPT_KERNEL_Release(struct inode *inode, struct file *filp)
{
    XSCRIPT_KERNEL_PrivData_t *priv = filp->private_data;

    XSCRIPT_KERNEL_TRACE(( "XSCRIPT_KERNEL_Release\n" ) );
    if( priv )
        kfree( priv ) ;

    return 0;
}

//====================== Local functions ==========================================================

//***************************************************************************
/**
 *  Memory cleanning during module destroy or error exit cases.
 *
 */
static void Cleanup(void)
{
    //memory deallocate
    XSCRIPT_KERNEL_TRACE(( "Cleanup\n" ) );

    return;
}

//***************************************************************************
/**
 *  Converts the clock ID to the equivalent string name.
 *
 *  @param  inClkID (in) The clock ID.
 *
 *  @return The clock name.
 *
 */
static const char* GetClkNameFromID(XS_BcmClk_t inClkID)
{
    const char* clkName = NULL;
    int i;

    for (i = 0; i < XS_BCM_CLK_TOTAL; i++)
    {
        if (inClkID == sClkMap[i].fClkID)
        {
            clkName = sClkMap[i].fClkName;
            break;
        }
    }

    return clkName;
}

//============ Module ===================================================================================

//***************************************************************************
/**
 *  Called by Linux I/O system to initialize module.
 *  @return int     0 if success, -1 if error
 *  @note
 *      API is defined by module_init macro
 */
static int __init XSCRIPT_KERNEL_ModuleInit(void)
{
    struct device *drvdata ;
    int result = 0;
	XSCRIPT_KERNEL_TRACE(("enter XSCRIPT_KERNEL_ModuleInit()\n"));

    //drive driver process:
    result = register_chrdev(BCM_XSCRIPT_MAJOR, XSCRIPT_KERNEL_MODULE_NAME, &sFileOperations);
	if ( result < 0 )
    {
        XSCRIPT_KERNEL_TRACE(("XSCRIPT_KERNEL_ModuleInit: register_chrdev failed for major number %d\n", BCM_XSCRIPT_MAJOR ) );
        Cleanup();
        return -1 ;
    }
	else if ( result != 0 )
	{
		// For dynamic major
		sXScriptMajorNum = result;
	}

    sModule.mDriverClass = class_create(THIS_MODULE, XSCRIPT_KERNEL_MODULE_NAME);
    if (IS_ERR(sModule.mDriverClass))
    {
        XSCRIPT_KERNEL_TRACE(( "XSCRIPT_KERNEL_ModuleInit: driver class_create failed\n" ) );
        Cleanup();
        unregister_chrdev( sXScriptMajorNum, XSCRIPT_KERNEL_MODULE_NAME ) ;
        return -1 ;
    }

    drvdata = device_create( sModule.mDriverClass, NULL, MKDEV(sXScriptMajorNum, 0), NULL, XSCRIPT_KERNEL_MODULE_NAME ) ;
    if( IS_ERR( drvdata ) )
    {
        XSCRIPT_KERNEL_TRACE(( "XSCRIPT_KERNEL_ModuleInit: device_create_drvdata failed\n" ) );
        Cleanup();
        unregister_chrdev( sXScriptMajorNum, XSCRIPT_KERNEL_MODULE_NAME ) ;
        return -1 ;
    }

    XSCRIPT_KERNEL_TRACE(("exit XSCRIPT_KERNEL_ModuleInit()\n"));
    return 0;
}

//***************************************************************************
/**
 *  Called by Linux I/O system to exit module.
 *  @return int     0 if success, -1 if error
 *  @note
 *      API is defined by module_exit macro
 **/
static void __exit XSCRIPT_KERNEL_ModuleExit(void)
{
    XSCRIPT_KERNEL_TRACE(("XSCRIPT_KERNEL_ModuleExit()\n"));
    Cleanup();
    unregister_chrdev( sXScriptMajorNum, XSCRIPT_KERNEL_MODULE_NAME ) ;
}



/**
 *  export module init and export functions
 **/
module_init(XSCRIPT_KERNEL_ModuleInit);
module_exit(XSCRIPT_KERNEL_ModuleExit);





/*****************************************************************************
* Copyright 2010 - 2011 Broadcom Corporation.  All rights reserved.
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

#include <linux/version.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/kernel.h>
#if 0
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/delay.h>
#include <linux/sysctl.h>
#include <mach/csp/mm_io.h>
#include <linux/kthread.h>
#include <asm-generic/div64.h>
#include <mach/csp/rdb/brcm_rdb_csr.h>
#endif

#include <chal/chal_mqos.h>
#include <linux/string.h>

static CHAL_MQOS_t gMQoSHandle;      /* SYS LPDDR2/DDR3 memory controller, dpending on BI or Hana*/
static CHAL_MQOS_t gMQoSHandleMM;    /* MM LPDDR2 memory controller*/


static void set_mqos( char *mem_type );
static void mqosInitCam(CHAL_MQOS_MEMTYPE_e mem_type);
// static void config_mqos_latency_default( char *mem_type, char *entry_type, int ctr_thold );
static void config_mqos_bandwidth_default( char *mem_type, char *entry_type, int avg_bw_in_MBPS , int thold );

/*****************************************************************************
*
*****************************************************************************/
int memc_qos_init( void )
{
   set_mqos( "sys" );

   config_mqos_bandwidth_default( "sys", "HVS", 1000, 144 );
//   config_mqos_latency_default( "sys", "HVS", 0 );
   return 0;
}

/****************************************************************************
*  @brief   void mqosInitCam()
*         :
*         NB: Initializes CAM0=default_cachtall, CAM1=unused, match none
*  @return
****************************************************************************/
static void mqosInitCam(CHAL_MQOS_MEMTYPE_e mem_type)
{
   unsigned int tag_str, i;

   for( i=0; i < CHAL_MQOS_CAM_ENTRY_ID_MAX; i++ )
   {
      if( i==0 )
      {
         tag_str = CHAL_MQOS_CAM_TAG_CFG_DEFAULT_CATCHALL; //CHAL_MQOS_CAM_TAG_ENABLE_DEFAULT_CATCHALL;
      }
      else
      {
         tag_str = CHAL_MQOS_CAM_TAG_CFG_DEFAULT_UNUSED;   //CHAL_MQOS_CAM_TAG_ENABLE_DEFAULT_UNUSED;
      }

      if(mem_type == CHAL_MQOS_MEMTYPE_SYS_EMI_OPEN)
      {
         chal_mqos_enable(&gMQoSHandle, tag_str, i);
      }
      else if(mem_type == CHAL_MQOS_MEMTYPE_VC4_EMI_OPEN)
      {
         chal_mqos_enable(&gMQoSHandleMM, tag_str, i);
      }
      else
      {
         printk("MQoS_Menu: Error in initinializing CAM_ENABLE reg. Incorrect memory type...\n");
      }
   }

   return;
}

/*****************************************************************************
*
*****************************************************************************/
static void set_mqos( char *mem_type )
{
   if (strcmp (mem_type, "sys") == 0)
   {
      gMQoSHandle.memc_type = CHAL_MQOS_MEMTYPE_SYS_EMI_OPEN;
      chal_mqos_get_handle(&gMQoSHandle);

      mqosInitCam(gMQoSHandle.memc_type);
   }
   else if(strcmp (mem_type, "mm") == 0)
   {
      gMQoSHandleMM.memc_type = CHAL_MQOS_MEMTYPE_VC4_EMI_OPEN;
      chal_mqos_get_handle(&gMQoSHandleMM);

      mqosInitCam(gMQoSHandleMM.memc_type);
   }

   return;
}

#if 0

/*****************************************************************************
*
*****************************************************************************/
static void config_mqos_latency_default( char *mem_type, char *entry_type, int ctr_thold)
{
   unsigned int cfg_str;
   unsigned int cfg_id;
   unsigned int tag_str;

   /* now set for the specified bus master */
   if(strcmp (entry_type, "A9") == 0)
   {
      cfg_id   = CHAL_MQOS_CAM_ENTRY_ASSGN_5_A9; //CHAL_MQOS_CAM_ENTRY_ID_A9;
      //cfg_str = CHAL_MQOS_CAM_ENTRY_CFG_DEFAULT_LATENCY;  /* A9 is latency type */
      cfg_str =  CHAL_MQOS_CAM_ENTRY_CFG_STR(CHAL_MQOS_CAM_READ_ENABLE_DEFAULT_LATENCY,
                                    CHAL_MQOS_CAM_WRITE_ENABLE_DEFAULT_LATENCY,
                                    CHAL_MQOS_CAM_PRIORITY_DEFAULT_LATENCY,
                                    CHAL_MQOS_CAM_LATENCY_DEFAULT_LATENCY,
                                    ctr_thold, /* in units of 6.5 micro-sec. ie 156 MHz clock */
                                    CHAL_MQOS_CAM_THOLD_DEFAULT_LATENCY);
      if(strcmp (mem_type, "sys") == 0)
      {
         tag_str = CHAL_MQOS_CAM_TAG_CFG_DEFAULT_A9;
      }
      else if (strcmp (mem_type, "mm") == 0)
      {
         tag_str = CHAL_MQOS_CAM_TAG_CFG_DEFAULT_MM_PL310;
      }
      else
      {
         printk("Error: invalid mem type specified.\n");
         return;
      }
   }
   else if(strcmp (entry_type, "HVS") == 0)
   {
      //HVS is more of bandwidth critical.  But it becomes latency critical when it goes through the vertical blank.
      cfg_id   = CHAL_MQOS_CAM_ENTRY_ASSGN_10_VCHVS; //CHAL_MQOS_CAM_ENTRY_ID_HVS;
      //cfg_str = CHAL_MQOS_CAM_ENTRY_CFG_DEFAULT_BANDWIDTH;  /* A9 is latency type */
      cfg_str =  CHAL_MQOS_CAM_ENTRY_CFG_STR(CHAL_MQOS_CAM_READ_ENABLE_DEFAULT_LATENCY,
                                    CHAL_MQOS_CAM_WRITE_ENABLE_DEFAULT_LATENCY,
                                    CHAL_MQOS_CAM_PRIORITY_DEFAULT_LATENCY,
                                    CHAL_MQOS_CAM_LATENCY_DEFAULT_LATENCY,
                                    ctr_thold, /* in units of 6.5 micro-sec. ie 156 MHz clock */
                                    CHAL_MQOS_CAM_THOLD_DEFAULT_LATENCY);
      tag_str = CHAL_MQOS_CAM_TAG_CFG_DEFAULT_HVS;
   }
   else if(strcmp (entry_type, "VCL2") == 0)
   {
      //VC4 L2$ Main is latency critical.  But it becomes latency critical when it goes through the vertical blank.
      cfg_id   = CHAL_MQOS_CAM_ENTRY_ASSGN_1_VCL2MAIN;
      cfg_str =  CHAL_MQOS_CAM_ENTRY_CFG_STR(CHAL_MQOS_CAM_READ_ENABLE_DEFAULT_LATENCY,
                                    CHAL_MQOS_CAM_WRITE_ENABLE_DEFAULT_LATENCY,
                                    CHAL_MQOS_CAM_PRIORITY_DEFAULT_LATENCY,
                                    CHAL_MQOS_CAM_LATENCY_DEFAULT_LATENCY,
                                    ctr_thold, /* in units of 6.5 micro-sec. ie 156 MHz clock */
                                    CHAL_MQOS_CAM_THOLD_DEFAULT_LATENCY);
      tag_str = CHAL_MQOS_CAM_TAG_CFG_DEFAULT_VC_L2;
   }
   else if(strcmp (entry_type, "R4") == 0)
   {
      cfg_id   = CHAL_MQOS_CAM_ENTRY_ASSGN_6_R4;
      //cfg_str = CHAL_MQOS_CAM_ENTRY_CFG_DEFAULT_BANDWIDTH;  /* A9 is latency type */
      cfg_str =  CHAL_MQOS_CAM_ENTRY_CFG_STR(CHAL_MQOS_CAM_READ_ENABLE_DEFAULT_LATENCY,
                                    CHAL_MQOS_CAM_WRITE_ENABLE_DEFAULT_LATENCY,
                                    CHAL_MQOS_CAM_PRIORITY_DEFAULT_LATENCY,
                                    CHAL_MQOS_CAM_LATENCY_DEFAULT_LATENCY,
                                    ctr_thold, /* in units of 6.5 micro-sec. ie 156 MHz clock */
                                    CHAL_MQOS_CAM_THOLD_DEFAULT_LATENCY);
      tag_str = CHAL_MQOS_CAM_TAG_CFG_DEFAULT_R4;
   }
   else if(strcmp (entry_type, "Modem") == 0)
   {
      cfg_id   = CHAL_MQOS_CAM_ENTRY_ASSGN_7_MODEM;
      //cfg_str = CHAL_MQOS_CAM_ENTRY_CFG_DEFAULT_BANDWIDTH;  /* A9 is latency type */
      cfg_str =  CHAL_MQOS_CAM_ENTRY_CFG_STR(CHAL_MQOS_CAM_READ_ENABLE_DEFAULT_LATENCY,
                                    CHAL_MQOS_CAM_WRITE_ENABLE_DEFAULT_LATENCY,
                                    CHAL_MQOS_CAM_PRIORITY_DEFAULT_LATENCY,
                                    CHAL_MQOS_CAM_LATENCY_DEFAULT_LATENCY,
                                    ctr_thold, /* in units of 6.5 micro-sec. ie 156 MHz clock */
                                    CHAL_MQOS_CAM_THOLD_DEFAULT_LATENCY);
      tag_str = CHAL_MQOS_CAM_TAG_CFG_DEFAULT_MODEM_ETC;
   }
   else if(strcmp (entry_type, "Core0") == 0)
   {
      cfg_id   = CHAL_MQOS_CAM_ENTRY_ASSGN_14_VCCORE0;
      //cfg_str = CHAL_MQOS_CAM_ENTRY_CFG_DEFAULT_BANDWIDTH;  /* A9 is latency type */
      cfg_str =  CHAL_MQOS_CAM_ENTRY_CFG_STR(CHAL_MQOS_CAM_READ_ENABLE_DEFAULT_LATENCY,
                                    CHAL_MQOS_CAM_WRITE_ENABLE_DEFAULT_LATENCY,
                                    CHAL_MQOS_CAM_PRIORITY_DEFAULT_LATENCY,
                                    CHAL_MQOS_CAM_LATENCY_DEFAULT_LATENCY,
                                    ctr_thold, /* in units of 6.5 micro-sec. ie 156 MHz clock */
                                    CHAL_MQOS_CAM_THOLD_DEFAULT_LATENCY);
      tag_str = CHAL_MQOS_CAM_TAG_CFG_DEFAULT_CORE0_V;
   }
   else if(strcmp (entry_type, "Core1") == 0)
   {
      cfg_id   = CHAL_MQOS_CAM_ENTRY_ASSGN_15_VCCORE1;
      //cfg_str = CHAL_MQOS_CAM_ENTRY_CFG_DEFAULT_BANDWIDTH;  /* A9 is latency type */
      cfg_str =  CHAL_MQOS_CAM_ENTRY_CFG_STR(CHAL_MQOS_CAM_READ_ENABLE_DEFAULT_LATENCY,
                                    CHAL_MQOS_CAM_WRITE_ENABLE_DEFAULT_LATENCY,
                                    CHAL_MQOS_CAM_PRIORITY_DEFAULT_LATENCY,
                                    CHAL_MQOS_CAM_LATENCY_DEFAULT_LATENCY,
                                    ctr_thold, /* in units of 6.5 micro-sec. ie 156 MHz clock */
                                    CHAL_MQOS_CAM_THOLD_DEFAULT_LATENCY);
      tag_str = CHAL_MQOS_CAM_TAG_CFG_DEFAULT_CORE1_V;
   }
   else if(strcmp (entry_type, "Cam") == 0)
   {
      cfg_id   = CHAL_MQOS_CAM_ENTRY_ASSGN_11_VCCAM01;
      //cfg_str = CHAL_MQOS_CAM_ENTRY_CFG_DEFAULT_BANDWIDTH;  /* A9 is latency type */
      cfg_str =  CHAL_MQOS_CAM_ENTRY_CFG_STR(CHAL_MQOS_CAM_READ_ENABLE_DEFAULT_LATENCY,
                                    CHAL_MQOS_CAM_WRITE_ENABLE_DEFAULT_LATENCY,
                                    CHAL_MQOS_CAM_PRIORITY_DEFAULT_LATENCY,
                                    CHAL_MQOS_CAM_LATENCY_DEFAULT_LATENCY,
                                    ctr_thold, /* in units of 6.5 micro-sec. ie 156 MHz clock */
                                    CHAL_MQOS_CAM_THOLD_DEFAULT_LATENCY);
      tag_str = CHAL_MQOS_CAM_TAG_CFG_DEFAULT_CAM;
   }
   else if(strcmp (entry_type, "V3D") == 0)  /* VC V3D */
   {
      cfg_id   = CHAL_MQOS_CAM_ENTRY_ASSGN_4_VCV3D0;
      //cfg_str = CHAL_MQOS_CAM_ENTRY_CFG_DEFAULT_BANDWIDTH;  /* A9 is latency type */
      cfg_str =  CHAL_MQOS_CAM_ENTRY_CFG_STR(CHAL_MQOS_CAM_READ_ENABLE_DEFAULT_LATENCY,
                                    CHAL_MQOS_CAM_WRITE_ENABLE_DEFAULT_LATENCY,
                                    CHAL_MQOS_CAM_PRIORITY_DEFAULT_LATENCY,
                                    CHAL_MQOS_CAM_LATENCY_DEFAULT_LATENCY,
                                    ctr_thold, /* in units of 6.5 micro-sec. ie 156 MHz clock */
                                    CHAL_MQOS_CAM_THOLD_DEFAULT_LATENCY);
      tag_str = CHAL_MQOS_CAM_TAG_CFG_DEFAULT_V3D0;
   }
   else
   {
      printk("Error: invalid confgiration type specified.\n");
      return;
   }

   if( strcmp (mem_type, "sys") == 0 )
   {
      chal_mqos_entry_config(&gMQoSHandle, cfg_str, tag_str, cfg_id);
   }
   else if (strcmp (mem_type, "mm") == 0)
   {
      chal_mqos_entry_config(&gMQoSHandleMM, cfg_str, tag_str, cfg_id);
   }
   else
   {
      printk("mQoS: Error: invalid mem type specified...\n");
   }

   return;
}
#endif

#define MQOS_DMESH_COUNTER_MHZ   156 /*156MHz*/
#define MQOS_DMESH_ENTRY_FIFO_SIZE   64   /* 64 bytes/req_entry*/
/*****************************************************************************
*
*****************************************************************************/
static void config_mqos_bandwidth_default( char *mem_type, char *entry_type, int avg_bw_in_MBPS , int thold)
{
   unsigned int cfg_str;
   unsigned int cfg_id;
   unsigned int tag_str;
   int avg_bw_in_156MHZcounter;

   /* first calculate the average access time in units of 156MHz counter */
   avg_bw_in_156MHZcounter = (MQOS_DMESH_ENTRY_FIFO_SIZE*MQOS_DMESH_COUNTER_MHZ)/avg_bw_in_MBPS; //translate it in units of 156MHz counter

   /* now set for the specified bus master */
   if(strcmp (entry_type, "SDMA") == 0)   /* Sys DMA*/
   {
      cfg_id  = CHAL_MQOS_CAM_ENTRY_ASSGN_2_SDMAC; //CHAL_MQOS_CAM_ENTRY_ID_HVS;
      cfg_str = CHAL_MQOS_CAM_ENTRY_CFG_STR(CHAL_MQOS_CAM_READ_ENABLE_DEFAULT_BANDWIDTH,
                                   CHAL_MQOS_CAM_WRITE_ENABLE_DEFAULT_BANDWIDTH,
                                   CHAL_MQOS_CAM_PRIORITY_DEFAULT_BANDWIDTH,
                                   CHAL_MQOS_CAM_LATENCY_DEFAULT_BANDWIDTH   ,
                                   avg_bw_in_156MHZcounter,
                                   thold /* in units of 6.5 micro-sec. ie 156 MHz clock */);
      tag_str = CHAL_MQOS_CAM_TAG_CFG_DEFAULT_SDMAC;
   }
   else if(strcmp (entry_type, "HVS") == 0)
   {
      cfg_id  = CHAL_MQOS_CAM_ENTRY_ASSGN_10_VCHVS; //CHAL_MQOS_CAM_ENTRY_ID_HVS;
      cfg_str = CHAL_MQOS_CAM_ENTRY_CFG_STR(CHAL_MQOS_CAM_READ_ENABLE_DEFAULT_BANDWIDTH,
                                   CHAL_MQOS_CAM_WRITE_ENABLE_DEFAULT_BANDWIDTH,
                                   CHAL_MQOS_CAM_PRIORITY_DEFAULT_BANDWIDTH,
                                   CHAL_MQOS_CAM_LATENCY_DEFAULT_BANDWIDTH   ,
                                 avg_bw_in_156MHZcounter,
                                 thold /* in units of 6.5 micro-sec. ie 156 MHz clock */);
      tag_str = CHAL_MQOS_CAM_TAG_CFG_DEFAULT_HVS;
   }
   else if(strcmp (entry_type, "ISP") == 0)  /* VC DMA0 and VC DAM1 */
   {
      cfg_id  = CHAL_MQOS_CAM_ENTRY_ASSGN_3_VCISP;
      cfg_str = CHAL_MQOS_CAM_ENTRY_CFG_STR(CHAL_MQOS_CAM_READ_ENABLE_DEFAULT_BANDWIDTH,
                                   CHAL_MQOS_CAM_WRITE_ENABLE_DEFAULT_BANDWIDTH,
                                   CHAL_MQOS_CAM_PRIORITY_DEFAULT_BANDWIDTH,
                                   CHAL_MQOS_CAM_LATENCY_DEFAULT_BANDWIDTH   ,
                                 avg_bw_in_156MHZcounter,
                                 thold /* in units of 6.5 micro-sec. ie 156 MHz clock */);
      tag_str = CHAL_MQOS_CAM_TAG_CFG_DEFAULT_ISP;
   }
   else if(strcmp (entry_type, "VDMA") == 0)  /* VC DMA0 and VC DAM1 */
   {
      cfg_id  = CHAL_MQOS_CAM_ENTRY_ASSGN_12_VCDMA01;
      cfg_str = CHAL_MQOS_CAM_ENTRY_CFG_STR(CHAL_MQOS_CAM_READ_ENABLE_DEFAULT_BANDWIDTH,
                                   CHAL_MQOS_CAM_WRITE_ENABLE_DEFAULT_BANDWIDTH,
                                   CHAL_MQOS_CAM_PRIORITY_DEFAULT_BANDWIDTH,
                                   CHAL_MQOS_CAM_LATENCY_DEFAULT_BANDWIDTH   ,
                                 avg_bw_in_156MHZcounter,
                                 thold /* in units of 6.5 micro-sec. ie 156 MHz clock */);
      tag_str = CHAL_MQOS_CAM_TAG_CFG_DEFAULT_VC_DMA;
   }
   else if(strcmp (entry_type, "VDMA2") == 0)  /* VC DMA2 */
   {
      cfg_id  = CHAL_MQOS_CAM_ENTRY_ASSGN_13_VCDMA2;
      cfg_str = CHAL_MQOS_CAM_ENTRY_CFG_STR(CHAL_MQOS_CAM_READ_ENABLE_DEFAULT_BANDWIDTH,
                                   CHAL_MQOS_CAM_WRITE_ENABLE_DEFAULT_BANDWIDTH,
                                   CHAL_MQOS_CAM_PRIORITY_DEFAULT_BANDWIDTH,
                                   CHAL_MQOS_CAM_LATENCY_DEFAULT_BANDWIDTH   ,
                                 avg_bw_in_156MHZcounter,
                                 thold /* in units of 6.5 micro-sec. ie 156 MHz clock */);
      tag_str = CHAL_MQOS_CAM_TAG_CFG_DEFAULT_VC_DMA2;
   }
   else
   {
      printk("Error: invalid confgiration type specified.\n");
      return;
   }


   if( strcmp(mem_type, "sys") == 0 )
   {
      chal_mqos_entry_config( &gMQoSHandle, cfg_str, tag_str, cfg_id );
   }
   else if( strcmp (mem_type, "mm") == 0 )
   {
      chal_mqos_entry_config(&gMQoSHandleMM, cfg_str, tag_str, cfg_id);
   }
   else
   {
      printk("mQoS: Error: invalid mem type specified...\n");
   }

   return;
}

EXPORT_SYMBOL(memc_qos_init);


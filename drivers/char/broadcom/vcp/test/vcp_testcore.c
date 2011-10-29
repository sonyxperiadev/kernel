/*****************************************************************************
* Copyright 2006 - 2010 Broadcom Corporation.  All rights reserved.
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
/**
*
*  @file    vcp_testcore.c
*
*  @brief   Test code for the VCP layer
*
****************************************************************************/

/* ---- Include Files ---------------------------------------------------- */

#include <linux/file.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/kthread.h>
#include <linux/vmalloc.h>
#include <linux/module.h>
#include <linux/fcntl.h>
#include <linux/proc_fs.h>
#include <linux/semaphore.h>                 /* For down_interruptible, up, etc. */
#include <linux/list.h>                      /* Linked list */
#include <linux/string.h>
#include <linux/time.h>
#include <linux/hrtimer.h>
#include <linux/delay.h>
#include <linux/jiffies.h>
#include <stdarg.h>
#include <asm/uaccess.h>

#include <linux/broadcom/vcp.h>
#include <linux/broadcom/knllog.h>

#include "vcp_test.h"
#include "vcptest_tables.h"

/* ---- Public Variables ------------------------------------------------- */
extern int op[];         /* array of integer settings parameters */
extern OPSTR opstr[];    /* Table of matching strings in case they are used.  */

/* ---- Private Constants and Types -------------------------------------- */

#define MAX_NUM_BUFS          80
#define MAX_PKT_SIZE          (128*1024)

/* Structure describing a variable size buffer */
struct varbuf
{
   int inuse;
   int size;
   void *bufp;
};

/* Overlay structure for the frame buffer to make use of the scratch
*  memory for linked list management
*/
struct framebuf_priv
{
   struct list_head lnode;
};

struct vcp_list
{
   struct list_head  list;       /* List head */
   struct semaphore  mutex;      /* Protect list */
   struct semaphore  count;      /* Blocking count */
};

struct vcp_pkt_loss
{
   struct semaphore mutex; /* to protect pkt loss settings and calcs */
   volatile unsigned int pkt_cnt; /* pkt cnt that wraps at N */
   atomic_t N; /* pkt loss rate = 1/N */
};

/* Test channel instance memory */
struct vcp_test_state
{
   char proc_name[20];
   char enc_name[32];
   char dec_name[32];
   struct varbuf bufs[MAX_NUM_BUFS];
   struct semaphore bufslock;     /* Protect buffers */

   struct vcp_list enclist; /* Encoder frame buffer list */

   struct vcp_pkt_loss pkt_loss_cfg;

   void *encthreadpid; /* Encoder and decoder thread IDs */
   void *decthreadpid;

   VCP_HDL encvcphdl;  /* Encoder and decoder pipeline handles */
   VCP_HDL decvcphdl;

   int enc_started;    /* 0=encoder stopped, otherwise encoder is started */
   int dec_started;    /* 0=decoder stopped, otherwise decoder is started */

   VCP_ENCODER_PARM  encParm;
   VCP_DECODER_PARM  decParm;

   volatile struct file *write_hdl;
   volatile struct file *read_hdl;

   int encFreeze;      /* 0 means the encoder is not frozen */
   volatile int enc_quitnow;       /* When set this flag causes the encoder thread to exit */
   volatile int encAddHeader;      /* When set this causes a "length" field to be prepended to each
                                    * encoder frame saved to a file. */
   volatile int enc_saveoutput;    /* When set this causes all encoder frames to be saved to a file */
   struct completion enc_thread_exit;

   volatile int loopback;          /* Loop the encoder frames to the decoder */
   volatile int decListen;         /* When set, decoder is waiting for loopback frames from the encoder */
   volatile int decFileNonstop;    /* When set, decoder data is read from a file and when the end-of-file
                                    * is reached the data is repeated. */
   volatile int decFramerate;      /* Frame rate used when decoder frames come from a file */

   unsigned int allocnum;
   unsigned int allocfail;
   unsigned int freenum;
   unsigned int enqueuenum;

   VCP_DEC_STREAM_INFO stream_info;
};

#define PROC_NAME                "vcptest_info"

#define VCPTST_LOG_SUPPORT       0
#if VCPTST_LOG_SUPPORT
#define VCPTST_LOG               printk
#else
#define VCPTST_LOG(s,args...)
#endif

/* ---- Private Variables ------------------------------------------------ */

static int gChanCount = 0;

/* ---- Private Function Prototypes -------------------------------------- */

static int vcptest_decProcessThread( void *privdata );
static int vcptest_encProcessThread( void *privdata );

static VCP_FRAMEBUF *vcptest_allocFrameBuffer( int bytelen, void *privdata );
static int vcptest_enqueueFrameBuffer( VCP_FRAMEBUF *encframep, void *privdata );
static void vcptest_freeFrameBuffer( VCP_FRAMEBUF *decframep, void *privdata );
static void vcptest_decEventHandler( VCP_EVENT_CODE eventCode, void *info, void *privdata );
static void vcptest_encEventHandler( VCP_EVENT_CODE eventCode, void *info, void *privdata );

static void vcptest_debug_init( struct vcp_test_state *statep );
static void vcptest_debug_exit( struct vcp_test_state *statep );

static VCP_HOST_ENC_CBKS gEncCbks =
{
   .alloc = vcptest_allocFrameBuffer,
   .enqueue = vcptest_enqueueFrameBuffer,
   .enc_event = vcptest_encEventHandler,
};

static VCP_HOST_DEC_CBKS gDecCbks =
{
   .free = vcptest_freeFrameBuffer,
   .dec_event = vcptest_decEventHandler,
};


/* ---- Functions -------------------------------------------------------- */
/***************************************************************************/
/**
*  Helper routine to initialize lists
*
*  @return  Nothing
*/
static void init_list(
   struct vcp_list *listp
)
{
   INIT_LIST_HEAD( &listp->list );
   sema_init( &listp->mutex, 1 );
   sema_init( &listp->count, 0 );
}

/***************************************************************************/
/**
*  Helper routine to initialize the packet loss data structure
*
*  @return  Nothing
*/
static void pkt_loss_init( struct vcp_pkt_loss *cfg )
{
   sema_init( &cfg->mutex, 1 );
   cfg->pkt_cnt = 0;
   atomic_set( &cfg->N, 0 );
}

/***************************************************************************/
/**
*  Helper routine to run the packet loss simulation algorithm, should be
*  called per packet processing
*
*  @return  1 to indicate that packet should be dropped, 0 otherwise
*/
static int pkt_loss_run( struct vcp_pkt_loss *cfg )
{
   int rc = 0;

   /* quit right away if N = 0, avoid calling "down" */
   if ( atomic_read( &cfg->N ) == 0 )
      return 0;

   down( &cfg->mutex );
   if ( ++cfg->pkt_cnt >= atomic_read( &cfg->N ))
   {
      rc = 1;
      cfg->pkt_cnt = 0;
   }
   up( &cfg->mutex );

   return rc;
}

/***************************************************************************/
/**
*  Helper routine to set the packet loss rate
*
*  @return  Nothing
*/
static void pkt_loss_rate_set( struct vcp_pkt_loss *cfg, unsigned int rate )
{
   down( &cfg->mutex );
   atomic_set( &cfg->N, rate );
   cfg->pkt_cnt = 0; /* also zero the packet counter */
   up( &cfg->mutex );
}

/***************************************************************************/
/**
*  Helper routine to reset the packet loss counter
*
*  @return  Nothing
*/
static void pkt_loss_reset( struct vcp_pkt_loss *cfg )
{
   down( &cfg->mutex );
   cfg->pkt_cnt = 0;
   up( &cfg->mutex );
}

/***************************************************************************/
/**
*  Helper routine to open VCP pipelines
*
*  @return  Nothing
*/
static int vcptest_open( struct vcp_test_state *statep,
      const char *encname, const char *decname )
{
   int err;

   /*
    * Since either encoder or decoder is not mandatory, we should just print a
    * warning message when they fail to be opened
    */

   err = vcp_open_encoder( encname, &gEncCbks, statep, &statep->encvcphdl );
   if ( err )
   {
      printk( "%s: Warning! Failed to open '%s' encoder pipeline, err=%i\n", __func__, encname, err );
   }

   err = vcp_open_decoder( decname, &gDecCbks, statep, &statep->decvcphdl );
   if ( err )
   {
      printk( "%s: Warning! Failed to open '%s' decoder pipeline, err=%i\n", __func__, decname, err );
   }

   return 0;
}

/***************************************************************************/
/**
*  Helper routine to close VCP pipelines
*
*  @return  Nothing
*/
static int vcptest_close( struct vcp_test_state *statep )
{
   int err, rc = 0;

   if ( statep->encvcphdl )
   {
      err = vcp_close( statep->encvcphdl );
      if ( err )
      {
         printk( "%s: Failed to close encoder pipeline, err=%i\n", __func__, err );
         rc = err;
      }
      statep->encvcphdl = NULL;
   }

   if ( statep->decvcphdl )
   {
      err = vcp_close( statep->decvcphdl );
      if ( err )
      {
         printk( "%s: Failed to close decoder pipeline, err=%i\n", __func__, err );
         rc = err;
      }
      statep->decvcphdl = NULL;
   }

   return rc;
}

/***************************************************************************/
/**
*  Initialize test environment
*
*  @return  0 for success, otherwise -ve error
*/
int vcptest_init( VCP_TEST_HDL *hdlp )
{
   struct vcp_test_state *statep;
   VCP_FRAMEBUF_HDR *hdrp;
   VCP_DECODER_PARM decParm = VCPTEST_DEFAULT_DECODER_PARAMETERS;
   VCP_ENCODER_PARM encParm = VCPTEST_DEFAULT_ENCODER_PARAMETERS;

   /* Error check that the framebuf header structure is compatible */
   if ( sizeof(struct framebuf_priv) > sizeof(hdrp->scratch) )
   {
      printk( "%s: Fatal error: sizeof framebuf_priv (%d) greater than VCP scratchmem (%d)\n",
            __func__, sizeof(struct framebuf_priv), sizeof(hdrp->scratch) );
      return -EPERM;
   }

   statep = kmalloc( sizeof(*statep), GFP_KERNEL );
   if ( statep == NULL )
   {
      return -ENOMEM;
   }
   memset( statep, 0, sizeof(*statep) );

   sema_init( &statep->bufslock, 1 );
   init_list( &statep->enclist );
   init_completion( &statep->enc_thread_exit );

   pkt_loss_init( &statep->pkt_loss_cfg );

   if ( gChanCount )
   {
      /* Add numerical suffix for subsequent channels. This
       * quirkiness is to maintain backward compatibility when only
       * one channel is instantiated
       */
      sprintf( statep->proc_name, "%s%i", PROC_NAME, gChanCount+1 );
   }
   else
   {
      sprintf( statep->proc_name, "%s", PROC_NAME );
   }

   vcptest_debug_init( statep );

   sprintf( statep->enc_name, "%s", "default" );
   sprintf( statep->dec_name, "%s", "default" );

   /* Set the default encoder and decoder parameters */
   statep->decParm = decParm;
   statep->encParm = encParm;

   /* Open VCP channels */
   vcptest_open( statep, statep->enc_name, statep->dec_name );

   *hdlp = statep;
   gChanCount++;

   return 0;
}

/***************************************************************************/
/**
*  Cleanup test environment
*
*  @return  0 for success, otherwise -ve error
*/
int vcptest_exit( VCP_TEST_HDL hdl )
{
   struct vcp_test_state *statep = hdl;
   int rc;
   int i;

   printk( "%s: statep=0x%x\n", __func__, (unsigned int)statep );
   if ( statep == NULL )
   {
      return -EINVAL;
   }
   rc = vcptest_stop_dec( hdl );

   rc = vcptest_close( statep );

   /* Cleanup memory */
   for ( i = 0; i < sizeof(statep->bufs)/sizeof(statep->bufs[0]); i++ )
   {
      if ( statep->bufs[i].bufp )
      {
         vfree( statep->bufs[i].bufp );
         statep->bufs[i].bufp = NULL;
         statep->bufs[i].size = 0;
      }
   }

   vcptest_debug_exit( statep );

   kfree( statep );
   return rc;
}

/***************************************************************************/
/**
*  Reinitialize VCP pipeline
*
*  @return  0 for success, otherwise -ve error
*
*  @remarks
*
*  Test string syntax: [encname] [decname]
*  encname     - Encoder name. If this is the only name provided, then it
*                is used for both encoder and decoder
*  decname     - Decoder name (optional)
*/
int vcptest_reinit( VCP_TEST_HDL hdl, int opargs )
{
   struct vcp_test_state *statep = hdl;
   int err, rc = 0;
   char *encname, *decname;

   printk( "%s: statep=0x%x\n", __func__, (unsigned int)statep );
   if ( statep == NULL )
   {
      return -EINVAL;
   }

   if( opargs > 1 )
   {
      encname = opstr[1].str;
      if( opargs > 2 )
      {
         decname = opstr[2].str;
      }
      else
      {
         decname = encname;
      }
   }
   else
   {
      return -EINVAL;
   }

   err = vcptest_stop_enc( statep );
   if ( err )
   {
      printk( "%s: Failed stopping existing encoder test, err=%i\n", __func__, err );
      rc = err;
   }
   err = vcptest_stop_dec( statep );
   if ( err )
   {
      printk( "%s: Failed stopping existing decoder test, err=%i\n", __func__, err );
      rc = err;
   }

   /* Cleanup existing handles */
   err = vcptest_close( statep );
   if ( err )
   {
      printk( "%s: Failed cleaning up existing pipelines, err=%i\n", __func__, err );
      rc = err;
   }

   /* Open new pipelines */
   err = vcptest_open( statep, encname, decname );
   if ( err )
   {
      printk( "%s: Failed openning new pipelines, err=%i\n", __func__, err );
      rc = err;
   }

   strncpy( statep->enc_name, encname, sizeof(statep->enc_name) );
   strncpy( statep->dec_name, decname, sizeof(statep->dec_name) );

   return rc;
}

/***************************************************************************/
/**
*  Configure VCP test packet loss simulation
*
*  @return  0 for success, otherwise -ve error
*
*  @remarks
*
*  Test string syntax: [N]
*  Packet loss rate = 1/N, where N=0 is a special case for no packet loss
*/
int vcptest_pkt_loss_cfg( VCP_TEST_HDL hdl, int opargs )
{
   struct vcp_test_state *statep = hdl;
   struct vcp_pkt_loss *pkt_loss_cfg;
   long tmpN;

   printk( "%s: statep=0x%x\n", __func__, (unsigned int)statep );
   if ( statep == NULL )
   {
      printk( "Invalid VCP handle\n");
      return -EINVAL;
   }

   tmpN = ( opargs > 1 ) ? op[1] : 0;   /* 0 means no loss */

   if ( tmpN < 0 )
   {
      printk( "Invalid packet loss N=%ld\n", tmpN );
      return -EINVAL;
   }

   pkt_loss_cfg = &statep->pkt_loss_cfg;
   pkt_loss_rate_set( pkt_loss_cfg, tmpN );

   if ( tmpN )
   {
      printk( "Packet loss rate configured to 1/%ld\n", tmpN );
   }
   else
   {
      printk( "No Packet loss configured\n");
   }
   return 0;
}

static void vcptestPrintEncParms( VCP_ENCODER_PARM *parm )
{
   printk("codectype=%d (%s)\n", parm->codectype, vcptestTableCode2Str(parm->codectype, vcptestCodecTbl) );
   printk("profile=%d (%s)\n", parm->profile, vcptestTableCode2Str(parm->profile, vcptestVideoProfile) );
   printk("level=%d (%s)\n", parm->level, vcptestTableCode2Str(parm->level, vcptestVideoLevel) );
   printk("framerate=%d (%s)\n", parm->framerate, vcptestTableCode2Str(parm->framerate, vcptestVideoFrameRate) );
   printk("bitrate=%d (%s)\n", parm->bitrate, vcptestTableCode2Str(parm->bitrate, vcptestVideoBitRate) );
   printk("rateControlMode=%d\n", parm->rateControlMode );
   printk("reswidth=%d\n", parm->reswidth );
   printk("resheight=%d\n", parm->resheight );
   printk("maxGOPsize=%d (%s)\n", parm->maxGOPsize, vcptestTableCode2Str(parm->maxGOPsize, vcptestVideoMaxGopSize) );
   printk("CustomMaxMBPS=%d\n", parm->CustomMaxMBPS );
   printk("MaxStaticMBPS=%d\n", parm->MaxStaticMBPS );
   printk("CustomMaxFS=%d\n", parm->CustomMaxFS );
   printk("CustomMaxDPB=%d\n", parm->CustomMaxDPB );
   printk("CustomMaxBRandCPB=%d\n", parm->CustomMaxBRandCPB );
   printk("max_rcmd_nal_unit_size=%d (%s)\n", parm->max_rcmd_nal_unit_size, vcptestTableCode2Str(parm->max_rcmd_nal_unit_size, vcptestVideoNalUnitSize) );
   printk("max_nal_unit_size=%d (%s)\n", parm->max_nal_unit_size, vcptestTableCode2Str(parm->max_nal_unit_size, vcptestVideoNalUnitSize) );
   printk("aspectx=%d\n", parm->aspectx );
   printk("aspecty=%d\n", parm->aspecty );
   printk("min_quant=%d\n", parm->min_quant );
   printk("max_quant=%d\n", parm->max_quant );
   printk("deblock_mode=%d\n", parm->deblock_mode);
   printk("eedeEnable=%d\n", parm->eedeEnable );
   printk("eedeLossRate=%d\n", parm->eedeLossRate );
}

static void vcptestPrintDecParms( VCP_DECODER_PARM *parm )
{
   printk("codectype=%d (%s)\n", parm->codectype, vcptestTableCode2Str(parm->codectype, vcptestCodecTbl) );
   printk("maxFrameSize=%d\n", parm->maxFrameSize );
   printk("maxFrameWidth=%d\n", parm->maxFrameWidth );
   printk("plcOption=%d (%s)\n", parm->plcOption, vcptestTableCode2Str(parm->plcOption, vcptestModeTbl) );
   printk("errConcealEnable=%d (%s)\n", parm->errConcealEnable, vcptestTableCode2Str(parm->errConcealEnable, vcptestModeTbl) );
   printk("sdpParmEnable=%d (%s)\n", parm->sdpParmEnable, vcptestTableCode2Str(parm->sdpParmEnable, vcptestModeTbl) );
   printk("spropParm=%s\n", parm->spropParm );
}

/***************************************************************************/
/**
*  Configure the decoder parameters vcptest uses, before the decoder is started
*
*  @return
*
*  @remarks
*/
int vcptest_cfg_decparms( VCP_TEST_HDL hdl, int opargs )
{
   struct vcp_test_state *statep = hdl;
   VCP_DECODER_PARM *parm = &statep->decParm;

   if ( statep->dec_started )
   {
      printk( "Decoder stream already running. Stop it before configuring decoder parameter\n");
      return -EPERM;
   }

   if( opargs > 1 )
   {
      if( vcptestTableStr2Code( &op[1], opstr[1].str, vcptestCodecTbl ) )
      {
         printk("Error: codec is out of range\n");
      }
      else parm->codectype=op[1];
   }
   if( opargs > 2 ) parm->maxFrameSize = op[2];
   if( opargs > 3 ) parm->maxFrameWidth = op[3];
   if( opargs > 4 )
   {
      if( vcptestTableStr2Code( &op[4], opstr[4].str, vcptestModeTbl ) )
      {
         printk("Error: plcOption is out of range\n");
      }
      else parm->plcOption = op[4];
   }
   if( opargs > 5 )
   {
      if( vcptestTableStr2Code( &op[5], opstr[5].str, vcptestModeTbl ) )
      {
         printk("Error: errConcealEnable is out of range\n");
      }
      else parm->errConcealEnable = op[5];
   }
   if( opargs > 6 )
   {
      if( vcptestTableStr2Code( &op[6], opstr[6].str, vcptestModeTbl ) )
      {
         printk("Error: sdpParmEnable is out of range\n");
      }
      else parm->sdpParmEnable = op[6];
   }
   if( opargs > 7 )
   {
      strncpy( parm->spropParm, opstr[7].str, sizeof(parm->spropParm) );
   }

   vcptestPrintDecParms( &statep->decParm );

   return 0;
}
/***************************************************************************/
/**
*  Configure the encoder parameters vcptest uses, before the encoder is started
*
*  @return
*
*  @remarks
*/
int vcptest_cfg_encparms( VCP_TEST_HDL hdl, int opargs )
{
   struct vcp_test_state *statep = hdl;
   VCP_ENCODER_PARM *parm = &statep->encParm;

   if ( statep->enc_started )
   {
      printk( "Encoder stream already running. Stop it before configuring encoder parameter\n" );
      return -EPERM;
   }
   if( opargs > 1 )
   {
      if( vcptestTableStr2Code( &op[1], opstr[1].str, vcptestCodecTbl ) )
      {
         printk("Error: codec is out of range\n");
      }
      else parm->codectype=op[1];
   }
   if( opargs > 2 )
   {
      if( vcptestTableStr2Code( &op[2], opstr[2].str, vcptestVideoProfile ) )
      {
         printk("Error: profile is out of range\n");
      }
      else parm->profile = op[2];
   }
   if( opargs > 3 )
   {
      if( vcptestTableStr2Code( &op[3], opstr[3].str, vcptestVideoLevel ) )
      {
         printk("Error: level is out of range\n");
      }
      else parm->level = op[3];
   }
   if( opargs > 4 )
   {
      if( vcptestTableStr2Code( &op[4], opstr[4].str, vcptestVideoFrameRate ) )
      {
         printk("Error: framerate is out of range\n");
      }
      else parm->framerate = op[4];
   }
   if( opargs > 5 )
   {
      if( vcptestTableStr2Code( &op[5], opstr[5].str, vcptestVideoBitRate ) )
      {
         printk("Error: framerate is out of range\n");
      }
      else parm->bitrate = op[5];
   }
   if( opargs > 6 ) parm->rateControlMode = op[6];

   if( opargs > 7 )
   {
      if( vcptestTableStr2Code( &op[7], opstr[7].str, vcptestVideoResolution) )
      {
         printk("Error: resolution is out of range\n");
      }
      else
      {
         switch( op[7] )
         {
            case VCP_RESOLUTION_SQCIF :
               parm->reswidth = VCP_SQCIF_WIDTH;
               parm->resheight = VCP_SQCIF_HEIGHT;
               break;
            case VCP_RESOLUTION_QCIF :
               parm->reswidth = VCP_QCIF_WIDTH;
               parm->resheight = VCP_QCIF_HEIGHT;
               break;
            case VCP_RESOLUTION_QVGA :
               parm->reswidth = VCP_QVGA_WIDTH;
               parm->resheight = VCP_QVGA_HEIGHT;
               break;
            case VCP_RESOLUTION_CIF :
               parm->reswidth = VCP_CIF_WIDTH;
               parm->resheight = VCP_CIF_HEIGHT;
               break;
            case VCP_RESOLUTION_VGA :
               parm->reswidth = VCP_VGA_WIDTH;
               parm->resheight = VCP_VGA_HEIGHT;
               break;
            case VCP_RESOLUTION_WVGA :
               parm->reswidth = VCP_WVGA_WIDTH;
               parm->resheight = VCP_WVGA_HEIGHT;
               break;
            case VCP_RESOLUTION_4CIF :
               parm->reswidth = VCP_4CIF_WIDTH;
               parm->resheight = VCP_4CIF_HEIGHT;
               break;
            case VCP_RESOLUTION_HD :
               parm->reswidth = VCP_HD_WIDTH;
               parm->resheight = VCP_HD_HEIGHT;
               break;
            case VCP_RESOLUTION_1080P :
               parm->reswidth = VCP_1080P_WIDTH;
               parm->resheight = VCP_1080P_HEIGHT;
               break;
            default:
               break;
         }
      }
   }
   if( opargs > 8 )
   {
      if( vcptestTableStr2Code( &op[8], opstr[8].str, vcptestVideoMaxGopSize ) )
      {
         printk("Error: framerate is out of range\n");
      }
      else parm->maxGOPsize = op[8];
   }
   if( opargs > 9 )  parm->CustomMaxMBPS = op[9];
   if( opargs > 10 ) parm->MaxStaticMBPS = op[10];
   if( opargs > 11 ) parm->CustomMaxFS = op[11];
   if( opargs > 12 ) parm->CustomMaxDPB = op[12];
   if( opargs > 13 ) parm->CustomMaxBRandCPB = op[13];
   if( opargs > 14 )
   {
      if( vcptestTableStr2Code( &op[14], opstr[14].str, vcptestVideoNalUnitSize ) )
      {
         printk("Error: max_rcmd_nal_unit_size is out of range\n");
      }
      else parm->max_rcmd_nal_unit_size = op[14];
   }
   if( opargs > 15 )
   {
      /* short term change, make nal_unit_size number of MB row per slice */
#if 0
      if( vcptestTableStr2Code( &op[15], opstr[15].str, vcptestVideoNalUnitSize ) )
      {
         printk("Error: max_nal_unit_size is out of range\n");
      }
#endif
      parm->max_nal_unit_size = op[15];
   }
   if( opargs > 16 ) parm->aspectx = op[16];
   if( opargs > 17 ) parm->aspecty = op[17];
   if( opargs > 18 ) parm->min_quant = op[18];
   if( opargs > 19 ) parm->max_quant = op[19];
   if( opargs > 20 ) parm->deblock_mode= op[20];
   if( opargs > 21 ) parm->eedeEnable = op[21];
   if( opargs > 22 ) parm->eedeLossRate = op[22];

   vcptestPrintEncParms( parm );
   return 0;
}

/***************************************************************************/
/**
*  Start encoder pipeline
*
*  @return  0 for success, otherwise -ve error
*
*  @remarks
*/
int vcptest_start_enc( VCP_TEST_HDL hdl, int opargs )
{
   struct vcp_test_state *statep = hdl;
   char *filename = "encode.bin";
   mm_segment_t old_fs;
   int err;
   VCP_ENCODER_PARM  *parm;

   printk( "%s: statep=0x%x\n", __func__, (unsigned int)statep );
   if ( statep == NULL )
   {
      return -EINVAL;
   }

   if ( !statep->encvcphdl )
   {
      printk( "Fail - Encoder is not open." );
      return -EPERM;
   }

   parm = &statep->encParm;

   if ( statep->enc_started )
   {
      printk( "Encoder stream already running. Stop it before restarting\n" );
      return -EPERM;
   }

   statep->encFreeze = 0;
   statep->enc_quitnow = 0;
   statep->write_hdl = NULL;

   /* Clear statistics */
   statep->allocnum = 0;
   statep->allocfail = 0;
   statep->freenum = 0;
   statep->enqueuenum = 0;

   if( opargs > 1 )
   {
      if( vcptestTableStr2Code( &op[1], opstr[1].str, vcptestEncTest ) )
      {
         printk("Error: test type is out of range\n");
         return -EINVAL;
      }
      else
      {
         switch( op[1] )
         {
            case VCPTEST_ENC_FILESAVE_RAW :
               statep->enc_saveoutput = 1;
               statep->encAddHeader = 0;
               statep->loopback = 0;
               break;
            case VCPTEST_ENC_FILESAVE_HEADER :
               statep->enc_saveoutput = 1;
               statep->encAddHeader = 1;
               statep->loopback = 0;
               break;
            case VCPTEST_ENC_FILE_NOSAVE :
               statep->enc_saveoutput = 0;
               statep->loopback = 0;
               break;
            case VCPTEST_ENC_LOOPBACK :
               statep->enc_saveoutput = 0;
               statep->encAddHeader = 0;
               statep->loopback = 1;
               break;
            default:
               break;
         }
      }
   }
   else
   {
      printk("Error: test type must be specified\n");
      return -EINVAL;
   }

   if( opargs > 2 ) filename = opstr[2].str;

   if ( statep->loopback == 1 )
   {
      if( !(statep->dec_started && statep->decListen) )
      {
         printk( "Error - Decoder must be started in LISTEN mode before loopback is requested\n");
         return -EPERM;
      }
   }
   if ( statep->enc_saveoutput == 1 )
   {
      struct file *write_hdl;

      old_fs = get_fs();
      set_fs ( get_ds() );
      write_hdl = filp_open( filename, O_TRUNC | O_WRONLY | O_CREAT, 0644 );
      set_fs( old_fs );
      if ( IS_ERR( write_hdl ))
      {
         printk( "failed to open file for storing bitstream\n" );
         return -EPERM;

      }
      statep->write_hdl = write_hdl;

      if( statep->encAddHeader )
      {
          printk( "Saving encoder output to '%s' with headers\n", filename );
      }
      else
      {
          printk( "Saving encoder output to '%s'\n", filename );
      }
   }

   /* setup encoding processing thread */
   statep->encthreadpid = kthread_run( vcptest_encProcessThread, statep, "vcptestEnc" );

   vcptestPrintEncParms( &statep->encParm );

   err = vcp_set_parameter( statep->encvcphdl, VCP_PARM_ENC_CONFIG, &statep->encParm );
   if ( err )
   {
      printk( "Failed to set VCP_PARM_ENC_CONFIG, err=%i\n", err );
      return err;
   }

   err = vcp_start( statep->encvcphdl );
   if ( err )
   {
      printk( "Failed to start encoder pipeline, err=%i\n", err );
      return err;
   }
   statep->enc_started = 1;

   return 0;
}

/***************************************************************************/
/**
*  Stop encoder pipeline
*
*  @return  0 for success, otherwise -ve error
*/
int vcptest_stop_enc( VCP_TEST_HDL hdl )
{
   struct vcp_test_state *statep = hdl;
   int err, rc = 0;

   printk( "%s: statep=0x%x\n", __func__, (unsigned int)statep );
   if ( statep == NULL )
   {
      return -EINVAL;
   }

   if ( statep->enc_started == 0 )
   {
      return 0;
   }
   if ( !statep->encvcphdl )
   {
      printk( "Fail - Encoder is not open." );
      return -EPERM;
   }

   err = vcp_stop( statep->encvcphdl );
   if ( err )
   {
      printk( "Failed to stop encoder pipeline, err=%i\n", err );
      rc = err;
   }

   if ( statep->encthreadpid )
   {
      statep->enc_quitnow = 1;
      up( &statep->enclist.count );
      wait_for_completion( &statep->enc_thread_exit );
      statep->encthreadpid = NULL;
   }

   pkt_loss_reset( &statep->pkt_loss_cfg );

   statep->enc_started = 0;

   return rc;
}

/***************************************************************************/
/**
*  start decoder pipeline
*
*  @return  0 for success, otherwise -ve error
*/
int vcptest_start_dec( VCP_TEST_HDL hdl, int opargs )
{
   struct vcp_test_state *statep = hdl;
   int err;
   mm_segment_t old_fs;
   struct file *read_hdl;
   char *filename = "decode.bin";   /* Default file name */

   printk( "%s: statep=0x%x\n", __func__, (unsigned int)statep );
   if ( statep == NULL )
   {
      return -EINVAL;
   }

   if ( statep->dec_started )
   {
      printk( "Decoder stream already running. Stop it before restarting\n" );
      return -EPERM;
   }

   if ( !statep->decvcphdl )
   {
      printk( "Fail - Decoder is not open." );
      return -EPERM;
   }

   if( opargs > 1 )
   {
      if( vcptestTableStr2Code( &op[1], opstr[1].str, vcptestDecTest ) )
      {
         printk("Error: test type is out of range\n");
         return -EINVAL;
      }
      else
      {
         switch( op[1] )
         {
            case VCPTEST_DEC_LISTEN :
               statep->decListen = 1;
               break;
            case VCPTEST_DEC_FILEONCE :
               statep->decListen = 0;
               statep->decFileNonstop = 0;
               break;
            case VCPTEST_DEC_FILENONSTOP :
               statep->decListen = 0;
               statep->decFileNonstop = 1;
               break;
            default:
               break;
         }
      }
   }
   else
   {
      printk("Error: test type must be specified\n");
      return -EINVAL;
   }

   if( !statep->decListen )
   {
      /* Get the frame rate */
      statep->decFramerate = (opargs > 2) ? op[2] : 30;

      /* Get file name */
      if( opargs > 3 ) filename = opstr[3].str;

      old_fs = get_fs();
      set_fs( get_ds() );
      read_hdl = filp_open( filename, O_RDONLY, 0644 );
      set_fs( old_fs );
      if ( IS_ERR( read_hdl ))
      {
         printk( "failed to open file for reading bitstream\n" );
         return -EINVAL;
      }
      statep->read_hdl = read_hdl;
   }

   vcptestPrintDecParms( &statep->decParm );

   VCPTST_LOG( "Configuring decoder ...\n");
   err = vcp_set_parameter( statep->decvcphdl, VCP_PARM_DEC_CONFIG, &statep->decParm );
   if ( err )
   {
      printk( "Failed to set VCP_PARM_DEC_CONFIG, err=%i\n", err );
      return err;
   }

   VCPTST_LOG( "Starting decoder ...\n");
   err = vcp_start( statep->decvcphdl );
   if ( err )
   {
      printk( "Failed to start decoder pipeline, err=%i\n", err );
      return err;
   }

   if ( statep->decListen == 0 )
   {
      printk( "Starting decoder test thread ...\n");
      statep->decthreadpid = kthread_run( vcptest_decProcessThread, statep, "vcptestDec" );
   }

   VCPTST_LOG( "decoder test started\n" );

   statep->dec_started = 1;

   return 0;
}

/***************************************************************************/
/**
*  Stop decoder pipeline
*
*  @return  0 for success, otherwise -ve error
*/
int vcptest_stop_dec( VCP_TEST_HDL hdl )
{
   struct vcp_test_state *statep = hdl;
   int err, rc = 0;

   printk( "%s: statep=0x%x\n", __func__, (unsigned int)statep );
   if ( statep == NULL )
   {
      return -EINVAL;
   }
   if ( (statep->dec_started == 0) || (!statep->decvcphdl) )
   {
      return 0;
   }
   if ( statep->enc_started && statep->loopback )
   {
      printk( "Failed to stop decoder.  Loopback running - you must stop the encoder first\n" );
      return -EPERM;
   }

   if ( statep->decthreadpid )
   {
      kthread_stop( statep->decthreadpid );
   }

   err = vcp_stop( statep->decvcphdl );
   if ( err )
   {
      printk( "Failed to stop decoder pipeline, err=%i\n", err );
      rc = err;
   }

   pkt_loss_reset( &statep->pkt_loss_cfg );

   statep->dec_started = 0;

   return rc;
}

/***************************************************************************/
/**
*  Configure the encoder camera preview window
*
*  @remarks
*    the encoder parameter are pass within a string in the following manner
*    <ori_x> <ori_y> <width> <height> <z-order layer> <device number>
*
*/
int vcptest_set_encdisp( VCP_TEST_HDL hdl, int opargs )
{
   struct vcp_test_state *statep = hdl;
   VCP_DISPLAY_PARM parm;
   int err;

   printk( "%s: statep=0x%x\n", __func__, (unsigned int)statep );
   if ( statep == NULL )
   {
      return -EINVAL;
   }

   if ( !statep->encvcphdl )
   {
      printk( "Fail - Encoder is not open." );
      return -EPERM;
   }

   /* Parse parameters */
   parm.src.xoffset  = ( opargs > 1 ) ? op[1] : 0;
   parm.src.yoffset  = ( opargs > 2 ) ? op[2] : 0;
   parm.src.width    = ( opargs > 3 ) ? op[3] : 0;
   parm.src.height   = ( opargs > 4 ) ? op[4] : 0;
   parm.dest.xoffset = ( opargs > 5 ) ? op[5] : 0;
   parm.dest.yoffset = ( opargs > 6 ) ? op[6] : 0;
   parm.dest.width   = ( opargs > 7 ) ? op[7] : 0;
   parm.dest.height  = ( opargs > 8 ) ? op[8] : 0;
   parm.layer        = ( opargs > 9 )  ? op[9] : 0;
   if( opargs > 10 )
   {
      if( vcptestTableStr2Code( &op[10], opstr[10].str, vcptestVideoDisplayDevice ) )
      {
         printk("Error: device is out of range\n");
      }
      else parm.device = op[10];
   }
   if( opargs > 11 )
   {
      if( vcptestTableStr2Code( &op[11], opstr[11].str, vcptestVideoDisplayTransform ) )
      {
         printk("Error: transform is out of range\n");
      }
      else parm.transform = op[11];
   }

   err = vcp_set_parameter( statep->encvcphdl, VCP_PARM_ENC_DISP, &parm );
   if ( err )
   {
      printk( "Failed to set VCP_PARM_ENC_DISP, err=%i\n", err );
      return err;
   }
   printk("VCP_PARM_ENC_DISP src=%d,%d,%d,%d  dest==%d,%d,%d,%d layer=%d device=%d transform=%d\n",
          parm.src.xoffset, parm.src.yoffset, parm.src.width, parm.src.height,
          parm.dest.xoffset, parm.dest.yoffset, parm.dest.width, parm.dest.height,
          parm.layer, parm.device, parm.transform );

   return 0;
}

/***************************************************************************/
/**
*  Configure the decoder display screen
*/
int vcptest_set_decdisp( VCP_TEST_HDL hdl, int opargs )
{
   struct vcp_test_state *statep = hdl;
   VCP_DISPLAY_PARM parm;
   int err;

   printk( "%s: statep=0x%x\n", __func__, (unsigned int)statep );
   if ( statep == NULL )
   {
      return -EINVAL;
   }

   if ( !statep->decvcphdl )
   {
      printk( "Fail - Decoder is not open." );
      return -EPERM;
   }

   /* Parse parameters */
   parm.src.xoffset  = ( opargs > 1 ) ? op[1] : 0;
   parm.src.yoffset  = ( opargs > 2 ) ? op[2] : 0;
   parm.src.width    = ( opargs > 3 ) ? op[3] : 0;
   parm.src.height   = ( opargs > 4 ) ? op[4] : 0;
   parm.dest.xoffset = ( opargs > 5 ) ? op[5] : 0;
   parm.dest.yoffset = ( opargs > 6 ) ? op[6] : 0;
   parm.dest.width   = ( opargs > 7 ) ? op[7] : 0;
   parm.dest.height  = ( opargs > 8 ) ? op[8] : 0;
   parm.layer        = ( opargs > 9 ) ? op[9] : 0;
   if( opargs > 10 )
   {
      if( vcptestTableStr2Code( &op[10], opstr[10].str, vcptestVideoDisplayDevice ) )
      {
         printk("Error: device is out of range\n");
      }
      else parm.device = op[10];
   }
   if( opargs > 11 )
   {
      if( vcptestTableStr2Code( &op[11], opstr[11].str, vcptestVideoDisplayTransform ) )
      {
         printk("Error: transform is out of range\n");
      }
      else parm.transform = op[11];
   }

   err = vcp_set_parameter( statep->decvcphdl, VCP_PARM_DEC_DISP, &parm );
   if ( err )
   {
      printk( "Failed to set VCP_PARM_DEC_DISP, err=%i\n", err );
      return err;
   }
   printk("VCP_PARM_DEC_DISP src=%d,%d,%d,%d  dest==%d,%d,%d,%d layer=%d device=%d transform=%d\n",
          parm.src.xoffset, parm.src.yoffset, parm.src.width, parm.src.height,
          parm.dest.xoffset, parm.dest.yoffset, parm.dest.width, parm.dest.height,
          parm.layer, parm.device, parm.transform );

   return 0;
}

/***************************************************************************/
/**
*  Configure a decoder display mask
*/
int vcptest_set_decdisp_mask( VCP_TEST_HDL hdl, int opargs )
{
   struct vcp_test_state *statep = hdl;
   VCP_DISPLAY_MASK_PARM parm;
   int err;

   printk( "%s: statep=0x%x\n", __func__, (unsigned int)statep );
   if ( statep == NULL )
   {
      return -EINVAL;
   }

   if ( !statep->decvcphdl )
   {
      printk( "Fail - Decoder is not open." );
      return -EPERM;
   }

   /* Parse parameters */
   parm.handle = ( opargs > 1 ) ? op[1] : 0;
   parm.layer  = ( opargs > 2 ) ? op[2] : 0;
   parm.rect.xoffset = ( opargs > 3 ) ? op[3] : 0;
   parm.rect.yoffset = ( opargs > 4 ) ? op[4] : 0;
   parm.rect.width   = ( opargs > 5 ) ? op[5] : 0;
   parm.rect.height  = ( opargs > 6 ) ? op[6] : 0;

   err = vcp_set_parameter( statep->decvcphdl, VCP_PARM_DISP_MASK, &parm );
   if ( err )
   {
      printk( "Failed to set VCP_PARM_DISP_MASK, err=%i\n", err );
      return err;
   }
   printk("VCP_PARM_DISP_MASK.  handle=%d, layer=%d, x=%d, y=%d, width=%d, height=%d\n",
          parm.handle, parm.layer, parm.rect.xoffset, parm.rect.yoffset, parm.rect.width, parm.rect.height );

   return 0;
}

/***************************************************************************/
/**
*  Configure an encoder display mask
*/
int vcptest_set_encdisp_mask( VCP_TEST_HDL hdl, int opargs )
{
   struct vcp_test_state *statep = hdl;
   VCP_DISPLAY_MASK_PARM parm;
   int err;

   printk( "%s: statep=0x%x\n", __func__, (unsigned int)statep );
   if ( statep == NULL )
   {
      return -EINVAL;
   }

   if ( !statep->encvcphdl )
   {
      printk( "Fail - Encoder is not open." );
      return -EPERM;
   }

   /* Parse parameters */
   parm.handle = ( opargs > 1 ) ? op[1] : 0;
   parm.layer  = ( opargs > 2 ) ? op[2] : 0;
   parm.rect.xoffset = ( opargs > 3 ) ? op[3] : 0;
   parm.rect.yoffset = ( opargs > 4 ) ? op[4] : 0;
   parm.rect.width   = ( opargs > 5 ) ? op[5] : 0;
   parm.rect.height  = ( opargs > 6 ) ? op[6] : 0;

   err = vcp_set_parameter( statep->encvcphdl, VCP_PARM_DISP_MASK, &parm );
   if ( err )
   {
      printk( "Failed to set VCP_PARM_DISP_MASK, err=%i\n", err );
      return err;
   }
   printk("VCP_PARM_DISP_MASK.  handle=%d, layer=%d, x=%d, y=%d, width=%d, height=%d\n",
          parm.handle, parm.layer, parm.rect.xoffset, parm.rect.yoffset, parm.rect.width, parm.rect.height );

   return 0;
}

/***************************************************************************/
/**
*  Freeze or unfreeze the encoder
*/
int vcptest_encfreeze( VCP_TEST_HDL hdl, int opargs )
{
   struct vcp_test_state *statep = hdl;
   int err;

   printk( "%s: statep=0x%x\n", __func__, (unsigned int)statep );
   if ( statep == NULL )
   {
      return -EINVAL;
   }
   if ( !statep->encvcphdl )
   {
      printk( "Fail - Encoder is not open." );
      return -EPERM;
   }

   /* Parse parameters */
   statep->encFreeze  = ( opargs > 1 ) ? op[1] : 0;

   err = vcp_set_parameter( statep->encvcphdl, VCP_PARM_ENC_FREEZE, &statep->encFreeze );
   if ( err )
   {
      printk( "Failed to set VCP_PARM_ENC_FREEZE, err=%i\n", err );
      return err;
   }

   return 0;
}

/***************************************************************************/
/**
*  Get encoder stats
*/
int vcptest_get_encstats( VCP_TEST_HDL hdl, int opargs )
{
   struct vcp_test_state *statep = hdl;
   int err;
   VCP_ENCODER_STATS stats;
   int reset;

   printk( "%s: statep=0x%x\n", __func__, (unsigned int)statep );
   if ( statep == NULL )
   {
      return -EINVAL;
   }
   if ( !statep->encvcphdl )
   {
      printk( "Fail - Encoder is not open." );
      return -EPERM;
   }

   /* Parse parameters */
   reset = ( opargs > 1 ) ? op[1] : 0;

   err = vcp_get_encoder_stats( statep->encvcphdl, reset, &stats );
   if ( err )
   {
      printk( "Failed to get VCP_ENCODER_STATS, err=%i\n", err );
      return err;
   }
   else
   {
      printk( "totalFrames = %d\n", stats.totalFrames );
      printk( "discardedFrames = %d\n", stats.discardedFrames );
      printk( "currFrameRate = %d\n", stats.currFrameRate );
      printk( "currBitRate = %d\n", stats.currBitRate );
      printk( "aveBitRate = %d\n", stats.aveBitRate );
      printk( "aveFrameRate = %d\n", stats.aveFrameRate );

      printk( "totalIntraFrames = %d\n", stats.totalIntraFrames );
      printk( "minIntraFrameSize = %d\n", stats.minIntraFrameSize );
      printk( "maxIntraFrameSize = %d\n", stats.maxIntraFrameSize );
      printk( "aveIntraFrameSize = %d\n", stats.aveIntraFrameSize );
      printk( "currIntraFrameRate = %d\n", stats.currIntraFrameRate );

      printk( "totalInterFrames = %d\n", stats.totalInterFrames );
      printk( "minInterFrameSize = %d\n", stats.minInterFrameSize );
      printk( "maxInterFrameSize = %d\n", stats.maxInterFrameSize );
      printk( "aveInterFrameSize = %d\n", stats.aveInterFrameSize );
      printk( "currInterFrameRate = %d\n", stats.currInterFrameRate );
   }
   return 0;
}

/***************************************************************************/
/**
*  Get decoder stats
*/
int vcptest_get_decstats( VCP_TEST_HDL hdl, int opargs )
{
   struct vcp_test_state *statep = hdl;
   int err;
   VCP_DECODER_STATS stats;
   int reset;

   printk( "%s: statep=0x%x\n", __func__, (unsigned int)statep );
   if ( statep == NULL )
   {
      return -EINVAL;
   }

   if ( !statep->decvcphdl )
   {
      printk( "Fail - Decoder is not open." );
      return -EPERM;
   }

   /* Parse parameters */
   reset = ( opargs > 1 ) ? op[1] : 0;

   err = vcp_get_decoder_stats( statep->decvcphdl, reset, &stats );
   if ( err )
   {
      printk( "Failed to get VCP_DECODER_STATS, err=%i\n", err );
      return err;
   }
   else
   {
      printk( "totalFrames = %d\n", stats.totalFrames );
      printk( "discardedFrames = %d\n", stats.discardedFrames );
      printk( "currFrameRate = %d\n", stats.currFrameRate );
      printk( "currBitRate = %d\n", stats.currBitRate );
      printk( "aveBitRate = %d\n", stats.aveBitRate );
      printk( "aveFrameRate = %d\n", stats.aveFrameRate );

      printk( "totalIntraFrames = %d\n", stats.totalIntraFrames );
      printk( "minIntraFrameSize = %d\n", stats.minIntraFrameSize );
      printk( "maxIntraFrameSize = %d\n", stats.maxIntraFrameSize );
      printk( "aveIntraFrameSize = %d\n", stats.aveIntraFrameSize );
      printk( "currIntraFrameRate = %d\n", stats.currIntraFrameRate );

      printk( "totalInterFrames = %d\n", stats.totalInterFrames );
      printk( "minInterFrameSize = %d\n", stats.minInterFrameSize );
      printk( "maxInterFrameSize = %d\n", stats.maxInterFrameSize );
      printk( "aveInterFrameSize = %d\n", stats.aveInterFrameSize );
      printk( "currInterFrameRate = %d\n", stats.currInterFrameRate );
   }
   return 0;
}

/***************************************************************************/
/**
*  Get decoder capabilities
*/
int vcptest_get_deccaps( VCP_TEST_HDL hdl )
{
   struct vcp_test_state *statep = hdl;
   int err;
   VCP_CAPABILITIES caps;

   printk( "%s: statep=0x%x\n", __func__, (unsigned int)statep );
   if ( statep == NULL )
   {
      return -EINVAL;
   }

   if ( !statep->decvcphdl )
   {
      printk( "Fail - Decoder is not open." );
      return -EPERM;
   }

   err = vcp_get_capabilities( statep->decvcphdl, &caps );
   if ( err )
   {
      printk( "Failed vcp_get_capabilities, err=%i\n", err );
      return err;
   }
   else
   {
      printk( "direction = %d\n", caps.direction );
      printk( "codeclist.codecs = %d\n", caps.codeclist.codecs );
      printk( "codeclist.list[0] = %d\n", caps.codeclist.list[0] );
      printk( "codeclist.list[1] = %d\n", caps.codeclist.list[1] );
      printk( "codeclist.list[2] = %d\n", caps.codeclist.list[2] );
      printk( "codeclist.list[3] = %d\n", caps.codeclist.list[3] );
      printk( "decoder_delay = %d\n", caps.decoder_delay );
   }
   return 0;
}

/***************************************************************************/
/**
*  Set encoder Full INTRA-Frame request
*/
int vcptest_set_encfir( VCP_TEST_HDL hdl )
{
   struct vcp_test_state *statep = hdl;
   int err;

   printk( "%s: statep=0x%x\n", __func__, (unsigned int)statep );
   if ( statep == NULL )
   {
      return -EINVAL;
   }
   if ( !statep->encvcphdl )
   {
      printk( "Fail - Encoder is not open." );
      return -EPERM;
   }

   err = vcp_set_parameter( statep->encvcphdl, VCP_PARM_ENC_FIR, NULL );
   if ( err )
   {
      printk( "Failed to set VCP_PARM_ENC_FIR, err=%i\n", err );
      return err;
   }
   return 0;
}

/***************************************************************************/
/**
*  Set the encoder privacy mode
*/
int vcptest_set_encpriv( VCP_TEST_HDL hdl, int opargs )
{
   struct vcp_test_state *statep = hdl;
   int err;
   int mode;        /* privacy mode */

   printk( "%s: statep=0x%x\n", __func__, (unsigned int)statep );
   if ( statep == NULL )
   {
      return -EINVAL;
   }
   if ( !statep->encvcphdl )
   {
      printk( "Fail - Encoder is not open." );
      return -EPERM;
   }

   /* Parse parameters */
   mode = ( opargs > 1 ) ? op[1] : 0;

   err = vcp_set_parameter( statep->encvcphdl, VCP_PARM_ENC_PRIVACY_MODE, &mode );
   if ( err )
   {
      printk( "Failed to set VCP_PARM_ENC_PRIVACY_MODE, err=%i\n", err );
      return err;
   }
   return 0;
}

/***************************************************************************/
/**
*  Configure the encoder pan and zoom display
*
*  @remarks
*
*/
int vcptest_set_encpanzoom( VCP_TEST_HDL hdl, int opargs )
{
   struct vcp_test_state *statep = hdl;
   VCP_PANZOOM_PARM parm;
   int err;

   printk( "%s: statep=0x%x\n", __func__, (unsigned int)statep );
   if ( statep == NULL )
   {
      return -EINVAL;
   }
   if ( !statep->encvcphdl )
   {
      printk( "Fail - Encoder is not open." );
      return -EPERM;
   }

   /* Parse parameters */
   parm.xoffset = ( opargs > 1 ) ? op[1] : 0;
   parm.yoffset = ( opargs > 2 ) ? op[2] : 0;
   parm.width   = ( opargs > 3 ) ? op[3] : 0;
   parm.height  = ( opargs > 4 ) ? op[4] : 0;

   err = vcp_set_parameter( statep->encvcphdl, VCP_PARM_ENC_PAN_ZOOM, &parm );
   if ( err )
   {
      printk( "Failed to set VCP_PARM_ENC_PAN_ZOOM, err=%i\n", err );
      return err;
   }
   return 0;
}

/***************************************************************************/
/**
*  Allocate frame buffer to store the video data.
*
*  @return  Valid pointer, otherwise NULL if out of memory
*
*  @remark  This routine may block.
*/
static VCP_FRAMEBUF *vcptest_allocFrameBuffer(
   int bytelen,                  /**< (i) Size of buffer in bytes */
   void *privdata                /**< (i) Registered private data */
)
{
   struct vcp_test_state *statep = privdata;
   int i;
   VCP_FRAMEBUF *fbp = NULL;
   struct varbuf *vbp;
   void *tmpbufp;

   if ( bytelen > MAX_PKT_SIZE )
   {
      VCPTST_LOG( "%s: bytelen=%i too large\n", __func__, bytelen );
      statep->allocfail++;
      return NULL;
   }

   vbp = &statep->bufs[0];
   down( &statep->bufslock );
   for ( i = 0; i < sizeof(statep->bufs)/sizeof(statep->bufs[0]); i++, vbp++ )
   {
      if ( !vbp->inuse )
      {
         if ( vbp->size < bytelen )
         {
            /* Resize buffer as needed */
            tmpbufp = vmalloc( bytelen );
            if ( tmpbufp == NULL )
            {
               break;
            }

            if ( vbp->bufp )
            {
               vfree( vbp->bufp );
            }

            vbp->bufp = tmpbufp;
            vbp->size = bytelen;
         }

         vbp->inuse = 1;
         fbp = vbp->bufp;
         break;
      }
   }
   up( &statep->bufslock );

   //VCPTST_LOG( "bytelen=%i fbp=0x%lx\n", bytelen, (unsigned long)fbp );

   if ( fbp )
   {
      statep->allocnum++;
   }
   else
   {
      statep->allocfail++;
   }

   return fbp;
}

/***************************************************************************/
/**
*  Enqueue encoded video data to the host. The memory was
*  allocated via the alloc callback.
*
*  @return  0 for success, otherwise -ve error code
*
*  @remark  Although this routine shouldn't block, it can. The issue
*           is that if it blocks, it holds up the encoder pipeline.
*/
static int vcptest_enqueueFrameBuffer(
   VCP_FRAMEBUF *encframep,      /**< (i) Ptr to frame buffer. */
   void *privdata                /**< (i) Registered private data */
)
{
   struct framebuf_priv *fbnodep = (void *)encframep;
   struct vcp_test_state *statep = privdata;
   struct vcp_list *listp = &statep->enclist;
#if VCPTST_LOG_SUPPORT
   char * ptr = encframep->data;
#endif

   VCPTST_LOG( "encframep=0x%lx ts %u len %d flag %d nalType 0x%x\n", (unsigned long)encframep, 
         encframep->hdr.timestamp, encframep->hdr.data_len, encframep->hdr.flags, ptr[4] );

   /* Check to ensure the frame has data.  If not then free the frame
    * memory and don't bother to process it.  We do this to
    * account for cases where the VCP_HOST_ENC_CBKS alloc function
    * was called but for some reason the frame could not be correctly
    * built.  The VCP_HOST_ENC_CBKS enqueue function called with
    * data_len=0 means the frame is incorrect and the previously
    * allocated memory must be freed */

   if( encframep->hdr.data_len == 0 )
   {
      vcptest_freeFrameBuffer( encframep, privdata );
      statep->enqueuenum++;
   }
   else
   {
      down( &listp->mutex );
      list_add_tail( &fbnodep->lnode, &listp->list );
      up( &listp->mutex );

      statep->enqueuenum++;

      /* Wake up the thread waiting for the buffer */
      up( &listp->count );
   }
   return 0;
}

/***************************************************************************/
/**
*  Free a frame buffer.
*
*  @return  Nothing
*/
static void vcptest_freeFrameBuffer(
   VCP_FRAMEBUF *framep,         /**< (i) Ptr to frame buffer to free */
   void *privdata                /**< (i) Registered private data */
)
{
   struct vcp_test_state *statep = privdata;
   struct varbuf *vbp;
   int i;

   VCPTST_LOG( "framep=0x%lx\n", (unsigned long)framep );

   vbp = &statep->bufs[0];
   down( &statep->bufslock );
   for ( i = 0; i < sizeof(statep->bufs)/sizeof(statep->bufs[0]); i++, vbp++ )
   {
      if ( vbp->bufp == framep )
      {
         vbp->inuse = 0;
         statep->freenum++;
         break;
      }
   }
   up( &statep->bufslock );
}

/***************************************************************************/
/**
*  Encoder processing thread
*
*  @return  Nothing
*/
static int vcptest_encProcessThread( void *privdata )
{
   struct vcp_test_state *statep = privdata;
   VCP_FRAMEBUF *fbp;
   struct framebuf_priv *fbnodep;
   mm_segment_t old_fs;
   //uint8_t annexB_hdr[4] = { 0, 0, 0, 1 };
   int err, freebuf;
   struct vcp_list *listp = &statep->enclist;
   int frameCount = 0;

   while ( 1 )
   {
      err = down_interruptible( &listp->count );
      if ( err < 0 || statep->enc_quitnow )
      {
         /* Quit on interruption, or if requested to quit */
         break;
      }

      freebuf = 1;

      /* Dequeue entry from list */
      down( &listp->mutex );
      if ( list_empty( &listp->list ))
      {
         VCPTST_LOG( "%s: List is empty!\n", __func__ );
         up( &listp->mutex );
         continue;
      }
      fbnodep = (void *)list_entry( listp->list.next, struct framebuf_priv, lnode );
      list_del( &fbnodep->lnode );
      up( &listp->mutex );

      frameCount++;

      fbp = (void *)fbnodep;

      //VCPTST_LOG( "fbp=0x%lx\n", (unsigned long)fbp );

      if ( statep->write_hdl )
      {
         struct file *write_hdl = (void *)statep->write_hdl;

         old_fs = get_fs();
         set_fs( get_ds() );

         if ( statep->encAddHeader )
         {
            /* Store length header for playback by decoder pipeline */
            uint32_t data_len = htonl( fbp->hdr.data_len );
            write_hdl->f_op->write( write_hdl, (void *)&data_len,
                  sizeof(data_len), &write_hdl->f_pos );
         }
#if 0
         else if ( statep->encParm.codectype == VCP_CODEC_H264 )
         {
			 // printk( "write annex B");
            write_hdl->f_op->write( write_hdl, annexB_hdr, sizeof(annexB_hdr), &write_hdl->f_pos );
         }
#endif

         write_hdl->f_op->write( write_hdl, fbp->data, fbp->hdr.data_len, &write_hdl->f_pos );

         set_fs( old_fs );
      }
      else if ( statep->loopback == 1 )
      {
         int drop_frame = 0;

         drop_frame = pkt_loss_run( &statep->pkt_loss_cfg );

         if (( !drop_frame ) && (fbp->hdr.dataType == 0)) /*give non-dropped packet and also none side information to the decoder*/
         {
            fbp->hdr.releaseTime = jiffies_to_msecs( jiffies );
            err = vcp_decode_frame( statep->decvcphdl, fbp ) ;
            if ( err )
            {
               printk( "%s: Failed to loopback data to decoder, err=%i\n", __func__, err );
            }
            else
            {
               /* buffer queued to decoder pipeline, skip free */
               freebuf = 0;
            }
         }
         else
         {
            printk("%s: dropping frame\n",__func__);
            if(fbp->hdr.dataType == 1)
            {
#if 1
               /*if we want to check the output of the estimator*/
               unsigned int firstVal;
               unsigned int secondVal;
               unsigned int thirdVal;
               unsigned int fourthVal;
               firstVal = *((unsigned int *)(fbp->data));
               secondVal = *((unsigned int *)(fbp->data+4));
               thirdVal = *((unsigned int *)(fbp->data+8));
               fourthVal = *((unsigned int *)(fbp->data+12));
               printk("we get sideinfo size:%u:%x %u %u %u\n", fbp->hdr.data_len, firstVal, secondVal, thirdVal, fourthVal);
#endif
            }
         }
      }

      if ( freebuf )
      {
         vcptest_freeFrameBuffer( fbp, statep );
      }
   }

   VCPTST_LOG( "thread quitting!  frameCount=%d\n", frameCount );

   if ( statep->write_hdl )
   {
      old_fs = get_fs();
      set_fs( get_ds() );
      VCPTST_LOG( "Closing write file\n" );
      filp_close( (void *)statep->write_hdl, current->files );
      set_fs( old_fs );
      statep->write_hdl = NULL;
   }

   complete_and_exit( &statep->enc_thread_exit, 0 );
}

/***************************************************************************/
/**
*  Decoder processing thread
*
*  @return  Nothing
*/
static int vcptest_decProcessThread( void *privdata )
{
   struct vcp_test_state *statep = privdata;
   VCP_FRAMEBUF *fbp;
   mm_segment_t old_fs;
   int seqNum = 0;
   int err;
   int drop_frame = 0;
   struct timespec start_time, end_time, elapsed_time, tot_time, delta_time;
   unsigned int clock;
   unsigned long tot_ms;

   clock = 0;

   while ( !kthread_should_stop() )
   {
      VCPTST_LOG( "frame processing with desired FPS of %i\n", statep->decFramerate );

      getnstimeofday(&start_time);

      if ( statep->read_hdl )
      {
         struct file *read_hdl = (void *)statep->read_hdl;
         uint32_t framesize = 0;

         /* The input file is expected to contain embedded framesize
          * information as stored by the encoder processing thread
          */
         old_fs = get_fs();
         set_fs( get_ds() );
         read_hdl->f_op->read( read_hdl, (void *)&framesize, sizeof(int), &read_hdl->f_pos );
         set_fs( old_fs );

         framesize = ntohl( framesize );
         if ( framesize <= 0 )
         {
            /* May have reached end of file or detected error in byte stream */
            if ( statep->decFileNonstop )
            {
               VCPTST_LOG( "Resetting file handle to play again. rc=%d\n", framesize );
               read_hdl->f_pos = 0;
               continue;
            }

            printk( "EOF or error detected, framesize=%d\n", framesize );
            break;
         }

         fbp = vcptest_allocFrameBuffer( framesize + sizeof(VCP_FRAMEBUF_HDR), statep );
         if ( fbp == NULL )
         {
            /* Failed to allocate buffer, rewind file pointer */
            printk( "%s: failed to allocate frame buffer size=%i\n", __func__, framesize );
            read_hdl->f_pos -= sizeof(int);
            goto wait_for_next_frame;
         }

         old_fs = get_fs();
         set_fs( get_ds() );
         read_hdl->f_op->read( read_hdl, fbp->data, framesize, &read_hdl->f_pos );
         set_fs( old_fs );

         /* timestamp is a don't care*/
         fbp->hdr.seq_num = seqNum++;
         fbp->hdr.timestamp = clock;
         fbp->hdr.flags = 1;
         fbp->hdr.releaseTime = jiffies_to_msecs( jiffies );
         /* data length is the one that matters */
         fbp->hdr.data_len = framesize;

         drop_frame = pkt_loss_run( &statep->pkt_loss_cfg );
         if ( !drop_frame )
         {
            err = vcp_decode_frame( statep->decvcphdl, fbp ) ;
            if ( err )
            {
               printk( "%s: Failed to send data to decoder, err=%i\n", __func__, err );
            }
         }
         else
         {
            vcptest_freeFrameBuffer( fbp, statep );
         }
      }
wait_for_next_frame:
      getnstimeofday( &end_time );

      /* cacluate elapsed time */
      elapsed_time = timespec_sub( end_time, start_time );

      /* total allowed frame period in ms */
      tot_ms = 1000 / statep->decFramerate;

      tot_time.tv_sec = tot_ms / 1000;
      tot_time.tv_nsec = ( tot_ms % 1000 ) * 1000000;

      /*
       * If elapsed time has already exceeded the allowed time, don't sleep,
       * do best effort processing
       */
      if ( timespec_compare( &elapsed_time, &tot_time ) >= 0 )
         continue;

      /* calculate the time to sleep */
      delta_time = timespec_sub( tot_time, elapsed_time );

      /* sleep for the amount of time requested */
      hrtimer_nanosleep( &delta_time, NULL, HRTIMER_MODE_REL, CLOCK_MONOTONIC );
      clock += (tot_ms * 90);
   }

   if ( statep->read_hdl )
   {
      old_fs = get_fs();
      set_fs( get_ds() );
      filp_close( (void *)statep->read_hdl, current->files );
      set_fs( old_fs );
      statep->read_hdl = NULL;
   }
   statep->decthreadpid = NULL;

   return 0;
}

/***************************************************************************/
/**
 *  Decoder pipeline event callback
 *
 *  @return  Nothing
 */
static void vcptest_decEventHandler(
   VCP_EVENT_CODE eventCode,     /**< (i) Event code */
   void *info,                   /**< (i) Ptr to structure returning info about event.
                                          NULL if no additional info provided. Refer to
                                          VCP_EVENT_CODE for more info. */
   void *privdata                /**< (i) Registered private data */
)
{
   VCP_ERR_RETCODE *rcp = info;
   VCP_ERR_DEC_CORRUPTMB *corruptmbp = info;
   struct vcp_test_state *statep = privdata;

   switch ( eventCode )
   {
      case VCP_EVENT_DEC_UNSUPPORTED:
         printk( "%s: VCP_EVENT_DEC_UNSUPPORTED rc=%i\n", __func__, rcp->internal_rc );
         break;

      case VCP_EVENT_DEC_OUT_OF_MEM:
         printk( "%s: VCP_EVENT_DEC_OUT_OF_MEM rc=%i\n", __func__,rcp->internal_rc );
         break;

      case VCP_EVENT_DEC_STRM_PROCESS_ERR:
         printk( "%s: VCP_EVENT_DEC_STRM_PROCESS_ERR rc=%i\n", __func__,rcp->internal_rc );
         break;

      case VCP_EVENT_DEC_QUEUE_ERR:
         printk( "%s: VCP_EVENT_DEC_QUEUE_ERR rc=%i\n", __func__,rcp->internal_rc );
         break;

      case VCP_EVENT_DEC_PIPELINE_FLUSH_ERR:
         printk( "%s: VCP_EVENT_DEC_PIPELINE_FLUSH_ERR rc=%i\n", __func__,rcp->internal_rc );
         break;

      case VCP_EVENT_DEC_MB_CORRUPTION:
         printk( "%s: VCP_EVENT_DEC_MB_CORRUPTION dec_type=%i width=%i height=%i idr_frame=%i totalCorruptMBs=%i\n",
               __func__,corruptmbp->dec_type, corruptmbp->frame_width, corruptmbp->frame_height,
               corruptmbp->idr_frame, corruptmbp->totalCorruptMBs );
         break;

      case VCP_EVENT_DEC_MISSING_CONFIG:
         printk( "%s: VCP_EVENT_DEC_MISSING_CONFIG\n", __func__ );
         break;

      case VCP_EVENT_DEC_FIRST_PIC_RDY:
         printk( "%s: VCP_EVENT_DEC_FIRST_PIC_RDY\n", __func__ );
         break;

      case VCP_EVENT_DEC_FRAME_LOST:
         if( !statep->encFreeze )
         {
            printk( "%s: VCP_EVENT_DEC_FRAME_LOST\n", __func__ );
         }
         break;

      case VCP_EVENT_DEC_STREAM_INFO:
         memcpy( &statep->stream_info, info, sizeof(statep->stream_info) );
         break;

      default:
         printk( "%s: Unknown event %i\n", __func__,eventCode );
   }
}

/***************************************************************************/
/**
 *  Encoder pipeline event callback
 *
 *  @return  Nothing
 */
static void vcptest_encEventHandler(
   VCP_EVENT_CODE eventCode,     /**< (i) Event code */
   void *info,                   /**< (i) Ptr to structure returning info about event.
                                          NULL if no additional info provided. Refer to
                                          VCP_EVENT_CODE for more info. */
   void *privdata                /**< (i) Registered private data */
)
{
   switch ( eventCode )
   {
      case VCP_EVENT_ENC_UNSUPPORTED:
      {
         printk( "%s: VCP_EVENT_ENC_UNSUPPORTED\n", __func__);
      }
      break;

      case VCP_EVENT_ENC_STREAM_INFO :
      {
         printk( "%s: VCP_EVENT_ENC_STREAM_INFO\n", __func__);
      }
      break;

      default:
         printk( "%s: Unknown event %i\n", __func__, eventCode );
         break;
   }
}

/***************************************************************************/
/**
*  Proc read callback function
*
*  @return  Number of characters to print
*/
static int read_proc( char *buf, char **start, off_t offset, int count,
   int *eof, void *data )
{
   struct vcp_test_state *statep = data;
   int len = 0;
   struct varbuf *vbp;
   int i;
   VCP_DEC_STREAM_INFO *stream_infop = &statep->stream_info;

   len += sprintf( buf+len, "State: %s/encvcphdl=0x%x %s/decvcphdl=0x%x enc_started=%i dec_started=%i\n",
         statep->enc_name, (unsigned int)statep->encvcphdl,
         statep->dec_name, (unsigned int)statep->decvcphdl,
         statep->enc_started, statep->dec_started );
   len += sprintf( buf+len, "Config: decFramerate=%i loopback=%i nonstop=%i encAddHeader=%i enc_saveoutput=%i\n",
         statep->decFramerate, statep->loopback, statep->decFileNonstop,
         statep->encAddHeader, statep->enc_saveoutput );
   len += sprintf( buf+len, "Stats: alloc=%i enqueue=%i free=%i allocfail=%i\n",
         statep->allocnum, statep->enqueuenum, statep->freenum, statep->allocfail );
   len += sprintf( buf+len, "Stream: dectype=%i profile=%i level=%i width=%i height=%i\n",
         stream_infop->dec_type, stream_infop->profile,
         stream_infop->level, stream_infop->frame_width, stream_infop->frame_height );
   len += sprintf( buf+len, "Buffers: count=%i\n", sizeof(statep->bufs)/sizeof(statep->bufs[0]) );
   vbp = &statep->bufs[0];
   for ( i = 0; i < sizeof(statep->bufs)/sizeof(statep->bufs[0]); i++, vbp++ )
   {
      if ( vbp->size )
      {
         len += sprintf( buf+len, " inuse=%i size=%i\n", vbp->inuse, vbp->size );
      }
   }

   *eof = 1;
   return len+1 /* NULL character */;
}

/***************************************************************************/
/**
*  Initialize debug interfaces.
*
*  @return  none
*
*  @remarks
*/
static void vcptest_debug_init( struct vcp_test_state *statep )
{
   create_proc_read_entry( statep->proc_name, 0, NULL, read_proc, statep /* data */ );
}

/***************************************************************************/
/**
*  Cleanup debug interfaces.
*
*  @return  none
*
*  @remarks
*/
static void vcptest_debug_exit( struct vcp_test_state *statep )
{
   remove_proc_entry( statep->proc_name, NULL );
}

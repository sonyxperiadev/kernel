/*****************************************************************************
* Copyright 2001 - 2011 Broadcom Corporation.  All rights reserved.
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
*  @file    vcp_testdrv.c
*
*  @brief   Implementation of the test driver front-end for VCP. It provides
*           video loopback, file IO and other test facilities.
*
*****************************************************************************/

/* ---- Include Files ---------------------------------------------------- */
#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/string.h>
#include <linux/broadcom/vcp.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>

#include "vcp_test.h"

/* ---- Public Variables ------------------------------------------------- */
int op[MAXOPS];         /* array of integer command-line parameters */
OPSTR opstr[MAXOPS];    /* array command-line strings */

/* ---- Private Constants and Types -------------------------------------- */
#define VCPTEST_CMDLINE_PROC_NAME       "vcptest_cmdline"

#define MAX_NUM_CHANS   3

#define STR_TOK               " "

typedef struct
{
   const char *usageString;
   const char *usageData;

} VCPTEST_USAGE_ENTRY;

   /* The following strings document the vcptest commands.
    * They are displayed by reading proc entries within /proc/vcptest_usage
    */
static const char usage_summary[] = \
" \nUSAGE: Write a command to file vcptest_cmdline (e.g. echo cmdName > /proc/vcptest_cmdline)\n"
"to start a test or send a VCP command, where cmdName is one of the commands listed below.\n"
"For more details on each command use 'cat /proc/vcptest_usage/<cmdName>'\n"
"e.g. cat /proc/vcptest_usage/enc_parms\n\n"
"1) enc_parms - Configure the encoder pipeline.\n"
"2) dec_parms - Configure the decoder pipeline.\n"
"3) dec - Start decoder pipeline test\n"
"4) dec - Start encoder pipeline test\n"
"5) stop_enc - Stop encoder pipeline\n"
"6) stop_dec - Stop decoder pipeline\n"
"7) disp_enc - Configure encoder (local preview) display parameters\n"
"8) disp_dec - Configure decoder display parameters\n"
"9) mask_dec - Send a display mask to the decoder\n"
"10) mask_enc - Send a display mask to the encoder\n"
"11)frz_enc - Freeze or unfreeze the encoder\n"
"12) stats_enc - Read the encoder statistics\n"
"13) stats_dec - Read the decoder statistics\n"
"14) caps_enc - Read the encoder capabilities\n"
"15) fir_enc - Set the encoder FIR parameter\n"
"16) priv_enc -  Set the encoder privacy mode\n"
"17) pan_enc - Set the encoder pan and zoom display parameters\n"
"18) pkt_loss - Configure packet loss simulation\n\n"
"vcptest can send commands to more than one encoder or decoder.  For two encoders\n"
"and decoders, use 'modprobe vcptest chans=2'.  Commands can be sent to the second\n"
"channel via 'echo cmdName > /proc/vcptest_cmdline2'\n";

static const char usage_enc_parms[] = \
"\nConfigure the encoder pipeline.  This does not start the pipeline.  Any parameters not specified will\n"
"not be changed from their previous or default values.  Note the parameters are the elements of\n"
"vcp.h structure VCP_ENCODER_PARM\n"
"SYNTAX - \n"
"  enc_parms codectype profile level framerate bitrate rateControlMode resolution maxGOPsize\n"
"            CustomMaxMBPS MaxStaticMBPS CustomMaxFS CustomMaxDPB CustomMaxBRandCPB max_rcmd_nal_unit_size\n"
"            max_nal_unit_size aspectx aspecty min_quant max_quant deblock_mode eedeEnable eedeLossRate\n"
"  e.g. enc_parms h264 h264_baseline h264_1.1 24 1m 0 vga 0.5s\n"
"  e.g. enc_parms 1 0 2 24 1000000 vga 128 (this is equivalent to the command in the line above)\n"
"  e.g. enc_parms (this command prints the parameters)\n"
"  codectype       - encoder type.  Either 'h263', 'mpeg4', 'h264', 'mjpeg'\n"
"                    or numericals values in vcp.h VCP_CODEC_TYPE\n"
"  profile         - either 'h264_baseline', 'h264_main', 'h264_high', 'h264_dummy'\n"
"                    or numericals values in vcp.h VCP_H264_PROFILE\n"
"  level           - either 'h264_1', 'h264_1b', 'h264_1.1', 'h264_1.2', 'h264_1.3', 'h264_2'\n"
"                    'h264_2.1', 'h264_2.2', 'h264_3', 'h264_3.1', 'h264_dummy'\n"
"                    or numericals values in vcp.h VCP_H264_LEVEL\n"
"  framerate       - either '5fps', '10fps', '20fps', '24fps', '30fps'\n"
"                    or numericals values in vcp.h VCP_FRAME_RATE\n"
"  bitrate         - '10k', '64k', '256k', '384k', '512k', '768k', '1m', '1.6m'\n"
"                    '2m', '3m', '4m', '6m', '8m', '10m', '12m', '14m'\n"
"                    or numericals values in vcp.h VCP_BIT_RATE\n"
"  rateControlMode - any value may be input (vcp.h specifies no limits)\n"
"  resolution      - either 'sqcif', 'qcif', 'qvga', 'cif', 'vga', 'wvga', '4cif', 'hd' or '1080p'\n"
"  maxGOPsize      - either '0.5s', '1s', '2s', '4s', '8s', '30s', 'no_iframe', 'all_iframe', 'alternate'\n"
"                    or numericals values in vcp.h VCP_GOP_SIZE\n"
"  CustomMaxMBPS   - any value may be input (vcp.h specifies no limits)\n"
"  MaxStaticMBPS   - any value may be input (vcp.h specifies no limits)\n"
"  CustomMaxFS     - any value may be input (vcp.h specifies no limits)\n"
"  CustomMaxDPB    - any value may be input (vcp.h specifies no limits)\n"
"  CustomMaxBRandCPB - any value may be input (vcp.h specifies no limits)\n"
"  max_rcmd_nal_unit_size - either '0', '500', '700', '1000', '1200' or '1500'\n"
"  max_nal_unit_size - either '0', '500', '700', '1000', '1200' or '1500'\n"
"  aspectx - any value may be input (vcp.h specifies no limits)\n"
"  aspecty - any value may be input (vcp.h specifies no limits)\n"
"  min_quant - any value may be input (vcp.h specifies no limits)\n"
"  max_quant - any value may be input (vcp.h specifies no limits)\n"
"  deblock_mode - values 0 to 2 may be input\n"
"  eedeEnable - 0 to disable\n"
"  eedeLossRate - any value may be input (vcp.h specifies no limits)\n";

static const char usage_dec_parms[] = \
"\nConfigure the decoder pipeline.  This does not start the pipeline.  Any parameters not specified will\n"
"not be changed from their previous or default values.  Note the parameters are the elements of\n"
"vcp.h structure VCP_DECODER_PARM\n"
"SYNTAX - \n"
"  dec_parms codecype maxFrameSize maxFrameWidth plcOption errConcealEnable sdpParmEnable spropParm\n"
"            CustomMaxMBPS MaxStaticMBPS CustomMaxFS CustomMaxDPB CustomMaxBRandCPB max_rcmd_nal_unit_size\n"
"            max_nal_unit_size aspectx aspecty min_quant max_quant eedeEnable eedeLossRate\n"
"  e.g. dec_parms h264 40 30 disable disable disable\n"
"  e.g. dec_parms 1 40 30 0 0 0 (this is equivalent to the command in the line above)\n"
"  e.g. dec_parms (this command prints the parameters)\n"
"  codectype        - decoder type.  Either 'h263', 'mpeg4', 'h264', 'mjpeg'\n"
"                     or numericals values in vcp.h VCP_CODEC_TYPE\n"
"  maxFrameSize     - any value may be input (vcp.h specifies no limits)\n"
"  maxFrameWidth    - any value may be input (vcp.h specifies no limits)\n"
"  plcOption        - either 'disable', 'enable', 0 or 1\n"
"  errConcealEnable - either 'disable', 'enable', 0 or 1\n"
"  sdpParmEnable    - either 'disable', 'enable', 0 or 1\n"
"  spropParm        - a string\n";

static const char usage_enc[] = \
"\nStart encoder pipeline test.  The configuration of the pipeline is either the most recent\n"
"configuration set via an 'enc_parms' command or, if that command has not occurred, the default\n"
"compiled into vcptest.\n"
"SYNTAX - enc <testMode> filename\n"
"  e.g. enc loopback\n"
"  e.g. enc save_hdr /tmp/encoder.bin\n"
"  testMode - specifies the type of test.\n"
"             'loopback' - send all encoder packets directly to the decoder.  You must issue command\n"
"                          'dec listen' before starting the encoder loopback test\n"
"             'save_raw' - save encoder frames to a file.  No headers or extra information about the\n"
"                          frames is saved to the file.\n"
"             'save_hdr' - save encoder frames to a file.  A header is added before each frame.  You\n"
"                          need this header if you wish to later play the file to the decoder.\n"
"             'nosave'   - discard encoder frames.\n"
"  filename - name of the file used when testMode is 'save_raw' or 'save_hdr'.  If a file name is not\n"
"             specified, then by default the frames are saved to file 'encode.bin'\n";

static const char usage_dec[] = \
"\nStart decoder pipeline test.  The configuration of the pipeline is either the most recent\n"
"configuration set via an 'dec_parms' command or, if that command has not occurred, the default\n"
"compiled into vcptest.\n"
"SYNTAX - dec <testMode> framerate filename\n"
"  e.g. dec listen\n"
"  e.g. dec file 30 /tmp/encoder.bin\n"
"  testMode - specifies the type of test.\n"
"             'listen'   - wait for frames to be looped from the encoder to the decoder.  The\n"
"                          'dec listen' command is followed by 'enc loopback' to start the loopback\n"
"             'file' - a file containing frames to be sent to the decoder.  Each frame must have a header\n"
"                      preceding it.  It must have been previously recorded using a 'dec save_hdr' command.\n"
"             'file_nonstop' - this is equivalent to 'dec file', except once the end of the file is reached\n"
"                              the test will begin reading frames from the start of the file again.\n"
"                              This continues indefinitely.\n"
"  framerate - specifies frames per second sent to the decoder when reading from a file.  This is ignored\n"
"               when testMode is 'listen'\n"
"  filename - name of the file used when testMode is 'file' or 'file_nonstop'.  If a file name is not\n"
"             specified, then by default the frames are saved to file 'decode.bin'\n";

static const char usage_stop_enc[] = \
"nStop encoder pipeline\n"
"SYNTAX - stop_enc\n";

static const char usage_stop_dec[] = \
"\nStop decoder pipeline\n"
"SYNTAX - stop_dec\n";

static const char usage_disp_enc[] = \
"\nConfigure encoder (local preview) display parameters\n"
"SYNTAX - disp_enc src_xoffset src_yoffset src_width src_height dst_xoffset dst_yoffset\n"
"         dst_width dst_height layer device transform\n"
"  e.g.  disp_enc 0 0 0 0 0 0 640 480 3 lcd rot0\n"
"  The parameters are as specified in vcp.h VCP_DISPLAY_PARM\n\n"
"  src_xoffset - any value may be input (vcp.h specifies no limits)\n"
"  src_yoffset - any value may be input (vcp.h specifies no limits)\n"
"  src_width - any value may be input (vcp.h specifies no limits)\n"
"  src_height - any value may be input (vcp.h specifies no limits)\n"
"  dst_xoffset - any value may be input (vcp.h specifies no limits)\n"
"  dst_yoffset - any value may be input (vcp.h specifies no limits)\n"
"  dst_width - any value may be input (vcp.h specifies no limits)\n"
"  dst_height\n - any value may be input (vcp.h specifies no limits)\n"
"  layer - any value may be input (vcp.h specifies no limits)\n"
"  device - either 'lcd', 'hdmi', or numericals values in vcp.h VCP_DISPLAY_DEV\n"
"  transform - either 'rot0', 'mirror_rot0', 'mirror_rot180', 'rot180', 'dummy',\n"
"              or numericals values in vcp.h VCP_DISPLAY_TRANSFORM\n";

static const char usage_disp_dec[] = \
"\nConfigure decoder display parameters\n"
"SYNTAX - disp_dec src_xoffset src_yoffset src_width src_height dst_xoffset dst_yoffset\n"
"         dst_width dst_height layer device transform\n"
"  e.g.  disp_dec 0 0 0 0 0 0 640 480 3 lcd rot0\n"
"  The parameters are as specified in vcp.h VCP_DISPLAY_PARM\n\n"
"  src_xoffset - any value may be input (vcp.h specifies no limits)\n"
"  src_yoffset - any value may be input (vcp.h specifies no limits)\n"
"  src_width - any value may be input (vcp.h specifies no limits)\n"
"  src_height - any value may be input (vcp.h specifies no limits)\n"
"  dst_xoffset - any value may be input (vcp.h specifies no limits)\n"
"  dst_yoffset - any value may be input (vcp.h specifies no limits)\n"
"  dst_width - any value may be input (vcp.h specifies no limits)\n"
"  dst_height\n - any value may be input (vcp.h specifies no limits)\n"
"  layer - any value may be input (vcp.h specifies no limits)\n"
"  device - either 'lcd', 'hdmi', or numericals values in vcp.h VCP_DISPLAY_DEV\n"
"  transform - either 'rot0', 'mirror_rot0', 'mirror_rot180', 'rot180', 'dummy',\n"
"              or numericals values in vcp.h VCP_DISPLAY_TRANSFORM\n";

static const char usage_disp_mask_dec[] = \
"\nSend a display mask to the decoder.\n"
"SYNTAX - mask_dec handle layer xoffset yoffset width height\n"
"  e.g.  mask_dec 1 3 0 0 320 240\n"
"  The parameters are as specified in vcp.h VCP_DISPLAY_MASK_PARM\n\n"
"  handle - a non-zero handle identifying the display mask.  Several display masks\n"
"           may be specified, each identified by a different handle\n"
"  layer - the layer of the mask.  To mask out the decoder display, 'layer' must be\n"
"          greater than the 'layer' set in the decoder display parameters.\n"
" xoffset - x coordinate of the mask in pixels\n"
" yoffset - y coordinate of the mask in pixels\n"
" width - width of the mask in pixels\n"
" height - height of the mask in pixels\n";

static const char usage_disp_mask_enc[] = \
"\nSend a display mask to the encoder.\n"
"SYNTAX - mask_enc handle layer xoffset yoffset width height\n"
"  e.g.  mask_enc 1 3 0 0 320 240\n"
"  The parameters are as specified in vcp.h VCP_DISPLAY_MASK_PARM\n\n"
"  handle - a non-zero handle identifying the display mask.  Several display masks\n"
"           may be specified, each identified by a different handle\n"
"  layer - the layer of the mask.  To mask out the decoder display, 'layer' must be\n"
"          greater than the 'layer' set in the decoder display parameters.\n"
" xoffset - x coordinate of the mask in pixels\n"
" yoffset - y coordinate of the mask in pixels\n"
" width - width of the mask in pixels\n"
" height - height of the mask in pixels\n";

static const char usage_frz_enc[] = \
"\nFreeze or unfreeze the encoder:\n"
"SYNTAX - frz_enc state.\n"
"state - 0=unfreeze, 1=freeze.  Defaults to 0 if not specified.\n";

static const char usage_stats_enc[] = \
"\nRead the encoder statistics.  Optionally reset them after reading.\n"
"SYNTAX - stats_enc state\n"
"state - 0=no reset, 1=reset.  Defaults to 0 if not specified. \n";

static const char usage_stats_dec[] = \
"\nRead the decoder statistics.  Optionally reset them after reading.\n"
"SYNTAX - stats_dec state\n"
"  state - 0=no reset, 1=reset.  Defaults to 0 if not specified.\n";

static const char usage_caps_dec[] = \
"\nRead the decoder capabilities\n"
"SYNTAX - caps_dec\n";

static const char usage_fir_enc[] = \
"\nSet the encoder FIR parameter\n"
"SYNTAX - fir_enc\n";

static const char usage_priv_enc[] = \
"\nSet the encoder privacy mode\n"
"SYNTAX - priv_enc mode\n"
"  mode - 0=disable, 1=enable.  Defaults to 0 if not specified. \n";

static const char usage_pan_enc[] = \
"\nSet the encoder pan and zoom display parameters\n"
"SYNTAX - pan_enc <xoffset> <yoffset> <width> <height>\n";

static const char usage_reinit[] = \
"\nStop, close, then open the encoder and decoder pipelines.\n"
"SYNTAX - reinit <encname> decname\n"
"  if 'decname' is not specified, then 'decname'='encname' is used\n";

static const char usage_pkt_loss[] = \
"\nConfigure packet loss simulation.  This specifies the rate of packets\n"
"intentionally discarded rather than being sent to the decoder.\n"
"SYNTAX - pkt_loss [N]  where packet loss rate = 1/N\n";


/* ---- Private Variables ------------------------------------------------ */
static VCP_TEST_HDL gVcpHandles[MAX_NUM_CHANS];

static int chans = 1;
module_param( chans, int, 0 );
MODULE_PARM_DESC( chans, "Number of VCP test channels" );

/* ---- Private Function Prototypes -------------------------------------- */

/* ---- Functions -------------------------------------------------------- */
/***************************************************************************/
/**
*  Parse the command line arguments, putting each argument into opstr[] and op[]
*
*  @return
*
*  @remarks
*/
static int parseCommandLine(
   char *cmdstr         /**< (i) command line */
)
{
   char *token;
   int opargs = 0;

   /* Reset saved command line arguments */
   memset ((void *)&op[0], 0, sizeof (int) * MAXOPS );
   memset ((void *)&opstr[0], 0, sizeof (OPSTR) * MAXOPS );

   while( (token = strsep(&cmdstr, STR_TOK)) )
   {
      if( opargs >= MAXOPS )
      {
         printk("Command line is too long to parse.  It is being truncated.\n");
         break;
      }
      /* Create 2 tables, one of command strings, and one of ascii converted to numbers. */
      strncpy(opstr[opargs].str, token, sizeof(opstr[opargs].str));
      op[opargs] = simple_strtol(token, NULL, 0);
      opargs++;
   }
   return opargs;
}

/***************************************************************************/
/**
*  Proc entry write. This runs a test or sends a VCP command.
*
*  @return
*
*  @remarks
*/
static int test_write_proc( struct file *file, const char *buffer,
      unsigned long count, void *data)
{
   char test_str[500];
   int opargs;
   VCP_TEST_HDL hdl = data;

   memset( test_str, 0, sizeof(test_str) );

   if ( count > sizeof(test_str) )
   {
      /* Limit input parsing length */
      count = sizeof(test_str);
   }

   if ( copy_from_user( &test_str, buffer, count ))
   {
      return -EFAULT;
   }
   test_str[count-1] = 0;

   opargs = parseCommandLine(test_str);

   if( opargs )
   {
      if( !strcmp( opstr[0].str, "dec_parms") )      vcptest_cfg_decparms( hdl, opargs );
      else if( !strcmp( opstr[0].str, "enc_parms") ) vcptest_cfg_encparms( hdl, opargs );
      else if( !strcmp( opstr[0].str, "enc") )       vcptest_start_enc( hdl, opargs );
      else if( !strcmp( opstr[0].str, "stop_enc") )  vcptest_stop_enc( hdl );
      else if( !strcmp( opstr[0].str, "dec") )       vcptest_start_dec( hdl, opargs );
      else if( !strcmp( opstr[0].str, "stop_dec") )  vcptest_stop_dec( hdl );
      else if( !strcmp( opstr[0].str, "disp_enc") )  vcptest_set_encdisp( hdl, opargs );
      else if( !strcmp( opstr[0].str, "disp_dec") )  vcptest_set_decdisp( hdl, opargs );
      else if( !strcmp( opstr[0].str, "mask_dec") )  vcptest_set_decdisp_mask( hdl, opargs );
      else if( !strcmp( opstr[0].str, "mask_enc") )  vcptest_set_encdisp_mask( hdl, opargs );
      else if( !strcmp( opstr[0].str, "frz_enc") )   vcptest_encfreeze( hdl, opargs );
      else if( !strcmp( opstr[0].str, "stats_enc") ) vcptest_get_encstats( hdl, opargs );
      else if( !strcmp( opstr[0].str, "stats_dec") ) vcptest_get_decstats( hdl, opargs );
      else if( !strcmp( opstr[0].str, "caps_dec") )  vcptest_get_deccaps( hdl );
      else if( !strcmp( opstr[0].str, "fir_enc") )   vcptest_set_encfir( hdl );
      else if( !strcmp( opstr[0].str, "priv_enc") )  vcptest_set_encpriv( hdl, opargs );
      else if( !strcmp( opstr[0].str, "pan_enc") )   vcptest_set_encpanzoom( hdl, opargs );
      else if( !strcmp( opstr[0].str, "reinit") )    vcptest_reinit( hdl, opargs );
      else if( !strcmp( opstr[0].str, "pkt_loss") )  vcptest_pkt_loss_cfg( hdl, opargs );
      else
      {
         printk("command not supported\n");
      }
   }
   return count;
}

/*
 * Definition of the proc entries within /proc/vcptest_usage
 */
static VCPTEST_USAGE_ENTRY usageEntries[] =
{
   {"summary", usage_summary},
   {"dec_parms", usage_dec_parms},
   {"enc_parms", usage_enc_parms},
   {"enc", usage_enc },
   {"stop_enc", usage_stop_enc },
   {"dec", usage_dec },
   {"stop_dec", usage_stop_dec },
   {"disp_enc", usage_disp_enc },
   {"disp_dec", usage_disp_dec },
   {"mask_dec", usage_disp_mask_dec },
   {"mask_enc", usage_disp_mask_enc },
   {"frz_enc", usage_frz_enc },
   {"stats_enc", usage_stats_enc },
   {"stats_dec", usage_stats_dec },
   {"caps_dec", usage_caps_dec },
   {"fir_enc", usage_fir_enc },
   {"priv_enc", usage_priv_enc },
   {"pan_enc", usage_pan_enc },
   {"reinit", usage_reinit },
   {"pkt_loss", usage_pkt_loss }
};

/***************************************************************************/
/**
*  Prints the /proc/vcptest_usage proc entries
*
*  @return
*
*  @remarks
*/
int command_usage(char *buf, char **start, off_t offset, int count, int *eof, void *data)
{
   int len = 0;

   len = sprintf( buf+len, data );
   *eof = 1;
   return len + 1 /* add null character */;
}

static struct proc_dir_entry    *gProcDir;

static int __init test_init( void )
{
   int err;
   struct proc_dir_entry *ret;
   int i;
   int usageEntryCount;
   char proc_name[30] = VCPTEST_CMDLINE_PROC_NAME;

   for ( i = 0; i < chans; i++ )
   {
      err = vcptest_init( &gVcpHandles[i] );
      if ( err )
      {
         printk( KERN_ERR "Failed to initialize test module, err=%i\n", err );
         return err;
      }

      if ( i > 0 )
      {
         /* Skip suffix for first proc file */
         sprintf( proc_name, "%s%i", VCPTEST_CMDLINE_PROC_NAME, i+1 );
      }

      ret = create_proc_entry( proc_name, 0644, NULL );
      if ( ret == NULL )
      {
         remove_proc_entry( proc_name, NULL );
         printk( KERN_ERR "%s: cannot initialize test driver\n", __func__ );
      }
      else
      {
         ret->read_proc  = NULL;
         ret->write_proc = &test_write_proc;
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,30)
         ret->owner = THIS_MODULE;
#endif
         ret->data = gVcpHandles[i];
      }
   }

   /* Create the vcptest usage proc entries.  These document vcptest commands */
   gProcDir = proc_mkdir( "vcptest_usage", NULL );

   usageEntryCount = sizeof(usageEntries)/sizeof(usageEntries[0]);

   for( i = 0; i < usageEntryCount; i ++ )
   {
      ret = create_proc_entry( usageEntries[i].usageString, 0644, gProcDir );
      if ( ret == NULL )
      {
         printk( KERN_ERR "%s: cannot create enc_parms prc entry\n", __func__ );
      }
      else
      {
         ret->read_proc  = &command_usage;
         ret->write_proc = NULL;
   #if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,30)
         ret->owner = THIS_MODULE;
   #endif
         ret->data = (void *)usageEntries[i].usageData;
      }
   }

   return 0;
}

static void __exit test_exit( void )
{
   char proc_name[30] = VCPTEST_CMDLINE_PROC_NAME;
   int err;
   int i;
   int usageEntryCount;

   for ( i = 0; i < chans; i++ )
   {
      if ( i > 0 )
      {
         /* Skip suffix for first proc file */
         sprintf( proc_name, "%s%i", VCPTEST_CMDLINE_PROC_NAME, i+1 );
      }
      remove_proc_entry( proc_name, NULL);

      err = vcptest_exit( gVcpHandles[i] );
      if ( err )
      {
         printk( KERN_ERR "Failed to cleanup test module, err=%i\n", err );
      }
   }
   /* Remove the vcptest usage proc entries */
   usageEntryCount = sizeof(usageEntries)/sizeof(usageEntries[0]);
   for( i = 0; i < usageEntryCount; i ++ )
   {
      remove_proc_entry(usageEntries[i].usageString, gProcDir);
   }
   remove_proc_entry("vcptest_usage", NULL);
}

module_init( test_init );
module_exit( test_exit );

MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("VCP Test Driver");
MODULE_LICENSE("GPL v2");

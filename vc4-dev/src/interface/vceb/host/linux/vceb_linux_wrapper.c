/*****************************************************************************
* Copyright 2008 - 2010 Broadcom Corporation.  All rights reserved.
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


#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/interrupt.h>
#include <linux/version.h>
#include <linux/string.h>
#include <linux/types.h>
#include <linux/param.h>
#include <linux/init.h>
#include <linux/poll.h>
#include <linux/delay.h>
#include <linux/sysctl.h>
#include <linux/proc_fs.h>
#include <linux/kthread.h>
#include <linux/vmalloc.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/semaphore.h>
#include <linux/file.h>
#include <linux/stat.h>

#include <asm/uaccess.h>
#include <linux/syscalls.h>
#include <asm/io.h>
#include <asm/atomic.h>

#include <linux/sched.h> // struct task_struct
#include <linux/dma-mapping.h>
#include <linux/platform_device.h>

//#include <linux/videocore/vc_boot_mode.h>

#include "vceb_linux_wrapper.h"
#include "vceb_bin_file.h"

//commands exposed by this module:

/*

/proc/vc/vceb_init [init / init_no_reset] [instance_name] [instance_params]
/proc/vc/vceb_download filename [instance_name] [vmcs_params]
/proc/vc/vceb_fb_control [enable / disable] [instance_name]/proc/vc/vceb_console_control [enable / disable] [instance_name]

*/

/******************************************************************************
Private types
******************************************************************************/

#define VCEB_LINUX_MAX_CMD_PARAMS   8

#define VCEB_MAX_INPUT_STR_LENGTH   256

typedef struct
{
    struct proc_dir_entry *vceb_dir;

} VCEB_GLOBAL_STATE_T;

struct opaque_vceb_linux_instanct_t
{
   VCEB_INSTANCE_T        vceb_inst;

   //proc entries
   struct proc_dir_entry *instance_dir;
   struct proc_dir_entry *driver;
   struct proc_dir_entry *init;
   struct proc_dir_entry *download;
   struct proc_dir_entry *fb_control;
   struct proc_dir_entry *console_control;
   struct proc_dir_entry *console_output;
   struct proc_dir_entry *stats;
   struct proc_dir_entry *bootloader_proc;
   struct proc_dir_entry *firmware_proc;

   VCEB_BIN_FILE_T  bootloader;
   VCEB_BIN_FILE_T  firmware;

   int              final_image_downloaded;

};

static int gVcebInstanceCount = 0;

/******************************************************************************
Static data
******************************************************************************/

static VCEB_GLOBAL_STATE_T *vceb_global_state = NULL;

/******************************************************************************
Static funcs
******************************************************************************/

int32_t vceb_parse_cmdline(   char *cmdline,
                              char **params,
                              const uint32_t max_params,
                              const uint32_t num_discrete_words )
{
   int num = -1;
   const char *white_space = " \t";
   char *s;

   // the string passed in from a line like
   // echo foo > /proc/xxx
   // will have a newline at the end. Replace this with a terminating nul character.

   if (( s = strchr( cmdline, '\n' )) != NULL )
   {
       *s = '\0';
   }

   if( (NULL != params) && max_params )
   {
      uint32_t count = 0;
      char *cur_ptr = cmdline;
   
      //loop over the spaces
      for( count = 0; (count < max_params) && (count <= num_discrete_words); )
      {
          if ( count > 0 )
          {
              // nul terminate the previous argument. We don't terminate it after
              // parsing so that we can exit early with num_discrete_words and have
              // the last argument include many words.

              *cur_ptr++ = '\0';
          }

          // Skip leading spaces
          cur_ptr += strspn( cur_ptr, white_space );
          params[count++] = cur_ptr;

          // Now skip non-whitespace characters
          cur_ptr += strcspn( cur_ptr, white_space );
          if ( *cur_ptr == '\0' )
          {
              break;
          }
      }
      
      //setup num
      num = count;
   }

   return num;
}

static int vceb_read_file( const char *filename,
                           void *buffer,
                           const uint32_t buffer_size,
                           uint32_t *file_size )
{
   int32_t success = -1;
   int fd = 0, nbytes = 0, flags = 0;
   mm_segment_t old_fs;
   char         overflow[16];

  //reset the output filesize
  *file_size = 0;

   //save the sys stack    
   old_fs = get_fs();
   set_fs(get_ds());

   // open the file
   flags = O_RDONLY;

   fd = sys_open( filename, flags, 0777);

   if(fd >= 0)
   {
      nbytes = sys_read(fd, buffer, buffer_size);

      //printk( KERN_ERR "nbytes = %i\n", nbytes );

      if( nbytes > 0 )
      {
         *file_size = nbytes;
         success = 0;

         if( buffer_size == nbytes )
         {
            if ( sys_read( fd, overflow, sizeof( overflow)) != 0 )
            {
                printk( KERN_ERR "Buffer overflow! - VCEB download buffer is full\n" );
                success = -1;
            }
         }
      }

      sys_close(fd);
   }
   else
   {
      printk( KERN_ERR "err = %i\n", fd );
   }

   //restore the sys stack
   set_fs(old_fs);

   return success;
}


static struct proc_dir_entry *vceb_create_proc_entry( VCEB_LINUX_INSTANCE_T linux_inst,
                                                      const char * const name,
                                                      read_proc_t *read_proc,
	                                                  write_proc_t *write_proc )
                                    
{
   struct proc_dir_entry *ret = NULL;
  
   ret = create_proc_entry( name, 0644, linux_inst->instance_dir );

   if (ret == NULL)
   {
      printk(KERN_ALERT "%s: could not create %s", __func__, name );
   }
   else
   {
      ret->data       = linux_inst;
      ret->read_proc  = read_proc;
      ret->write_proc = write_proc;
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,30)
      ret->owner 	  = THIS_MODULE;
#endif
      ret->mode 	  = S_IFREG;
      if ( read_proc != NULL )
      {
          ret->mode |= S_IRUGO;
      }
      if ( write_proc != NULL )
      {
          ret->mode |= S_IWUSR;
      }

      ret->uid 	  = 0;
      ret->gid 	  = 0;
      ret->size   = 0;
   }

   return ret;
}

static void vceb_remove_proc_entry( VCEB_LINUX_INSTANCE_T linux_inst, 
                                    const char * const name )
{
   remove_proc_entry( name, linux_inst->instance_dir );
}

/*
   Input string: echo init %1 %2..%n > /proc/vceb_init
   
      %1 is optional param for $HOST_INTERFACE_NAME$
         When $1 is defined, a specific host interface is created by this name
      %2..$n is optional param for $HOST_INTERFACE_PARAM$
         Optional parameters for the host interface to be created with
         
   Outcome is that a new /proc/vc/vceb_%1_ set of entries is created specific to this host interface
   The first instance will have symlinks created for compatability with the previous scheme
*/
static int vceb_init_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
   char *init_string = NULL;
   int init = 0;
   int noreset = 0;
//   int resume = 0;
   char *cmd_params[ VCEB_LINUX_MAX_CMD_PARAMS ];
   int num_args_parsed = 0;
   VCEB_LINUX_INSTANCE_T linux_inst = data;
   int32_t  success;

   init_string = vmalloc(VCEB_MAX_INPUT_STR_LENGTH);

   if(NULL == init_string)
      return -EFAULT;

   memset(init_string, 0, VCEB_MAX_INPUT_STR_LENGTH);

   count = (count > VCEB_MAX_INPUT_STR_LENGTH) ? VCEB_MAX_INPUT_STR_LENGTH : count;

   if(copy_from_user(init_string, buffer, count))
   {
      return -EFAULT;
   }

   //terminate the string, just incase
   init_string[ VCEB_MAX_INPUT_STR_LENGTH  - 1 ] = 0;      

   //compare the first string in the list - is it one we know about?
   if( 0 == strncmp( "init_no_reset", init_string, strlen( "init_no_reset" ) ) )
   {
      init = 1;
      noreset = 1;
   } 
   else if( 0 == strncmp( "init", init_string, strlen( "init" ) ) )
   {
      init = 1;
   }
#if 0
   else if( 0 == strncmp( "resume", init_string, strlen( "resume" ) ) )
   {
      resume = 1;
   }
#endif

   //parse the rest of the args
   //    [instance_name] [instance_params]      
   num_args_parsed = vceb_parse_cmdline(  init_string,
                                          cmd_params,
                                          VCEB_LINUX_MAX_CMD_PARAMS,
                                          1 /* we expect 2 words followed by lots of spaces */ );

   //get a ptr to this bus interface

   //make sure we want to init this
   if( init )
   {
      printk( KERN_ERR "vceb_init %s\n", (0 == noreset) ? "" : "with no reset" );

      //init the vceb library
      //this will do nothing but setup the hardware which is safe to do multiple times (or is protected at the hardware level)
      success = vceb_initialise( linux_inst->vceb_inst, noreset );
      
      //printk( KERN_ERR "vceb_initialise = %i", success );
   }
#if 0
   else if( resume )
   {
      success = vceb_resume_videocore( linux_inst->vceb_inst );
      
      printk( KERN_ERR "vceb_resume_videocore = %i", success );
   }
#endif
   else
   {
      printk( KERN_ERR "Unknown command" );
   }

   vfree(init_string);

   (void)success; // In case the printk's are commented out.

   return count;
}

static int vceb_driver_read(char *buf, char **start, off_t offset, int count, int *eof, void *data)
{
   VCEB_LINUX_INSTANCE_T linux_inst = data;
   VCEB_HOST_INTERFACE_INSTANCE_T   host_interface_instance;
   char *p = buf;

   vceb_get_host_interface_instance( linux_inst->vceb_inst, &host_interface_instance );

   if (( host_interface_instance == NULL )
   ||  ( host_interface_instance->funcs->driver_name == NULL ))
   {
       p += sprintf( p, "none\n" );
   }
   else
   {
       p += sprintf( p, "%s\n", host_interface_instance->funcs->driver_name );
   }
   
   *eof = 1;
   return p - buf;
}

static int vceb_download_read(char *buf, char **start, off_t offset, int count, int *eof, void *data)
{
   VCEB_LINUX_INSTANCE_T linux_inst = data;
   VCEB_HOST_INTERFACE_INSTANCE_T   host_interface_instance;
   char *p = buf;

   p += sprintf( p, "loaded=%d\n", linux_inst->final_image_downloaded );

   vceb_get_host_interface_instance( linux_inst->vceb_inst, &host_interface_instance );

   if (( host_interface_instance != NULL ) && ( host_interface_instance->funcs->downloadStatus != NULL ))
   {
       p += host_interface_instance->funcs->downloadStatus( host_interface_instance, p, count - ( p - buf ));
   }

   *eof = 1;
   return p - buf;
}

static int vceb_download_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
   char *download_str = NULL;
   int32_t success = -1;
   char *cmd_params[ VCEB_LINUX_MAX_CMD_PARAMS ];
   int num_args_parsed = 0;
   const char *passed_params = "";
   const char *platform_params = "";
   char *params = NULL;
   size_t params_len = 0;

   VCEB_LINUX_INSTANCE_T            linux_inst = data;
   VCEB_HOST_INTERFACE_INSTANCE_T   host_interface_instance;

   linux_inst->final_image_downloaded = 0;

   download_str = vmalloc(VCEB_MAX_INPUT_STR_LENGTH);

   if(NULL == download_str)
      return -EFAULT;

   memset(download_str, 0, VCEB_MAX_INPUT_STR_LENGTH);

   count = (count > VCEB_MAX_INPUT_STR_LENGTH) ? VCEB_MAX_INPUT_STR_LENGTH : count;

   if(copy_from_user(download_str, buffer, count))
   {
      return -EFAULT;
   }

   //terminate the string, just incase
   download_str[ VCEB_MAX_INPUT_STR_LENGTH - 1 ] = 0;

   //parse the rest of the args
   //    [vmcs_params]
   num_args_parsed = vceb_parse_cmdline(  download_str,
                                          cmd_params,
                                          VCEB_LINUX_MAX_CMD_PARAMS,
                                          1 /* we expect 1 words followed by lots of spaces */ );

   if( num_args_parsed == 2 )
   {
      passed_params = cmd_params[1];
      params_len = strlen( passed_params );
   }


   platform_params = vceb_get_videocore_command_line( linux_inst->vceb_inst );
   if ( platform_params == NULL )
   {
       platform_params = "";
   }
   else
   {
       params_len += strlen( platform_params );
   }
   params_len += 2; // +1 for space, +1 for string terminator

   params = kmalloc( params_len, GFP_KERNEL );
   *params = '\0';

   if ( *platform_params != '\0' )
   {
       strlcat( params, platform_params, params_len );
   }
   if ( *passed_params != '\0' )
   {
       if ( *params != '\0' )
       {
           strlcat( params, " ", params_len );
       }
       strlcat( params, passed_params, params_len );
   }

   //if there is anyhting in the string, we try and open the file and then download it
   if( (num_args_parsed >= 1) && strlen( cmd_params[0] ) )
   {
      void *file_buffer = NULL;
      uint32_t file_size = 0;
      const uint32_t max_file_size = 3072 * 1024;

      if(*(cmd_params[0]) == '\n')
         *(cmd_params[0]) = 0;

      // malloc the buffer
      file_buffer = vmalloc( max_file_size );

      if( NULL != file_buffer )
      {
         printk( KERN_INFO "VCEB: Loading image '%s' onto %s\n",
                 cmd_params[0], vceb_get_instance_name( linux_inst->vceb_inst ));

         //read the file
         vceb_read_file( cmd_params[0], file_buffer, max_file_size, &file_size );

         if( file_size )
         {
            if (( file_size & 31 ) != 0 )
            {
                // The command line is aligned to a 32-byte boundary, so make sure
                // that the portion before the command line is padded out.

                memset( (char *)file_buffer + file_size, 0, 32 - ( file_size & 31 ));
                file_size = ( file_size + 31 ) & ~31;
            }

            {
                char *file_cmd_line = (char *)file_buffer + file_size - 1024;

                // Check to see if the .cmd_line section is present

                if ( memcmp( file_cmd_line, "\0CommandLine", 13 ) == 0 )
                {
                    strlcpy( file_cmd_line, params, 1024 );

                    printk( KERN_ERR "VCEB Command Line: '%s'\n", file_cmd_line );
                }
            }

            vceb_get_host_interface_instance( linux_inst->vceb_inst, &host_interface_instance );
            success = vceb_exit_and_load_final_image( linux_inst->vceb_inst, file_buffer, file_size );
                
            printk( KERN_ERR "VCEB download %s\n", (0 == success) ? "OK" : "FAILED" );
    
            linux_inst->final_image_downloaded = ( success == 0 );
         }
         else
         {
            printk( KERN_ERR "File '%s' can't be found (or is empty)\n", download_str );
         }

         //free the memory allocated
         vfree( file_buffer );
      }
      else
      {
         printk( KERN_ERR "VCEB Download - unable to malloc %i\n", max_file_size );
      }
   }

   if ( params != NULL )
   {
       kfree( params );
   }
   vfree(download_str);

   // We no longer need vceb.bin in memory either, so release its memory
   // We may still need the 2nd stage bootloader for suspend/resume

   vceb_free_bin_file_data( &linux_inst->firmware );

   return count;
}

static int vceb_fb_control_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
   char *fb_control_str = NULL;
   int32_t success = -1;
   VCEB_LINUX_INSTANCE_T linux_inst = data;

   fb_control_str = vmalloc(VCEB_MAX_INPUT_STR_LENGTH);

   if(NULL == fb_control_str)
      return -EFAULT;

   memset(fb_control_str, 0, VCEB_MAX_INPUT_STR_LENGTH);

   count = (count > VCEB_MAX_INPUT_STR_LENGTH) ? VCEB_MAX_INPUT_STR_LENGTH : count;

   if(copy_from_user(fb_control_str, buffer, count))
   {
      return -EFAULT;
   }

   //terminate the string, just incase
   fb_control_str[ VCEB_MAX_INPUT_STR_LENGTH - 1 ] = 0;

   //does it say enable or disable?
   if( 0 == strncasecmp( fb_control_str, "enable", strlen( "enable" ) ) )
   {      
      success = vceb_framebuffer_overlay_enable( linux_inst->vceb_inst, 1 );
      printk( KERN_ERR "Enabling VCEB framebuffer %s\n", (0 == success) ? "OK" : "BAD" );
   }
   else if( 0 == strncasecmp( fb_control_str, "disable", strlen( "disable" ) ) )
   {      
      success = vceb_framebuffer_overlay_enable( linux_inst->vceb_inst, 0 );
      printk( KERN_ERR "Disabling VCEB framebuffer %s\n", (0 == success) ? "OK" : "BAD" );
   }
   else
   {
      printk( KERN_ERR "vceb_fb_control - %s is bad input\n", fb_control_str );
   }
   vfree(fb_control_str);

   return count;
}

static int vceb_console_control_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
   char *console_control_str = NULL;
   VCEB_LINUX_INSTANCE_T linux_inst = data;

   console_control_str = vmalloc(VCEB_MAX_INPUT_STR_LENGTH);

   if(NULL == console_control_str)
      return -EFAULT;

   memset(console_control_str, 0, VCEB_MAX_INPUT_STR_LENGTH);

   count = (count > VCEB_MAX_INPUT_STR_LENGTH) ? VCEB_MAX_INPUT_STR_LENGTH : count;

   if(copy_from_user(console_control_str, buffer, count))
   {
      return -EFAULT;
   }

   //terminate the string, just incase
   console_control_str[ VCEB_MAX_INPUT_STR_LENGTH - 1 ] = 0;

   //does it say enable or disable?
   if( 0 == strncasecmp( console_control_str, "enable", strlen( "enable" )) )
   {
      vceb_console_enable( linux_inst->vceb_inst, 1 );
   }
   else if( 0 == strncasecmp( console_control_str, "disable", strlen( "disable" )) )
   {
      vceb_console_enable( linux_inst->vceb_inst, 0 );
   }

   vfree(console_control_str);

   return count;
}

static int vceb_console_control_output(struct file *file, const char *buffer, unsigned long count, void *data)
{
   char *console_output = NULL;
   unsigned long bytesRemaining = count;
   VCEB_LINUX_INSTANCE_T linux_inst = data;

   console_output = vmalloc(VCEB_MAX_INPUT_STR_LENGTH);

   if(NULL == console_output)
      return -EFAULT;

   printk( "%s: buffer = '%s', count = %lu\n", __func__, buffer, count );

   while( bytesRemaining )
   {
      uint32_t copy_this_time = 0;
      uint32_t send_count = 0;

      memset(console_output, 0, VCEB_MAX_INPUT_STR_LENGTH);

      copy_this_time = (bytesRemaining > VCEB_MAX_INPUT_STR_LENGTH) ? VCEB_MAX_INPUT_STR_LENGTH : bytesRemaining;

      printk( "%s: count = %lu copy_this_time = %d\n", __func__, bytesRemaining, copy_this_time );

      if(copy_from_user(console_output, buffer, copy_this_time ))
      {
         return -EFAULT;
      }

      for( send_count = 0; send_count < copy_this_time; send_count += 8 )
      {
         vceb_console_putstring( linux_inst->vceb_inst, &console_output[ send_count ] );
      }

      bytesRemaining -= copy_this_time;
      buffer += copy_this_time;
   }

   vfree(console_output);

   return count;
}


static int vceb_stats(struct file *file, const char *buffer, unsigned long count, void *data)
{
   char *stat = NULL;
   char output_stats[ 256 ];
   char *stat_walk = NULL;
   VCEB_LINUX_INSTANCE_T linux_inst = data;

   stat = vmalloc(VCEB_MAX_INPUT_STR_LENGTH);

   if(NULL == stat)
      return -EFAULT;

   memset(stat, 0, VCEB_MAX_INPUT_STR_LENGTH);

   count = (count > VCEB_MAX_INPUT_STR_LENGTH) ? VCEB_MAX_INPUT_STR_LENGTH : count;

   if(copy_from_user(stat, buffer, count))
   {
      return -EFAULT;
   }

   //terminate the string, just incase
   stat[ VCEB_MAX_INPUT_STR_LENGTH - 1 ] = 0;

   //first, prune the string to remove any trailing characters (i.e. carriage return)
   stat_walk = stat;

   while( *stat_walk != 0 )
   {
      if( *stat_walk < 32 )
      {
         *stat_walk = 0;
         break;
      }
      stat_walk++;
   }

   //get the stats using this string
   if( 0 == vceb_get_stat( linux_inst->vceb_inst, stat, output_stats, sizeof( output_stats ) ) )
   {
      //print out the buffer
      //TODO - use the read function to get this instead of dumping using printk
      printk( KERN_ERR "%s\n", output_stats );
   }
   else
   {
      printk( KERN_ERR "vceb: failed to find stat %s\n", stat );
   }

   vfree(stat);

   return count;
}

static int vceb_bin_file_read(char *buf, char **start, off_t offset, int count, int *eof, void *data)
{
    VCEB_BIN_FILE_T *binFile = data;
    char            *s = buf;

    if (offset > 0)
    {
       *eof = 1;
       return 0;
    }

    s += sprintf( s, "Filename: %s\n", binFile->filename );
    s += sprintf( s, "Type: %s\n", binFile->type ?: "none" );
    s += sprintf( s, "Size (in memory): %d bytes\n", binFile->len );

    *eof = 1;
    return s - buf;
}

static int vceb_bin_file_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
    VCEB_BIN_FILE_T *binFile = data;
    size_t           nameLen;
    char            *s;
    int              rc;
    uint32_t         actual_size;

    vceb_free_bin_file( binFile );

    nameLen = count;
    if ( nameLen > VCEB_MAX_BIN_FILENAME_LEN )
    {
        nameLen = VCEB_MAX_BIN_FILENAME_LEN;
    }

    if ( copy_from_user( binFile->filename, buffer, nameLen ))
    {
        return -EFAULT;
    }
    binFile->filename[ nameLen - 1 ] = '\0';

    // echo leaves a trailing newline - remove it
    if (( s = strchr( binFile->filename, '\n' )) != NULL )
    {
        *s = '\0';
    }

    {
        struct kstat    st;
        mm_segment_t    old_fs = get_fs();
        set_fs( get_ds() );
        rc = vfs_stat( binFile->filename, &st );
        set_fs( old_fs );

        if ( rc != 0 )
        {
            return rc;
        }

        if ( vceb_alloc_bin_file_data( binFile, st.size ) != 0 )
        {
            return -ENOMEM;
        }
    }

    if ( vceb_read_file( binFile->filename, binFile->data, binFile->len, &actual_size ) != 0 )
    {
        printk( KERN_ERR "Error reading '%s'\n", binFile->filename );
        vceb_free_bin_file( binFile );
        return -EIO;
    }

    vceb_register_bin_file( binFile );

    return count;
}

/******************************************************************************
Global functions.
******************************************************************************/

int32_t vceb_linux_create_instance( const VCEB_HOST_INTERFACE_CREATE_PARAMS_T *createParams,
                                    VCEB_HOST_INTERFACE_INSTANCE_T host_interface_instance,
                                    VCEB_INSTANCE_T *vceb_inst_out,
                                    VCEB_LINUX_INSTANCE_T *linux_inst_out )
{
    VCEB_LINUX_INSTANCE_T   linux_inst;

    *linux_inst_out = NULL;
    *vceb_inst_out = NULL;

    if (( linux_inst = kzalloc( sizeof( *linux_inst ), GFP_KERNEL )) == NULL )
    {
        printk( KERN_ERR "%s: Unable to allocate %d bytes for instance state\n",
                __func__, sizeof( *linux_inst ));
        return -ENOMEM;
    }

    // We've got a device and a driver. Create a vceb instance.

    if ( vceb_create_instance( createParams, host_interface_instance, &linux_inst->vceb_inst ) != 0 )
    {
        printk( KERN_ERR "%s: Unable to create vceb instance '%s'\n", 
                __func__, createParams->instance_name );

        kfree( linux_inst );
        return -ENODEV;
    }

    gVcebInstanceCount++;

    if ( vceb_global_state->vceb_dir == NULL )
    {
        vceb_global_state->vceb_dir = proc_mkdir( "vceb", NULL );
    }
    if ( createParams->instance_name != '\0' )
    {
        linux_inst->instance_dir = proc_mkdir( createParams->instance_name, vceb_global_state->vceb_dir );
    }
    else
    {
        linux_inst->instance_dir = vceb_global_state->vceb_dir;
    }

    linux_inst->init            = vceb_create_proc_entry( linux_inst, "vceb_init", NULL, vceb_init_write );
    linux_inst->download        = vceb_create_proc_entry( linux_inst, "vceb_download", vceb_download_read, vceb_download_write );
    linux_inst->driver          = vceb_create_proc_entry( linux_inst, "vceb_driver", vceb_driver_read, NULL );
    linux_inst->fb_control      = vceb_create_proc_entry( linux_inst, "vceb_fb_control", NULL, vceb_fb_control_write );
    linux_inst->console_control = vceb_create_proc_entry( linux_inst, "vceb_console_control", NULL, vceb_console_control_write );  
    linux_inst->console_output  = vceb_create_proc_entry( linux_inst, "vceb_console_output", NULL, vceb_console_control_output );
    linux_inst->stats           = vceb_create_proc_entry( linux_inst, "vceb_statistics", NULL, vceb_stats );
    linux_inst->bootloader_proc = vceb_create_proc_entry( linux_inst, "vceb_bootloader", vceb_bin_file_read, vceb_bin_file_write );
    linux_inst->firmware_proc   = vceb_create_proc_entry( linux_inst, "vceb_firmware", vceb_bin_file_read, vceb_bin_file_write );

    linux_inst->bootloader.instance_name = createParams->instance_name;
    linux_inst->bootloader.type = "2nd-stage";
    linux_inst->firmware.instance_name = createParams->instance_name;
    linux_inst->firmware.type = "vceb";

    if ( linux_inst->bootloader_proc != NULL )
    {
        linux_inst->bootloader_proc->data = &linux_inst->bootloader;
    }
    if ( linux_inst->firmware_proc != NULL )
    {
        linux_inst->firmware_proc->data = &linux_inst->firmware;
    }
    linux_inst->final_image_downloaded = 0;

    *linux_inst_out = linux_inst;
    *vceb_inst_out = linux_inst->vceb_inst;

    return 0;
}

void vceb_linux_delete_instance( VCEB_LINUX_INSTANCE_T linux_inst )
{
    vceb_remove_proc_entry( linux_inst, "vceb_init" );
    vceb_remove_proc_entry( linux_inst, "vceb_driver" );
    vceb_remove_proc_entry( linux_inst, "vceb_download" );
    vceb_remove_proc_entry( linux_inst, "vceb_fb_control" );
    vceb_remove_proc_entry( linux_inst, "vceb_console_control" );
    vceb_remove_proc_entry( linux_inst, "vceb_console_output" );
    vceb_remove_proc_entry( linux_inst, "vceb_statistics" );
    vceb_remove_proc_entry( linux_inst, "vceb_bootmode" );
    vceb_remove_proc_entry( linux_inst, "vceb_bootloader" );
    vceb_remove_proc_entry( linux_inst, "vceb_firmware" );

    if (( linux_inst->instance_dir != NULL )
    &&  ( linux_inst->instance_dir != vceb_global_state->vceb_dir ))
    {
        // Remove /proc/vceb/xxx 

        remove_proc_entry( linux_inst->instance_dir->name, vceb_global_state->vceb_dir );
    }

    gVcebInstanceCount--;
    if ( gVcebInstanceCount == 0 )
    {
        remove_proc_entry( "vceb", NULL );
    }

    vceb_free_bin_file( &linux_inst->bootloader );
    vceb_free_bin_file( &linux_inst->firmware );

    kfree( linux_inst );
}

static int __init vceb_linux_wrapper_init_drv( void )
{
#if 0
    if ( vc_boot_mode_skip() )
    {
        return -ENODEV;
    }
#endif

    vceb_global_state = kzalloc( sizeof( *vceb_global_state ), GFP_KERNEL );
    if ( vceb_global_state == NULL )
    {
        return -ENOMEM;
    }
    return 0;
}

static void __exit vceb_linux_wrapper_exit_drv( void )
{
    if ( vceb_global_state != NULL )
    {
        kfree( vceb_global_state );
    }
}
EXPORT_SYMBOL( vceb_initialise );
EXPORT_SYMBOL( vceb_get_instance );
EXPORT_SYMBOL( vceb_create_instance );
//EXPORT_SYMBOL( vceb_get_first_instance );
//EXPORT_SYMBOL( vceb_resume_videocore );
EXPORT_SYMBOL( vceb_control_run_pin );
EXPORT_SYMBOL( vceb_framebuffer_overlay_enable );
EXPORT_SYMBOL( vceb_framebuffer_overlay_set );
EXPORT_SYMBOL( vceb_linux_create_instance );
EXPORT_SYMBOL( vceb_linux_delete_instance );

// We want the vceb stuff to get called before this code. So we need to ensure that
// this code uses an initcall which comes before the one in vchiq_kernel.c

//arch_initcall(vceb_linux_wrapper_init_drv);
module_init(vceb_linux_wrapper_init_drv);
module_exit(vceb_linux_wrapper_exit_drv);

MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("VCEB Driver");
MODULE_LICENSE("GPL");

/* ************************************ The End ***************************************** */


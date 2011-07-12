/*
 *  linux/arch/arm/mach-bcm2708/vchiq_arm.c
 *
 *  Copyright (C) 2010 Broadcom
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/cdev.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/pagemap.h>
#include <linux/slab.h>
#include <asm/pgtable.h>
#include <asm/io.h>
#include <asm/uaccess.h>

#include <linux/vchiq_platform_data.h>

#include "vchiq_core.h"
#include "vchiq_ioctl.h"

#include "vchiq_if.h"
#include "vchiq_memdrv.h"

#define DEVICE_NAME "vchiq"

/* Override the default prefix, which would be vchiq_arm (from the filename) */
#undef MODULE_PARAM_PREFIX
#define MODULE_PARAM_PREFIX DEVICE_NAME "."

#define VCHIQ_MINOR 0

/* Some per-instance constants */
#define MAX_COMPLETIONS 16
#define MAX_SERVICES VCHIQ_MAX_SERVICES
#define MAX_ELEMENTS 8
#define MAX_FRAGMENTS (VCHIQ_NUM_CURRENT_BULKS * 2)

#define CACHE_LINE_SIZE 32
#define PAGELIST_WRITE 0
#define PAGELIST_READ 1
#define PAGELIST_READ_WITH_FRAGMENTS 2

#define MIN(a,b) ((a)<(b)?(a):(b))

VCOS_LOG_CAT_T vchiq_arm_log_category;
#define  VCOS_LOG_CATEGORY (&vchiq_arm_log_category)

typedef struct
{
    unsigned int                 instNum;

    const char                  *instance_name;
    const VCHIQ_PLATFORM_DATA_T *platform_data;

    VCOS_CFG_ENTRY_T             instance_cfg_dir;
    VCOS_CFG_ENTRY_T             vchiq_control_cfg_entry;
} VCHIQ_KERNEL_STATE_T;


#define VCHIQ_NUM_VIDEOCORES 1

static VCHIQ_KERNEL_STATE_T    *vchiq_kernel_state[ VCHIQ_NUM_VIDEOCORES ];
static unsigned int             vchiq_num_instances = 0;

/* Global 'vc' proc entry used by all instances */
VCOS_CFG_ENTRY_T         vc_cfg_dir;

struct vchiq_user_file_data_s;
typedef struct vchiq_user_file_data_s VCHIQ_USER_FILE_DATA_T;

typedef struct client_service_struct {
    VCHIQ_SERVICE_HANDLE_T  service;
	void *userdata;
	VCHIQ_USER_FILE_DATA_T *fileData;
	int handle;
} USER_SERVICE_T;

struct vchiq_user_file_data_s {
	VCHIQ_COMPLETION_DATA_T completions[MAX_COMPLETIONS];
	volatile int completion_insert;
	volatile int completion_remove;
	VCOS_EVENT_T insert_event;
	VCOS_EVENT_T remove_event;

	USER_SERVICE_T services[MAX_SERVICES];

    void       *mmap_kernel_start;  /* memory mapped pointer in kernel space */
    void       *mmap_kernel_end;    /* mmap_kernel_start + mmap_size */
	void       *mmap_user_start;    /* memory mapped pointer in user space */
    void       *mmap_user_end;      /* mmap_user_start + mmap_size */
    uint32_t    mmap_size;          /* size of mmap area */
	int closing;

    VCHIQ_KERNEL_STATE_T   *kernState;
    VCHIQ_INSTANCE_T        vchiq_instance;
};

static struct cdev    vchiq_cdev;
static dev_t          vchiq_devid;
static struct class  *vchiq_class;
static struct device *vchiq_dev;

static char *log_level = "warn";

module_param(log_level, charp, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IROTH);
MODULE_PARM_DESC(log_level, "The logging level - error, warn, trace, info or never");

/****************************************************************************
*
*   find_service_by_handle
*
***************************************************************************/

static inline USER_SERVICE_T *find_service_by_handle(
    VCHIQ_USER_FILE_DATA_T *fileData,
    int handle )
{
    USER_SERVICE_T *user_service;

    if (( handle >= 0 )
    &&  ( handle < MAX_SERVICES ))
    {
        user_service = &fileData->services[ handle ];

        if ( user_service->service != NULL )
        {
            return user_service;
        }
    }

    return NULL;
}

/****************************************************************************
*
*   find_avail_service_handle
*
***************************************************************************/

static inline USER_SERVICE_T *find_avail_service_handle(
    VCHIQ_USER_FILE_DATA_T *fileData)
{
    int handle;

    for ( handle = 0; handle < MAX_SERVICES; handle++ )
    {
        if ( fileData->services[handle].service == NULL )
        {
            fileData->services[handle].fileData = fileData;
            fileData->services[handle].handle = handle;

            return &fileData->services[handle];
        }
    }
    return NULL;
}

/****************************************************************************
*
*   user_ptr_to_kernel
*
*   The usermode process mmaps in the message buffer so that we don't have
*   to copy the data. We need to validate the user pointer and convert it
*   into the equivalent kernel pointer.
*
***************************************************************************/

static inline void *user_ptr_to_kernel(
    VCHIQ_USER_FILE_DATA_T *fileData,
    void *user_ptr)
{
    if (( user_ptr >= fileData->mmap_user_start )
    &&  ( user_ptr <= fileData->mmap_user_end ))
    {
        return fileData->mmap_kernel_start
             + ((uint8_t *)user_ptr - (uint8_t *)fileData->mmap_user_start);
    }
    return NULL;
}


/****************************************************************************
*
*   kernel_ptr_to_user
*
*   The usermode process mmaps in the message buffer so that we don't have
*   to copy the data. We need to validate the kernel pointer and convert it
*   into the equivalent user pointer.
*
***************************************************************************/

static inline void *kernel_ptr_to_user(
    VCHIQ_USER_FILE_DATA_T *fileData,
    void *kernel_ptr)
{
    if (( kernel_ptr >= fileData->mmap_kernel_start )
    &&  ( kernel_ptr <= fileData->mmap_kernel_end ))
    {
        return fileData->mmap_user_start
             + ((uint8_t *)kernel_ptr - (uint8_t *)fileData->mmap_kernel_start);
    }
    return NULL;
}

/****************************************************************************
*
*   vchiq_set_log_level
*
***************************************************************************/

static void vchiq_set_log_level(const char *buf)
{
	vcos_log_trace("vchiq_set_log_level - %s", buf);
	if (strcmp(buf,"error") == 0)
		vcos_log_set_level(VCOS_LOG_CATEGORY, VCOS_LOG_ERROR);
	else if (strcmp(buf,"warn") == 0)
		vcos_log_set_level(VCOS_LOG_CATEGORY, VCOS_LOG_WARN);
	else if (strcmp(buf,"warning") == 0)
		vcos_log_set_level(VCOS_LOG_CATEGORY, VCOS_LOG_WARN);
	else if (strcmp(buf,"info") == 0)
		vcos_log_set_level(VCOS_LOG_CATEGORY, VCOS_LOG_INFO);
	else if (strcmp(buf,"trace") == 0)
		vcos_log_set_level(VCOS_LOG_CATEGORY, VCOS_LOG_TRACE);
	else if (strcmp(buf,"never") == 0)
		vcos_log_set_level(VCOS_LOG_CATEGORY, VCOS_LOG_NEVER);
	else
	{
		vcos_log_set_level(VCOS_LOG_CATEGORY, VCOS_LOG_ERROR);
		vcos_log_error("Invalid trace level '%s'", buf);
	}
}

/****************************************************************************
*
*   vchiq_control_cfg_parse
*
***************************************************************************/

static void vchiq_control_cfg_parse( VCOS_CFG_BUF_T buf, void *data )
{
    VCHIQ_KERNEL_STATE_T    *kernState = data;
    char                    *command;

    command = vcos_cfg_buf_get_str( buf );

    if ( strncmp( "connect", command, strlen( "connect" )) == 0 )
    {
        if ( vchiq_memdrv_initialise() != VCHIQ_SUCCESS )
        {
            printk( KERN_ERR "%s: failed to initialize vchiq for '%s'\n",
                    __func__, kernState->instance_name );
        }
        else
        {
            printk( KERN_INFO "%s: initialized vchiq for '%s'\n", __func__,
                    kernState->instance_name );
        }
    }
    //TODO support "disconnect", "resume", "suspend"
    else
    {
        printk( KERN_ERR "%s: unknown command '%s'\n", __func__, command );
    }
}

/****************************************************************************
*
* vchiq_userdrv_create_instance
*
*   The lower level drivers (vchiq_memdrv or vchiq_busdrv) will call this
*   function for each videcoreo that exists. We then register a character
*   driver which is what userspace uses to talk to us.
*
***************************************************************************/

VCHIQ_STATUS_T vchiq_userdrv_create_instance( const VCHIQ_PLATFORM_DATA_T *platform_data )
{
    VCHIQ_KERNEL_STATE_T   *kernState;

    printk( "%s: vchiq_num_instances = %d, VCHIQ_NUM_VIDEOCORES = %d\n",
             __func__, vchiq_num_instances, VCHIQ_NUM_VIDEOCORES );

    if ( vchiq_num_instances >= VCHIQ_NUM_VIDEOCORES )
    {
        printk( KERN_ERR "%s: already created %d instances\n", __func__,
                VCHIQ_NUM_VIDEOCORES );

        return VCHIQ_ERROR;
    }

    /* Allocate some memory */
    kernState = kmalloc( sizeof( *kernState ), GFP_KERNEL );
    if ( kernState == NULL )
    {
        printk( KERN_ERR "%s: failed to allocate memory\n", __func__ );

        return VCHIQ_ERROR;
    }

    memset( kernState, 0, sizeof( *kernState ));

    vchiq_kernel_state[ vchiq_num_instances ] = kernState;

    /* Do some bookkeeping */
    kernState->instNum = vchiq_num_instances++;
    kernState->instance_name = platform_data->instance_name;
    kernState->platform_data = platform_data;

    /* Create proc entries */
    if ( !vcos_cfg_is_entry_created( vc_cfg_dir ))
    {
        if ( vcos_cfg_mkdir( &vc_cfg_dir, NULL, "vc" ) != VCOS_SUCCESS )
        {
            printk( KERN_ERR "%s: failed to create proc entry", __func__ );

            return VCHIQ_ERROR;
        }
    }

    if ( vcos_cfg_mkdir( &kernState->instance_cfg_dir, &vc_cfg_dir,
                         kernState->instance_name ) != VCOS_SUCCESS )
    {
        printk( KERN_ERR "%s: failed to create proc entry", __func__ );

        return VCHIQ_ERROR;
    }

    if ( vcos_cfg_create_entry( &kernState->vchiq_control_cfg_entry,
                                &kernState->instance_cfg_dir,
                                "vchiq_control",
                                NULL,
                                vchiq_control_cfg_parse,
                                kernState ) != VCOS_SUCCESS )
    {
        printk( KERN_ERR "%s: failed to create proc entry", __func__ );

        return VCHIQ_ERROR;
    }

    /* Direct connect the vchiq to get vmcs-fb and vmcs-sm device module built in */
    if ( vchiq_memdrv_initialise() != VCHIQ_SUCCESS )
    {
        printk( KERN_ERR "%s: failed to initialize vchiq for '%s'\n",
                    __func__, kernState->instance_name );
    }
    else
    {
        printk( KERN_INFO "%s: initialized vchiq for '%s'\n", __func__,
                    kernState->instance_name );
    }	
	
    printk( KERN_INFO "%s: successfully initialized '%s' videocore\n",
            __func__, kernState->instance_name );

    return VCHIQ_SUCCESS;
}

EXPORT_SYMBOL( vchiq_userdrv_create_instance );

/****************************************************************************
*
*   service_callback
*
***************************************************************************/

VCHIQ_STATUS_T service_callback(VCHIQ_REASON_T reason, VCHIQ_HEADER_T * header,
				VCHIQ_SERVICE_HANDLE_T handle,
				void *bulk_userdata)
{
	/* How do we ensure the callback goes to the right client?
	   The service_user data points to a USER_SERVICE_T record containing the
	   original callback and the user state structure, which contains a circular
	   buffer for completion records.
	 */
	USER_SERVICE_T *service =
	    (USER_SERVICE_T *) VCHIQ_GET_SERVICE_USERDATA(handle);
	VCHIQ_USER_FILE_DATA_T  *fileData = service->fileData;
	VCHIQ_COMPLETION_DATA_T *completion;

	vcos_log_trace
	    ("service_callback - service %lx(%d), reason %d, header %lx,"
	     "instance %lx, bulk_userdata %lx",
	     (unsigned long)service, ((VCHIQ_SERVICE_T *) handle)->localport,
	     reason, (unsigned long)header,
	     (unsigned long)service, (unsigned long)bulk_userdata);

	if (!fileData || fileData->closing) {
#if 0 //FIXME!!
		if (bulk_userdata != NULL)
			free_pagelist((PAGELIST_T *) bulk_userdata);
#endif
		return VCHIQ_SUCCESS;
	}

	while (fileData->completion_insert ==
	       (fileData->completion_remove + MAX_COMPLETIONS)) {
		/* Out of space - wait for the client */
		if (vcos_event_wait(&fileData->remove_event) != VCOS_SUCCESS) {
			vcos_log_info("service_callback interrupted");
			return VCHIQ_RETRY;
		} else if (fileData->closing) {
			vcos_log_info("service_callback closing");
			return VCHIQ_ERROR;
		}
	}

	completion =
	    &fileData->completions[fileData->
				   completion_insert & (MAX_COMPLETIONS - 1)];
	completion->reason = reason;
	completion->header = kernel_ptr_to_user(fileData, header);
	completion->service_userdata = service->userdata;
	completion->bulk_userdata = bulk_userdata;

	fileData->completion_insert++;

	vcos_event_signal(&fileData->insert_event);

	return VCHIQ_SUCCESS;
}

/****************************************************************************
*
*   vchiq_ioctl
*
***************************************************************************/

static long vchiq_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    VCHIQ_USER_FILE_DATA_T  *fileData = file->private_data;
	VCHIQ_STATUS_T status = VCHIQ_SUCCESS;
	long ret = 0;
	int i;

#if 0
    // The following aren't needed and will be taken care of by the default case in the switch statement

	if (_IOC_TYPE(cmd) != VCHIQ_IOC_MAGIC)
		return -ENOTTY;
	if (_IOC_NR(cmd) > VCHIQ_IOC_MAX)
		return -ENOTTY;
#endif

	switch (cmd) {
	case VCHIQ_IOC_SHUTDOWN:
        vcos_log_trace("%s: VCHIQ_IOC_SHUTDOWN(%p)", __func__, fileData->vchiq_instance);

        /*
         * This functionality really belongs in the vchiq_release function.
         * We're guaranteed that the vchiq_release function will be called
         * We're not guaranteed that this particular ioctl will be called.
         */

		if (!vchiq_is_connected( fileData->vchiq_instance ))
			break;

		/* Remove all services associated with the user process */
		for (i = 0; i < MAX_SERVICES; i++) {
			USER_SERVICE_T *service = &fileData->services[i];
			if (service->service != NULL) {
				status =
				    vchiq_remove_service(service->service);
				if (status != VCHIQ_SUCCESS)
					break;
				service->service = NULL;
			}
		}

		if (status == VCHIQ_SUCCESS) {
			/* Wake the completion thread and ask it to exit */
			fileData->closing = 1;
			vcos_event_signal(&fileData->insert_event);
		}

		break;

	case VCHIQ_IOC_CONNECT:
        vcos_log_trace("%s: VCHIQ_IOC_CONNECT(%p)", __func__, fileData->vchiq_instance);

		if (vchiq_is_connected( fileData->vchiq_instance )) {
			ret = -EINVAL;
			break;
		}
        status = vchiq_connect(fileData->vchiq_instance);
        if ( status == VCHIQ_RETRY )
        {
            ret = -EINTR;
        }
		break;

	case VCHIQ_IOC_ADD_SERVICE:
        vcos_log_trace("%s: VCHIQ_IOC_ADD_SERVICE(%p)", __func__, fileData->vchiq_instance);
		{
			VCHIQ_ADD_SERVICE_T args;
			USER_SERVICE_T     *user_service;
            VCHIQ_ADD_SERVICE_T __user *user_arg
                = (VCHIQ_ADD_SERVICE_T __user *)arg;

			if (copy_from_user(&args, user_arg, sizeof(args)) != 0)
				return -EFAULT;

            user_service = find_avail_service_handle(fileData);
			if (user_service == NULL) {
				ret = -EMFILE;
				break;
			}

            vcos_log_trace( "%s:     fourcc %c%c%c%c",
                            __func__, VCHIQ_FOURCC_AS_4CHARS(args.fourcc));

            status = vchiq_add_service( fileData->vchiq_instance,
                                        args.fourcc,
                                        service_callback,
                                        user_service,
                                        &user_service->service );
            user_service->userdata = args.service_userdata;

            if ( status == VCHIQ_SUCCESS )
            {
                if (copy_to_user(&user_arg->handle,
                                 &user_service->handle,
                                 sizeof(user_arg->handle)) != 0)
                    ret = -EFAULT;
            } else {
                ret = -EEXIST;
            }
            if ( ret != VCHIQ_SUCCESS )
            {
                user_service->service = NULL;
            }
        }
        break;

    case VCHIQ_IOC_OPEN_SERVICE:
        vcos_log_trace("%s: VCHIQ_IOC_OPEN_SERVICE(%p)", __func__, fileData->vchiq_instance);
        {
            VCHIQ_ADD_SERVICE_T args;
            USER_SERVICE_T     *user_service;
            VCHIQ_ADD_SERVICE_T __user *user_arg
                = (VCHIQ_ADD_SERVICE_T __user *)arg;

            if ( !vchiq_is_connected(fileData->vchiq_instance)) {
                ret = -ENOTCONN;
                break;
            }

            if (copy_from_user(&args, user_arg, sizeof(args)) != 0) {
                ret = -EFAULT;
                break;
            }

            user_service = find_avail_service_handle(fileData);
            if (user_service == NULL) {
                ret = -EMFILE;
                break;
            }

            vcos_log_trace( "%s:     fourcc %c%c%c%c",
                            __func__, VCHIQ_FOURCC_AS_4CHARS(args.fourcc));

            status = vchiq_open_service( fileData->vchiq_instance,
                                         args.fourcc,
                                         service_callback,
                                         user_service,
                                         &user_service->service );

            user_service->userdata = args.service_userdata;

            if ( status == VCHIQ_SUCCESS )
            {
                if (copy_to_user(&user_arg->handle,
                                 &user_service->handle,
                                 sizeof(user_arg->handle)) != 0)
                    ret = -EFAULT;
            } else if ( status == VCHIQ_RETRY ) {
                ret = -EINTR;
            }
            else {
                ret = -EEXIST;
            }

            if ( ret != VCHIQ_SUCCESS )
            {
                user_service->service = NULL;
            }
        }
		break;

	case VCHIQ_IOC_REMOVE_SERVICE:
        vcos_log_trace("%s: VCHIQ_IOC_REMOVE_SERVICE(%p)", __func__, fileData->vchiq_instance);
		{
			USER_SERVICE_T *user_service;
			int handle = (int)arg;

			user_service = find_service_by_handle( fileData, handle );
            if ( user_service != NULL )
            {
                status = vchiq_remove_service(user_service->service);
                if (status == VCHIQ_SUCCESS)
                    user_service->service = NULL;
			} else
				ret = -EINVAL;
		}
		break;

	case VCHIQ_IOC_QUEUE_MESSAGE:
        vcos_log_trace("%s: VCHIQ_IOC_QUEUE_MESSAGE(%p)", __func__, fileData->vchiq_instance);
		{
			VCHIQ_QUEUE_MESSAGE_T args;
			USER_SERVICE_T *user_service;

			if (copy_from_user
			    (&args, (const void __user *)arg,
			     sizeof(args)) != 0) {
				ret = -EFAULT;
				break;
			}
			user_service = find_service_by_handle(fileData, args.handle);
			if ((user_service != NULL)&& (args.count <= MAX_ELEMENTS)) {
				/* Copy elements into kernel space */
				VCHIQ_ELEMENT_T elements[MAX_ELEMENTS];
				if (copy_from_user(elements,
                                   args.elements,
				                   args.count * sizeof(elements[0])) == 0)
					status =
					    vchiq_queue_message(user_service->service,
								            elements,
								            args.count);
				else
					ret = -EFAULT;
			} else {
				ret = -EINVAL;
			}
		}
		break;

	case VCHIQ_IOC_QUEUE_BULK_TRANSMIT:
        vcos_log_trace("%s: VCHIQ_IOC_QUEUE_BULK_TRANSMIT(%p)", __func__, fileData->vchiq_instance);
		{
			VCHIQ_QUEUE_BULK_TRANSMIT_T args;
			USER_SERVICE_T *user_service;

			if (copy_from_user
			    (&args, (const void __user *)arg,
			     sizeof(args)) != 0)
				return -EFAULT;
			user_service = find_service_by_handle(fileData, args.handle);
			if (user_service != NULL ){
				status =
				    vchiq_queue_bulk_transmit_handle(user_service->service,
                                                     (VCHI_MEM_HANDLE_T)current,
                                                     args.data,
                                                     args.size,
                                                     args.userdata);
			} else {
				ret = -EINVAL;
			}
		}
		break;

	case VCHIQ_IOC_QUEUE_BULK_RECEIVE:
        vcos_log_trace("%s: VCHIQ_IOC_QUEUE_BULK_RECEIVE(%p)", __func__, fileData->vchiq_instance);
		{
			VCHIQ_QUEUE_BULK_RECEIVE_T args;
			USER_SERVICE_T *user_service;

			if (copy_from_user
			    (&args, (const void __user *)arg,
			     sizeof(args)) != 0)
				return -EFAULT;
			user_service = find_service_by_handle(fileData, args.handle);
			if (user_service != NULL) {
				status =
				    vchiq_queue_bulk_receive_handle(user_service->service,
                                                    (VCHI_MEM_HANDLE_T)current,
                                                    args.data,
                                                    args.size,
                                                    args.userdata);
			} else {
				ret = -EINVAL;
			}
		}
		break;

	case VCHIQ_IOC_AWAIT_COMPLETION:
        vcos_log_trace("%s: VCHIQ_IOC_AWAIT_COMPLETION(%p)", __func__, fileData->vchiq_instance);
		{
			VCHIQ_AWAIT_COMPLETION_T args;

			if (!vchiq_is_connected( fileData->vchiq_instance ))
				return -ENOTCONN;

			if (copy_from_user
			    (&args, (const void __user *)arg,
			     sizeof(args)) != 0)
				return -EFAULT;

			while ((fileData->completion_remove ==
				fileData->completion_insert)
			       && !fileData->closing) {

				if (vcos_event_wait(&fileData->insert_event) !=
				    VCOS_SUCCESS) {
					vcos_log_trace("AWAIT_COMPLETION interrupted");
					ret = -EINTR;
					break;
				}
			}

			if (ret == 0) {
				for (ret = 0; ret < args.count; ret++) {
					VCHIQ_COMPLETION_DATA_T *completion;

					if (fileData->completion_remove ==
					    fileData->completion_insert)
						break;
					completion =
					    &fileData->completions[fileData->
								   completion_remove &
								   (MAX_COMPLETIONS - 1)];

					if (copy_to_user
					    ((void __user *)((size_t) args.buf +
							     ret *
							     sizeof
							     (VCHIQ_COMPLETION_DATA_T)),
					     completion,
					     sizeof(VCHIQ_COMPLETION_DATA_T)) !=
					    0) {
						ret = -EFAULT;
						break;
					}

					fileData->completion_remove++;
				}
			}

			if (ret != 0)
				vcos_event_signal(&fileData->remove_event);
		}
		break;

	case VCHIQ_IOC_RELEASE_MESSAGE:
        vcos_log_trace("%s: VCHIQ_IOC_RELEASE_MESSAGE(%p)", __func__, fileData->vchiq_instance);
		{
			VCHIQ_RELEASE_MESSAGE_T args;
			USER_SERVICE_T *user_service;

			if (copy_from_user
			    (&args, (const void __user *)arg,
			     sizeof(args)) != 0)
				return -EFAULT;
			user_service = find_service_by_handle(fileData, args.handle);
            if ( user_service != NULL ) {
				VCHIQ_HEADER_T *header;
                header = user_ptr_to_kernel(fileData, args.header);
				vcos_log_trace("RELEASE_MESSAGE 0x%p", header );
				vchiq_release_message(user_service->service,
						              header);
			}
		}
		break;

	default:
        vcos_log_trace("%s: unknown IOCTL(%p)", __func__, fileData->vchiq_instance);

		ret = -ENOTTY;
		break;
	}

	if (ret == 0) {
		if (status == VCHIQ_ERROR)
			ret = -EIO;
		else if (status == VCHIQ_RETRY)
			ret = -EINTR;
	}

	vcos_log_trace("%s   status %d returing %ld", __func__, status, ret );
	return ret;
}

/****************************************************************************
*
*   vchiq_mmap
*
***************************************************************************/

static int vchiq_mmap(struct file *file, struct vm_area_struct *vma)
{
    VCHIQ_SHARED_MEM_INFO_T  info;
	VCHIQ_USER_FILE_DATA_T  *fileData = file->private_data;
	unsigned long start = vma->vm_start;
	unsigned long size = vma->vm_end - vma->vm_start;

    if ( size > VCHIQ_CHANNEL_SIZE )
    {
        size = VCHIQ_CHANNEL_SIZE;
    }

	vcos_log_trace("vchiq_mmap: start=0x%lx, size=0x%lx, pgoff=0x%lx, flags=0x%lx, page_prot=0x%lx",
	     start, size, vma->vm_pgoff, vma->vm_flags, vma->vm_page_prot);

	if ((vma->vm_pgoff != 0) || (size != VCHIQ_CHANNEL_SIZE) ||
	    ((vma->vm_flags & VM_WRITE) != 0))
		return -EINVAL;

    vchiq_get_shared_mem_info( &info );

    /* do not cache the memory map */
    vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);

	if (remap_pfn_range(vma, vma->vm_start,
		                (info.vcSharedMemPhys >> PAGE_SHIFT) + vma->vm_pgoff,
		                size, vma->vm_page_prot)) {
		vcos_log_error("remap_pfn_range failed");
		return -EAGAIN;
	}

	fileData->mmap_user_start = (void *)vma->vm_start;
    fileData->mmap_user_end = (uint8_t *)fileData->mmap_user_start
                            + VCHIQ_CHANNEL_SIZE;

    fileData->mmap_kernel_start = info.vcSharedMemVirt;
    fileData->mmap_kernel_end   = (uint8_t *)fileData->mmap_kernel_start
                                + VCHIQ_CHANNEL_SIZE;

	return 0;
}

/****************************************************************************
*
*   vchiq_open
*
***************************************************************************/

static int vchiq_open(struct inode *inode, struct file *file)
{
    VCHIQ_USER_FILE_DATA_T *fileData;
    VCHIQ_KERNEL_STATE_T   *kernState;

	int dev = iminor(inode) & 0x0f;

    if (( dev >= VCHIQ_NUM_VIDEOCORES )
    ||  ( vchiq_kernel_state[ dev ] == NULL ))
    {
        vcos_log_error( "Unknown minor device: %d", dev );
        return -ENXIO;
    }
    kernState = vchiq_kernel_state[ dev ];

    fileData = kzalloc( sizeof( *fileData ), GFP_KERNEL );
    if ( fileData == NULL )
    {
        return -ENOMEM;
    }

    if ( vchiq_initialise( &fileData->vchiq_instance ) != VCHIQ_SUCCESS )
    {
        kfree( fileData );
        return -ENOMEM;
    }

    fileData->kernState = kernState;
    vcos_event_create(&fileData->insert_event, DEVICE_NAME);
    vcos_event_create(&fileData->remove_event, DEVICE_NAME);

    file->private_data = fileData;

    vcos_log_trace( "%s: fileData=%p", __func__, fileData );

	return 0;
}

/****************************************************************************
*
*   vchiq_release
*
***************************************************************************/

static int vchiq_release(struct inode *inode, struct file *file)
{
    int                     i;
    VCHIQ_USER_FILE_DATA_T *fileData = file->private_data;

    vcos_log_trace( "%s: fileData=%p", __func__, fileData );

    fileData->closing = 1;

    /* Remove all services */
    for (i = 0; i < MAX_SERVICES; i++) {
        USER_SERVICE_T *user_service = &fileData->services[i];
        if (user_service->service != NULL)
            vchiq_terminate_service(user_service->service);
    }

    /*
     * Need to make sure that the completion thread has exited. We can't assume
     * that the user process shutdown cleanly.
    */

    vcos_event_delete(&fileData->insert_event);
    vcos_event_delete(&fileData->remove_event);
    kfree(fileData);
    file->private_data = NULL;

	return 0;
}

/****************************************************************************
*
*   File Operations for the driver.
*
***************************************************************************/

static const struct file_operations vchiq_fops = {
    .owner = THIS_MODULE,
    .unlocked_ioctl = vchiq_ioctl,
    .open = vchiq_open,
    .release = vchiq_release,
    .mmap = vchiq_mmap,
};

/****************************************************************************
*
*   vchiq_init - called when the module is loaded.
*
***************************************************************************/

static int __init vchiq_init(void)
{
	int err = -ENODEV;

    /* enable logging */
    vcos_logging_init();

    vcos_log_register( "vchiq_arm", &vchiq_arm_log_category );
	vchiq_set_log_level(log_level);

    /* create a device class for create dev nodes */
    vchiq_class = class_create(THIS_MODULE, "videocore");
    if (IS_ERR(vchiq_class))
    {
        vcos_log_error("Unable to create device class");
        err = PTR_ERR(vchiq_class);
        goto out_err;
    }

    /* create a character driver */
    if ((err = alloc_chrdev_region(&vchiq_devid, VCHIQ_MINOR, 1, DEVICE_NAME)) != 0)
    {
        vcos_log_error("Unable to allocate device number");
        goto out_dev_class_destroy;
    }

	cdev_init(&vchiq_cdev, &vchiq_fops);
	if ((err = cdev_add(&vchiq_cdev, vchiq_devid, 1)) != 0)
	{
		vcos_log_error("Unable to register device");
		goto out_chrdev_unreg;
	}

    /* create a device node */
    vchiq_dev = device_create(vchiq_class, NULL,
                              MKDEV(MAJOR(vchiq_devid), VCHIQ_MINOR),
                              NULL, "vchiq");
    if (IS_ERR(vchiq_dev))
    {
        vcos_log_error("Unable to create device node");
        err = PTR_ERR(vchiq_dev);
        goto out_chrdev_del;
    }

    vcos_log_error("Initialised");

    return 0;

out_chrdev_del:
	cdev_del(&vchiq_cdev);

out_chrdev_unreg:
	unregister_chrdev_region(vchiq_devid, 1);

out_dev_class_destroy:
    class_destroy(vchiq_class);
    vchiq_class = NULL;

out_err:
	return err;
}

/****************************************************************************
*
*   vchiq_exit - called when the module is unloaded.
*
***************************************************************************/

static void __exit vchiq_exit(void)
{
	cdev_del(&vchiq_cdev);
	unregister_chrdev_region(vchiq_devid, 1);

   free_irq( BCM_INT_ID_IPC_OPEN, NULL ); //TODO move to vchiq_shutdown (when implemented)

   vcos_log_unregister(&vchiq_arm_log_category);
}

module_init(vchiq_init);
module_exit(vchiq_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Broadcom Corporation");


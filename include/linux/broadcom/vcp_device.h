/*****************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
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
*  @file    vcp_device.h
*
*  @brief   Defines the Video Conference Pipeline (VCP) Device API
*
*****************************************************************************/
#if !defined( VCP_DEVICE_H )
#define VCP_DEVICE_H

/* ---- Include Files ---------------------------------------------------- */
#if defined( __KERNEL__ )
#include <linux/types.h>	/* Needed for standard types */
#else
#include <stdint.h>
#endif

#include <linux/broadcom/vcp.h>
#include <linux/broadcom/vcp_conference.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---- Constants and Types ---------------------------------------------- */

/* A handle to a registered VCP device */
	typedef void *VCP_DEV_HDL;

/* A handle to a VCP pipeline */
	typedef void *VCP_PIPE_HDL;

/***************************************************************************/
/**
*  Close pipeline
*
*  @return  0 on success, otherwise -ve error
*/
	typedef int (*VCP_DEV_CLOSE_FNC) (VCP_PIPE_HDL hdl,
				 /**< (i) Pipeline handle */
					  void *devdata
				 /**< (i) Private device data */
	    );

/***************************************************************************/
/**
*  Start pipeline
*
*  @return  0 on success, otherwise -ve error
*/
	typedef int (*VCP_DEV_START_FNC) (VCP_PIPE_HDL hdl,
				 /**< (i) Pipeline handle */
					  void *devdata
				 /**< (i) Private device data */
	    );

/***************************************************************************/
/**
*  Stop pipelilne
*
*  @return  0 on success, otherwise -ve error
*/
	typedef int (*VCP_DEV_STOP_FNC) (VCP_PIPE_HDL hdl,
				 /**< (i) Pipeline handle */
					 void *devdata
				 /**< (i) Private device data */
	    );

/***************************************************************************/
/**
*  Set pipeline parameters
*
*  @return  0 on success, otherwise -ve error
*/
	typedef int (*VCP_DEV_SETPARM_FNC) (VCP_PIPE_HDL hdl,
				 /**< (i) Pipeline handle */
					    VCP_PARM_INDEX index,
				 /**< (i) Parameter index */
					    const void *datap,
				 /**< (i) Ptr to command related data */
					    void *devdata
				 /**< (i) Private device data */
	    );

/***************************************************************************/
/**
*  Query pipeline parameters
*
*  @return  0 on success, otherwise -ve error
*/
	typedef int (*VCP_DEV_GETPARM_FNC) (VCP_PIPE_HDL hdl,
				 /**< (i) Pipeline handle */
					    VCP_PARM_INDEX index,
				 /**< (i) Parameter index */
					    void *datap,
				 /**< (o) Ptr to config data */
					    void *devdata
				 /**< (i) Private device data */
	    );

/***************************************************************************/
/**
*  Query pipeline capabilities.
*
*  @return  0 on success, otherwise -ve error
*/
	typedef int (*VCP_DEV_GETCAP_FNC) (VCP_PIPE_HDL hdl,
				 /**< (i) Pipeline handle */
					   VCP_CAPABILITIES *cap,
				 /**< (o) Ptr to capabilities data */
					   void *devdata
				 /**< (i) Private device data */
	    );

/***************************************************************************/
/**
*  Configure the device channels's conference configuration
*
* @return 0 on success, otherwise -ve error
*/
	typedef int (*VCP_DEV_SET_CONFERENCE) (VCP_PIPE_HDL *hdlp,
				       /**< (i) Ptr to device handle */
					       VCP_CONFERENCE_PARM_INDEX index,
				       /**< (i) Parameter index */
					       const void *datap,
				       /**< (i) Ptr to command related data */
					       void *devdata
				       /**< (i) Private device data */
	    );

/***************************************************************************/
/**
*  Configure the device channels's conference configuration
*
* @return 0 on success, otherwise -ve error
*/
	typedef int (*VCP_DEV_GET_CONFERENCE) (VCP_PIPE_HDL *hdlp,
				       /**< (i) Ptr to device handle */
					       VCP_CONFERENCE_PARM_INDEX index,
				       /**< (i) Parameter index */
					       const void *datap,
				       /**< (o) Ptr to command related data */
					       void *devdata
				       /**< (i) Private device data */
	    );

/**
*  Common video device operations (base class)
*/
#define VCP_COMMON_DEVICE_OPS_BASE_CLASS \
   VCP_DEV_CLOSE_FNC close; \
   VCP_DEV_START_FNC start; \
   VCP_DEV_STOP_FNC stop; \
   VCP_DEV_SETPARM_FNC setparm; \
   VCP_DEV_GETPARM_FNC getparm; \
   VCP_DEV_GETCAP_FNC getcap; \
   VCP_DEV_SET_CONFERENCE setconference; \
   VCP_DEV_GET_CONFERENCE getconference

/***************************************************************************/
/**
*  Common video device operations
*/
	typedef struct vcp_base_device_ops {
		VCP_COMMON_DEVICE_OPS_BASE_CLASS;	/* MUST BE FIRST! */
	} VCP_BASE_DEVICE_OPS;

/***************************************************************************/
/**
*  Video encoder device operations
*/
	typedef struct vcp_enc_device_ops {
		VCP_COMMON_DEVICE_OPS_BASE_CLASS;	/* MUST BE FIRST! */

   /**
   *  Open pipeline. Some devices may support multiple pipelines.
   *
   *  @return  0 on success, otherwise -ve error
   */
		int (*open) (VCP_HOST_ENC_CBKS *cbksp,
				    /**< (i) Ptr to host callbacks */
			     void *cbdata,
				    /**< (i) Private data for callbacks */
			     VCP_PIPE_HDL *hdlp,
				    /**< (o) Ptr to return handle */
			     void *devdata
				    /**< (i) Private device data */
		    );

   /**
   *  Get encoder statistics.
   *
   *  @return  0 on success, otherwise -ve error
   */
		int (*getencstats) (VCP_PIPE_HDL hdl,
				    /**< (i) Pipeline Handle */
				    int reset,
				    /**< (i) 0=>do not reset stats after reading them, otherwise reset them */
				    VCP_ENCODER_STATS *stats,
				    /**< (o) Ptr to statistics */
				    void *devdata
				    /**< (i) Private device data */
		    );
	} VCP_ENC_DEVICE_OPS;

/***************************************************************************/
/**
*  Video decoder device operations
*/
	typedef struct vcp_dec_device_ops {
		VCP_COMMON_DEVICE_OPS_BASE_CLASS;	/* MUST BE FIRST! */

   /**
   *  Open pipeline. Some devices may support multiple pipelines
   *
   *  @return  0 on success, otherwise -ve error
   */
		int (*open) (VCP_HOST_DEC_CBKS *cbksp,
				    /**< (i) Ptr to host callbacks */
			     void *cbdata,
				    /**< (i) Private data for callbacks */
			     VCP_PIPE_HDL *hdlp,
				    /**< (o) Ptr to return handle */
			     void *devdata
				    /**< (i) Private device data */
		    );

   /**
   *  Decode frame
   *
   *  @return  0 on success, otherwise -ve error
   */
		int (*decode) (VCP_PIPE_HDL hdl,
				    /**< (i) Pipeline handle */
			       VCP_FRAMEBUF *framebufp,
				    /**< (i) Ptr to frame buffer to decode */
			       void *devdata
				    /**< (i) Private device data */
		    );

   /**
   *  Get decoder statistics.
   *
   *  @return  0 on success, otherwise -ve error
   */
		int (*getdecstats) (VCP_PIPE_HDL hdl,
				    /**< (i) Pipeline Handle */
				    int reset,
				    /**< (i) 0=>do not reset stats after reading them, otherwise reset them */
				    VCP_DECODER_STATS *stats,
				    /**< (o) Ptr to statistics */
				    void *devdata
				    /**< (i) Private device data */
		    );
	} VCP_DEC_DEVICE_OPS;

/* ---- Variable Externs ------------------------------------------------- */

/* ---- Function Prototypes ---------------------------------------------- */

/***************************************************************************/
/**
*  Register a video encoder device.
*
*  @return  0        on success, otherwise -ve on error
*           EINVAL   Invalid parmeters, e.g. missing ops
*           EPERM    Cannot register, device already exists
*           ENOMEM   Insufficient memory
*/
	int vcp_register_enc_device(const char *devname,
				    /**< (i) Device name */
				    const VCP_ENC_DEVICE_OPS *ops,
				    /**< (i) Device callbacks */
				    void *devdata,
				    /**< (i) Private device data */
				    VCP_DEV_HDL *hdlp
				    /**< (o) Ptr to output handle */
	    );

/***************************************************************************/
/**
*  Register a video decoder device.
*
*  @return  0  on success, -ve on error
*/
	int vcp_register_dec_device(const char *devname,
				    /**< (i) Device name */
				    const VCP_DEC_DEVICE_OPS *ops,
				    /**< (i) Device callbacks */
				    void *devdata,
				    /**< (i) Private device data */
				    VCP_DEV_HDL *hdlp
				    /**< (o) Ptr to output handle */
	    );

/***************************************************************************/
/**
*  Deregister a video decoder or video encoder device
*
*  @return  0        on success, otherwise -ve on error
*/
	int vcp_deregister_device(VCP_DEV_HDL hdl
				    /**< (i) Handle */
	    );

#ifdef __cplusplus
}
#endif
#endif				/* VCP_DEVICE_H */

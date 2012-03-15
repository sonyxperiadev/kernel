/*****************************************************************************
* Copyright 2009-2011 Broadcom Corporation.  All rights reserved.
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
*  @file    amxr_ports.h
*
*****************************************************************************/
#if !defined( AMXR_PORTS_H )
#define AMXR_PORTS_H

/* ---- Constants and Types ---------------------------------------------- */

/* MakeDefs: Off */

/**
*  Mixer port callbacks. Each port can choose to implement all or some
*  of the callbacks.
*
*  Two callbacks are used by the mixer to obtain destination and
*  source data pointers, and two other callbacks are used to indicate
*  when the mixer has finished its business with the pointers. All data
*  pointers point to 16-bit signed data.
*
*  Destination and source directions are relative to the port. A destination
*  pointer is used to write data into the port, where a source point is
*  used to read data from the port.
*/
typedef struct amxr_port_cb {
	/* This callback returns a pointer to the audio source data for
	 * one frame period and it takes a frame size in bytes as a parameter.
	 * It may be called more than once in a frame period with the same
	 * or different frame size. The callback may return a valid pointer
	 * or a NULL pointer if the frame size is invalid. If a NULL pointer
	 * is returned, the Audio Mixer will not make use of the pointer.
	 *
	 * Atomic operation.
	 */
	int16_t *(*getsrc) (int bytes, /**< (i) size of the buffer in bytes */
			    void *privdata
				       /**< (i) user supplied data */
	    );

	/* This callback indicates when the Audio Mixer is done with the
	 * source data for one audio frame. It is only called once per frame
	 * period and is thus typically used for timing and buffer management.
	 *
	 * Atomic operation.
	 */
	void (*srcdone) (int bytes,    /**< (i) size of the buffer in bytes */
			 void *privdata/**< (i) user supplied data */
	    );

	/* This callback returns a pointer to the destination audio buffer
	 * for one frame period and takes a frame size in bytes as a parameter.
	 * It may be called more than once in a frame period with the same or
	 * different frame size. The callback may return a valid pointer or a
	 * NULL pointer if the frame size is invalid. If a NULL pointer is
	 * returned, the Audio Mixer will not make use of the pointer.
	 *
	 * Atomic operation.
	 */
	int16_t *(*getdst) (int bytes, /**< (i) size of the buffer in bytes */
			    void *privdata
				       /**< (i) user supplied data */
	    );

	/* This callback indicates when the Audio Mixer is done with the
	 * destination buffer for one audio frame. It is only called once per
	 * frame period and is typically used for timing and buffer management.
	 *
	 * Atomic operation.
	 */
	void (*dstdone) (int bytes,    /**< (i) size of the buffer in bytes */
			 void *privdata/**< (i) user supplied data */
	    );

	/* This callback indicates when the last destination connection to
	 * the port is removed. This callback may be useful to trigger a flush
	 * of a port's destination buffers.
	 */
	void (*dstcnxsremoved) (void *privdata
				       /**< (i) user supplied data */
	    );
} AMXR_PORT_CB;

/* MakeDefs: On */

/* ---- Variable Externs ------------------------------------------------- */
/* ---- Function Prototypes ---------------------------------------------- */

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************/
/**
*  Create a mixer port.
*
*  @return
*     0        On success
*     -ve      Error code
*/
	int amxrCreatePort(const char *name,
				    /**< (i) Name string */
			   AMXR_PORT_CB * cb,
				    /**< (i) Callbacks */
			   void *privdata,
				    /**< (i) Private data passed back to callbacks */
			   int dst_hz,
				    /**< (i) Current destination sampling frequency in Hz */
			   int dst_chans,
				    /**< (i) Number of channels, i.e. mono = 1 */
			   int dst_bytes,	/*<< (i) Destination period size in bytes */
			   int src_hz,
				    /**< (i) Current source sampling frequency in Hz */
			   int src_chans,
				    /**< (i) Number of channels, i.e. mono = 1 */
			   int src_bytes,	/*<< (i) Source period size in bytes */
			   AMXR_PORT_ID * portidp
				    /**< (o) Ptr to port ID */
	    );

/***************************************************************************/
/**
*  Remove an existing mixer port. All connections to this port will also
*  be removed.
*
*  @return
*     0           On success
*     -ve         On general failure
*
*  @remarks
*     Typically ports are not removed unless the entire user is removed.
*     Although it is conceivable that the API supports dynamic creation and
*     deletion of ports, it is not practical for most applications since
*     applications will have to constantly query what ports still exists
*     and keep track of valid port IDs.
*/
	int amxrRemovePort(AMXR_PORT_ID port
				    /**< (i) Port to remove */
	    );

/***************************************************************************/
/**
*  Set destination port sampling frequency. Connections using the
*  port are also updated accordingly.
*
*  @return
*     0           On success
*     -ve         On general failure
*/
	int amxrSetPortDstFreq(AMXR_PORT_ID portid,
				    /**< (i) Destination port id */
			       int dst_hz,	/*<< (i) Destination sampling frequency in Hz */
			       int dst_bytes	/*<< (i) Destination period size in bytes */
	    );

/***************************************************************************/
/**
*  Set source port sampling frequency. Connections using the port
*  are also updated accordingly.
*
*  @return
*     0           On success
*     -ve         On general failure
*/
	int amxrSetPortSrcFreq(AMXR_PORT_ID portid,
				    /**< (i) Source port id */
			       int src_hz,	/*<< (i) Source sampling frequency in Hz */
			       int src_bytes	/*<< (i) Source period size in bytes */
	    );

/***************************************************************************/
/**
*  Set port sampling frequencies. Connections using the port
*  are also updated accordingly.
*
*  @return
*     0           On success
*     -ve         On general failure
*
*  @remarks
*     Source and destination sampling frequencies may be asymmetric.
*/
	static inline int amxrSetPortFreq(AMXR_PORT_ID portid,
				    /**< (i) port id */
					  int dst_hz,
				    /**< (i) Destination sampling frequency in Hz */
					  int dst_bytes,	/*<< (i) Destination period size in bytes */
					  int src_hz,
				    /**< (i) Source sampling frequency in Hz */
					  int src_bytes	/*<< (i) Source period size in bytes */
	    ) {
		int err;
		 err = amxrSetPortDstFreq(portid, dst_hz, dst_bytes);
		if (!err) {
			err = amxrSetPortSrcFreq(portid, src_hz, src_bytes);
		}
		return err;
	}

/***************************************************************************/
/**
*  Set the channel configuration for a destination port.
*  Connections using the port are updated, where appropriate.
*
*  @return
*     0           On success
*     -ve         On general failure
*
*  @remarks    Not all existing connections with this source port
*              will be maintained after changing the number of channels.
*/
	int amxrSetPortDstChannels(AMXR_PORT_ID portid,
				    /**< (i) Destination port id */
				   int dst_chans,	/*<< (i) Number of channels: 1 for mono, 2 for stereo, etc */
				   int dst_bytes	/*<< (i) Destination period size in bytes */
	    );

/***************************************************************************/
/**
*  Set the channel configuration for a source port.
*  Connections using the port are updated, where appropriate.
*
*  @return
*     0           On success
*     -ve         On general failure
*
*  @remarks    Not all existing connections with this source port
*              will be maintained after changing the number of channels.
*/
	int amxrSetPortSrcChannels(AMXR_PORT_ID portid,
				    /**< (i) Source port id */
				   int src_chans,	/*<< (i) Number of src channels: 1 for mono, 2 for stereo, etc. */
				   int src_bytes	/*<< (i) Source period size in bytes */
	    );

#if !defined( __KERNEL__ )
/* User space only APIs */

/***************************************************************************/
/**
*  Direct write for port's src data to support low latency applications.
*
*  @return        Number of bytes written, or negative error code
*
*  @remarks       If this method is desired, do not install user getsrc
*                 callback when creating the port.
*
*                 Caller has to take care to check return codes to ensure
*                 data has been queued as it is non-blocking. A -ENOMEM
*                 is returned if there is insufficient memory to queue
*                 data.
*/
	int amxrWritePortSrcData(AMXR_PORT_ID portid,
				    /**< (i) Source port id */
				 const void *bufp,
				    /**< (i) Buffer ptr */
				 size_t bytes
				    /**< (i) Number of bytes to write */
	    );
#endif

#if defined( __KERNEL__ )
/* Move following definitions here from amxr.h to resolve circular dependency
 * between amxr.h and amxr_ports.h. These definitions will be made obsolete
 * soon once the driver and core code are refactored
 */

/**
*  Structure containing function pointer types for the Audio Mixer API.
*  This structure is used to install a proprietary implementation of the
*  Audio Mixer.
*
*  The function pointer type must match the parameters of the actual API.
*  Refer to the API function prototypes for the explanation of the
*  parameters and description of the API.
*/
	typedef struct amxr_api_funcs {
		int (*getPortInfo) (AMXR_HDL, AMXR_PORT_ID, AMXR_PORT_INFO *);
		int (*getInfo) (AMXR_HDL, AMXR_INFO *);
		int (*setCnxLoss) (AMXR_HDL, AMXR_PORT_ID, AMXR_PORT_ID,
				   AMXR_CONNECT_DESC, unsigned int);
		int (*getCnxLoss) (AMXR_HDL, AMXR_PORT_ID, AMXR_PORT_ID,
				   AMXR_CONNECT_DESC, unsigned int *);
		int (*connect) (AMXR_HDL, AMXR_PORT_ID, AMXR_PORT_ID,
				AMXR_CONNECT_DESC);
		int (*disconnect) (AMXR_HDL, AMXR_PORT_ID, AMXR_PORT_ID);
		int (*getCnxListBySrc) (AMXR_HDL, AMXR_PORT_ID, AMXR_CNXS *,
					int);
		int (*getCnxListByDst) (AMXR_HDL, AMXR_PORT_ID, AMXR_CNXS *,
					int);

		/* Kernel only API */
		int (*createPort) (const char *, AMXR_PORT_CB *, void *, int,
				   int, int, int, int, int, AMXR_PORT_ID *);
		int (*removePort) (AMXR_PORT_ID);
		int (*setPortDstFreq) (AMXR_PORT_ID, int, int);
		int (*setPortSrcFreq) (AMXR_PORT_ID, int, int);
		int (*setPortDstChannels) (AMXR_PORT_ID, int, int);
		int (*setPortSrcChannels) (AMXR_PORT_ID, int, int);
		void (*serviceUnsyncPort) (AMXR_PORT_ID);
	} AMXR_API_FUNCS;
/***************************************************************************/
/**
*  Setup proprietary Audio Mixer implementation
*
*  @return    none
*/
	void amxrSetApiFuncs(const AMXR_API_FUNCS * funcs
				    /**< (i) Ptr to API functions.
                                             If NULL, clears registration */
	    );
#endif

#ifdef __cplusplus
}
#endif

#endif /* AMXR_PORTS_H */

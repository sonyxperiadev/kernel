/*****************************************************************************
*  Copyright 2003 - 2007 Broadcom Corporation.  All rights reserved.
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
*****************************************************************************/
#include <linux/io.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/string.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/uaccess.h>

#include <linux/poll.h>

#include <linux/mm.h>
#include <linux/cdev.h>
#include <linux/fcntl.h>
#include <linux/list.h>
#include <linux/ioctl.h>
#include <linux/types.h>
#include <linux/unistd.h>
#include <linux/errno.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/jiffies.h>
#include <asm/pgtable.h>
#include <linux/proc_fs.h>

#include <linux/broadcom/bcm_major.h>

#include "rpc_ipc_config.h"
#include "vsp_debug.h"

#include "mobcom_types.h"
#include <linux/broadcom/resultcode.h>
#include "taskmsgs.h"

#include <linux/broadcom/ipcproperties.h>
#include "rpc_ipc.h"

#include "xdr_porting_layer.h"
#include "xdr.h"
#include "rpc_api.h"

static struct class *vsp_class;

#define CSD_BUFFER_SIZE  2048

/* During VT connect, CP will send 80 bytes twice every 20 msec to AP */
#define CSD_DOWNLINK_DATA_LEN  80

/* SIM ID for Dual SIM VT call */
#define VT_IO_MAGIC   'V'
#define VT_SIM1       _IO(VT_IO_MAGIC, 1)
#define VT_SIM2       _IO(VT_IO_MAGIC, 2)

static int sSimId;

static UInt8 stempBuf[CSD_BUFFER_SIZE];
static UInt32 sPendingBytes;
static UInt8 sCsdTempBuf[CSD_BUFFER_SIZE];
static UInt32 sCsdPendingBytes;
static UInt8 sSendTempBuf[CSD_BUFFER_SIZE];
static UInt8 sReadTempBuf[CSD_BUFFER_SIZE];

#ifdef CSD_LOOPBACK_TEST
static UInt8  sAplookback_tempBuf[CSD_BUFFER_SIZE];
static UInt32 sAplookback_PendingBytes;
static spinlock_t csLocker_aploopback;
static spinlock_t csLocker_cploopback;
#endif /* CSD_LOOPBACK_TEST */

static spinlock_t csLocker;

/* used to generate valid RPC client ID for fuse_net driver */
/*extern unsigned char SYS_GenClientID(void);*/
static unsigned char g_CsdClientId;
static UInt8 csdULframeNum = 1;
static UInt8 g_first_init;
static UInt32 sCopiedBytes;

static wait_queue_head_t g_csd_wait;

/*******************************************************************************

	Function to get received data length

	@param	void

	@note
	Check the current pending bytes in CSD downlink buffer.

*******************************************************************************/
UInt32 GetRxBytesAvailable(void)
{
#ifdef CSD_LOOPBACK_TEST
	return sAplookback_PendingBytes;
#else
	return sCsdPendingBytes;
#endif /* CSD_LOOPBACK_TEST */
}

#ifdef CSD_LOOPBACK_TEST
/*******************************************************************************

	Function to AP loop back data

	@param	pInBuf (in) point to one buffer that get AP loop back data.
	@return read data length.
	@note
	Read AP loopback data.

*******************************************************************************/
UInt32 CSD_Read(UInt8 *pInBuf)
{
	UInt32 dwDataSize = 0;

	/* Get buffer access */
	spin_lock_bh(&csLocker_aploopback);

	if (NULL == pInBuf) {
		sAplookback_PendingBytes = 0;
		spin_unlock_bh(&csLocker_aploopback);
		return 0;
	}

	if (sAplookback_PendingBytes > 0) {
		memcpy(pInBuf, sAplookback_tempBuf, sAplookback_PendingBytes);
		dwDataSize = sAplookback_PendingBytes;
		sAplookback_PendingBytes = 0;
	}

	/* Release buffer access */
	spin_unlock_bh(&csLocker_aploopback);
	return dwDataSize;
}

/*******************************************************************************

	Function to send AP loop back data

 @param	pInBuf (in) point to one buffer that has AP loopback data to be sent.
 @param	len (in) the available send data length
 @return sent data length
 @note
 Send AP loop back data.

*******************************************************************************/
UInt32 CSD_Send(const void *pBuf, const UInt32 len)
{
	/* Get buffer access */
	spin_lock_bh(&csLocker_aploopback);

	if (CSD_BUFFER_SIZE - sAplookback_PendingBytes >= len)	{
		memcpy(&sAplookback_tempBuf[sAplookback_PendingBytes]
			, pBuf,	len);
		sAplookback_PendingBytes += len;
	}

	/* Release buffer access */
	spin_unlock_bh(&csLocker_aploopback);

	return len;
}

/*******************************************************************************

	Function to send CP loop back data

 @param	pInBuf (in) point to one buffer that has CP loopback data to be sent.
	@param		len (in) the available send data length.
	@return   sent data length
	@note
	First Make sure that can get free bufffer from IPC.
	Then Send CP loop back data to IPC.

*******************************************************************************/
UInt32 CSD_CP_Loopback_Send(const void *pBuf, const UInt32 len)
{
	PACKET_BufHandle_t bufHandle = NULL;
	void *bufferPtr = NULL;
	UInt8 cid = 0;

	if (!(1 == sSimId || 2 == sSimId)) {
		VSP_DEBUG(DBG_ERROR, "vsp: sSimId is %d. Error!!!\n", sSimId);
		sSimId = 1; /* set default to avoid CP crash */
	}

	if (0 == csdULframeNum)
		csdULframeNum++;

	cid = csdULframeNum++;

	/* Get ptr to buffer */
	bufHandle = RPC_PACKET_AllocateBuffer(INTERFACE_CSD, len, cid);
	if (!bufHandle) {
		VSP_DEBUG(DBG_ERROR, "vsp: bufHandle is NULL.\n");
		return 0;
	}

	/* Copy data */
	bufferPtr = RPC_PACKET_GetBufferData(bufHandle);
	if (!bufferPtr) {
		VSP_DEBUG(DBG_ERROR, "vsp: bufferPtr is NULL.\n");
		return 0;
	}
	memcpy(bufferPtr, (char *)pBuf, len);

	/* Set the buffer len */
	RPC_PACKET_SetBufferLength(bufHandle, len);

	RPC_PACKET_SetContext(INTERFACE_CSD, bufHandle, sSimId);

	if (RPC_RESULT_OK != RPC_PACKET_SendData(g_CsdClientId,
			INTERFACE_CSD, cid, bufHandle))
		VSP_DEBUG(DBG_ERROR, "vsp: FIFO is full.\n");

	return len;
}

/*******************************************************************************

	Function to control returning CP loop back data

	@param		void.
	@return   void.
	@note
	First check the available CP loop back data size.
	Then Send CP loop back data.

*******************************************************************************/
void CSD_CP_Lookback_Write(void)
{

	if (sCsdPendingBytes >= 160) {
		UInt32 dwWriteLen;
		UInt32 dwSentBytes = 0;
		UInt32 dwSentLen = 0;
		UInt8 *pBuf = sCsdTempBuf;

		/* Get buffer access */
		/*spin_lock_bh(&csLocker_cploopback);*/

		dwWriteLen = sCsdPendingBytes;

		while (dwWriteLen) {
			if (CFG_RPC_CSDDATA_PKT_SIZE < dwWriteLen)
				dwSentLen =
	CSD_CP_Loopback_Send((pBuf + dwSentBytes), CFG_RPC_CSDDATA_PKT_SIZE);
			else
	   dwSentLen = CSD_CP_Loopback_Send((pBuf + dwSentBytes), dwWriteLen);

			if (dwSentLen == 0) {
				/* When RPC_PACKET_AllocateBuffer failed, this
				may cause CSD_Send failed. */
				VSP_DEBUG(DBG_ERROR, "vsp: dwSentLen = 0\n");
				break;
			}

			dwWriteLen -= dwSentLen;
			dwSentBytes += dwSentLen;
		}

		sCsdPendingBytes = 0;

		/* Release buffer access */
		/*spin_unlock_bh(&csLocker_cploopback);*/
	}
}

#else /* CSD_LOOPBACK_TEST */

/*******************************************************************************

	Function to read downlink csd data

 @param pInBuf (in) point to one buffer that get downlink csd data.
	@return   read data length.
	@note
	Read downlink csd data from temp buffer(sCsdTempBuf).

*******************************************************************************/
UInt32 CSD_Read(UInt8 *pInBuf)
{
	UInt32 dwDataSize = 0;

	/* Get buffer access */
	spin_lock_bh(&csLocker);

	if (NULL == pInBuf) {
		sCsdPendingBytes = 0;
		spin_unlock_bh(&csLocker);
		return 0;
	}

	if (sCsdPendingBytes > 0) {
		memcpy(pInBuf, sCsdTempBuf, sCsdPendingBytes);
		dwDataSize = sCsdPendingBytes;
		sCsdPendingBytes = 0;
	}

	/* Release buffer access */
	spin_unlock_bh(&csLocker);
	return dwDataSize;
}

/*******************************************************************************

	Function to send uplink csd data

	@param pInBuf (in) point to one buffer that has uplink data to be sent.
	@return   sent data length.
	@note
	First Make sure that can get free bufffer from IPC.
	Then Send uplink data to IPC.

*******************************************************************************/
UInt32 CSD_Send(const void *pBuf, const UInt32 len)
{
	PACKET_BufHandle_t bufHandle = NULL;
	void *bufferPtr = NULL;
	UInt8 cid = 0;
	static UInt32 sSimCheckFlag = 1;

	if ((!(1 == sSimId || 2 == sSimId)) && sSimCheckFlag) {
		sSimCheckFlag = 0;
		VSP_DEBUG(DBG_ERROR, "vsp: sSimId is %d. Error!!!\n", sSimId);
	}

	if (0 == csdULframeNum)
		csdULframeNum++;

	cid = csdULframeNum++;

	/* Get ptr to buffer */
	bufHandle = RPC_PACKET_AllocateBuffer(INTERFACE_CSD, len, cid);
	if (!bufHandle) {
		VSP_DEBUG(DBG_DATA, "vsp: bufHandle is NULL.\n");
		return 0;
	}

	/* Copy data */
	bufferPtr = RPC_PACKET_GetBufferData(bufHandle);
	if (!bufferPtr) {
		VSP_DEBUG(DBG_ERROR, "vsp: bufferPtr is NULL.\n");
		return 0;
	}
	memcpy(bufferPtr, (char *)pBuf, len);

	/* Set the buffer len */
	RPC_PACKET_SetBufferLength(bufHandle, len);

	RPC_PACKET_SetContext(INTERFACE_CSD, bufHandle, sSimId);

	if (RPC_RESULT_OK != RPC_PACKET_SendData(g_CsdClientId, INTERFACE_CSD
			, cid, bufHandle))
		VSP_DEBUG(DBG_ERROR, "vsp: FIFO is full.\n");

	return len;
}
#endif /* CSD_LOOPBACK_TEST */

/*******************************************************************************

	Function callback to get csd data

	@param interfaceType (in) Interface the packet was received
	@param channel (in) Context id for INTERFACE_PACKET OR Call Index for
		INTERFACE_CSD (others interface is N/A)
 @param dataBufHandle (in) Buffer handle. Use RPC_PACKET_GetBufferData
		and RPC_PACKET_GetBufferLength to get the actual data
		pointer and length.
	@return
		RPC_RESULT_OK : success dataBuf will be released after the call
			returns. Client MUST make copy.
		RPC_RESULT_ERROR : failure, dataBuf will be released after
			the call returns. In addtion RPC MAY initiate flow
			control to slow the downlink packet data ( TBD )
		RPC_RESULT_PENDING : pending, The client decide to delay
			consuming packet,in which case the buffer will NOT be
			relased by RPC after function return.
			The Client needs to call RPC_PACKET_FreeBuffer to
			free buffer later
	@note
		The client can distinguish between different Primary PDP context
		session by channel for INTERFACE_PACKET.
	@note
		The data buffer is already mapped to calling thread process
		space ( virtual / kernal address space )

*******************************************************************************/
RPC_Result_t CSD_DataIndCB(PACKET_InterfaceType_t interfaceType
		, UInt8 channel, PACKET_BufHandle_t dataBufHandle)
{
	RPC_Result_t result = RPC_RESULT_ERROR ;

	if (interfaceType == INTERFACE_CSD) {
		UInt8 *data = NULL;
		UInt32 len = 0;

		/* Get buffer access */
#ifdef CSD_LOOPBACK_TEST
	spin_lock_bh(&csLocker_cploopback);
#else
	spin_lock_bh(&csLocker);
#endif /* CSD_LOOPBACK_TEST */

	data = (UInt8 *) RPC_PACKET_GetBufferData(dataBufHandle);
	len = (UInt32) RPC_PACKET_GetBufferLength(dataBufHandle);

	if (CSD_BUFFER_SIZE - sCsdPendingBytes >= len) {
		memcpy(&sCsdTempBuf[sCsdPendingBytes], data, len);
		sCsdPendingBytes += len;
		/*VSP_DEBUG(DBG_INFO, "vsp: len:%ld sCsdPendingBytes:%ld\n"
		, len, sCsdPendingBytes);*/
		wake_up_interruptible(&g_csd_wait);
	}

	RPC_PACKET_FreeBuffer(dataBufHandle);

#ifdef CSD_LOOPBACK_TEST
	CSD_CP_Lookback_Write();
#endif /* CSD_LOOPBACK_TEST */

/* Release buffer access */
#ifdef CSD_LOOPBACK_TEST
	spin_unlock_bh(&csLocker_cploopback);
#else
	spin_unlock_bh(&csLocker);
#endif

	result = RPC_RESULT_PENDING;
	}

	return result;
}

/*******************************************************************************

	Function callback to handle Flow control

	@param  event (in ) RPC_FLOW_START to resume flow control and
				RPC_FLOW_STOP to stop
	@param	channel (in) Context id for INTERFACE_PACKET OR Call Index for
				INTERFACE_CSD
	@return	None

	@note
	In VSP driver, this function is empty.

*******************************************************************************/
void CSD_FlowCtrlCB(RPC_FlowCtrlEvent_t event, UInt8 channel)
{
	return;
}

/* callback for RPC Ntf events */
void CSD_RpcNotificationCb(struct RpcNotificationEvent_t event)
{
	VSP_DEBUG(DBG_INFO, "CSD RPC Ntf event %d param %d interface %d\n",
		  (int)event.event, (int)event.param, (int)event.ifType);

	switch (event.event) {
	case RPC_CPRESET_EVT:
		/* for now, just ack as ready for reset... */
		if (RPC_CPRESET_START == event.param)
			RPC_PACKET_AckReadyForCPReset(0, INTERFACE_CSD);
		break;
	default:
		/* Unhandled event */
		break;
	}
}

/*******************************************************************************

 Function Name: vsp_open

 Description:  Use this function to open vsp driver

 Notes:

*******************************************************************************/
static int vsp_open(struct inode *inode, struct file *filp)
{
	VSP_DEBUG(DBG_INFO, "vsp: VSP driver open\n");

	if (0 == g_first_init) {
		g_CsdClientId = SYS_GenClientID();
		if (RPC_RESULT_OK !=
		    RPC_PACKET_RegisterDataInd(g_CsdClientId, INTERFACE_CSD,
					       CSD_DataIndCB, CSD_FlowCtrlCB,
					       CSD_RpcNotificationCb)) {
			VSP_DEBUG(DBG_ERROR, "vsp: RPC Register failed!!!\n");
			return 1;
		}

		VSP_DEBUG(DBG_ERROR, "%s: client ID[%d]\n", __func__
			, g_CsdClientId);
		g_first_init = 1;
	}
#ifdef CSD_LOOPBACK_TEST
	sAplookback_PendingBytes = 0;
#endif

	sCsdPendingBytes = 0;
	sPendingBytes = 0;
	filp->private_data = &g_csd_wait;

	return 0;
}

/*******************************************************************************

 Function Name: vsp_release

 Description:  Use this function to close vsp driver

 Notes:

*******************************************************************************/
static int vsp_release(struct inode *inode, struct file *filp)
{
	VSP_DEBUG(DBG_INFO, "vsp: VSP driver close\n");
	return 0;
}

/*******************************************************************************

 Function Name: vsp_read

 Description:  Use this function to read csd data

 Notes:

*******************************************************************************/
int vsp_read(struct file *filp, char __user *buf, size_t size, loff_t *offset)
{
	UInt32 dwInputLen = size;
	UInt32 dwRetSize = 0;
	UInt8 *ppBuf = sReadTempBuf;

	if (0 == size) {
		VSP_DEBUG(DBG_ERROR, "vsp: size = 0\n");
		return 0;
	}

	if (!buf) {
		VSP_DEBUG(DBG_ERROR, "vsp: buf is NULL\n");
		return 0;
	}

	if (sPendingBytes) {
		if (dwInputLen >= sPendingBytes) {
			memcpy(ppBuf, (UInt8 *)(stempBuf + sCopiedBytes)
				, sPendingBytes);
			dwRetSize = sPendingBytes;
			sCopiedBytes = 0;
			sPendingBytes = 0;
		}	else	{
			memcpy(ppBuf, (UInt8 *)(stempBuf + sCopiedBytes)
				, dwInputLen);
			sCopiedBytes += dwInputLen;
			sPendingBytes -= dwInputLen;
			dwRetSize = dwInputLen;
		}

		/* Count how much memory space remain in the output buffer */
		dwInputLen -= dwRetSize;
		ppBuf += dwRetSize;
	}

	while (dwInputLen >= CSD_DOWNLINK_DATA_LEN && sPendingBytes == 0 &&
				GetRxBytesAvailable() > 0)	{
		sPendingBytes = CSD_Read((UInt8 *)stempBuf);

		if (dwInputLen >= sPendingBytes) {
			memcpy(ppBuf, (UInt8 *)stempBuf, sPendingBytes);
			dwRetSize += sPendingBytes;
			ppBuf += sPendingBytes;
			dwInputLen -= sPendingBytes;
			sCopiedBytes = 0;
			sPendingBytes = 0;
		}	else {
			memcpy(ppBuf, (UInt8 *)stempBuf, dwInputLen);
			dwRetSize += dwInputLen;
			ppBuf += dwInputLen;
			sCopiedBytes = dwInputLen;
			sPendingBytes -= dwInputLen;
			dwInputLen = 0;
		}
	}

	if (0 == dwRetSize) {
		return 0;
	}	else if (dwRetSize > size) {
		VSP_DEBUG(DBG_ERROR, "vsp: dwRetSize:%ld > size:%d, Error!!!\n"
			, dwRetSize, size);
		if (0 != copy_to_user(buf, sReadTempBuf, size)) {
			VSP_DEBUG(DBG_ERROR, "vsp: copy_to_user Fail!!!\n");
			return 0;
		}
	}	else {
		if (0 != copy_to_user(buf, sReadTempBuf, dwRetSize)) {
			VSP_DEBUG(DBG_ERROR, "vsp: copy_to_user Fail!!!\n");
			return 0;
		}
	}

	return dwRetSize;
}

/*******************************************************************************

 Function Name: vsp_write

 Description:  Use this function to write csd data

 Notes:

*******************************************************************************/
int vsp_write(struct file *filp, const char __user *buf
				, size_t size, loff_t *offset)
{
	UInt32 dwWriteLen = size;
	UInt32 dwSentBytes = 0;
	UInt32 dwSentLen = 0;
	UInt8 *pBuf = sSendTempBuf;

	if (0 == size) {
		VSP_DEBUG(DBG_ERROR, "vsp: size = 0\n");
		return 0;
	}

	if (!buf) {
		VSP_DEBUG(DBG_ERROR, "vsp: buf is NULL\n");
		return 0;
	}

	if (copy_from_user(sSendTempBuf, buf, size) != 0) {
		VSP_DEBUG(DBG_ERROR, "vsp: copy_from_user is fail\n");
		return 0;
	}

	while (dwWriteLen)	{
		if (CFG_RPC_CSDDATA_PKT_SIZE < dwWriteLen)
			dwSentLen = CSD_Send((pBuf + dwSentBytes)
				, CFG_RPC_CSDDATA_PKT_SIZE);
		else
			dwSentLen = CSD_Send((pBuf + dwSentBytes), dwWriteLen);

		if (dwSentLen == 0) {
			/* When RPC_PACKET_AllocateBuffer failed,
			this may cause CSD_Send failed. */
			VSP_DEBUG(DBG_DATA, "vsp: dwSentLen = 0\n");
			break;
		}

		dwWriteLen -= dwSentLen;
		dwSentBytes += dwSentLen;
	}

	if (dwSentBytes != size)
		VSP_DEBUG(DBG_DATA, "vsp: size:%d != dwSentBytes:%ld\n"
			, size, dwSentBytes);

	return dwSentBytes;
}

/*******************************************************************************

 Function Name: vsp_ioctl

 Description:  Currently this function is empty

 Notes:
 Provide a interface for VT application can transmit SIM ID
 to VSP driver.

*******************************************************************************/
static long vsp_ioctl(struct file *filp, unsigned int cmd, UInt32 arg)
{
	int rc = -EINVAL;

	switch (cmd) {
	case VT_SIM1:
		sSimId = 1;
		rc = 0;
		break;

	case VT_SIM2:
		sSimId = 2;
		rc = 0;
		break;

	default:
		VSP_DEBUG(DBG_ERROR, "vsp: Unknow SIM ID:%d Error!!!\n"
			, _IOC_NR(cmd));
		break;
	}

	VSP_DEBUG(DBG_ERROR, "vsp: SIM ID:%d\n", sSimId);
	return rc;
}

static unsigned int vsp_poll(struct file *filp, poll_table *wait)
{
	int mask = 0;

	poll_wait(filp, &g_csd_wait, wait);

	spin_lock_bh(&csLocker);

	if ((0 < sCsdPendingBytes) || (0 < sPendingBytes))
		mask |= (POLLIN | POLLRDNORM);

	spin_unlock_bh(&csLocker);

	return mask;
}

static const struct file_operations vsp_ops = {
	.owner = THIS_MODULE,
	.open  = vsp_open,
	.read  = vsp_read,
	.write = vsp_write,
	.unlocked_ioctl = vsp_ioctl,
	.poll  = vsp_poll,
	.mmap	 = NULL,
	.release = vsp_release,
};

/*******************************************************************************

 Function Name: VSP_Init

 Description:   VSP initialise

 Notes:

*******************************************************************************/
int VSP_Init(void)
{
	sSimId = 0;
	sPendingBytes = 0;
	sCsdPendingBytes = 0;
	g_CsdClientId = 0;
	g_first_init = 0;

#ifdef CSD_LOOPBACK_TEST
	sAplookback_PendingBytes = 0;
	spin_lock_init(&csLocker_aploopback);
	spin_lock_init(&csLocker_cploopback);
#endif
	spin_lock_init(&csLocker);

	init_waitqueue_head(&g_csd_wait);

	return 0;
}

/*******************************************************************************

 Function Name: bcm_fuse_vsp_init_module

 Description:   Start VSP(virtual serial port) driver initialise

 Notes:

*******************************************************************************/
static int __init bcm_fuse_vsp_init_module(void)
{
	int ret = 0;
	struct device *drv;

	/* check for AP only boot mode
	if ( AP_ONLY_BOOT == get_ap_boot_mode() )	{
		VSP_DEBUG(DBG_INFO, "AP only; don't register driver\n");
		return ret;
	}*/

	ret = register_chrdev(BCM_VSP_MAJOR, "bcm_vsp", &vsp_ops);
	if (ret < 0) {
		VSP_DEBUG(DBG_ERROR, "vsp: register failed major %d\n",
			BCM_VSP_MAJOR);
		return ret;
	}

	vsp_class = class_create(THIS_MODULE, "bcm_vsp");
	if (IS_ERR(vsp_class)) {
		VSP_DEBUG(DBG_ERROR, "vsp: class_create failed\n");
		unregister_chrdev(BCM_VSP_MAJOR, "bcm_vsp");
		return PTR_ERR(vsp_class);
	}

	drv = device_create(vsp_class, NULL, MKDEV(BCM_VSP_MAJOR, 0)
		, NULL, "bcm_vsp");
	if (IS_ERR(drv)) {
		VSP_DEBUG(DBG_ERROR, "device_create failed\n");
		unregister_chrdev(BCM_VSP_MAJOR, "bcm_vsp");
		class_destroy(vsp_class);
		return PTR_ERR(drv);
	}

	VSP_DEBUG(DBG_INFO, "vsp: bcm_vsp driver(major %d) installed\n",
		BCM_VSP_MAJOR);

	/* Init VSP Driver */
	ret = VSP_Init();

	if (ret) {
		ret = -1;
		VSP_DEBUG(DBG_ERROR, "vsp: Initialise failed !!!\n");
	}

	return ret;
}

/*******************************************************************************

 Function Name: bcm_fuse_vsp_exit_module

 Description:   Unload VSP driver

 Notes:

*******************************************************************************/
static void __exit bcm_fuse_vsp_exit_module(void)
{
	device_destroy(vsp_class, MKDEV(BCM_VSP_MAJOR, 0));
	class_destroy(vsp_class);
	unregister_chrdev(BCM_VSP_MAJOR, "bcm_vsp");
	return;
}

module_init(bcm_fuse_vsp_init_module);
module_exit(bcm_fuse_vsp_exit_module);

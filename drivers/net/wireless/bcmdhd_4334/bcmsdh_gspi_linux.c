/*
 * BCMSDH Function Driver for the native gSPI driver in the Linux Kernel
 *
 * Copyright (C) 1999-2012, Broadcom Corporation
 * 
 *      Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed to you
 * under the terms of the GNU General Public License version 2 (the "GPL"),
 * available at http://www.broadcom.com/licenses/GPLv2.php, with the
 * following added to such license:
 * 
 *      As a special exception, the copyright holders of this software give you
 * permission to link this software with independent modules, and to copy and
 * distribute the resulting executable under terms of your choice, provided that
 * you also meet, for each linked independent module, the terms and conditions of
 * the license of that module.  An independent module is a module which is not
 * derived from this software.  The special exception does not apply to any
 * modifications of the software.
 * 
 *      Notwithstanding the above, under no circumstances may you combine this
 * software in any way with any other Broadcom software provided under a license
 * other than the GPL, without Broadcom's express prior written consent.
 *
 * $Id: bcmpcispi.c 241182 2011-02-17 21:50:03Z $
 */

#include <typedefs.h>
#include <bcmutils.h>

#include <bcmsdbus.h>	/* bcmsdh to/from specific controller APIs */
#include <sdiovar.h>	/* to get msglevel bit values */
#include <bcmspibrcm.h>

#include <linux/spi/spi.h>

static struct spi_device *gBCMSPI = NULL;

extern int bcmsdh_probe(struct device *dev);
extern int bcmsdh_remove(struct device *dev);

static int bcmsdh_spi_probe(struct spi_device *spi_dev)
{
	int ret = 0;

	gBCMSPI = spi_dev;
	ret = bcmsdh_probe(&spi_dev->dev);

	return ret;
}

static int  bcmsdh_spi_remove(struct spi_device *spi_dev)
{
	int ret = 0;

	ret = bcmsdh_remove(&spi_dev->dev);
	gBCMSPI = NULL;

	return ret;
}

static struct spi_driver bcmsdh_spi_driver = {
	.probe		= bcmsdh_spi_probe,
	.remove		= bcmsdh_spi_remove,
	.driver		= {
		.name = "wlan_spi",
		.bus    = &spi_bus_type,
		.owner  = THIS_MODULE,
		},
};

/*
 * module init
*/
int spi_function_init(void)
{
	int error = 0;
	sd_trace(("bcmsdh_gspi: %s Enter\n", __FUNCTION__));

	error = spi_register_driver(&bcmsdh_spi_driver);

	return error;
}

/*
 * module cleanup
*/
void spi_function_cleanup(void)
{
	sd_trace(("%s Enter\n", __FUNCTION__));
	spi_unregister_driver(&bcmsdh_spi_driver);
}

/* Register with Linux for interrupts */
int
spi_register_irq(sdioh_info_t *sd, uint irq)
{
	return SUCCESS;
}

/* Free Linux irq */
void
spi_free_irq(uint irq, sdioh_info_t *sd)
{
}

int
spi_osinit(sdioh_info_t *sd)
{
	return BCME_OK;
}

void
spi_osfree(sdioh_info_t *sd)
{
}

/* Interrupt enable/disable */
SDIOH_API_RC
sdioh_interrupt_set(sdioh_info_t *sd, bool enable)
{
	return SDIOH_API_RC_SUCCESS;
}

/* Protect against reentrancy (disable device interrupts while executing) */
void
spi_lock(sdioh_info_t *sd)
{
}

/* Enable client interrupt */
void
spi_unlock(sdioh_info_t *sd)
{
}

int bcmgspi_dump = 0;		/* Set to dump complete trace of all SPI bus transactions */

static void
hexdump(char *pfx, unsigned char *msg, int msglen)
{
	int i, col;
	char buf[80];

	ASSERT(strlen(pfx) + 49 <= sizeof(buf));

	col = 0;

	for (i = 0; i < msglen; i++, col++) {
		if (col % 16 == 0)
			strcpy(buf, pfx);
		sprintf(buf + strlen(buf), "%02x", msg[i]);
		if ((col + 1) % 16 == 0)
			printf("%s\n", buf);
		else
			sprintf(buf + strlen(buf), " ");
	}

	if (col % 16 != 0)
		printf("%s\n", buf);
}

/* Send/Receive an SPI Packet */
void
spi_sendrecv(sdioh_info_t *sd, uint8 *msg_out, uint8 *msg_in, int msglen)
{
	int write = 0;
	int tx_len = 0;
	struct spi_message msg;
	struct spi_transfer t[2];

	spi_message_init(&msg);
	memset(t, 0, 2*sizeof(struct spi_transfer));

	if (sd->wordlen == 2)
		write = msg_out[2] & 0x80;
	if (sd->wordlen == 4)
		write = msg_out[0] & 0x80;

	if (bcmgspi_dump) {
		hexdump(" OUT: ", msg_out, msglen);
	}

	tx_len = write ? msglen-4 : 4;

	t[0].tx_buf = (char *)&msg_out[0];
	t[0].rx_buf = 0;
	t[0].len = tx_len;

	spi_message_add_tail(&t[0], &msg);

	t[1].rx_buf = (char *)&msg_in[tx_len];
	t[1].tx_buf = 0;
	t[1].len = msglen-tx_len;

	spi_message_add_tail(&t[1], &msg);
	spi_sync(gBCMSPI, &msg);

	if (bcmgspi_dump) {
		hexdump(" IN  : ", msg_in, msglen);
	}
}

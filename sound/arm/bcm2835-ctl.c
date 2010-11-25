/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 */

#include <linux/platform_device.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/jiffies.h>
#include <linux/slab.h>
#include <linux/time.h>
#include <linux/wait.h>
#include <linux/delay.h>
#include <linux/moduleparam.h>
#include <linux/sched.h>

#include <sound/core.h>
#include <sound/control.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/rawmidi.h>
#include <sound/initval.h>
#include <sound/tlv.h>

#include "bcm2835.h"

static int snd_bcm2835_ctl_info(struct snd_kcontrol * kcontrol,
        struct snd_ctl_elem_info * uinfo)
{
	if (kcontrol->private_value == PCM_PLAYBACK_VOLUME)
	{
		uinfo->type					= SNDRV_CTL_ELEM_TYPE_INTEGER;
		uinfo->count				= 1;
		uinfo->value.integer.min	= -10240;
		uinfo->value.integer.max	= 2303;
	}
	else if (kcontrol->private_value == PCM_PLAYBACK_MUTE)
	{
		uinfo->type					= SNDRV_CTL_ELEM_TYPE_INTEGER;
		uinfo->count				= 1;
		uinfo->value.integer.min	= 0;
		uinfo->value.integer.max	= 1;
	}
	else if (kcontrol->private_value == PCM_PLAYBACK_DEVICE)
	{
		uinfo->type					= SNDRV_CTL_ELEM_TYPE_INTEGER;
		uinfo->count				= 1;
		uinfo->value.integer.min	= AUDIO_DEST_LOCAL;
		uinfo->value.integer.max	= AUDIO_DEST_HDMI;
	}

	return 0;
}

static int snd_bcm2835_ctl_get(struct snd_kcontrol * kcontrol, struct snd_ctl_elem_value * ucontrol)
{
	struct bcm2835_chip *chip = snd_kcontrol_chip(kcontrol);

	if (kcontrol->private_value == PCM_PLAYBACK_VOLUME)
	{
		ucontrol->value.integer.value[0] = chip->volume;
	}
	else if (kcontrol->private_value == PCM_PLAYBACK_MUTE)
	{
		ucontrol->value.integer.value[0] = chip->mute;
	}
	else if (kcontrol->private_value == PCM_PLAYBACK_DEVICE)
	{
		ucontrol->value.integer.value[0] = chip->dest;
	}
	return 0;
}

static int snd_bcm2835_ctl_put(struct snd_kcontrol * kcontrol, struct snd_ctl_elem_value * ucontrol)
{
	int changed = 0;
	struct bcm2835_chip *chip = snd_kcontrol_chip(kcontrol);

	if (kcontrol->private_value == PCM_PLAYBACK_VOLUME)
	{
		if (chip->mute)
		{
			chip->mute = 0;
			changed = 1;
		}

		if (changed || (ucontrol->value.integer.value[0] != chip->volume))
		{
			int atten;

			chip->volume = ucontrol->value.integer.value[0];
			changed = 1;

			atten = -((chip->volume << 8) / 100);

			writel(atten, chip->reg_base + AUDIO_VOLUME_OFFSET);
	}

	}
	else if (kcontrol->private_value == PCM_PLAYBACK_MUTE)
	{
	  // Not implemented
		if (ucontrol->value.integer.value[0] != chip->mute)
		{
			chip->mute = ucontrol->value.integer.value[0];
			changed = 0;
		}
	}
	else if (kcontrol->private_value == PCM_PLAYBACK_DEVICE)
	{
		if (ucontrol->value.integer.value[0] != chip->dest)
		{
			chip->dest= ucontrol->value.integer.value[0];
			changed = 1;
			writel(chip->dest, chip->reg_base + AUDIO_DESTINATION_OFFSET);
		}
	}

	if (changed) {
		ipc_notify_vc_event(chip->irq);
	}
	return changed;
}

static DECLARE_TLV_DB_SCALE(snd_bcm2835_db_scale, -10240, 1, 1);

static struct snd_kcontrol_new snd_bcm2835_ctl[] __devinitdata =
{
	{
		.iface = SNDRV_CTL_ELEM_IFACE_MIXER,
		.name  = "PCM Playback Volume",
		.index = 0,
		.access= SNDRV_CTL_ELEM_ACCESS_READWRITE | SNDRV_CTL_ELEM_ACCESS_TLV_READWRITE,
		.private_value = PCM_PLAYBACK_VOLUME,
		.info  = snd_bcm2835_ctl_info,
		.get   = snd_bcm2835_ctl_get,
		.put   = snd_bcm2835_ctl_put,
		.count = 1,
        .tlv = { .p = snd_bcm2835_db_scale }
	},
	{
		.iface = SNDRV_CTL_ELEM_IFACE_MIXER,
		.name  = "PCM Playback Switch",
		.index = 0,
		.access= SNDRV_CTL_ELEM_ACCESS_READWRITE,
		.private_value = PCM_PLAYBACK_MUTE,
		.info  = snd_bcm2835_ctl_info,
		.get   = snd_bcm2835_ctl_get,
		.put   = snd_bcm2835_ctl_put,
		.count = 1,
	},
	{
		.iface = SNDRV_CTL_ELEM_IFACE_MIXER,
		.name  = "PCM Playback Route",
		.index = 0,
		.access= SNDRV_CTL_ELEM_ACCESS_READWRITE,
		.private_value = PCM_PLAYBACK_DEVICE,
		.info  = snd_bcm2835_ctl_info,
		.get   = snd_bcm2835_ctl_get,
		.put   = snd_bcm2835_ctl_put,
		.count = 1,
	},
};

int __devinit snd_bcm2835_new_ctl(bcm2835_chip_t *chip)
{
	int err;
	unsigned int idx;

	strcpy(chip->card->mixername, "Broadcom Mixer");
	for (idx = 0; idx < ARRAY_SIZE(snd_bcm2835_ctl); idx++)
	{
		err = snd_ctl_add(chip->card, snd_ctl_new1(&snd_bcm2835_ctl[idx], chip));
		if (err < 0)
			return err;
	}
	return 0;
}

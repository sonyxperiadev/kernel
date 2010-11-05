#include <linux/platform_device.h>

#include <linux/init.h>
#include <linux/slab.h>

#include "bcm2835.h"

/* module parameters (see "Module Parameters") */
/* SNDRV_CARDS: maximum number of cards supported by this module */
static int index[SNDRV_CARDS] = SNDRV_DEFAULT_IDX;
static char *id[SNDRV_CARDS] = SNDRV_DEFAULT_STR;
static int enable[SNDRV_CARDS] = SNDRV_DEFAULT_ENABLE_PNP;


static int snd_bcm2835_free(bcm2835_chip_t *chip)
{
	kfree(chip);
	return 0;
}


/* component-destructor
 * (see "Management of Cards and Components")
 */
static int snd_bcm2835_dev_free(struct snd_device *device)
{
	return snd_bcm2835_free(device->device_data);
}


/* chip-specific constructor
 * (see "Management of Cards and Components")
 */
static int __devinit snd_bcm2835_create(struct snd_card *card,
        struct platform_device *pdev,
        bcm2835_chip_t **rchip)
{
	bcm2835_chip_t *chip;
	int err;
	static struct snd_device_ops ops = {
		.dev_free = snd_bcm2835_dev_free,
	};

	*rchip = NULL;

	chip = kzalloc(sizeof(*chip), GFP_KERNEL);
	if (chip == NULL)
		return -ENOMEM;

	chip->card = card;

	err = snd_device_new(card, SNDRV_DEV_LOWLEVEL, chip, &ops);
	if (err < 0) {
		snd_bcm2835_free(chip);
		return err;
	}

	snd_card_set_dev(card, &pdev->dev);

	*rchip = chip;
	return 0;
}

static int __devinit snd_bcm2835_alsa_probe(struct platform_device *pdev)
{
	static int dev;
	struct snd_card *card;
	bcm2835_chip_t *chip;
	struct resource *r;
	int err;

	printk("############ PROBING FOR bcm2835 ALSA CARD %d %d###############\n", dev, enable[dev]);

	if (dev >= SNDRV_CARDS)
		return -ENODEV;
	if (!enable[dev]) {
		dev++;
		return -ENOENT;
	}

	err = snd_card_create(index[dev], id[dev], THIS_MODULE, 0, &card);
	if (err < 0)
		goto out;

	printk("Creating 2...\n");

	err = snd_bcm2835_create(card, pdev, &chip);
	if (err < 0) {
		printk(KERN_ERR"Failed to create bcm2835 chip\n");
		goto out_bcm2835_create;
	}

	chip->card = card;

	printk("Creating 3.\n");
	strcpy(card->driver, "BRCM bcm2835 ALSA Driver");
	strcpy(card->shortname, "bcm2835 ALSA");
	sprintf(card->longname, "%s", card->shortname);

	r = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (r == NULL) {
		printk(KERN_ERR"Failed to get bcm2835_ALSA mem resource\n");
		err = -ENXIO;
		goto out_get_resource;

	}

	/* Setup the register base for this chip */
	chip->reg_base = (void __iomem *)r->start;

	/* Request memory region ?? - Dont do it for now  : ssp */

	/* get irq */
	chip->irq = platform_get_irq(pdev, 0);
	if (chip->irq < 0) {
		printk(KERN_ERR"Unable to get IRQ resource for BCM2835 ALSA device\n");
		err = -ENXIO;
		goto out_get_irq;
	}

	err = snd_bcm2835_new_pcm(chip);
	if (err < 0) {
		printk(KERN_ERR"Failed to create new BCM2835 pcm device\n");
		goto out_bcm2835_new_pcm;
	}

	printk("Creating 4.\n");

	err = snd_card_register(card);
	if (err < 0) {
		printk(KERN_ERR"Failed to register bcm2835 ALSA card \n");
		goto out_card_register;
	}

	platform_set_drvdata(pdev, card);
	dev++;

	printk("bcm2835 ALSA CARD CREATED!\n");

	return 0;

out_card_register:
out_bcm2835_new_pcm:
out_get_irq:
out_get_resource:
out_bcm2835_create:
	snd_card_free(card);
out:
	printk(KERN_ERR"BCM2835 ALSA Probe failed !!\n");
	return err;
}

static int snd_bcm2835_alsa_remove(struct platform_device *pdev)
{
	snd_card_free(platform_get_drvdata(pdev));
	platform_set_drvdata(pdev, NULL);
	return 0;
}


#ifdef CONFIG_PM
static int snd_bcm2835_alsa_suspend(struct platform_device *pdev, pm_message_t state)
{
	return 0;
}

static int snd_bcm2835_alsa_resume(struct platform_device *pdev)
{
	return 0;
}

#endif

static struct platform_driver bcm2835_alsa_driver =
{
	.probe      = snd_bcm2835_alsa_probe,
	.remove     = snd_bcm2835_alsa_remove,
#ifdef CONFIG_PM
	.suspend    = snd_bcm2835_alsa_suspend,
	.resume     = snd_bcm2835_alsa_resume,
#endif
	.driver     = {
		.name   = "bcm2835_AUDO",
		.owner  = THIS_MODULE,
	},
};


static int __devinit bcm2835_alsa_device_init(void)
{
	int err;

	err = platform_driver_register(&bcm2835_alsa_driver);
	if (err) {
		printk("Error registering bcm2835_alsa_driver %d .\n", err);
		return err;
	}

	printk(KERN_INFO"### BCM2835 ALSA driver init %s ### \n",err ? "FAILED": "OK");

	return err;
}



static void __devexit bcm2835_alsa_device_exit(void)
{
    platform_driver_unregister(&bcm2835_alsa_driver);
}

late_initcall(bcm2835_alsa_device_init);
module_exit(bcm2835_alsa_device_exit);

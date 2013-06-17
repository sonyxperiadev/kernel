/*  Copyright (c) 2011  Bosch Sensortec GmbH
    Copyright (c) 2011  Unixphere

    Based on:
    BMP085 driver, bmp085.c
    Copyright (c) 2010  Christoph Mair <christoph.mair@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include <linux/module.h>
#include <linux/spi/spi.h>
#include <linux/input.h>
#include "linux/bmp18x.h"

static int bmp18x_spi_write_byte(void *client, u8 reg, u8 value)
{
	u8 data[2] = {reg, value};
	return spi_write(client, data, 2);
}

static int bmp18x_spi_read_block(void *client, u8 reg, int len, char *buf)
{
	int rc = bmp18x_spi_write_byte(client, reg, 0);
	if (rc < 0)
		return rc;

	return spi_read(client, buf, len);
}

static int bmp18x_spi_read_byte(void *client, u8 reg)
{
	u8 data;
	int rc = bmp18x_spi_write_byte(client, reg, 0);
	if (rc < 0)
		return rc;

	rc = spi_read(client, &data, 1);
	if (rc < 0)
		return rc;

	return data;
}

static const struct bmp18x_bus_ops bmp18x_spi_bus_ops = {
	.bus_type	= BUS_SPI,
	.read_block	= bmp18x_spi_read_block,
	.read_byte	= bmp18x_spi_read_byte,
	.write_byte	= bmp18x_spi_write_byte
};

static int __devinit bmp18x_spi_probe(struct spi_device *client)
{
	int rc;
	struct bmp18x_data_bus data_bus = {
		.bops = &bmp18x_spi_bus_ops,
		.client = client
	};

	client->bits_per_word = 8;
	rc = spi_setup(client);
	if (rc < 0) {
		dev_err(&client->dev, "spi_setup failed!\n");
		return rc;
	}

	return bmp18x_probe(&client->dev, &data_bus);
}

static void bmp18x_spi_shutdown(struct spi_device *client)
{
	bmp18x_disable(&client->dev);
}

static int bmp18x_spi_remove(struct spi_device *client)
{
	return bmp18x_remove(&client->dev);
}

#ifdef CONFIG_PM
static int bmp18x_spi_suspend(struct device *dev)
{
	return bmp18x_disable(dev);
}

static int bmp18x_spi_resume(struct device *dev)
{
	return bmp18x_enable(dev);
}

static const struct dev_pm_ops bmp18x_spi_pm_ops = {
	.suspend	= bmp18x_spi_suspend,
	.resume		= bmp18x_spi_resume
};
#endif

static const struct spi_device_id bmp18x_id[] = {
	{ BMP18X_NAME, 0 },
	{ }
};
MODULE_DEVICE_TABLE(spi, bmp18x_id);

static struct spi_driver bmp18x_spi_driver = {
	.driver = {
		.owner	= THIS_MODULE,
		.name	= BMP18X_NAME,
#ifdef CONFIG_PM
		.pm	= &bmp18x_spi_pm_ops,
#endif
	},
	.id_table	= bmp18x_id,
	.probe		= bmp18x_spi_probe,
	.shutdown	= bmp18x_spi_shutdown,
	.remove		= __devexit_p(bmp18x_spi_remove)
};

static int __init bmp18x_spi_init(void)
{
	return spi_register_driver(&bmp18x_spi_driver);
}

static void __exit bmp18x_spi_exit(void)
{
	spi_unregister_driver(&bmp18x_spi_driver);
}


MODULE_AUTHOR("Eric Andersson <eric.andersson@unixphere.com>");
MODULE_DESCRIPTION("BMP18X SPI bus driver");
MODULE_LICENSE("GPL");

module_init(bmp18x_spi_init);
module_exit(bmp18x_spi_exit);

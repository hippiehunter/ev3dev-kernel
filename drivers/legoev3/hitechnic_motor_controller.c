/*
 * HiTechnic motor controller device driver for LEGO Mindstorms EV3
 *
 * Copyright (C) 2014 Jeff Greene<jeff.greene@synergex.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.

 * This program is distributed "as is" WITHOUT ANY WARRANTY of any
 * kind, whether express or implied; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/device.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/bug.h>
#include <linux/i2c-legoev3.h>
#include <linux/legoev3/legoev3_ports.h>
#include <linux/legoev3/ev3_input_port.h>
#include <linux/legoev3/nxt_i2c_sensor.h>

#define VENDOR_ID	"HiTechnc"
#define PRODUCT_ID	"MotorCon"


struct hit_mc_data {
	struct i2c_client *client;
	struct legoev3_port_device *in_port;
	char fw_ver[ID_STR_LEN + 1];
};


static int __devinit hit_mc_probe(struct i2c_client *client,
				  const struct i2c_device_id *id)
{
	struct hit_mc_data *hit_mc;
	struct i2c_legoev3_platform_data *pdata =
					client->adapter->dev.platform_data;
	int err;

	if (WARN_ON(!pdata))
		return -EINVAL;
	if (WARN_ON(!pdata->in_port))
		return -EINVAL;
	hit_mc = kzalloc(sizeof(struct hit_mc_data), GFP_KERNEL);
	if (!hit_mc)
		return -ENOMEM;

	hit_mc->client = client;
	hit_mc->in_port = pdata->in_port;

	nxt_i2c_read_string(client, FIRMWARE_REG, hit_mc->fw_ver, ID_STR_LEN);

	ev3_input_port_set_pin1_out(hit_mc->in_port, 1);
	i2c_set_clientdata(client, hit_mc);
	dev_info(&client->dev, "HiTechnic motor controller registered as '%s'\n",
		 dev_name(&client->dev));

	return 0;
}

static int __devexit hit_mc_remove(struct i2c_client *client)
{
	struct hit_mc_data *hit_mc = i2c_get_clientdata(client);

	dev_info(&client->dev, "HiTechnic motor controller '%s' removed.\n",
		 dev_name(&client->dev));
	ev3_input_port_set_pin1_out(hit_mc->in_port, 0);
	kfree(hit_mc);

	return 0;
}

static int hit_mc_detect(struct i2c_client *client, struct i2c_board_info *info)
{
	int ret;

	ret = nxt_i2c_test_id_string(client, VENDOR_ID_REG, VENDOR_ID);
	if (ret)
	{
		char ch[9];
		nxt_i2c_read_string(client, VENDOR_ID_REG, &ch, 9);
		printk("failed i2c test with VENDOR_ID %s\n", ch);
		nxt_i2c_read_string(client, DEVICE_ID_REG, &ch, 9);
		printk("failed i2c test with DEVICE_ID %s\n", ch);
		return -ENODEV;
	}

	msleep(1);
	ret = nxt_i2c_test_id_string(client, DEVICE_ID_REG, PRODUCT_ID);
	if (ret)
	{
		char ch[9];
		nxt_i2c_read_string(client, DEVICE_ID_REG, &ch, 9);
		printk("failed i2c test with DEVICE_ID %s\n", ch);
		return -ENODEV;
	}

	sprintf(info->type, "hit-mc");
	return 0;
}

static struct i2c_device_id hit_mc_idtable[] = {
	{ "hit-mc", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, hit_mc_idtable);

static struct i2c_driver hit_mc_driver = {
	.driver = {
		.name	= "hitechnic-motor-controller",
	},
	.id_table	= hit_mc_idtable,
	.probe		= hit_mc_probe,
	.remove		= __devexit_p(hit_mc_remove),
	.class		= I2C_CLASS_LEGOEV3,
	.detect		= hit_mc_detect,
	.address_list	= I2C_ADDRS(0x10),
};
module_i2c_driver(hit_mc_driver);

MODULE_DESCRIPTION("HiTechnic motor controller device driver for LEGO Mindstorms EV3");
MODULE_AUTHOR("Jeff Greene <jeff.greene@synergex.com>");
MODULE_LICENSE("GPL");
MODULE_ALIAS("legoev3:hit-i2c-mc");

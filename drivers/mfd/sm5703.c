// SPDX-License-Identifier: GPL-2.0-only

#include <linux/err.h>
#include <linux/delay.h>
#include <linux/gpio/consumer.h>
#include <linux/i2c.h>
#include <linux/mfd/core.h>
#include <linux/mfd/sm5703.h>
#include <linux/module.h>
#include <linux/of_device.h>
#include <linux/regmap.h>

static const struct mfd_cell sm5703_devs[] = {
	{ .name = "sm5703-regulator", },
};

static const struct regmap_config sm5703_regmap_config = {
	.reg_bits	= 8,
	.val_bits	= 8,
};

static int sm5703_i2c_probe(struct i2c_client *i2c,
			    const struct i2c_device_id *id)
{
	struct sm5703_dev *sm5703;
	struct device *dev = &i2c->dev;
	unsigned int dev_id;
	int ret;

	sm5703 = devm_kzalloc(dev, sizeof(*sm5703), GFP_KERNEL);
	if (!sm5703)
		return -ENOMEM;

	i2c_set_clientdata(i2c, sm5703);
	sm5703->dev = dev;

	sm5703->regmap = devm_regmap_init_i2c(i2c, &sm5703_regmap_config);
	if (IS_ERR(sm5703->regmap))
		return dev_err_probe(dev, PTR_ERR(sm5703->regmap),
				     "Failed to allocate the register map\n");

	sm5703->reset_gpio = devm_gpiod_get(dev, "reset", GPIOD_OUT_HIGH);
	if (IS_ERR(sm5703->reset_gpio))
		return dev_err_probe(dev, PTR_ERR(sm5703->reset_gpio), "Cannot get reset GPIO\n");

	gpiod_set_value_cansleep(sm5703->reset_gpio, 1);
	msleep(20);

	ret = regmap_read(sm5703->regmap, SM5703_DEVICE_ID, &dev_id);
	if (ret)
		return dev_err_probe(dev, -ENODEV, "Device not found\n");

	ret = devm_mfd_add_devices(sm5703->dev, -1, sm5703_devs,
				   ARRAY_SIZE(sm5703_devs), NULL, 0, NULL);
	if (ret)
		return dev_err_probe(dev, ret, "Failed to add child devices\n");

	return 0;
}

static const struct of_device_id sm5703_of_match[] = {
	{ .compatible = "siliconmitus,sm5703", },
	{ }
};
MODULE_DEVICE_TABLE(of, sm5703_of_match);

static struct i2c_driver sm5703_driver = {
	.driver = {
		.name = "sm5703",
		.of_match_table = sm5703_of_match,
	},
	.probe = sm5703_i2c_probe,
};
module_i2c_driver(sm5703_driver);

MODULE_DESCRIPTION("Silicon Mitus SM5703 multi-function device driver");
MODULE_AUTHOR("Markuss Broks <markuss.broks@gmail.com>");
MODULE_LICENSE("GPL");

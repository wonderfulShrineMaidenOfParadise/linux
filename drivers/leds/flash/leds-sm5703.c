#include <linux/delay.h>
#include <linux/gpio/consumer.h>
#include <linux/i2c.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/property.h>
#include <linux/regmap.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/of_regulator.h>
#include <linux/of_gpio.h>
#include <linux/platform_device.h>
#include <linux/led-class-flash.h>

#include <linux/mfd/sm5703.h>

#define SM5703_FLASH_CURRENT(mA) mA<700?(((mA-300)/25) & 0x1f):(((((mA-700)/50)+0x0F)) & 0x1f)
#define SM5703_MOVIE_CURRENT(mA) (((mA - 10) /10) & 0x1f)

struct sm5703_fled {
    struct device* dev;
	struct device_node *child_node;
	struct led_classdev_flash fled_cdev;

    struct sm5703_dev* sm5703;

    struct gpio_desc *torch_gpio;
    struct gpio_desc *flash_gpio;
};

static struct sm5703_fled *fled_cdev_to_led(
				struct led_classdev_flash *fled_cdev)
{
	return container_of(fled_cdev, struct sm5703_fled, fled_cdev);
}

struct sm5703_led_config_data {
	/* maximum LED current in movie mode */
	u32 movie_max_microamp;
	/* maximum LED current in flash mode */
	u32 flash_max_microamp;
};

static int sm5703_parse_dt(struct sm5703_fled *fled, struct device *dev,
			    struct sm5703_led_config_data *cfg)
{
	struct device_node *np = dev_of_node(dev);
	int ret;

	if (!dev_of_node(dev))
		return -ENXIO;

	fled->torch_gpio = devm_gpiod_get_optional(dev, "torch", GPIOD_ASIS);
	if (IS_ERR(fled->torch_gpio)) {
		ret = PTR_ERR(fled->torch_gpio);
		dev_err(dev, "cannot get torch gpio %d\n", ret);
		return ret;
	}

	fled->flash_gpio = devm_gpiod_get_optional(dev, "flash", GPIOD_ASIS);
	if (IS_ERR(fled->flash_gpio)) {
		ret = PTR_ERR(fled->flash_gpio);
		dev_err(dev, "cannot get flash gpio %d\n", ret);
		return ret;
	}

	fled->child_node = of_get_next_available_child(np, NULL);
	if (!fled->child_node) {
		dev_err(dev, "No DT child node found for connected LED.\n");
		return -EINVAL;
	}

	ret = of_property_read_u32(fled->child_node, "led-max-microamp",
				   &cfg->movie_max_microamp);
	if (ret) {
		dev_err(dev, "failed to parse led-max-microamp\n");
		goto err_parse_dt;
	}

	ret = of_property_read_u32(fled->child_node, "flash-max-microamp",
				   &cfg->flash_max_microamp);
	if (ret) {
		dev_err(dev, "failed to parse flash-max-microamp\n");
		goto err_parse_dt;
	}

err_parse_dt:
	of_node_put(fled->child_node);
	return ret;
}

static int sm5703_led_brightness_set(struct led_classdev *led_cdev,
				       enum led_brightness brightness) {
    int ret;

	struct led_classdev_flash *fled_cdev = lcdev_to_flcdev(led_cdev);
	struct sm5703_fled *fled = fled_cdev_to_led(fled_cdev);

    if (brightness == 0) {
        ret = regmap_clear_bits(fled->sm5703->regmap, SM5703_FLEDCNTL1, SM5703_FLEDEN_MASK);
        if (ret)
            return ret;

        ret = regmap_update_bits(fled->sm5703->regmap, SM5703_REG_CNTL, SM5703_OPERATION_MODE_MASK,
                                SM5703_OPERATION_MODE_CHARGING_ON);
        if (ret)
            return ret;
    }
    else {
        ret = regmap_write(fled->sm5703->regmap, SM5703_VBUSCNTL, 0x8);
        if (ret)
            return ret;

        ret = regmap_update_bits(fled->sm5703->regmap, SM5703_REG_CNTL, SM5703_OPERATION_MODE_MASK,
                                SM5703_OPERATION_MODE_FLASH_BOOST_MODE);
        if (ret)
            return ret;

        ret = regmap_set_bits(fled->sm5703->regmap, SM5703_FLEDCNTL6, SM5703_BSTOUT_4P5);
        if (ret)
            return ret;

        ret = regmap_update_bits(fled->sm5703->regmap, SM5703_FLEDCNTL4, SM5703_IFLED_MASK, brightness);
        if (ret)
            return ret;

        ret = regmap_set_bits(fled->sm5703->regmap, SM5703_FLEDCNTL1, SM5703_FLEDEN_MOVIE_MODE);
        if (ret)
            return ret;
    }

    return 0;
}

static int sm5703_flash_configure (struct sm5703_fled *fled, struct sm5703_led_config_data *cfg) {
    int ret;

    ret = regmap_write(fled->sm5703->regmap, SM5703_FLEDCNTL1, 0x1C);
    if (ret) {
        return dev_err_probe(fled->dev, ret, "Failed to set configuration 1\n");
    }

    ret = regmap_write(fled->sm5703->regmap, SM5703_FLEDCNTL2, 0x94);
    if (ret) {
        return dev_err_probe(fled->dev, ret, "Failed to set configuration 2\n");
    }
    
    return 0;
}

static int sm5703_flash_probe (struct platform_device *pdev) {
	struct device* dev = &pdev->dev;
	struct led_classdev *led_cdev;
	struct led_classdev_flash *fled_cdev;
	struct led_init_data init_data = {};
	struct sm5703_led_config_data led_cfg;
    struct sm5703_fled *fled;
	int ret;

    fled = devm_kzalloc(dev, sizeof(*fled), GFP_KERNEL);
	if (!fled)
		return -ENOMEM;

    fled_cdev = &fled->fled_cdev;
	led_cdev = &fled_cdev->led_cdev;

	fled->sm5703 = dev_get_drvdata(pdev->dev.parent);
    fled->dev = dev;

    dev_err(dev, "flash probe start\n");

	ret = sm5703_parse_dt(fled, dev, &led_cfg);
	if (ret)
		return ret;

    ret = sm5703_flash_configure(fled, &led_cfg);
    if (ret) return ret;

	led_cdev->max_brightness = SM5703_MOVIE_CURRENT(led_cfg.movie_max_microamp / 1000) & 0x1f;
	led_cdev->brightness_set_blocking = sm5703_led_brightness_set;
	led_cdev->flags |= LED_CORE_SUSPENDRESUME | LED_DEV_CAP_FLASH;

	init_data.fwnode = &fled->child_node->fwnode;

    platform_set_drvdata(pdev, fled);

	ret = devm_led_classdev_register_ext(dev, led_cdev, &init_data);
	if (ret) {
		dev_err(&pdev->dev, "can't register LED: %d\n", ret);
		return ret;
	}

	return 0;
}

static const struct platform_device_id sm5703_flash_id[] = {
	{ "sm5703-flash", },
	{ }
};
MODULE_DEVICE_TABLE(platform, sm5703_flash_id);

static struct platform_driver sm5703_flash_driver = {
	.driver = {
		.name = "sm5703-flash",
	},
	.probe	= sm5703_flash_probe,
	.id_table	= sm5703_flash_id,
};

module_platform_driver(sm5703_flash_driver);

MODULE_DESCRIPTION("Samsung SM5703");
MODULE_AUTHOR("Samsung Electronics");
MODULE_LICENSE("GPL");

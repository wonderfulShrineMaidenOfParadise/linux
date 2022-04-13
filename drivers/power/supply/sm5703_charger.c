#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/property.h>
#include <linux/of_gpio.h>
#include <linux/power_supply.h>
#include <linux/regmap.h>
#include <linux/platform_device.h>
#include <linux/mfd/sm5703.h>
#include <linux/property.h>

struct sm5703_charger {
    struct platform_device *pdev;
    struct sm5703_dev *sm5703;
    struct power_supply *psy;
    bool use_autoset, use_autostop;
};

static enum power_supply_property sm5703_charger_props[] = {
    POWER_SUPPLY_PROP_PRESENT,
	POWER_SUPPLY_PROP_STATUS,
	POWER_SUPPLY_PROP_HEALTH,
	POWER_SUPPLY_PROP_ONLINE,
};

static int sm5703_charger_get_property(struct power_supply *psy,
				   enum power_supply_property psp,
				   union power_supply_propval *val) 
{
    int error;
    unsigned int value;
    struct sm5703_charger *drv;

    drv = power_supply_get_drvdata(psy);
    switch (psp) {
        case POWER_SUPPLY_PROP_PRESENT:
            error = regmap_read(drv->sm5703->regmap, SM5703_IRQ_STATUS2, &value);
            val->intval = (value & BIT(4)) ? 0 : 1;
            break;
        case POWER_SUPPLY_PROP_ONLINE:
            error = regmap_read(drv->sm5703->regmap, SM5703_IRQ_STATUS5, &value);
            val->intval = value & SM5703_STATUS5_VBUSOK ? 1 : 0;
            break;
        case POWER_SUPPLY_PROP_STATUS:
            error = regmap_read(drv->sm5703->regmap, SM5703_IRQ_STATUS3, &value);
            if (value & SM5703_STATUS3_DONE)
                val->intval = POWER_SUPPLY_STATUS_FULL;
            else if (value & SM5703_STATUS3_CHGON)
                val->intval = POWER_SUPPLY_STATUS_CHARGING;
            else
                val->intval = POWER_SUPPLY_STATUS_DISCHARGING;
            break;
        case POWER_SUPPLY_PROP_HEALTH:
            error = regmap_read(drv->sm5703->regmap, SM5703_IRQ_STATUS5, &value);
            if (value & SM5703_STATUS5_VBUSOK)
                val->intval = POWER_SUPPLY_HEALTH_GOOD;
            else if (value & SM5703_STATUS5_VBUSOVP)
                val->intval = POWER_SUPPLY_HEALTH_OVERVOLTAGE;
            else
                val->intval = POWER_SUPPLY_HEALTH_UNKNOWN;
            break;
        default:
            return -EINVAL;
    }
    return 0;
}

static const struct power_supply_desc sm5703_charger_desc = {
	.name			= "sm5703_charger",
	.type			= POWER_SUPPLY_TYPE_USB,
	.properties		= sm5703_charger_props,
	.num_properties		= ARRAY_SIZE(sm5703_charger_props),
	.get_property		= sm5703_charger_get_property,
};

static int sm5703_charger_probe (struct platform_device *pdev) {
    int error;
    unsigned int value;
    struct device* dev = &pdev->dev;
    struct power_supply_config charger_cfg = {};
    struct sm5703_charger *drv;
    bool use_autoset, use_autostop, use_topoff_timer;

	drv = devm_kzalloc(&pdev->dev, sizeof(*drv), GFP_KERNEL);
	if (!drv)
		return -ENOMEM;

    drv->sm5703 = dev_get_drvdata(pdev->dev.parent);

    charger_cfg.drv_data = drv;
	charger_cfg.of_node = pdev->dev.of_node;

    drv->use_autoset = device_property_read_bool(dev, "siliconmitus,enable-autoset");
	drv->use_autostop = device_property_read_bool(dev, "siliconmitus,enable-autostop");

    error = regmap_update_bits(drv->sm5703->regmap, SM5703_CHGCNTL4, SM5703_AUTOSTOP_MASK, 
                            (drv->use_autostop << 7));
    if (error)
         return dev_err_probe(dev, error, "Unable to set autostop register\n");

    error = regmap_update_bits(drv->sm5703->regmap, SM5703_REG_CNTL, SM5703_AUTOSET_MASK,
                            (drv->use_autoset << 4));
    if (error)
         return dev_err_probe(dev, error, "Unable to set autoset register\n");


    error = regmap_read(drv->sm5703->regmap, SM5703_CHGCNTL2, &value);
    dev_info(dev, "fast charging current: 0x%x\n", value);
    error = regmap_read(drv->sm5703->regmap, SM5703_CHGCNTL3, &value);
    dev_info(dev, "regulation voltage: 0x%x\n", value);
    error = regmap_read(drv->sm5703->regmap, SM5703_CHGCNTL4, &value);
    dev_info(dev, "top-off current threshold: 0x%x\n", value);
    error = regmap_read(drv->sm5703->regmap, SM5703_CHGCNTL5, &value);
    dev_info(dev, "automatic input current limiting: 0x%x\n", value);
    error = regmap_read(drv->sm5703->regmap, SM5703_CHGCNTL6, &value);
    dev_info(dev, "frequency select: 0x%x\n", value);
    error = regmap_read(drv->sm5703->regmap, SM5703_VBUSCNTL, &value);
    dev_info(dev, "input current limit: 0x%x\n", value);

	drv->psy = devm_power_supply_register(dev, &sm5703_charger_desc,
						   &charger_cfg);

	if (IS_ERR(drv->psy)) {
		dev_err(dev, "failed to register power supply\n");
		return PTR_ERR(drv->psy);
	}

	return 0;
}

static const struct platform_device_id sm5703_charger_id[] = {
	{ "sm5703-charger", },
	{ }
};
MODULE_DEVICE_TABLE(platform, sm5703_charger_id);

static struct platform_driver sm5703_charger_driver = {
	.driver = {
		.name = "sm5703-charger",
	},
	.probe	= sm5703_charger_probe,
	.id_table	= sm5703_charger_id,
};

module_platform_driver(sm5703_charger_driver);

MODULE_DESCRIPTION("Silicon Mitus SM5703 fuel gauge driver");
MODULE_AUTHOR("Markuss Broks <markuss.broks@gmail.com>");
MODULE_LICENSE("GPL");

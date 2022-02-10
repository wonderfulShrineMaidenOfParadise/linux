#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/property.h>
#include <linux/of_gpio.h>
#include <linux/power_supply.h>
#include <linux/regmap.h>
#include <linux/mfd/sm5703.h>

#define FIXED_POINT_8_8_EXTEND_TO_INT(fp_value, extend_orders) ((((fp_value & 0xff00) >> 8) * extend_orders) + (((fp_value & 0xff) * extend_orders) / 256))

#define EXTEND_MICRO 1000000
#define EXTEND_MILLI 1000
#define EXTEND_DECI 10
#define EXTEND_NONE 1

struct sm5703_fg {
    struct i2c_client* i2c;
    struct power_supply *psy;
    struct regmap* regmap;
};

static enum power_supply_property sm5703_fg_props[] = {
	POWER_SUPPLY_PROP_TEMP,
	POWER_SUPPLY_PROP_CAPACITY,
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
    POWER_SUPPLY_PROP_VOLTAGE_OCV,
	POWER_SUPPLY_PROP_CURRENT_NOW,
};

static int sm5703_fg_get_property(struct power_supply *psy,
				   enum power_supply_property psp,
				   union power_supply_propval *val) 
{
    int error;
    unsigned int value;
    struct sm5703_fg *drv;

    drv = power_supply_get_drvdata(psy);
    switch (psp) {
        case POWER_SUPPLY_PROP_TEMP:
            error = regmap_read(drv->regmap, SM5703_FG_REG_TEMPERATURE, &value);
            if (error < 0)
		        return error;
            val->intval = FIXED_POINT_8_8_EXTEND_TO_INT(value, EXTEND_DECI);
            break;
        case POWER_SUPPLY_PROP_CAPACITY:
            error = regmap_read(drv->regmap, SM5703_FG_REG_SOC, &value);
            if (error < 0)
		        return error;
            val->intval = FIXED_POINT_8_8_EXTEND_TO_INT(value, EXTEND_NONE);
            break;
        case POWER_SUPPLY_PROP_VOLTAGE_NOW:
            error = regmap_read(drv->regmap, SM5703_FG_REG_VOLTAGE, &value);
            if (error < 0)
		        return error;
            val->intval = FIXED_POINT_8_8_EXTEND_TO_INT(value, EXTEND_MICRO);
            break;
        case POWER_SUPPLY_PROP_VOLTAGE_OCV:
            error = regmap_read(drv->regmap, SM5703_FG_REG_OCV, &value);
            if (error < 0)
		        return error;
            val->intval = FIXED_POINT_8_8_EXTEND_TO_INT(value, EXTEND_MICRO);
            break;
        case POWER_SUPPLY_PROP_CURRENT_NOW:
            error = regmap_read(drv->regmap, SM5703_FG_REG_CURRENT, &value);
            if (error < 0)
		        return error;
            value &= 0x7ff;
            value = FIXED_POINT_8_8_EXTEND_TO_INT((unsigned short)value, EXTEND_MILLI) * 1000;
            val->intval = value;
            break;
        default:
            return -EINVAL;
    }
    return 0;
}

static const struct power_supply_desc sm5703_fg_desc = {
	.name			= "sm5703_fg",
	.type			= POWER_SUPPLY_TYPE_BATTERY,
	.properties		= sm5703_fg_props,
	.num_properties		= ARRAY_SIZE(sm5703_fg_props),
	.get_property		= sm5703_fg_get_property,
};

static const struct regmap_config sm5703_fg_regmap = {
	.reg_bits	= 8,
	.val_bits	= 16,
    .val_format_endian = REGMAP_ENDIAN_LITTLE,
};

static int sm5703_fg_probe (struct i2c_client* i2c) {
    int error;
    struct device* dev = &i2c->dev;
    struct power_supply_config fg_cfg = {};
    struct sm5703_fg *drv;

	drv = devm_kzalloc(&i2c->dev, sizeof(*drv), GFP_KERNEL);
	if (!drv)
		return -ENOMEM;
    drv->i2c = i2c;

    fg_cfg.drv_data = drv;
	fg_cfg.of_node = i2c->dev.of_node;

    drv->regmap = devm_regmap_init_i2c(i2c, &sm5703_fg_regmap);
    if (IS_ERR(drv->regmap))
		return PTR_ERR(drv->regmap);

	drv->psy = devm_power_supply_register(&i2c->dev, &sm5703_fg_desc,
						   &fg_cfg);

	if (IS_ERR(drv->psy)) {
		dev_err(&i2c->dev, "failed to register power supply\n");
		return PTR_ERR(drv->psy);
	}

	return 0;
}

static struct of_device_id sm5703_fg_of_match[] = {
	{ .compatible = "siliconmitus,sm5703-fg", },
	{ },
};
MODULE_DEVICE_TABLE(of, sm5703_fg_of_match);

static struct i2c_driver sm5703_fg_driver = {
	.driver = {
		   .name = "sm5703-fg",
		   .of_match_table = of_match_ptr(sm5703_fg_of_match),
	},
	.probe = sm5703_fg_probe,
};
module_i2c_driver(sm5703_fg_driver);

MODULE_DESCRIPTION("Silicon Mitus SM5703 fuel gauge driver");
MODULE_AUTHOR("Markuss Broks <markuss.broks@gmail.com>");
MODULE_LICENSE("GPL");

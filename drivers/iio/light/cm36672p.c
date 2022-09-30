// SPDX-License-Identifier: GPL-2.0-only
// Copyright (C) 2022, Raymond Hackley

#include <linux/delay.h>
#include <linux/err.h>
#include <linux/i2c.h>
#include <linux/mutex.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/regmap.h>
#include <linux/regulator/consumer.h>
#include <linux/iio/iio.h>
#include <linux/iio/sysfs.h>
#include <linux/iio/events.h>

#define CM36672P_REGMAP_NAME	"cm36672p_regmap"

/* register addresses */

#define REG_PS_CONF1			0x03
#define REG_PS_CONF3			0x04
#define REG_PS_CANC			0x05
#define REG_PS_THD_LOW			0x06
#define REG_PS_THD_HIGH			0x07
#define REG_PS_DATA			0x08

#define REG_INT_FLAG			0x0C

#define REG_DEV_ID			0x0D

/* register settings */

#define REG_INT_PS_AWAY			(BIT(0)<<8)
#define REG_INT_PS_CLOSE		(BIT(1)<<8)

#define CM36672P_DEFAULT_HI_THD		0x0011
#define CM36672P_DEFAULT_LOW_THD	0x000d
#define CM36672P_CANCEL_HI_THD		0x000a
#define CM36672P_CANCEL_LOW_THD		0x0007
#define CM36672P_DEFAULT_CONF1		0x0320
#define CM36672P_DEFAULT_CONF3		0x4000
#define CM36672P_DEFAULT_TRIM		0x0000

#define CM36686_DEFAULT_HI_THD		0x0015
#define CM36686_DEFAULT_LOW_THD		0x000f
#define CM36686_CANCEL_HI_THD		0x000f
#define CM36686_CANCEL_LOW_THD		0x000a
#define CM36686_DEFAULT_CONF1		0x03A4
#define CM36686_DEFAULT_CONF3		0x4210
#define CM36686_DEFAULT_TRIM		0x0005

#define DEFAULT_CONF3_SMART_PERS	0x4010

#define CM36672P_CMD_READ_RAW_PROXIMITY	0x01

struct cm366xx_properties {
	unsigned int default_hi_thd;
	unsigned int default_low_thd;
	unsigned int cancel_hi_thd;
	unsigned int cancel_low_thd;
	unsigned int default_conf1;
	unsigned int default_conf3;
	unsigned int default_trim;
};

static const struct iio_event_spec cm36672p_event_spec[] = {
	{
		.type = IIO_EV_TYPE_THRESH,
		.dir = IIO_EV_DIR_EITHER,
		.mask_separate = BIT(IIO_EV_INFO_VALUE) |
				BIT(IIO_EV_INFO_ENABLE),
	}
};

static const struct iio_chan_spec cm36672p_channels[] = {
	{
		.type = IIO_PROXIMITY,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
		.address = REG_PS_DATA,
		.event_spec = cm36672p_event_spec,
		.num_event_specs = ARRAY_SIZE(cm36672p_event_spec),
	},
};

struct cm36672p_data {
	struct i2c_client *client;
	struct regmap *regmap;
	struct mutex lock;
	struct regulator_bulk_data supplies[3];
	bool smart_pers;
	const struct cm366xx_properties *data;
};

static int cm36672p_read_raw(struct iio_dev *iio_dev,
			    struct iio_chan_spec const *chan,
			    int *val, int *val2, long mask)
{
	struct cm36672p_data *cm36672p = iio_priv(iio_dev);
	unsigned int buf = 0;
	int ret = IIO_VAL_INT;
	int err = 0;

	mutex_lock(&cm36672p->lock);

	switch (mask) {
	case IIO_CHAN_INFO_RAW:
		if (chan->type == IIO_PROXIMITY) {
			err = regmap_read(cm36672p->regmap, chan->address, &buf);
			if (err < 0)
				return -EINVAL;
			*val = buf;
		} else {
			ret = -EINVAL;
		}
		break;
	default:
		ret = -EINVAL;
	}

	mutex_unlock(&cm36672p->lock);

	return ret;
}

static const struct iio_info cm36672p_info = {
	.read_raw = &cm36672p_read_raw,
};

irqreturn_t cm36672p_irq_handler(int irq, void *data)
{
	struct iio_dev *iio_dev = data;
	struct cm36672p_data *cm36672p = iio_priv(iio_dev);
	struct i2c_client *client = cm36672p->client;
	struct device *dev = &client->dev;
	int int_flag = 0;
	int ev_dir;
	u64 ev_code;
	int ret;

	/* the act of reading this register out acks the interrupt */
	ret = regmap_read(cm36672p->regmap, REG_INT_FLAG, &int_flag);
	if (ret < 0) {
		dev_err(dev, "irq: failed to read interrupt flag: %d\n", ret);
		return ret;
	}

	if (int_flag & REG_INT_PS_AWAY) {
		ev_dir = IIO_EV_DIR_FALLING;
	} else if (int_flag & REG_INT_PS_CLOSE) {
		ev_dir = IIO_EV_DIR_RISING;
	} else {
		dev_err(dev, "irq: unknown interrupt reason; flags: 0x%2x\n",
			int_flag<<8);
		return IRQ_HANDLED;
	}

	ev_code = IIO_UNMOD_EVENT_CODE(IIO_PROXIMITY,
				       CM36672P_CMD_READ_RAW_PROXIMITY,
				       IIO_EV_TYPE_THRESH, ev_dir);

	iio_push_event(iio_dev, ev_code, iio_get_time_ns(iio_dev));

	return IRQ_HANDLED;
}

static int cm36672p_setup_reg(struct cm36672p_data *cm36672p)
{
	struct i2c_client *client = cm36672p->client;
	struct device *dev = &client->dev;
	int ret;

	cm36672p->data = device_get_match_data(dev);

	/* PS initialization */
	ret = regmap_write(cm36672p->regmap, REG_PS_CONF1,
			   cm36672p->data->default_conf1);
	if (ret < 0) {
		dev_err(dev, "PS_CONF1 register setup failed: %d\n", ret);
		return ret;
	}

	if (cm36672p->smart_pers)
		ret = regmap_write(cm36672p->regmap, REG_PS_CONF3,
				   DEFAULT_CONF3_SMART_PERS);
	else
		ret = regmap_write(cm36672p->regmap, REG_PS_CONF3,
				   cm36672p->data->default_conf3);

	if (ret < 0) {
		dev_err(dev, "PS_CONF3 register setup failed: %d\n", ret);
		return ret;
	}

	ret = regmap_write(cm36672p->regmap, REG_PS_THD_HIGH,
			   cm36672p->data->default_hi_thd);
	if (ret < 0) {
		dev_err(dev,
			"proximity sensor treshold high setup failed: %d\n",
			ret);
		return ret;
	}

	ret = regmap_write(cm36672p->regmap, REG_PS_THD_LOW,
			   cm36672p->data->default_low_thd);
	if (ret < 0) {
		dev_err(dev, "proximity sensor treshold low setup failed: %d\n",
			ret);
		return ret;
	}

	ret = regmap_write(cm36672p->regmap, REG_PS_CANC,
			   cm36672p->data->default_trim);
	if (ret < 0) {
		dev_err(dev, "PS_CANC register setup failed: %d\n",
			ret);
		return ret;
	}

	return 0;
}

static bool cm36672p_is_volatile_reg(struct device *dev, unsigned int reg)
{
	switch (reg) {
	case REG_PS_DATA:
	case REG_INT_FLAG:
		return true;
	default:
		return false;
	}
}

static const struct regmap_config cm36672p_regmap_config = {
	.name		= CM36672P_REGMAP_NAME,
	.reg_bits	= 8,
	.val_bits	= 16,
	.max_register	= REG_DEV_ID,
	.cache_type	= REGCACHE_RBTREE,
	.volatile_reg	= cm36672p_is_volatile_reg,
	.val_format_endian = REGMAP_ENDIAN_LITTLE,
};

static void cm36672p_disable(void *data)
{
	struct cm36672p_data *cm36672p = data;
	struct i2c_client *client = cm36672p->client;
	struct device *dev = &client->dev;
	int ret;

	ret = regulator_bulk_disable(ARRAY_SIZE(cm36672p->supplies),
				     cm36672p->supplies);
	if (ret < 0)
		dev_warn(dev, "Failed to disable regulators: %d\n", ret);
}

static int cm36672p_init_regulators(struct cm36672p_data *cm36672p)
{
	struct i2c_client *client = cm36672p->client;

	cm36672p->supplies[0].supply = "vdd";
	cm36672p->supplies[1].supply = "vddio";
	cm36672p->supplies[2].supply = "vled";
	return devm_regulator_bulk_get(&client->dev,
				       ARRAY_SIZE(cm36672p->supplies),
				       cm36672p->supplies);
}

static int cm36672p_power_on(struct cm36672p_data *cm36672p)
{
	int error;

	error = regulator_bulk_enable(ARRAY_SIZE(cm36672p->supplies),
				      cm36672p->supplies);
	if (error)
		return error;

	return 0;
}

static int cm36672p_i2c_probe(struct i2c_client *client,
			      const struct i2c_device_id *id)
{
	struct cm36672p_data *cm36672p = NULL;
	struct iio_dev *iio_dev;
	struct device *dev = &client->dev;
	int ret;

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C))
		return dev_err_probe(dev, ret,
				     "i2c functionality check failed!\n");

	iio_dev = devm_iio_device_alloc(dev, sizeof(*cm36672p));
	if (!iio_dev)
		return dev_err_probe(dev, -ENOMEM,
				     "Failed to allocate memory for cm36672p module data\n");

	cm36672p = iio_priv(iio_dev);

	cm36672p->client = client;
	i2c_set_clientdata(client, iio_dev);

	ret = cm36672p_init_regulators(cm36672p);
	if (ret < 0)
		return dev_err_probe(dev, ret,
				     "Failed to get regulators: %d\n", ret);

	ret = cm36672p_power_on(cm36672p);
	if (ret < 0)
		return dev_err_probe(dev, ret,
				     "Failed to enable regulators: %d\n", ret);

	ret = devm_add_action_or_reset(dev, cm36672p_disable, iio_dev);
	if (ret < 0)
		return dev_err_probe(dev, ret,
				     "Failed to install poweroff handler: %d\n",
				     ret);

	cm36672p->smart_pers = of_property_read_bool(dev->of_node,
						     "cm36672p,smart-pers");

	cm36672p->regmap = devm_regmap_init_i2c(client,
						&cm36672p_regmap_config);
	if (IS_ERR(cm36672p->regmap))
		return dev_err_probe(dev, PTR_ERR(cm36672p->regmap),
				     "regmap_init failed!\n");

	mutex_init(&cm36672p->lock);
	iio_dev->dev.parent = dev;
	iio_dev->channels = cm36672p_channels;
	iio_dev->num_channels = ARRAY_SIZE(cm36672p_channels);
	iio_dev->info = &cm36672p_info;
	iio_dev->name = id->name;
	iio_dev->modes = INDIO_DIRECT_MODE;

	ret = cm36672p_setup_reg(cm36672p);
	if (ret < 0)
		return dev_err_probe(dev, ret,
				     "Failed to setup regmap: %d\n", ret);

	ret = request_threaded_irq(client->irq, NULL, cm36672p_irq_handler,
					IRQF_TRIGGER_FALLING | IRQF_ONESHOT,
							"cm36672p", iio_dev);
	if (ret < 0)
		return dev_err_probe(dev, ret,
				     "Failed to request IRQ: %d\n", ret);

	ret = iio_device_register(iio_dev);
	if (ret < 0)
		return dev_err_probe(dev, ret,
				     "Failed to register iio device: %d\n",
		       ret);

	return 0;
}

static void cm36672p_i2c_remove(struct i2c_client *client)
{
	struct iio_dev *iio_dev = i2c_get_clientdata(client);
	struct cm36672p_data *cm36672p = iio_priv(iio_dev);

	cm36672p_disable(cm36672p);
}

static const struct i2c_device_id cm36672p_id[] = {
	{"cm36672p", 0},
	{}
};

MODULE_DEVICE_TABLE(i2c, cm36672p_id);

static const struct cm366xx_properties cm36672p_data = {
	.default_hi_thd = CM36672P_DEFAULT_HI_THD,
	.default_low_thd = CM36672P_DEFAULT_LOW_THD,
	.cancel_hi_thd = CM36672P_CANCEL_HI_THD,
	.cancel_low_thd = CM36672P_CANCEL_LOW_THD,
	.default_conf1 = CM36672P_DEFAULT_CONF1,
	.default_conf3 = CM36672P_DEFAULT_CONF3,
	.default_trim = CM36672P_DEFAULT_TRIM,
};

static const struct cm366xx_properties cm36686_data = {
	.default_hi_thd = CM36686_DEFAULT_HI_THD,
	.default_low_thd = CM36686_DEFAULT_LOW_THD,
	.cancel_hi_thd = CM36686_CANCEL_HI_THD,
	.cancel_low_thd = CM36686_CANCEL_LOW_THD,
	.default_conf1 = CM36686_DEFAULT_CONF1,
	.default_conf3 = CM36686_DEFAULT_CONF3,
	.default_trim = CM36686_DEFAULT_TRIM,
};

static const struct of_device_id cm36672p_of_match[] = {
	{ .compatible = "capella,cm36672p", .data = &cm36672p_data},
	{ .compatible = "capella,cm36686", .data = &cm36686_data},
	{},
};

MODULE_DEVICE_TABLE(of, cm36672p_of_match);

static struct i2c_driver cm36672p_driver = {
	.driver = {
		   .name = "cm36672p",
		   .owner = THIS_MODULE,
		   .of_match_table = cm36672p_of_match,
	},
	.probe = cm36672p_i2c_probe,
	.remove = cm36672p_i2c_remove,
	.id_table = cm36672p_id,
};

module_i2c_driver(cm36672p_driver);

MODULE_AUTHOR("Raymond Hackley");
MODULE_DESCRIPTION("cm36672p proximity sensor driver");
MODULE_LICENSE("GPL");

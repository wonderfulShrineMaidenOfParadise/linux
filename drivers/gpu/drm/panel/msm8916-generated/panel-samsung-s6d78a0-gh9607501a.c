// SPDX-License-Identifier: GPL-2.0-only
// Copyright (c) 2023 FIXME
// Generated with linux-mdss-dsi-panel-driver-generator from vendor device tree:
//   Copyright (c) 2013, The Linux Foundation. All rights reserved. (FIXME)

#include <linux/backlight.h>
#include <linux/delay.h>
#include <linux/gpio/consumer.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/regulator/consumer.h>

#include <video/mipi_display.h>

#include <drm/drm_mipi_dsi.h>
#include <drm/drm_modes.h>
#include <drm/drm_panel.h>

struct s6d78a0_gh9607501a {
	struct drm_panel panel;
	struct mipi_dsi_device *dsi;
	struct regulator_bulk_data supplies[2];
	struct gpio_desc *reset_gpio;
	bool prepared;
};

static inline
struct s6d78a0_gh9607501a *to_s6d78a0_gh9607501a(struct drm_panel *panel)
{
	return container_of(panel, struct s6d78a0_gh9607501a, panel);
}

static void s6d78a0_gh9607501a_reset(struct s6d78a0_gh9607501a *ctx)
{
	gpiod_set_value_cansleep(ctx->reset_gpio, 0);
	usleep_range(10000, 11000);
	gpiod_set_value_cansleep(ctx->reset_gpio, 1);
	usleep_range(5000, 6000);
	gpiod_set_value_cansleep(ctx->reset_gpio, 0);
	msleep(30);
}

static int s6d78a0_gh9607501a_on(struct s6d78a0_gh9607501a *ctx)
{
	struct mipi_dsi_device *dsi = ctx->dsi;
	struct device *dev = &dsi->dev;
	int ret;

	dsi->mode_flags |= MIPI_DSI_MODE_LPM;

	mipi_dsi_dcs_write_seq(dsi, 0xf0, 0x5a, 0x5a);
	mipi_dsi_dcs_write_seq(dsi, 0xf1, 0x5a, 0x5a);
	mipi_dsi_dcs_write_seq(dsi, 0xfc, 0xa5, 0xa5);

	ret = mipi_dsi_dcs_exit_sleep_mode(dsi);
	if (ret < 0) {
		dev_err(dev, "Failed to exit sleep mode: %d\n", ret);
		return ret;
	}
	msleep(120);

	mipi_dsi_dcs_write_seq(dsi, 0xb1, 0x93);
	mipi_dsi_dcs_write_seq(dsi, 0xb5, 0x10);
	mipi_dsi_dcs_write_seq(dsi, 0xf4,
			       0x01, 0x10, 0x32, 0x00, 0x24, 0x26, 0x28, 0x27,
			       0x27, 0x27, 0xb7, 0x2b, 0x2c, 0x65, 0x6a, 0x34,
			       0x20);
	mipi_dsi_dcs_write_seq(dsi, 0xef,
			       0x01, 0x01, 0x81, 0x22, 0x83, 0x04, 0x00, 0x00,
			       0x00, 0x00, 0x28, 0x81, 0x00, 0x21, 0x21, 0x03,
			       0x03, 0x40, 0x00, 0x10);
	mipi_dsi_dcs_write_seq(dsi, 0xf2,
			       0x19, 0x04, 0x08, 0x08, 0x08, 0x14, 0x14, 0x00);
	mipi_dsi_dcs_write_seq(dsi, 0xf6, 0x93, 0x23, 0x15, 0x07, 0x07, 0x0c);
	mipi_dsi_dcs_write_seq(dsi, 0xe1, 0x01, 0xff, 0x01, 0x1b, 0x20, 0x17);
	mipi_dsi_dcs_write_seq(dsi, 0xe2, 0xed, 0xc7, 0x23);
	mipi_dsi_dcs_write_seq(dsi, 0xf7,
			       0x01, 0x01, 0x0a, 0x0b, 0x05, 0x1b, 0x1a, 0x01,
			       0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
			       0x01, 0x01, 0x01, 0x01, 0x01, 0x08, 0x09, 0x04,
			       0x1b, 0x1a, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
			       0x01, 0x01, 0x01, 0x01, 0x01, 0x01);
	mipi_dsi_dcs_write_seq(dsi, MIPI_DCS_SET_ADDRESS_MODE, 0x10);
	mipi_dsi_dcs_write_seq(dsi, 0xfa,
			       0x00, 0x19, 0x21, 0x1e, 0x14, 0x0b, 0x10, 0x0e,
			       0x09, 0x0b, 0x00, 0x00, 0x0a, 0x00, 0x1d, 0x21,
			       0x1e, 0x14, 0x0b, 0x10, 0x0e, 0x09, 0x0b, 0x00,
			       0x00, 0x0a, 0x00, 0x1c, 0x21, 0x1e, 0x14, 0x0b,
			       0x10, 0x0e, 0x09, 0x0b, 0x00, 0x00, 0x0a);
	mipi_dsi_dcs_write_seq(dsi, 0xfb,
			       0x07, 0x2d, 0x22, 0x24, 0x18, 0x0e, 0x11, 0x0c,
			       0x05, 0x05, 0x00, 0x00, 0x0a, 0x00, 0x29, 0x22,
			       0x24, 0x18, 0x0e, 0x11, 0x0c, 0x05, 0x05, 0x00,
			       0x00, 0x0a, 0x07, 0x2a, 0x22, 0x24, 0x18, 0x0e,
			       0x11, 0x0c, 0x05, 0x05, 0x00, 0x00, 0x0a);

	ret = mipi_dsi_dcs_set_display_on(dsi);
	if (ret < 0) {
		dev_err(dev, "Failed to set display on: %d\n", ret);
		return ret;
	}
	msleep(20);

	mipi_dsi_dcs_write_seq(dsi, 0xf0, 0xa5, 0xa5);
	mipi_dsi_dcs_write_seq(dsi, 0xf1, 0xa5, 0xa5);
	mipi_dsi_dcs_write_seq(dsi, 0xfc, 0x5a, 0x5a);

	return 0;
}

static int s6d78a0_gh9607501a_off(struct s6d78a0_gh9607501a *ctx)
{
	struct mipi_dsi_device *dsi = ctx->dsi;

	dsi->mode_flags &= ~MIPI_DSI_MODE_LPM;

	mipi_dsi_dcs_write_seq(dsi, 0x28, 0x00);
	msleep(40);
	mipi_dsi_dcs_write_seq(dsi, 0x10, 0x00);
	msleep(100);

	return 0;
}

static int s6d78a0_gh9607501a_prepare(struct drm_panel *panel)
{
	struct s6d78a0_gh9607501a *ctx = to_s6d78a0_gh9607501a(panel);
	struct device *dev = &ctx->dsi->dev;
	int ret;

	if (ctx->prepared)
		return 0;

	ret = regulator_bulk_enable(ARRAY_SIZE(ctx->supplies), ctx->supplies);
	if (ret < 0) {
		dev_err(dev, "Failed to enable regulators: %d\n", ret);
		return ret;
	}

	s6d78a0_gh9607501a_reset(ctx);

	ret = s6d78a0_gh9607501a_on(ctx);
	if (ret < 0) {
		dev_err(dev, "Failed to initialize panel: %d\n", ret);
		gpiod_set_value_cansleep(ctx->reset_gpio, 1);
		regulator_bulk_disable(ARRAY_SIZE(ctx->supplies), ctx->supplies);
		return ret;
	}

	ctx->prepared = true;
	return 0;
}

static int s6d78a0_gh9607501a_unprepare(struct drm_panel *panel)
{
	struct s6d78a0_gh9607501a *ctx = to_s6d78a0_gh9607501a(panel);
	struct device *dev = &ctx->dsi->dev;
	int ret;

	if (!ctx->prepared)
		return 0;

	ret = s6d78a0_gh9607501a_off(ctx);
	if (ret < 0)
		dev_err(dev, "Failed to un-initialize panel: %d\n", ret);

	gpiod_set_value_cansleep(ctx->reset_gpio, 1);
	regulator_bulk_disable(ARRAY_SIZE(ctx->supplies), ctx->supplies);

	ctx->prepared = false;
	return 0;
}

static const struct drm_display_mode s6d78a0_gh9607501a_mode = {
	.clock = (540 + 10 + 14 + 10) * (960 + 8 + 2 + 6) * 60 / 1000,
	.hdisplay = 540,
	.hsync_start = 540 + 10,
	.hsync_end = 540 + 10 + 14,
	.htotal = 540 + 10 + 14 + 10,
	.vdisplay = 960,
	.vsync_start = 960 + 8,
	.vsync_end = 960 + 8 + 2,
	.vtotal = 960 + 8 + 2 + 6,
	.width_mm = 62,
	.height_mm = 110,
};

static int s6d78a0_gh9607501a_get_modes(struct drm_panel *panel,
					struct drm_connector *connector)
{
	struct drm_display_mode *mode;

	mode = drm_mode_duplicate(connector->dev, &s6d78a0_gh9607501a_mode);
	if (!mode)
		return -ENOMEM;

	drm_mode_set_name(mode);

	mode->type = DRM_MODE_TYPE_DRIVER | DRM_MODE_TYPE_PREFERRED;
	connector->display_info.width_mm = mode->width_mm;
	connector->display_info.height_mm = mode->height_mm;
	drm_mode_probed_add(connector, mode);

	return 1;
}

static const struct drm_panel_funcs s6d78a0_gh9607501a_panel_funcs = {
	.prepare = s6d78a0_gh9607501a_prepare,
	.unprepare = s6d78a0_gh9607501a_unprepare,
	.get_modes = s6d78a0_gh9607501a_get_modes,
};

static int s6d78a0_gh9607501a_bl_update_status(struct backlight_device *bl)
{
	struct mipi_dsi_device *dsi = bl_get_data(bl);
	u16 brightness = backlight_get_brightness(bl);
	int ret;

	dsi->mode_flags &= ~MIPI_DSI_MODE_LPM;

	ret = mipi_dsi_dcs_set_display_brightness(dsi, brightness);
	if (ret < 0)
		return ret;

	dsi->mode_flags |= MIPI_DSI_MODE_LPM;

	return 0;
}

static const struct backlight_ops s6d78a0_gh9607501a_bl_ops = {
	.update_status = s6d78a0_gh9607501a_bl_update_status,
};

static struct backlight_device *
s6d78a0_gh9607501a_create_backlight(struct mipi_dsi_device *dsi)
{
	struct device *dev = &dsi->dev;
	const struct backlight_properties props = {
		.type = BACKLIGHT_RAW,
		.brightness = 255,
		.max_brightness = 255,
	};

	return devm_backlight_device_register(dev, dev_name(dev), dev, dsi,
					      &s6d78a0_gh9607501a_bl_ops, &props);
}

static int s6d78a0_gh9607501a_probe(struct mipi_dsi_device *dsi)
{
	struct device *dev = &dsi->dev;
	struct s6d78a0_gh9607501a *ctx;
	int ret;

	ctx = devm_kzalloc(dev, sizeof(*ctx), GFP_KERNEL);
	if (!ctx)
		return -ENOMEM;

	ctx->supplies[0].supply = "vddio";
	ctx->supplies[1].supply = "vdd";
	ret = devm_regulator_bulk_get(dev, ARRAY_SIZE(ctx->supplies),
				      ctx->supplies);
	if (ret < 0)
		return dev_err_probe(dev, ret, "Failed to get regulators\n");

	ctx->reset_gpio = devm_gpiod_get(dev, "reset", GPIOD_OUT_HIGH);
	if (IS_ERR(ctx->reset_gpio))
		return dev_err_probe(dev, PTR_ERR(ctx->reset_gpio),
				     "Failed to get reset-gpios\n");

	ctx->dsi = dsi;
	mipi_dsi_set_drvdata(dsi, ctx);

	dsi->lanes = 2;
	dsi->format = MIPI_DSI_FMT_RGB888;
	dsi->mode_flags = MIPI_DSI_MODE_VIDEO | MIPI_DSI_MODE_VIDEO_SYNC_PULSE |
			  MIPI_DSI_MODE_NO_EOT_PACKET |
			  MIPI_DSI_CLOCK_NON_CONTINUOUS;

	drm_panel_init(&ctx->panel, dev, &s6d78a0_gh9607501a_panel_funcs,
		       DRM_MODE_CONNECTOR_DSI);
	ctx->panel.prepare_prev_first = true;

	ret = drm_panel_of_backlight(&ctx->panel);
	if (ret)
		return dev_err_probe(dev, ret, "Failed to get backlight\n");

	/* Fallback to DCS backlight if no backlight is defined in DT */
	if (!ctx->panel.backlight) {
		ctx->panel.backlight = s6d78a0_gh9607501a_create_backlight(dsi);
		if (IS_ERR(ctx->panel.backlight))
			return dev_err_probe(dev, PTR_ERR(ctx->panel.backlight),
					     "Failed to create backlight\n");
	}

	drm_panel_add(&ctx->panel);

	ret = mipi_dsi_attach(dsi);
	if (ret < 0) {
		dev_err(dev, "Failed to attach to DSI host: %d\n", ret);
		drm_panel_remove(&ctx->panel);
		return ret;
	}

	return 0;
}

static void s6d78a0_gh9607501a_remove(struct mipi_dsi_device *dsi)
{
	struct s6d78a0_gh9607501a *ctx = mipi_dsi_get_drvdata(dsi);
	int ret;

	ret = mipi_dsi_detach(dsi);
	if (ret < 0)
		dev_err(&dsi->dev, "Failed to detach from DSI host: %d\n", ret);

	drm_panel_remove(&ctx->panel);
}

static const struct of_device_id s6d78a0_gh9607501a_of_match[] = {
	{ .compatible = "samsung,s6d78a0-gh9607501a" }, // FIXME
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, s6d78a0_gh9607501a_of_match);

static struct mipi_dsi_driver s6d78a0_gh9607501a_driver = {
	.probe = s6d78a0_gh9607501a_probe,
	.remove = s6d78a0_gh9607501a_remove,
	.driver = {
		.name = "panel-s6d78a0-gh9607501a",
		.of_match_table = s6d78a0_gh9607501a_of_match,
	},
};
module_mipi_dsi_driver(s6d78a0_gh9607501a_driver);

MODULE_AUTHOR("linux-mdss-dsi-panel-driver-generator <fix@me>"); // FIXME
MODULE_DESCRIPTION("DRM driver for S6D78A0 qhd video mode dsi panel");
MODULE_LICENSE("GPL");

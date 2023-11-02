// SPDX-License-Identifier: GPL-2.0-only
// Copyright (c) 2023 FIXME
// Generated with linux-mdss-dsi-panel-driver-generator from vendor device tree:
//   Copyright (c) 2013, The Linux Foundation. All rights reserved. (FIXME)

#include <linux/delay.h>
#include <linux/gpio/consumer.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/regulator/consumer.h>

#include <video/mipi_display.h>

#include <drm/drm_mipi_dsi.h>
#include <drm/drm_modes.h>
#include <drm/drm_panel.h>

struct ili9881c {
	struct drm_panel panel;
	struct mipi_dsi_device *dsi;
	struct regulator *supply;
	struct gpio_desc *reset_gpio;
	bool prepared;
};

static inline struct ili9881c *to_ili9881c(struct drm_panel *panel)
{
	return container_of(panel, struct ili9881c, panel);
}

static void ili9881c_reset(struct ili9881c *ctx)
{
	gpiod_set_value_cansleep(ctx->reset_gpio, 0);
	msleep(20);
	gpiod_set_value_cansleep(ctx->reset_gpio, 1);
	msleep(20);
	gpiod_set_value_cansleep(ctx->reset_gpio, 0);
	msleep(20);
}

static int ili9881c_on(struct ili9881c *ctx)
{
	struct mipi_dsi_device *dsi = ctx->dsi;
	struct device *dev = &dsi->dev;
	int ret;

	dsi->mode_flags |= MIPI_DSI_MODE_LPM;

	mipi_dsi_dcs_write_seq(dsi, 0xff, 0x98, 0x81, 0x03);
	mipi_dsi_dcs_write_seq(dsi, 0x01, 0x00);
	mipi_dsi_dcs_write_seq(dsi, 0x02, 0x00);
	mipi_dsi_dcs_write_seq(dsi, 0x03, 0x53);
	mipi_dsi_dcs_write_seq(dsi, 0x04, 0x13);
	mipi_dsi_dcs_write_seq(dsi, 0x05, 0x13);
	mipi_dsi_dcs_write_seq(dsi, 0x06, 0x06);
	mipi_dsi_dcs_write_seq(dsi, 0x07, 0x00);
	mipi_dsi_dcs_write_seq(dsi, 0x08, 0x04);
	mipi_dsi_dcs_write_seq(dsi, 0x09, 0x00);
	mipi_dsi_dcs_write_seq(dsi, 0x0a, 0x00);
	mipi_dsi_dcs_write_seq(dsi, 0x0b, 0x00);
	mipi_dsi_dcs_write_seq(dsi, 0x0c, 0x00);
	mipi_dsi_dcs_write_seq(dsi, 0x0d, 0x00);
	mipi_dsi_dcs_write_seq(dsi, 0x0e, 0x00);
	mipi_dsi_dcs_write_seq(dsi, 0x0f, 0x00);
	mipi_dsi_dcs_write_seq(dsi, 0x10, 0x00);
	mipi_dsi_dcs_write_seq(dsi, 0x11, 0x00);
	mipi_dsi_dcs_write_seq(dsi, 0x12, 0x00);
	mipi_dsi_dcs_write_seq(dsi, 0x13, 0x00);
	mipi_dsi_dcs_write_seq(dsi, 0x14, 0x00);
	mipi_dsi_dcs_write_seq(dsi, 0x15, 0x00);
	mipi_dsi_dcs_write_seq(dsi, 0x16, 0x00);
	mipi_dsi_dcs_write_seq(dsi, 0x17, 0x00);
	mipi_dsi_dcs_write_seq(dsi, 0x18, 0x00);
	mipi_dsi_dcs_write_seq(dsi, 0x19, 0x00);
	mipi_dsi_dcs_write_seq(dsi, 0x1a, 0x00);
	mipi_dsi_dcs_write_seq(dsi, 0x1b, 0x00);
	mipi_dsi_dcs_write_seq(dsi, 0x1c, 0x00);
	mipi_dsi_dcs_write_seq(dsi, 0x1d, 0x00);
	mipi_dsi_dcs_write_seq(dsi, 0x1e, 0xc0);
	mipi_dsi_dcs_write_seq(dsi, 0x1f, 0x80);
	mipi_dsi_dcs_write_seq(dsi, 0x20, 0x04);
	mipi_dsi_dcs_write_seq(dsi, 0x21, 0x0b);
	mipi_dsi_dcs_write_seq(dsi, 0x22, 0x00);
	mipi_dsi_dcs_write_seq(dsi, 0x23, 0x00);
	mipi_dsi_dcs_write_seq(dsi, 0x24, 0x00);
	mipi_dsi_dcs_write_seq(dsi, 0x25, 0x00);
	mipi_dsi_dcs_write_seq(dsi, MIPI_DCS_SET_GAMMA_CURVE, 0x00);
	mipi_dsi_dcs_write_seq(dsi, 0x27, 0x00);
	mipi_dsi_dcs_write_seq(dsi, 0x28, 0x55);
	mipi_dsi_dcs_write_seq(dsi, 0x29, 0x03);
	mipi_dsi_dcs_write_seq(dsi, 0x2a, 0x00);
	mipi_dsi_dcs_write_seq(dsi, 0x2b, 0x00);
	mipi_dsi_dcs_write_seq(dsi, MIPI_DCS_WRITE_MEMORY_START, 0x00);
	mipi_dsi_dcs_write_seq(dsi, MIPI_DCS_WRITE_LUT, 0x00);
	mipi_dsi_dcs_write_seq(dsi, MIPI_DCS_READ_MEMORY_START, 0x00);
	mipi_dsi_dcs_write_seq(dsi, 0x2f, 0x00);
	mipi_dsi_dcs_write_seq(dsi, MIPI_DCS_SET_PARTIAL_ROWS, 0x00);
	mipi_dsi_dcs_write_seq(dsi, MIPI_DCS_SET_PARTIAL_COLUMNS, 0x00);
	mipi_dsi_dcs_write_seq(dsi, 0x32, 0x00);
	mipi_dsi_dcs_write_seq(dsi, 0x33, 0x00);
	mipi_dsi_dcs_write_seq(dsi, 0x34, 0x04);
	mipi_dsi_dcs_write_seq(dsi, 0x35, 0x05);
	mipi_dsi_dcs_write_seq(dsi, MIPI_DCS_SET_ADDRESS_MODE, 0x05);
	mipi_dsi_dcs_write_seq(dsi, 0x37, 0x00);
	mipi_dsi_dcs_write_seq(dsi, 0x38, 0x3c);
	mipi_dsi_dcs_write_seq(dsi, 0x39, 0x00);

	ret = mipi_dsi_dcs_set_pixel_format(dsi, 0x40);
	if (ret < 0) {
		dev_err(dev, "Failed to set pixel format: %d\n", ret);
		return ret;
	}

	mipi_dsi_dcs_write_seq(dsi, 0x3b, 0x40);
	mipi_dsi_dcs_write_seq(dsi, MIPI_DCS_WRITE_MEMORY_CONTINUE, 0x00);
	mipi_dsi_dcs_write_seq(dsi, MIPI_DCS_SET_3D_CONTROL, 0x00);
	mipi_dsi_dcs_write_seq(dsi, MIPI_DCS_READ_MEMORY_CONTINUE, 0x00);
	mipi_dsi_dcs_write_seq(dsi, 0x3f, 0x00);
	mipi_dsi_dcs_write_seq(dsi, MIPI_DCS_SET_VSYNC_TIMING, 0x00);
	mipi_dsi_dcs_write_seq(dsi, 0x41, 0x00);
	mipi_dsi_dcs_write_seq(dsi, 0x42, 0x00);
	mipi_dsi_dcs_write_seq(dsi, 0x43, 0x00);
	mipi_dsi_dcs_write_seq(dsi, 0x44, 0x00);
	mipi_dsi_dcs_write_seq(dsi, 0x50, 0x01);

	ret = mipi_dsi_dcs_set_display_brightness(dsi, 0x0023);
	if (ret < 0) {
		dev_err(dev, "Failed to set display brightness: %d\n", ret);
		return ret;
	}

	mipi_dsi_dcs_write_seq(dsi, 0x52, 0x45);
	mipi_dsi_dcs_write_seq(dsi, MIPI_DCS_WRITE_CONTROL_DISPLAY, 0x67);
	mipi_dsi_dcs_write_seq(dsi, 0x54, 0x89);
	mipi_dsi_dcs_write_seq(dsi, MIPI_DCS_WRITE_POWER_SAVE, 0xab);
	mipi_dsi_dcs_write_seq(dsi, 0x56, 0x01);
	mipi_dsi_dcs_write_seq(dsi, 0x57, 0x23);
	mipi_dsi_dcs_write_seq(dsi, 0x58, 0x45);
	mipi_dsi_dcs_write_seq(dsi, 0x59, 0x67);
	mipi_dsi_dcs_write_seq(dsi, 0x5a, 0x89);
	mipi_dsi_dcs_write_seq(dsi, 0x5b, 0xab);
	mipi_dsi_dcs_write_seq(dsi, 0x5c, 0xcd);
	mipi_dsi_dcs_write_seq(dsi, 0x5d, 0xef);
	mipi_dsi_dcs_write_seq(dsi, MIPI_DCS_SET_CABC_MIN_BRIGHTNESS, 0x01);
	mipi_dsi_dcs_write_seq(dsi, 0x5f, 0x14);
	mipi_dsi_dcs_write_seq(dsi, 0x60, 0x15);
	mipi_dsi_dcs_write_seq(dsi, 0x61, 0x0c);
	mipi_dsi_dcs_write_seq(dsi, 0x62, 0x0d);
	mipi_dsi_dcs_write_seq(dsi, 0x63, 0x0e);
	mipi_dsi_dcs_write_seq(dsi, 0x64, 0x0f);
	mipi_dsi_dcs_write_seq(dsi, 0x65, 0x10);
	mipi_dsi_dcs_write_seq(dsi, 0x66, 0x11);
	mipi_dsi_dcs_write_seq(dsi, 0x67, 0x08);
	mipi_dsi_dcs_write_seq(dsi, 0x68, 0x02);
	mipi_dsi_dcs_write_seq(dsi, 0x69, 0x0a);
	mipi_dsi_dcs_write_seq(dsi, 0x6a, 0x02);
	mipi_dsi_dcs_write_seq(dsi, 0x6b, 0x02);
	mipi_dsi_dcs_write_seq(dsi, 0x6c, 0x02);
	mipi_dsi_dcs_write_seq(dsi, 0x6d, 0x02);
	mipi_dsi_dcs_write_seq(dsi, 0x6e, 0x02);
	mipi_dsi_dcs_write_seq(dsi, 0x6f, 0x02);
	mipi_dsi_dcs_write_seq(dsi, 0x70, 0x02);
	mipi_dsi_dcs_write_seq(dsi, 0x71, 0x02);
	mipi_dsi_dcs_write_seq(dsi, 0x72, 0x06);
	mipi_dsi_dcs_write_seq(dsi, 0x73, 0x02);
	mipi_dsi_dcs_write_seq(dsi, 0x74, 0x02);
	mipi_dsi_dcs_write_seq(dsi, 0x75, 0x14);
	mipi_dsi_dcs_write_seq(dsi, 0x76, 0x15);
	mipi_dsi_dcs_write_seq(dsi, 0x77, 0x11);
	mipi_dsi_dcs_write_seq(dsi, 0x78, 0x10);
	mipi_dsi_dcs_write_seq(dsi, 0x79, 0x0f);
	mipi_dsi_dcs_write_seq(dsi, 0x7a, 0x0e);
	mipi_dsi_dcs_write_seq(dsi, 0x7b, 0x0d);
	mipi_dsi_dcs_write_seq(dsi, 0x7c, 0x0c);
	mipi_dsi_dcs_write_seq(dsi, 0x7d, 0x06);
	mipi_dsi_dcs_write_seq(dsi, 0x7e, 0x02);
	mipi_dsi_dcs_write_seq(dsi, 0x7f, 0x0a);
	mipi_dsi_dcs_write_seq(dsi, 0x80, 0x02);
	mipi_dsi_dcs_write_seq(dsi, 0x81, 0x02);
	mipi_dsi_dcs_write_seq(dsi, 0x82, 0x02);
	mipi_dsi_dcs_write_seq(dsi, 0x83, 0x02);
	mipi_dsi_dcs_write_seq(dsi, 0x84, 0x02);
	mipi_dsi_dcs_write_seq(dsi, 0x85, 0x02);
	mipi_dsi_dcs_write_seq(dsi, 0x86, 0x02);
	mipi_dsi_dcs_write_seq(dsi, 0x87, 0x02);
	mipi_dsi_dcs_write_seq(dsi, 0x88, 0x08);
	mipi_dsi_dcs_write_seq(dsi, 0x89, 0x02);
	mipi_dsi_dcs_write_seq(dsi, 0x8a, 0x02);
	mipi_dsi_dcs_write_seq(dsi, 0xff, 0x98, 0x81, 0x04);
	mipi_dsi_dcs_write_seq(dsi, 0x6c, 0x15);
	mipi_dsi_dcs_write_seq(dsi, 0x6e, 0x3b);
	mipi_dsi_dcs_write_seq(dsi, 0x6f, 0x53);

	ret = mipi_dsi_dcs_set_pixel_format(dsi, 0xa4);
	if (ret < 0) {
		dev_err(dev, "Failed to set pixel format: %d\n", ret);
		return ret;
	}

	mipi_dsi_dcs_write_seq(dsi, 0x8d, 0x15);
	mipi_dsi_dcs_write_seq(dsi, 0x87, 0xba);
	mipi_dsi_dcs_write_seq(dsi, 0xb2, 0xd1);
	mipi_dsi_dcs_write_seq(dsi, MIPI_DCS_SET_GAMMA_CURVE, 0x76);
	mipi_dsi_dcs_write_seq(dsi, 0x88, 0x0b);
	mipi_dsi_dcs_write_seq(dsi, 0xff, 0x98, 0x81, 0x01);
	mipi_dsi_dcs_write_seq(dsi, 0x22, 0x0a);
	mipi_dsi_dcs_write_seq(dsi, MIPI_DCS_SET_PARTIAL_COLUMNS, 0x00);
	mipi_dsi_dcs_write_seq(dsi, MIPI_DCS_WRITE_CONTROL_DISPLAY, 0x88);
	mipi_dsi_dcs_write_seq(dsi, MIPI_DCS_WRITE_POWER_SAVE, 0x83);
	mipi_dsi_dcs_write_seq(dsi, 0x50, 0xa6);

	ret = mipi_dsi_dcs_set_display_brightness(dsi, 0x00a6);
	if (ret < 0) {
		dev_err(dev, "Failed to set display brightness: %d\n", ret);
		return ret;
	}

	mipi_dsi_dcs_write_seq(dsi, 0x60, 0x14);
	mipi_dsi_dcs_write_seq(dsi, 0xa0, 0x08);
	mipi_dsi_dcs_write_seq(dsi, MIPI_DCS_READ_DDB_START, 0x22);
	mipi_dsi_dcs_write_seq(dsi, MIPI_DCS_READ_PPS_START, 0x30);
	mipi_dsi_dcs_write_seq(dsi, 0xa3, 0x15);
	mipi_dsi_dcs_write_seq(dsi, 0xa4, 0x17);
	mipi_dsi_dcs_write_seq(dsi, 0xa5, 0x2b);
	mipi_dsi_dcs_write_seq(dsi, 0xa6, 0x1f);
	mipi_dsi_dcs_write_seq(dsi, 0xa7, 0x1f);
	mipi_dsi_dcs_write_seq(dsi, MIPI_DCS_READ_DDB_CONTINUE, 0x88);
	mipi_dsi_dcs_write_seq(dsi, MIPI_DCS_READ_PPS_CONTINUE, 0x1c);
	mipi_dsi_dcs_write_seq(dsi, 0xaa, 0x28);
	mipi_dsi_dcs_write_seq(dsi, 0xab, 0x6c);
	mipi_dsi_dcs_write_seq(dsi, 0xac, 0x1b);
	mipi_dsi_dcs_write_seq(dsi, 0xad, 0x18);
	mipi_dsi_dcs_write_seq(dsi, 0xae, 0x4b);
	mipi_dsi_dcs_write_seq(dsi, 0xaf, 0x20);
	mipi_dsi_dcs_write_seq(dsi, 0xb0, 0x26);
	mipi_dsi_dcs_write_seq(dsi, 0xb1, 0x43);
	mipi_dsi_dcs_write_seq(dsi, 0xb2, 0x52);
	mipi_dsi_dcs_write_seq(dsi, 0xb3, 0x2c);
	mipi_dsi_dcs_write_seq(dsi, 0xc0, 0x08);
	mipi_dsi_dcs_write_seq(dsi, 0xc1, 0x22);
	mipi_dsi_dcs_write_seq(dsi, 0xc2, 0x30);
	mipi_dsi_dcs_write_seq(dsi, 0xc3, 0x15);
	mipi_dsi_dcs_write_seq(dsi, 0xc4, 0x17);
	mipi_dsi_dcs_write_seq(dsi, 0xc5, 0x2b);
	mipi_dsi_dcs_write_seq(dsi, 0xc6, 0x1f);
	mipi_dsi_dcs_write_seq(dsi, 0xc7, 0x1f);
	mipi_dsi_dcs_write_seq(dsi, 0xc8, 0x88);
	mipi_dsi_dcs_write_seq(dsi, 0xc9, 0x1c);
	mipi_dsi_dcs_write_seq(dsi, 0xca, 0x28);
	mipi_dsi_dcs_write_seq(dsi, 0xcb, 0x6c);
	mipi_dsi_dcs_write_seq(dsi, 0xcc, 0x1b);
	mipi_dsi_dcs_write_seq(dsi, 0xcd, 0x18);
	mipi_dsi_dcs_write_seq(dsi, 0xce, 0x4b);
	mipi_dsi_dcs_write_seq(dsi, 0xcf, 0x20);
	mipi_dsi_dcs_write_seq(dsi, 0xd0, 0x26);
	mipi_dsi_dcs_write_seq(dsi, 0xd1, 0x43);
	mipi_dsi_dcs_write_seq(dsi, 0xd2, 0x52);
	mipi_dsi_dcs_write_seq(dsi, 0xd3, 0x2c);
	mipi_dsi_dcs_write_seq(dsi, 0xff, 0x98, 0x81, 0x00);

	ret = mipi_dsi_dcs_exit_sleep_mode(dsi);
	if (ret < 0) {
		dev_err(dev, "Failed to exit sleep mode: %d\n", ret);
		return ret;
	}
	msleep(120);

	ret = mipi_dsi_dcs_set_display_on(dsi);
	if (ret < 0) {
		dev_err(dev, "Failed to set display on: %d\n", ret);
		return ret;
	}
	msleep(32);

	return 0;
}

static int ili9881c_off(struct ili9881c *ctx)
{
	struct mipi_dsi_device *dsi = ctx->dsi;
	struct device *dev = &dsi->dev;
	int ret;

	dsi->mode_flags &= ~MIPI_DSI_MODE_LPM;

	ret = mipi_dsi_dcs_set_display_off(dsi);
	if (ret < 0) {
		dev_err(dev, "Failed to set display off: %d\n", ret);
		return ret;
	}
	msleep(50);

	ret = mipi_dsi_dcs_enter_sleep_mode(dsi);
	if (ret < 0) {
		dev_err(dev, "Failed to enter sleep mode: %d\n", ret);
		return ret;
	}
	msleep(120);

	return 0;
}

static int ili9881c_prepare(struct drm_panel *panel)
{
	struct ili9881c *ctx = to_ili9881c(panel);
	struct device *dev = &ctx->dsi->dev;
	int ret;

	if (ctx->prepared)
		return 0;

	ret = regulator_enable(ctx->supply);
	if (ret < 0) {
		dev_err(dev, "Failed to enable regulator: %d\n", ret);
		return ret;
	}

	ili9881c_reset(ctx);

	ret = ili9881c_on(ctx);
	if (ret < 0) {
		dev_err(dev, "Failed to initialize panel: %d\n", ret);
		gpiod_set_value_cansleep(ctx->reset_gpio, 1);
		regulator_disable(ctx->supply);
		return ret;
	}

	ctx->prepared = true;
	return 0;
}

static int ili9881c_unprepare(struct drm_panel *panel)
{
	struct ili9881c *ctx = to_ili9881c(panel);
	struct device *dev = &ctx->dsi->dev;
	int ret;

	if (!ctx->prepared)
		return 0;

	ret = ili9881c_off(ctx);
	if (ret < 0)
		dev_err(dev, "Failed to un-initialize panel: %d\n", ret);

	gpiod_set_value_cansleep(ctx->reset_gpio, 1);
	regulator_disable(ctx->supply);

	ctx->prepared = false;
	return 0;
}

static const struct drm_display_mode ili9881c_mode = {
	.clock = (720 + 128 + 8 + 160) * (1280 + 8 + 8 + 20) * 60 / 1000,
	.hdisplay = 720,
	.hsync_start = 720 + 128,
	.hsync_end = 720 + 128 + 8,
	.htotal = 720 + 128 + 8 + 160,
	.vdisplay = 1280,
	.vsync_start = 1280 + 8,
	.vsync_end = 1280 + 8 + 8,
	.vtotal = 1280 + 8 + 8 + 20,
	.width_mm = 0,
	.height_mm = 0,
};

static int ili9881c_get_modes(struct drm_panel *panel,
			      struct drm_connector *connector)
{
	struct drm_display_mode *mode;

	mode = drm_mode_duplicate(connector->dev, &ili9881c_mode);
	if (!mode)
		return -ENOMEM;

	drm_mode_set_name(mode);

	mode->type = DRM_MODE_TYPE_DRIVER | DRM_MODE_TYPE_PREFERRED;
	connector->display_info.width_mm = mode->width_mm;
	connector->display_info.height_mm = mode->height_mm;
	drm_mode_probed_add(connector, mode);

	return 1;
}

static const struct drm_panel_funcs ili9881c_panel_funcs = {
	.prepare = ili9881c_prepare,
	.unprepare = ili9881c_unprepare,
	.get_modes = ili9881c_get_modes,
};

static int ili9881c_probe(struct mipi_dsi_device *dsi)
{
	struct device *dev = &dsi->dev;
	struct ili9881c *ctx;
	int ret;

	ctx = devm_kzalloc(dev, sizeof(*ctx), GFP_KERNEL);
	if (!ctx)
		return -ENOMEM;

	ctx->supply = devm_regulator_get(dev, "power");
	if (IS_ERR(ctx->supply))
		return dev_err_probe(dev, PTR_ERR(ctx->supply),
				     "Failed to get power regulator\n");

	ctx->reset_gpio = devm_gpiod_get(dev, "reset", GPIOD_OUT_HIGH);
	if (IS_ERR(ctx->reset_gpio))
		return dev_err_probe(dev, PTR_ERR(ctx->reset_gpio),
				     "Failed to get reset-gpios\n");

	ctx->dsi = dsi;
	mipi_dsi_set_drvdata(dsi, ctx);

	dsi->lanes = 4;
	dsi->format = MIPI_DSI_FMT_RGB888;
	dsi->mode_flags = MIPI_DSI_MODE_VIDEO | MIPI_DSI_MODE_VIDEO_BURST |
			  MIPI_DSI_MODE_VIDEO_HSE | MIPI_DSI_MODE_NO_EOT_PACKET |
			  MIPI_DSI_CLOCK_NON_CONTINUOUS;

	drm_panel_init(&ctx->panel, dev, &ili9881c_panel_funcs,
		       DRM_MODE_CONNECTOR_DSI);
	ctx->panel.prepare_prev_first = true;

	ret = drm_panel_of_backlight(&ctx->panel);
	if (ret)
		return dev_err_probe(dev, ret, "Failed to get backlight\n");

	drm_panel_add(&ctx->panel);

	ret = mipi_dsi_attach(dsi);
	if (ret < 0) {
		dev_err(dev, "Failed to attach to DSI host: %d\n", ret);
		drm_panel_remove(&ctx->panel);
		return ret;
	}

	return 0;
}

static void ili9881c_remove(struct mipi_dsi_device *dsi)
{
	struct ili9881c *ctx = mipi_dsi_get_drvdata(dsi);
	int ret;

	ret = mipi_dsi_detach(dsi);
	if (ret < 0)
		dev_err(&dsi->dev, "Failed to detach from DSI host: %d\n", ret);

	drm_panel_remove(&ctx->panel);
}

static const struct of_device_id ili9881c_of_match[] = {
	{ .compatible = "fareastone,ili9881c" }, // FIXME
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, ili9881c_of_match);

static struct mipi_dsi_driver ili9881c_driver = {
	.probe = ili9881c_probe,
	.remove = ili9881c_remove,
	.driver = {
		.name = "panel-ili9881c",
		.of_match_table = ili9881c_of_match,
	},
};
module_mipi_dsi_driver(ili9881c_driver);

MODULE_AUTHOR("linux-mdss-dsi-panel-driver-generator <fix@me>"); // FIXME
MODULE_DESCRIPTION("DRM driver for ili9881c hd video mode dsi panel");
MODULE_LICENSE("GPL");

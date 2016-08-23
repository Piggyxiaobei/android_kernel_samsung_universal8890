/*
 * drivers/video/decon/panels/S6E3HA0_lcd_ctrl.c
 *
 * Samsung SoC MIPI LCD CONTROL functions
 *
 * Copyright (c) 2014 Samsung Electronics
 *
 * Jiun Yu, <jiun.yu@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/of_gpio.h>
#include "panel_info.h"
#include "dsim_panel.h"

#ifdef CONFIG_PANEL_AID_DIMMING
#include "aid_dimming.h"
#include "dimming_core.h"
#include "s6e3ha5_wqhd_aid_dimming.h"
#endif
#include "s6e3hf4_s6e3ha5_wqhd_param.h"
#include "../dsim.h"
#include <video/mipi_display.h>

static unsigned int dynamic_lcd_type = 0;
static unsigned int hw_rev = 0;	// for dcdc set
static unsigned int lcdtype = 0;

#if defined(CONFIG_LCD_RES) && defined(CONFIG_FB_DSU)
#error cannot use both of CONFIG_LCD_RES and CONFIG_FB_DSU
#endif

#ifdef CONFIG_ALWAYS_RELOAD_MTP_FACTORY_BUILD
void update_mdnie_coordinate(u16 coordinate0, u16 coordinate1);
static int lcd_reload_mtp(int lcd_type, struct dsim_device *dsim);
#endif

#ifdef CONFIG_PANEL_AID_DIMMING
static const unsigned char *ACL_CUTOFF_TABLE[ACL_STATUS_MAX] = { SEQ_ACL_OFF, SEQ_ACL_ON };

static const unsigned char *ACL_OPR_TABLE_HF4[ACL_OPR_MAX] = { S6E3HF4_SEQ_ACL_OFF_OPR, S6E3HF4_SEQ_ACL_ON_OPR_3, S6E3HF4_SEQ_ACL_ON_OPR_6, S6E3HF4_SEQ_ACL_ON_OPR_8, S6E3HF4_SEQ_ACL_ON_OPR_12, S6E3HF4_SEQ_ACL_ON_OPR_15};
static const unsigned char *ACL_OPR_TABLE_HA5[ACL_OPR_MAX] = { S6E3HA5_SEQ_ACL_OFF_OPR, S6E3HA5_SEQ_ACL_ON_OPR_3, S6E3HA5_SEQ_ACL_ON_OPR_6, S6E3HA5_SEQ_ACL_ON_OPR_8, S6E3HA5_SEQ_ACL_ON_OPR_12, S6E3HA5_SEQ_ACL_ON_OPR_15};

static unsigned int br_tbl_hero2_420_a3_da[EXTEND_BRIGHTNESS + 1] = {
	2, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 5, 5,
	5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7,
	7, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13, 14,
	14, 15, 15, 16, 16, 17, 17, 19, 19, 20, 21, 21, 22, 22, 24, 24,
	25, 25, 27, 27, 29, 29, 30, 32, 32, 34, 34, 37, 37, 39, 39, 41,
	41, 44, 44, 47, 47, 50, 50, 53, 53, 56, 56, 60, 60, 64, 64, 68,
	68, 72, 72, 77, 77, 82, 82, 87, 87, 93, 93, 98, 98, 105, 105, 111,
	111, 119, 119, 126, 126, 134, 134, 143, 143, 152, 152, 162, 162, 172, 172, 183,
	183, 195, 195, 195, 195, 195, 195, 195, 207, 207, 207, 207, 207, 207, 207, 220,
	220, 220, 220, 220, 220, 220, 234, 234, 234, 234, 234, 234, 234, 234, 249, 249,
	249, 249, 249, 249, 249, 249, 265, 265, 265, 265, 265, 265, 265, 265, 265, 282,
	282, 282, 282, 282, 282, 282, 282, 282, 300, 300, 300, 300, 300, 300, 300, 300,
	300, 300, 316, 316, 316, 316, 316, 316, 316, 316, 333, 333, 333, 333, 333, 333,
	333, 333, 333, 350, 350, 350, 350, 350, 350, 350, 350, 350, 357, 357, 357, 357,
	365, 365, 365, 365, 372, 372, 372, 380, 380, 380, 380, 387, 387, 387, 387, 395,
	395, 395, 395, 403, 403, 403, 403, 412, 412, 412, 412, 420, 420, 420, 420, 420,
	[256 ... 281] = 420,
	[282 ... 295] = 465,
	[296 ... 309] = 488,
	[310 ... 323] = 510,
	[324 ... 336] = 533,
	[337 ... 350] = 555,
	[351 ... 364] = 578,
	[365 ... 365] = 600
};

static unsigned char inter_aor_tbl_hf4_a3_da[512] = {
	0x90, 0xE3,   0x90, 0xDF,	0x90, 0xDD,   0x90, 0xD3,	0x90, 0xD1,   0x90, 0xCF,	0x90, 0xCA,   0x90, 0xC3,	0x90, 0xC1,   0x90, 0xBF,	0x90, 0xBD,   0x90, 0xB3,	0x90, 0xB1,   0x90, 0xAF,	0x90, 0xAD,   0x90, 0xA3,
	0x90, 0xA1,   0x90, 0x9F,	0x90, 0x9D,   0x90, 0x93,	0x90, 0x91,   0x90, 0x8F,	0x90, 0x8D,   0x90, 0x83,	0x90, 0x81,   0x90, 0x7F,	0x90, 0x7D,   0x90, 0x7C,	0x90, 0x77,   0x90, 0x73,	0x90, 0x71,   0x90, 0x6F,
	0x90, 0x6D,   0x90, 0x63,	0x90, 0x61,   0x90, 0x57,	0x90, 0x4D,   0x90, 0x3F,	0x90, 0x31,   0x90, 0x27,	0x90, 0x1D,   0x90, 0x0F,	0x90, 0x02,   0x80, 0xF8,	0x80, 0xEE,   0x80, 0xE1,	0x80, 0xD4,   0x80, 0xCA,
	0x80, 0xC0,   0x80, 0xB2,	0x80, 0xA4,   0x80, 0x99,	0x80, 0x8E,   0x80, 0x81,	0x80, 0x73,   0x80, 0x5B,	0x80, 0x43,   0x80, 0x25,	0x80, 0x1C,   0x80, 0x13,	0x70, 0xEB,   0x70, 0xC3,	0x70, 0xC2,   0x70, 0xC1,
	0x70, 0xB7,   0x70, 0xAE,	0x70, 0x91,   0x70, 0x75,	0x70, 0x5C,   0x70, 0x43,	0x70, 0x2E,   0x70, 0x12,	0x60, 0xF5,   0x60, 0xDC,	0x60, 0xC3,   0x60, 0x9C,	0x60, 0x75,   0x60, 0x5C,	0x60, 0x43,   0x60, 0x28,
	0x60, 0x0D,   0x50, 0xE8,	0x50, 0xC3,   0x50, 0x9A,	0x50, 0x72,   0x50, 0x4B,	0x50, 0x23,   0x40, 0xFA,	0x40, 0xD1,   0x40, 0xAA,	0x40, 0x83,   0x40, 0x4C,	0x40, 0x15,   0x30, 0xE2,	0x30, 0xAF,   0x30, 0xC5,
	0x30, 0x94,   0x30, 0xC2,	0x30, 0x94,   0x30, 0xCA,	0x30, 0x94,   0x30, 0xC7,	0x30, 0x94,   0x30, 0xC4,	0x30, 0x94,   0x30, 0xCA,	0x30, 0x94,   0x30, 0xBF,	0x30, 0x94,   0x30, 0xCC,	0x30, 0x94,   0x30, 0xC1,
	0x30, 0x94,   0x30, 0xCC,	0x30, 0x94,   0x30, 0xC2,	0x30, 0x94,   0x30, 0xC6,	0x30, 0x94,   0x30, 0xC8,	0x30, 0x94,   0x30, 0xC5,	0x30, 0x94,   0x30, 0xC7,	0x30, 0x94,   0x30, 0xC4,	0x30, 0x94,   0x30, 0xC6,
	0x30, 0x94,   0x30, 0x83,	0x30, 0x73,   0x30, 0x62,	0x30, 0x51,   0x30, 0x40,	0x30, 0x30,   0x30, 0x1F,	0x30, 0x0C,   0x20, 0xF8,	0x20, 0xE5,   0x20, 0xD2,	0x20, 0xBF,   0x20, 0xAB,	0x20, 0x98,   0x20, 0x87,
	0x20, 0x75,   0x20, 0x64,	0x20, 0x53,   0x20, 0x42,	0x20, 0x30,   0x20, 0x1F,	0x20, 0x0C,   0x10, 0xF9,	0x10, 0xE7,   0x10, 0xD4,	0x10, 0xC1,   0x10, 0xAE,	0x10, 0x9C,   0x10, 0x89,	0x10, 0x7E,   0x10, 0x6C,
	0x10, 0x5B,   0x10, 0x49,	0x10, 0x38,   0x10, 0x26,	0x10, 0x15,   0x10, 0x03,	0x10, 0x80,   0x10, 0x70,	0x10, 0x61,   0x10, 0x51,	0x10, 0x41,   0x10, 0x32,	0x10, 0x22,   0x10, 0x13,	0x10, 0x03,   0x10, 0x80,
	0x10, 0x70,   0x10, 0x60,	0x10, 0x51,   0x10, 0x41,	0x10, 0x32,   0x10, 0x22,	0x10, 0x13,   0x10, 0x03,	0x10, 0x81,   0x10, 0x73,	0x10, 0x65,   0x10, 0x57,	0x10, 0x49,   0x10, 0x3B,	0x10, 0x2D,   0x10, 0x1F,
	0x10, 0x11,   0x10, 0x03,	0x10, 0x6A,   0x10, 0x5B,	0x10, 0x4D,   0x10, 0x3E,	0x10, 0x2F,   0x10, 0x20,	0x10, 0x12,   0x10, 0x03,	0x10, 0x6D,   0x10, 0x5F,	0x10, 0x52,   0x10, 0x45,	0x10, 0x38,   0x10, 0x2B,
	0x10, 0x1D,   0x10, 0x10,	0x10, 0x03,   0x10, 0x67,	0x10, 0x5B,   0x10, 0x4E,	0x10, 0x42,   0x10, 0x35,	0x10, 0x29,   0x10, 0x1C,	0x10, 0x10,   0x10, 0x03,	0x10, 0x25,   0x10, 0x1A,	0x10, 0x0E,   0x10, 0x03,
	0x10, 0x29,   0x10, 0x1C,	0x10, 0x10,   0x10, 0x03,	0x00, 0xEE,   0x00, 0xD8,	0x00, 0xC3,   0x00, 0xB5,	0x00, 0xA6,   0x00, 0x98,	0x00, 0x8A,   0x00, 0x85,	0x00, 0x80,   0x00, 0x7C,	0x00, 0x77,   0x00, 0x66,
	0x00, 0x55,   0x00, 0x44,	0x00, 0x33,   0x00, 0x32,	0x00, 0x26,   0x00, 0x19,	0x00, 0x0C,   0x00, 0x36,	0x00, 0x28,   0x00, 0x1A,	0x00, 0x0C,   0x00, 0x31,	0x00, 0x24,   0x00, 0x18,	0x00, 0x0C,   0x00, 0x0C,
};

static const short center_gamma[NUM_VREF][CI_MAX] = {
	{0x080, 0x080, 0x080},
	{0x080, 0x080, 0x080},
	{0x080, 0x080, 0x080},
	{0x080, 0x080, 0x080},
	{0x080, 0x080, 0x080},
	{0x080, 0x080, 0x080},
	{0x080, 0x080, 0x080},
	{0x080, 0x080, 0x080},
	{0x080, 0x080, 0x080},
	{0x100, 0x100, 0x100},
};

struct SmtDimInfo dimming_info_HF4_A3_da[MAX_BR_INFO] = {
	{ .br = 2, .refBr = 116, .cGma = gma2p15, .rTbl = a3_da_rtbl2nit, .cTbl = a3_da_ctbl2nit, .aid = aid9795, .elvCaps = elvCaps5, .elv = elv5, .way = W1, .elvss_offset = elvss_offset1},
	{ .br = 3, .refBr = 116, .cGma = gma2p15, .rTbl = a3_da_rtbl3nit, .cTbl = a3_da_ctbl3nit, .aid = aid9698, .elvCaps = elvCaps4, .elv = elv4, .way = W1, .elvss_offset = elvss_offset2},
	{ .br = 4, .refBr = 116, .cGma = gma2p15, .rTbl = a3_da_rtbl4nit, .cTbl = a3_da_ctbl4nit, .aid = aid9594, .elvCaps = elvCaps3, .elv = elv3, .way = W1, .elvss_offset = elvss_offset3},
	{ .br = 5, .refBr = 116, .cGma = gma2p15, .rTbl = a3_da_rtbl5nit, .cTbl = a3_da_ctbl5nit, .aid = aid9485, .elvCaps = elvCaps2, .elv = elv2, .way = W1, .elvss_offset = elvss_offset4},
	{ .br = 6, .refBr = 116, .cGma = gma2p15, .rTbl = a3_da_rtbl6nit, .cTbl = a3_da_ctbl6nit, .aid = aid9396, .elvCaps = elvCaps1, .elv = elv1, .way = W1, .elvss_offset = elvss_offset5},
	{ .br = 7, .refBr = 116, .cGma = gma2p15, .rTbl = a3_da_rtbl7nit, .cTbl = a3_da_ctbl7nit, .aid = aid9292, .elvCaps = elvCaps1, .elv = elv1, .way = W1, .elvss_offset = elvss_offset5},
	{ .br = 8, .refBr = 116, .cGma = gma2p15, .rTbl = a3_da_rtbl8nit, .cTbl = a3_da_ctbl8nit, .aid = aid9214, .elvCaps = elvCaps1, .elv = elv1, .way = W1, .elvss_offset = elvss_offset5},
	{ .br = 9, .refBr = 116, .cGma = gma2p15, .rTbl = a3_da_rtbl9nit, .cTbl = a3_da_ctbl9nit, .aid = aid9106, .elvCaps = elvCaps1, .elv = elv1, .way = W1, .elvss_offset = elvss_offset5},
	{ .br = 10, .refBr = 116, .cGma = gma2p15, .rTbl = a3_da_rtbl10nit, .cTbl = a3_da_ctbl10nit, .aid = aid9029, .elvCaps = elvCaps1, .elv = elv1, .way = W1, .elvss_offset = elvss_offset5},
	{ .br = 11, .refBr = 116, .cGma = gma2p15, .rTbl = a3_da_rtbl11nit, .cTbl = a3_da_ctbl11nit, .aid = aid8924, .elvCaps = elvCaps1, .elv = elv1, .way = W1, .elvss_offset = elvss_offset5},
	{ .br = 12, .refBr = 116, .cGma = gma2p15, .rTbl = a3_da_rtbl12nit, .cTbl = a3_da_ctbl12nit, .aid = aid8847, .elvCaps = elvCaps1, .elv = elv1, .way = W1, .elvss_offset = elvss_offset5},
	{ .br = 13, .refBr = 116, .cGma = gma2p15, .rTbl = a3_da_rtbl13nit, .cTbl = a3_da_ctbl13nit, .aid = aid8746, .elvCaps = elvCaps1, .elv = elv1, .way = W1, .elvss_offset = elvss_offset5},
	{ .br = 14, .refBr = 116, .cGma = gma2p15, .rTbl = a3_da_rtbl14nit, .cTbl = a3_da_ctbl14nit, .aid = aid8669, .elvCaps = elvCaps1, .elv = elv1, .way = W1, .elvss_offset = elvss_offset5},
	{ .br = 15, .refBr = 116, .cGma = gma2p15, .rTbl = a3_da_rtbl15nit, .cTbl = a3_da_ctbl15nit, .aid = aid8560, .elvCaps = elvCaps1, .elv = elv1, .way = W1, .elvss_offset = elvss_offset5},
	{ .br = 16, .refBr = 116, .cGma = gma2p15, .rTbl = a3_da_rtbl16nit, .cTbl = a3_da_ctbl16nit, .aid = aid8475, .elvCaps = elvCaps1, .elv = elv1, .way = W1, .elvss_offset = elvss_offset5},
	{ .br = 17, .refBr = 116, .cGma = gma2p15, .rTbl = a3_da_rtbl17nit, .cTbl = a3_da_ctbl17nit, .aid = aid8371, .elvCaps = elvCaps1, .elv = elv1, .way = W1, .elvss_offset = elvss_offset5},
	{ .br = 19, .refBr = 116, .cGma = gma2p15, .rTbl = a3_da_rtbl19nit, .cTbl = a3_da_ctbl19nit, .aid = aid8185, .elvCaps = elvCaps1, .elv = elv1, .way = W1, .elvss_offset = elvss_offset5},
	{ .br = 20, .refBr = 116, .cGma = gma2p15, .rTbl = a3_da_rtbl20nit, .cTbl = a3_da_ctbl20nit, .aid = aid8069, .elvCaps = elvCaps1, .elv = elv1, .way = W1, .elvss_offset = elvss_offset5},
	{ .br = 21, .refBr = 116, .cGma = gma2p15, .rTbl = a3_da_rtbl21nit, .cTbl = a3_da_ctbl21nit, .aid = aid7999, .elvCaps = elvCaps1, .elv = elv1, .way = W1, .elvss_offset = elvss_offset5},
	{ .br = 22, .refBr = 116, .cGma = gma2p15, .rTbl = a3_da_rtbl22nit, .cTbl = a3_da_ctbl22nit, .aid = aid7690, .elvCaps = elvCaps1, .elv = elv1, .way = W1, .elvss_offset = elvss_offset5},
	{ .br = 24, .refBr = 116, .cGma = gma2p15, .rTbl = a3_da_rtbl24nit, .cTbl = a3_da_ctbl24nit, .aid = aid7682, .elvCaps = elvCaps1, .elv = elv1, .way = W1, .elvss_offset = elvss_offset5},
	{ .br = 25, .refBr = 116, .cGma = gma2p15, .rTbl = a3_da_rtbl25nit, .cTbl = a3_da_ctbl25nit, .aid = aid7608, .elvCaps = elvCaps1, .elv = elv1, .way = W1, .elvss_offset = elvss_offset5},
	{ .br = 27, .refBr = 116, .cGma = gma2p15, .rTbl = a3_da_rtbl27nit, .cTbl = a3_da_ctbl27nit, .aid = aid7388, .elvCaps = elvCaps1, .elv = elv1, .way = W1, .elvss_offset = elvss_offset5},
	{ .br = 29, .refBr = 116, .cGma = gma2p15, .rTbl = a3_da_rtbl29nit, .cTbl = a3_da_ctbl29nit, .aid = aid7194, .elvCaps = elvCaps1, .elv = elv1, .way = W1, .elvss_offset = elvss_offset5},
	{ .br = 30, .refBr = 116, .cGma = gma2p15, .rTbl = a3_da_rtbl30nit, .cTbl = a3_da_ctbl30nit, .aid = aid7113, .elvCaps = elvCaps1, .elv = elv1, .way = W1, .elvss_offset = elvss_offset5},
	{ .br = 32, .refBr = 116, .cGma = gma2p15, .rTbl = a3_da_rtbl32nit, .cTbl = a3_da_ctbl32nit, .aid = aid6892, .elvCaps = elvCaps1, .elv = elv1, .way = W1, .elvss_offset = elvss_offset5},
	{ .br = 34, .refBr = 116, .cGma = gma2p15, .rTbl = a3_da_rtbl34nit, .cTbl = a3_da_ctbl34nit, .aid = aid6699, .elvCaps = elvCaps1, .elv = elv1, .way = W1, .elvss_offset = elvss_offset5},
	{ .br = 37, .refBr = 116, .cGma = gma2p15, .rTbl = a3_da_rtbl37nit, .cTbl = a3_da_ctbl37nit, .aid = aid6397, .elvCaps = elvCaps1, .elv = elv1, .way = W1, .elvss_offset = elvss_offset5},
	{ .br = 39, .refBr = 116, .cGma = gma2p15, .rTbl = a3_da_rtbl39nit, .cTbl = a3_da_ctbl39nit, .aid = aid6204, .elvCaps = elvCaps1, .elv = elv1, .way = W1, .elvss_offset = elvss_offset5},
	{ .br = 41, .refBr = 116, .cGma = gma2p15, .rTbl = a3_da_rtbl41nit, .cTbl = a3_da_ctbl41nit, .aid = aid5995, .elvCaps = elvCaps1, .elv = elv1, .way = W1, .elvss_offset = elvss_offset5},
	{ .br = 44, .refBr = 116, .cGma = gma2p15, .rTbl = a3_da_rtbl44nit, .cTbl = a3_da_ctbl44nit, .aid = aid5708, .elvCaps = elvCaps1, .elv = elv1, .way = W1, .elvss_offset = elvss_offset5},
	{ .br = 47, .refBr = 116, .cGma = gma2p15, .rTbl = a3_da_rtbl47nit, .cTbl = a3_da_ctbl47nit, .aid = aid5395, .elvCaps = elvCaps1, .elv = elv1, .way = W1, .elvss_offset = elvss_offset5},
	{ .br = 50, .refBr = 116, .cGma = gma2p15, .rTbl = a3_da_rtbl50nit, .cTbl = a3_da_ctbl50nit, .aid = aid5089, .elvCaps = elvCaps1, .elv = elv1, .way = W1, .elvss_offset = elvss_offset5},
	{ .br = 53, .refBr = 116, .cGma = gma2p15, .rTbl = a3_da_rtbl53nit, .cTbl = a3_da_ctbl53nit, .aid = aid4772, .elvCaps = elvCaps1, .elv = elv1, .way = W1, .elvss_offset = elvss_offset5},
	{ .br = 56, .refBr = 116, .cGma = gma2p15, .rTbl = a3_da_rtbl56nit, .cTbl = a3_da_ctbl56nit, .aid = aid4470, .elvCaps = elvCaps1, .elv = elv1, .way = W1, .elvss_offset = elvss_offset5},
	{ .br = 60, .refBr = 116, .cGma = gma2p15, .rTbl = a3_da_rtbl60nit, .cTbl = a3_da_ctbl60nit, .aid = aid4044, .elvCaps = elvCaps1, .elv = elv1, .way = W1, .elvss_offset = elvss_offset5},
	{ .br = 64, .refBr = 116, .cGma = gma2p15, .rTbl = a3_da_rtbl64nit, .cTbl = a3_da_ctbl64nit, .aid = aid3649, .elvCaps = elvCaps1, .elv = elv1, .way = W1, .elvss_offset = elvss_offset5},
	{ .br = 68, .refBr = 121, .cGma = gma2p15, .rTbl = a3_da_rtbl68nit, .cTbl = a3_da_ctbl68nit, .aid = aid3545, .elvCaps = elvCaps1, .elv = elv1, .way = W1, .elvss_offset = elvss_offset5},
	{ .br = 72, .refBr = 127, .cGma = gma2p15, .rTbl = a3_da_rtbl72nit, .cTbl = a3_da_ctbl72nit, .aid = aid3545, .elvCaps = elvCaps1, .elv = elv1, .way = W1, .elvss_offset = elvss_offset5},
	{ .br = 77, .refBr = 137, .cGma = gma2p15, .rTbl = a3_da_rtbl77nit, .cTbl = a3_da_ctbl77nit, .aid = aid3545, .elvCaps = elvCaps1, .elv = elv1, .way = W1, .elvss_offset = elvss_offset5},
	{ .br = 82, .refBr = 142, .cGma = gma2p15, .rTbl = a3_da_rtbl82nit, .cTbl = a3_da_ctbl82nit, .aid = aid3545, .elvCaps = elvCaps1, .elv = elv1, .way = W1, .elvss_offset = elvss_offset5},
	{ .br = 87, .refBr = 153, .cGma = gma2p15, .rTbl = a3_da_rtbl87nit, .cTbl = a3_da_ctbl87nit, .aid = aid3545, .elvCaps = elvCaps1, .elv = elv1, .way = W1, .elvss_offset = elvss_offset5},
	{ .br = 93, .refBr = 163, .cGma = gma2p15, .rTbl = a3_da_rtbl93nit, .cTbl = a3_da_ctbl93nit, .aid = aid3545, .elvCaps = elvCaps1, .elv = elv1, .way = W1, .elvss_offset = elvss_offset5},
	{ .br = 98, .refBr = 172, .cGma = gma2p15, .rTbl = a3_da_rtbl98nit, .cTbl = a3_da_ctbl98nit, .aid = aid3545, .elvCaps = elvCaps1, .elv = elv1, .way = W1, .elvss_offset = elvss_offset5},
	{ .br = 105, .refBr = 181, .cGma = gma2p15, .rTbl = a3_da_rtbl105nit, .cTbl = a3_da_ctbl105nit, .aid = aid3545, .elvCaps = elvCaps1, .elv = elv1, .way = W1, .elvss_offset = elvss_offset5},
	{ .br = 111, .refBr = 193, .cGma = gma2p15, .rTbl = a3_da_rtbl111nit, .cTbl = a3_da_ctbl111nit, .aid = aid3545, .elvCaps = elvCaps1, .elv = elv1, .way = W1, .elvss_offset = elvss_offset5},
	{ .br = 119, .refBr = 205, .cGma = gma2p15, .rTbl = a3_da_rtbl119nit, .cTbl = a3_da_ctbl119nit, .aid = aid3545, .elvCaps = elvCaps1, .elv = elv1, .way = W1, .elvss_offset = elvss_offset5},
	{ .br = 126, .refBr = 217, .cGma = gma2p15, .rTbl = a3_da_rtbl126nit, .cTbl = a3_da_ctbl126nit, .aid = aid3545, .elvCaps = elvCaps1, .elv = elv1, .way = W1, .elvss_offset = elvss_offset5},
	{ .br = 134, .refBr = 228, .cGma = gma2p15, .rTbl = a3_da_rtbl134nit, .cTbl = a3_da_ctbl134nit, .aid = aid3545, .elvCaps = elvCaps2, .elv = elv2, .way = W1, .elvss_offset = elvss_offset5},
	{ .br = 143, .refBr = 241, .cGma = gma2p15, .rTbl = a3_da_rtbl143nit, .cTbl = a3_da_ctbl143nit, .aid = aid3545, .elvCaps = elvCaps2, .elv = elv2, .way = W1, .elvss_offset = elvss_offset5},
	{ .br = 152, .refBr = 252, .cGma = gma2p15, .rTbl = a3_da_rtbl152nit, .cTbl = a3_da_ctbl152nit, .aid = aid3545, .elvCaps = elvCaps3, .elv = elv3, .way = W1, .elvss_offset = elvss_offset5},
	{ .br = 162, .refBr = 265, .cGma = gma2p15, .rTbl = a3_da_rtbl162nit, .cTbl = a3_da_ctbl162nit, .aid = aid3545, .elvCaps = elvCaps4, .elv = elv4, .way = W1, .elvss_offset = elvss_offset5},
	{ .br = 172, .refBr = 277, .cGma = gma2p15, .rTbl = a3_da_rtbl172nit, .cTbl = a3_da_ctbl172nit, .aid = aid3545, .elvCaps = elvCaps4, .elv = elv4, .way = W1, .elvss_offset = elvss_offset5},
	{ .br = 183, .refBr = 294, .cGma = gma2p15, .rTbl = a3_da_rtbl183nit, .cTbl = a3_da_ctbl183nit, .aid = aid3545, .elvCaps = elvCaps4, .elv = elv4, .way = W1, .elvss_offset = elvss_offset5},
	{ .br = 195, .refBr = 294, .cGma = gma2p15, .rTbl = a3_da_rtbl195nit, .cTbl = a3_da_ctbl195nit, .aid = aid3092, .elvCaps = elvCaps4, .elv = elv4, .way = W1, .elvss_offset = elvss_offset5},
	{ .br = 207, .refBr = 294, .cGma = gma2p15, .rTbl = a3_da_rtbl207nit, .cTbl = a3_da_ctbl207nit, .aid = aid2570, .elvCaps = elvCaps4, .elv = elv4, .way = W1, .elvss_offset = elvss_offset5},
	{ .br = 220, .refBr = 294, .cGma = gma2p15, .rTbl = a3_da_rtbl220nit, .cTbl = a3_da_ctbl220nit, .aid = aid2101, .elvCaps = elvCaps5, .elv = elv5, .way = W1, .elvss_offset = elvss_offset5},
	{ .br = 234, .refBr = 294, .cGma = gma2p15, .rTbl = a3_da_rtbl234nit, .cTbl = a3_da_ctbl234nit, .aid = aid1521, .elvCaps = elvCaps5, .elv = elv5, .way = W1, .elvss_offset = elvss_offset5},
	{ .br = 249, .refBr = 300, .cGma = gma2p15, .rTbl = a3_da_rtbl249nit, .cTbl = a3_da_ctbl249nit, .aid = aid1002, .elvCaps = elvCaps5, .elv = elv5, .way = W1, .elvss_offset = elvss_offset5},
	{ .br = 265, .refBr = 316, .cGma = gma2p15, .rTbl = a3_da_rtbl265nit, .cTbl = a3_da_ctbl265nit, .aid = aid1002, .elvCaps = elvCaps5, .elv = elv5, .way = W1, .elvss_offset = elvss_offset5},
	{ .br = 282, .refBr = 332, .cGma = gma2p15, .rTbl = a3_da_rtbl282nit, .cTbl = a3_da_ctbl282nit, .aid = aid1002, .elvCaps = elvCaps6, .elv = elv6, .way = W1, .elvss_offset = elvss_offset5},
	{ .br = 300, .refBr = 348, .cGma = gma2p15, .rTbl = a3_da_rtbl300nit, .cTbl = a3_da_ctbl300nit, .aid = aid1002, .elvCaps = elvCaps6, .elv = elv6, .way = W1, .elvss_offset = elvss_offset5},
	{ .br = 316, .refBr = 361, .cGma = gma2p15, .rTbl = a3_da_rtbl316nit, .cTbl = a3_da_ctbl316nit, .aid = aid1002, .elvCaps = elvCaps7, .elv = elv7, .way = W1, .elvss_offset = elvss_offset5},
	{ .br = 333, .refBr = 378, .cGma = gma2p15, .rTbl = a3_da_rtbl333nit, .cTbl = a3_da_ctbl333nit, .aid = aid1002, .elvCaps = elvCaps8, .elv = elv8, .way = W1, .elvss_offset = elvss_offset5},
	{ .br = 350, .refBr = 395, .cGma = gma2p15, .rTbl = a3_da_rtbl350nit, .cTbl = a3_da_ctbl350nit, .aid = aid1002, .elvCaps = elvCaps8, .elv = elv8, .way = W1, .elvss_offset = elvss_offset5},
	{ .br = 357, .refBr = 399, .cGma = gma2p15, .rTbl = a3_da_rtbl357nit, .cTbl = a3_da_ctbl357nit, .aid = aid1002, .elvCaps = elvCaps8, .elv = elv8, .way = W1, .elvss_offset = elvss_offset5},
	{ .br = 365, .refBr = 406, .cGma = gma2p15, .rTbl = a3_da_rtbl365nit, .cTbl = a3_da_ctbl365nit, .aid = aid1002, .elvCaps = elvCaps9, .elv = elv9, .way = W1, .elvss_offset = elvss_offset5},
	{ .br = 372, .refBr = 406, .cGma = gma2p15, .rTbl = a3_da_rtbl372nit, .cTbl = a3_da_ctbl372nit, .aid = aid755, .elvCaps = elvCaps9, .elv = elv9, .way = W1, .elvss_offset = elvss_offset5},
	{ .br = 380, .refBr = 406, .cGma = gma2p15, .rTbl = a3_da_rtbl380nit, .cTbl = a3_da_ctbl380nit, .aid = aid534, .elvCaps = elvCaps9, .elv = elv9, .way = W1, .elvss_offset = elvss_offset5},
	{ .br = 387, .refBr = 406, .cGma = gma2p15, .rTbl = a3_da_rtbl387nit, .cTbl = a3_da_ctbl387nit, .aid = aid461, .elvCaps = elvCaps10, .elv = elv10, .way = W1, .elvss_offset = elvss_offset5},
	{ .br = 395, .refBr = 406, .cGma = gma2p15, .rTbl = a3_da_rtbl395nit, .cTbl = a3_da_ctbl395nit, .aid = aid197, .elvCaps = elvCaps10, .elv = elv10, .way = W1, .elvss_offset = elvss_offset5},
	{ .br = 403, .refBr = 409, .cGma = gma2p15, .rTbl = a3_da_rtbl403nit, .cTbl = a3_da_ctbl403nit, .aid = aid46, .elvCaps = elvCaps10, .elv = elv10, .way = W1, .elvss_offset = elvss_offset5},
	{ .br = 412, .refBr = 416, .cGma = gma2p15, .rTbl = a3_da_rtbl412nit, .cTbl = a3_da_ctbl412nit, .aid = aid46, .elvCaps = elvCaps11, .elv = elv11, .way = W1, .elvss_offset = elvss_offset5},
	{ .br = 420, .refBr = 420, .cGma = gma2p20, .rTbl = a3_da_rtbl420nit, .cTbl = a3_da_ctbl420nit, .aid = aid46, .elvCaps = elvCaps11, .elv = elv11, .way = W2, .elvss_offset = elvss_offset5},
/*hbm interpolation */
	{ .br = 443, .refBr = 443, .cGma = gma2p20, .rTbl = a3_da_rtbl420nit, .cTbl = a3_da_ctbl420nit, .aid = aid46, .elvCaps = elvCaps12, .elv = elv12, .way = W3, .elvss_offset = elvss_offset5},
	{ .br = 465, .refBr = 465, .cGma = gma2p20, .rTbl = a3_da_rtbl420nit, .cTbl = a3_da_ctbl420nit, .aid = aid46, .elvCaps = elvCaps21, .elv = elv21, .way = W3, .elvss_offset = elvss_offset5},
	{ .br = 488, .refBr = 488, .cGma = gma2p20, .rTbl = a3_da_rtbl420nit, .cTbl = a3_da_ctbl420nit, .aid = aid46, .elvCaps = elvCaps14, .elv = elv14, .way = W3, .elvss_offset = elvss_offset5},
	{ .br = 510, .refBr = 510, .cGma = gma2p20, .rTbl = a3_da_rtbl420nit, .cTbl = a3_da_ctbl420nit, .aid = aid46, .elvCaps = elvCaps15, .elv = elv15, .way = W3, .elvss_offset = elvss_offset5},
	{ .br = 533, .refBr = 533, .cGma = gma2p20, .rTbl = a3_da_rtbl420nit, .cTbl = a3_da_ctbl420nit, .aid = aid46, .elvCaps = elvCaps16, .elv = elv16, .way = W3, .elvss_offset = elvss_offset5},
	{ .br = 555, .refBr = 555, .cGma = gma2p20, .rTbl = a3_da_rtbl420nit, .cTbl = a3_da_ctbl420nit, .aid = aid46, .elvCaps = elvCaps18, .elv = elv18, .way = W3, .elvss_offset = elvss_offset5},
	{ .br = 578, .refBr = 578, .cGma = gma2p20, .rTbl = a3_da_rtbl420nit, .cTbl = a3_da_ctbl420nit, .aid = aid46, .elvCaps = elvCaps19, .elv = elv19, .way = W3, .elvss_offset = elvss_offset5},
/* hbm */
	{ .br = 600, .refBr = 600, .cGma = gma2p20, .rTbl = a3_da_rtbl420nit, .cTbl = a3_da_ctbl420nit, .aid = aid46, .elvCaps = elvCaps20, .elv = elv20, .way = W4, .elvss_offset = elvss_offset5},
};


static int set_gamma_to_center(struct SmtDimInfo *brInfo)
{
	int i, j;
	int ret = 0;
	unsigned int index = 0;
	unsigned char *result = brInfo->gamma;

	result[index++] = OLED_CMD_GAMMA;

	for (i = V255; i >= 0; i--) {
		for (j = 0; j < CI_MAX; j++) {
			if (i == V255) {
				result[index++] = (unsigned char)((center_gamma[i][j] >> 8) & 0x01);
				result[index++] = (unsigned char)center_gamma[i][j] & 0xff;
			}
			else {
				result[index++] = (unsigned char)center_gamma[i][j] & 0xff;
			}
		}
	}
	result[index++] = 0x00;
	result[index++] = 0x00;

	return ret;
}

/* gamma interpolaion table */
const unsigned int tbl_hbm_inter[7] = {
	131, 256, 387, 512, 643, 768, 899
};

static int interpolation_gamma_to_hbm(struct SmtDimInfo *dimInfo, int br_idx)
{
	int i, j;
	int ret = 0;
	int idx = 0;
	int tmp = 0;
	int hbmcnt, refcnt, gap = 0;
	int ref_idx = 0;
	int hbm_idx = 0;
	int rst = 0;
	int hbm_tmp, ref_tmp;
	unsigned char *result = dimInfo[br_idx].gamma;

	for (i = 0; i < MAX_BR_INFO; i++) {
		if (dimInfo[i].br == S6E3HA3_MAX_BRIGHTNESS)
			ref_idx = i;

		if (dimInfo[i].br == S6E3HA3_HBM_BRIGHTNESS)
			hbm_idx = i;
	}

	if ((ref_idx == 0) || (hbm_idx == 0)) {
		dsim_info("%s failed to get index ref index : %d, hbm index : %d\n", __func__, ref_idx, hbm_idx);
		ret = -EINVAL;
		goto exit;
	}

	result[idx++] = OLED_CMD_GAMMA;
	tmp = (br_idx - ref_idx) - 1;

	hbmcnt = 1;
	refcnt = 1;

	for (i = V255; i >= 0; i--) {
		for (j = 0; j < CI_MAX; j++) {
			if (i == V255) {
				hbm_tmp = (dimInfo[hbm_idx].gamma[hbmcnt] << 8) | (dimInfo[hbm_idx].gamma[hbmcnt + 1]);
				ref_tmp = (dimInfo[ref_idx].gamma[refcnt] << 8) | (dimInfo[ref_idx].gamma[refcnt + 1]);

				if (hbm_tmp > ref_tmp) {
					gap = hbm_tmp - ref_tmp;
					rst = (gap * tbl_hbm_inter[tmp]) >> 10;
					rst += ref_tmp;
				}
				else {
					gap = ref_tmp - hbm_tmp;
					rst = (gap * tbl_hbm_inter[tmp]) >> 10;
					rst = ref_tmp - rst;
				}
				result[idx++] = (unsigned char)((rst >> 8) & 0x01);
				result[idx++] = (unsigned char)rst & 0xff;
				hbmcnt += 2;
				refcnt += 2;
			}
			else {
				hbm_tmp = dimInfo[hbm_idx].gamma[hbmcnt++];
				ref_tmp = dimInfo[ref_idx].gamma[refcnt++];

				if (hbm_tmp > ref_tmp) {
					gap = hbm_tmp - ref_tmp;
					rst = (gap * tbl_hbm_inter[tmp]) >> 10;
					rst += ref_tmp;
				}
				else {
					gap = ref_tmp - hbm_tmp;
					rst = (gap * tbl_hbm_inter[tmp]) >> 10;
					rst = ref_tmp - rst;
				}
				result[idx++] = (unsigned char)rst & 0xff;
			}
		}
	}

	dsim_info("tmp index : %d\n", tmp);

exit:
	return ret;
}


#endif

#ifdef CONFIG_DSIM_ESD_REMOVE_DISP_DET
int esd_get_ddi_status(struct dsim_device *dsim, int reg)
{
	int ret = 0;
	unsigned char err_buf[4] = { 0, };

	dsim_info("[REM_DISP_DET] %s : reg = %x \n", __func__, reg);

	if (reg == 0x0A) {
		ret = dsim_read_hl_data(dsim, 0x0A, 3, err_buf);
		if (ret != 3) {
			dsim_err("[REM_DISP_DET] %s : can't read Panel's 0A Reg\n", __func__);
			goto dump_exit;
		}

		dsim_err("[REM_DISP_DET] %s : 0x0A : buf[0] = %x\n", __func__, err_buf[0]);

		ret = err_buf[0];
	}
	else {
		dsim_dbg("[REM_DISP_DET] %s : reg = %x is not ready \n", __func__, reg);
	}

dump_exit:
	return ret;

}
#endif

// convert NEW SPEC to OLD SPEC
static int VT_RGB2GRB(unsigned char *vt_in_mtp)
{
	int ret = 0;
	int r, g, b;

	// new SPEC = RG0B
	r = (vt_in_mtp[0] & 0xF0) >> 4;
	g = (vt_in_mtp[0] & 0x0F);
	b = (vt_in_mtp[1] & 0x0F);

	// old spec = GR0B
	vt_in_mtp[0] = g << 4 | r;
	vt_in_mtp[1] = b;

	return ret;
}

#ifdef CONFIG_LCD_HMT
void display_off_for_VR(struct dsim_device *dsim)
{
	int ret = 0;
	ret = dsim_write_hl_data(dsim, SEQ_DISPLAY_OFF, ARRAY_SIZE(SEQ_DISPLAY_OFF));
	if (ret < 0) {
		dsim_err("%s : fail to write CMD : DISPLAY_OFF\n", __func__);
	}
	pr_info("%s\n", __func__);
}
#endif

#if defined(CONFIG_FB_DSU) || defined(CONFIG_LCD_RES)
static int last_dsc_enabled = true;	// bootloader is true
#endif

/******************** HF4 ********************/

static int s6e3hf4_set_gamma_to_hbm(struct SmtDimInfo *brInfo, struct dim_data *dimData, u8 * hbm)
{
	int ret = 0;
	unsigned int index = 0;
	unsigned char *result = brInfo->gamma;
	int i = 0;
	memset(result, 0, OLED_CMD_GAMMA_CNT);

	result[index++] = OLED_CMD_GAMMA;

	while (index < OLED_CMD_GAMMA_CNT) {
		if (((i >= 50) && (i < 65)) || ((i >= 68) && (i < 88)))
			result[index++] = hbm[i];
		i++;
	}
	for (i = 0; i < OLED_CMD_GAMMA_CNT; i++)
		dsim_info("%d : %d\n", i + 1, result[i]);
	return ret;
}

static int hf4_init_dimming(struct dsim_device *dsim, u8 * mtp, u8 * hbm)
{
	int i, j;
	int pos = 0;
	int ret = 0;
	short temp;
	int method = 0;
	static struct dim_data *dimming = NULL;

	struct panel_private *panel = &dsim->priv;
	struct SmtDimInfo *diminfo = NULL;
	int string_offset;
	char string_buf[1024];

	if (dimming == NULL) {
		dimming = (struct dim_data *)kmalloc(sizeof(struct dim_data), GFP_KERNEL);
		if (!dimming) {
			dsim_err("failed to allocate memory for dim data\n");
			ret = -ENOMEM;
			goto error;
		}
	}
	memset(panel->irc_table, 0x00, sizeof(panel->irc_table));

	diminfo = (void *)dimming_info_HF4_A3_da;
	panel->inter_aor_tbl = inter_aor_tbl_hf4_a3_da;
	panel->br_tbl = (unsigned int *)br_tbl_hero2_420_a3_da;
	memcpy(panel->irc_table, irc_table_HF4_A3_da, sizeof(irc_table_HF4_A3_da));

	panel->dim_data = (void *)dimming;
	panel->dim_info = (void *)diminfo;

	for (j = 0; j < CI_MAX; j++) {
		temp = ((mtp[pos] & 0x01) ? -1 : 1) * mtp[pos + 1];
		dimming->t_gamma[V255][j] = (int)center_gamma[V255][j] + temp;
		dimming->mtp[V255][j] = temp;
		pos += 2;
	}

	for (i = V203; i >= 0; i--) {
		for (j = 0; j < CI_MAX; j++) {
			temp = ((mtp[pos] & 0x80) ? -1 : 1) * (mtp[pos] & 0x7f);
			dimming->t_gamma[i][j] = (int)center_gamma[i][j] + temp;
			dimming->mtp[i][j] = temp;
			pos++;
		}
	}
	/* for vt */
	temp = (mtp[pos + 1]) << 8 | mtp[pos];

	for (i = 0; i < CI_MAX; i++)
		dimming->vt_mtp[i] = (temp >> (i * 4)) & 0x0f;
#ifdef SMART_DIMMING_DEBUG
	dimm_info("Center Gamma Info : \n");
	for (i = 0; i < VMAX; i++) {
		dsim_info("Gamma : %3d %3d %3d : %3x %3x %3x\n",
			  dimming->t_gamma[i][CI_RED], dimming->t_gamma[i][CI_GREEN], dimming->t_gamma[i][CI_BLUE],
			  dimming->t_gamma[i][CI_RED], dimming->t_gamma[i][CI_GREEN], dimming->t_gamma[i][CI_BLUE]);
	}
#endif
	dimm_info("VT MTP : \n");
	dimm_info("Gamma : %3d %3d %3d : %3x %3x %3x\n",
		  dimming->vt_mtp[CI_RED], dimming->vt_mtp[CI_GREEN], dimming->vt_mtp[CI_BLUE],
		  dimming->vt_mtp[CI_RED], dimming->vt_mtp[CI_GREEN], dimming->vt_mtp[CI_BLUE]);

	dimm_info("MTP Info : \n");
	for (i = 0; i < VMAX; i++) {
		dimm_info("Gamma : %3d %3d %3d : %3x %3x %3x\n",
			  dimming->mtp[i][CI_RED], dimming->mtp[i][CI_GREEN], dimming->mtp[i][CI_BLUE],
			  dimming->mtp[i][CI_RED], dimming->mtp[i][CI_GREEN], dimming->mtp[i][CI_BLUE]);
	}

	ret = generate_volt_table(dimming);
	if (ret) {
		dimm_err("[ERR:%s] failed to generate volt table\n", __func__);
		goto error;
	}

	for (i = 0; i < MAX_BR_INFO; i++) {
		method = diminfo[i].way;

		if (method == DIMMING_METHOD_FILL_CENTER) {
			ret = set_gamma_to_center(&diminfo[i]);
			if (ret) {
				dsim_err("%s : failed to get center gamma\n", __func__);
				goto error;
			}
		}
		else if (method == DIMMING_METHOD_FILL_HBM) {
			ret = s6e3hf4_set_gamma_to_hbm(&diminfo[i], dimming, hbm);
			if (ret) {
				dsim_err("%s : failed to get hbm gamma\n", __func__);
				goto error;
			}
		}
	}

	for (i = 0; i < MAX_BR_INFO; i++) {
		method = diminfo[i].way;
		if (method == DIMMING_METHOD_AID) {
			ret = cal_gamma_from_index(dimming, &diminfo[i]);
			if (ret) {
				dsim_err("%s : failed to calculate gamma : index : %d\n", __func__, i);
				goto error;
			}
		}
		if (method == DIMMING_METHOD_INTERPOLATION) {
			ret = interpolation_gamma_to_hbm(diminfo, i);
			if (ret) {
				dsim_err("%s : failed to calculate gamma : index : %d\n", __func__, i);
				goto error;
			}
		}
	}

	for (i = 0; i < MAX_BR_INFO; i++) {
		memset(string_buf, 0, sizeof(string_buf));
		string_offset = sprintf(string_buf, "gamma[%3d] : ", diminfo[i].br);

		for (j = 0; j < GAMMA_CMD_CNT; j++)
			string_offset += sprintf(string_buf + string_offset, "%02x ", diminfo[i].gamma[j]);

		dsim_info("%s\n", string_buf);
	}

error:
	return ret;

}

#ifdef CONFIG_LCD_HMT
static unsigned int hmt_br_tbl[EXTEND_BRIGHTNESS + 1] = {
	10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
	10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 11, 11, 11, 12, 12,
	13, 13, 14, 14, 14, 15, 15, 16, 16, 16, 17, 17, 17, 17, 17, 19,
	19, 20, 20, 21, 21, 21, 22, 22, 23, 23, 23, 23, 23, 25, 25, 25,
	25, 25, 27, 27, 27, 27, 27, 29, 29, 29, 29, 29, 31, 31, 31, 31,
	31, 33, 33, 33, 33, 35, 35, 35, 35, 35, 37, 37, 37, 37, 37, 39,
	39, 39, 39, 39, 41, 41, 41, 41, 41, 41, 41, 44, 44, 44, 44, 44,
	44, 44, 44, 47, 47, 47, 47, 47, 47, 47, 50, 50, 50, 50, 50, 50,
	50, 53, 53, 53, 53, 53, 53, 53, 56, 56, 56, 56, 56, 56, 56, 56,
	56, 56, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 64, 64, 64, 64,
	64, 64, 64, 64, 64, 64, 68, 68, 68, 68, 68, 68, 68, 68, 68, 72,
	72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 77, 77, 77, 77, 77,
	77, 77, 77, 77, 77, 77, 77, 77, 82, 82, 82, 82, 82, 82, 82, 82,
	82, 82, 82, 82, 87, 87, 87, 87, 87, 87, 87, 87, 87, 87, 87, 87,
	87, 87, 93, 93, 93, 93, 93, 93, 93, 93, 93, 93, 93, 93, 93, 93,
	93, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 105,
	[UI_MAX_BRIGHTNESS + 1 ... EXTEND_BRIGHTNESS] = 105
};

struct SmtDimInfo hmt_dimming_info_HF4_A3_da[HMT_MAX_BR_INFO] = {
	{.br = 10, .refBr = 55, .cGma = gma2p15, .rTbl = HF4_A3_da_HMTrtbl10nit, .cTbl = HF4_A3_da_HMTctbl10nit, .aid = HF4_A3_HMTaid7999, .elvCaps = HF4_HMTelvCaps, .elv = HF4_HMTelv },
	{.br = 11, .refBr = 60, .cGma = gma2p15, .rTbl = HF4_A3_da_HMTrtbl11nit, .cTbl = HF4_A3_da_HMTctbl11nit, .aid = HF4_A3_HMTaid7999, .elvCaps = HF4_HMTelvCaps, .elv = HF4_HMTelv },
	{.br = 12, .refBr = 66, .cGma = gma2p15, .rTbl = HF4_A3_da_HMTrtbl12nit, .cTbl = HF4_A3_da_HMTctbl12nit, .aid = HF4_A3_HMTaid7999, .elvCaps = HF4_HMTelvCaps, .elv = HF4_HMTelv },
	{.br = 13, .refBr = 70, .cGma = gma2p15, .rTbl = HF4_A3_da_HMTrtbl13nit, .cTbl = HF4_A3_da_HMTctbl13nit, .aid = HF4_A3_HMTaid7999, .elvCaps = HF4_HMTelvCaps, .elv = HF4_HMTelv },
	{.br = 14, .refBr = 75, .cGma = gma2p15, .rTbl = HF4_A3_da_HMTrtbl14nit, .cTbl = HF4_A3_da_HMTctbl14nit, .aid = HF4_A3_HMTaid7999, .elvCaps = HF4_HMTelvCaps, .elv = HF4_HMTelv },
	{.br = 15, .refBr = 81, .cGma = gma2p15, .rTbl = HF4_A3_da_HMTrtbl15nit, .cTbl = HF4_A3_da_HMTctbl15nit, .aid = HF4_A3_HMTaid7999, .elvCaps = HF4_HMTelvCaps, .elv = HF4_HMTelv },
	{.br = 16, .refBr = 86, .cGma = gma2p15, .rTbl = HF4_A3_da_HMTrtbl16nit, .cTbl = HF4_A3_da_HMTctbl16nit, .aid = HF4_A3_HMTaid7999, .elvCaps = HF4_HMTelvCaps, .elv = HF4_HMTelv },
	{.br = 17, .refBr = 91, .cGma = gma2p15, .rTbl = HF4_A3_da_HMTrtbl17nit, .cTbl = HF4_A3_da_HMTctbl17nit, .aid = HF4_A3_HMTaid7999, .elvCaps = HF4_HMTelvCaps, .elv = HF4_HMTelv },
	{.br = 19, .refBr = 101, .cGma = gma2p15, .rTbl = HF4_A3_da_HMTrtbl19nit, .cTbl = HF4_A3_da_HMTctbl19nit, .aid = HF4_A3_HMTaid7999, .elvCaps = HF4_HMTelvCaps, .elv = HF4_HMTelv },
	{.br = 20, .refBr = 105, .cGma = gma2p15, .rTbl = HF4_A3_da_HMTrtbl20nit, .cTbl = HF4_A3_da_HMTctbl20nit, .aid = HF4_A3_HMTaid7999, .elvCaps = HF4_HMTelvCaps, .elv = HF4_HMTelv },
	{.br = 21, .refBr = 109, .cGma = gma2p15, .rTbl = HF4_A3_da_HMTrtbl21nit, .cTbl = HF4_A3_da_HMTctbl21nit, .aid = HF4_A3_HMTaid7999, .elvCaps = HF4_HMTelvCaps, .elv = HF4_HMTelv },
	{.br = 22, .refBr = 114, .cGma = gma2p15, .rTbl = HF4_A3_da_HMTrtbl22nit, .cTbl = HF4_A3_da_HMTctbl22nit, .aid = HF4_A3_HMTaid7999, .elvCaps = HF4_HMTelvCaps, .elv = HF4_HMTelv },
	{.br = 23, .refBr = 120, .cGma = gma2p15, .rTbl = HF4_A3_da_HMTrtbl23nit, .cTbl = HF4_A3_da_HMTctbl23nit, .aid = HF4_A3_HMTaid7999, .elvCaps = HF4_HMTelvCaps, .elv = HF4_HMTelv },
	{.br = 25, .refBr = 126, .cGma = gma2p15, .rTbl = HF4_A3_da_HMTrtbl25nit, .cTbl = HF4_A3_da_HMTctbl25nit, .aid = HF4_A3_HMTaid7999, .elvCaps = HF4_HMTelvCaps, .elv = HF4_HMTelv },
	{.br = 27, .refBr = 139, .cGma = gma2p15, .rTbl = HF4_A3_da_HMTrtbl27nit, .cTbl = HF4_A3_da_HMTctbl27nit, .aid = HF4_A3_HMTaid7999, .elvCaps = HF4_HMTelvCaps, .elv = HF4_HMTelv },
	{.br = 29, .refBr = 147, .cGma = gma2p15, .rTbl = HF4_A3_da_HMTrtbl29nit, .cTbl = HF4_A3_da_HMTctbl29nit, .aid = HF4_A3_HMTaid7999, .elvCaps = HF4_HMTelvCaps, .elv = HF4_HMTelv },
	{.br = 31, .refBr = 157, .cGma = gma2p15, .rTbl = HF4_A3_da_HMTrtbl31nit, .cTbl = HF4_A3_da_HMTctbl31nit, .aid = HF4_A3_HMTaid7999, .elvCaps = HF4_HMTelvCaps, .elv = HF4_HMTelv },
	{.br = 33, .refBr = 165, .cGma = gma2p15, .rTbl = HF4_A3_da_HMTrtbl33nit, .cTbl = HF4_A3_da_HMTctbl33nit, .aid = HF4_A3_HMTaid7999, .elvCaps = HF4_HMTelvCaps, .elv = HF4_HMTelv },
	{.br = 35, .refBr = 174, .cGma = gma2p15, .rTbl = HF4_A3_da_HMTrtbl35nit, .cTbl = HF4_A3_da_HMTctbl35nit, .aid = HF4_A3_HMTaid7999, .elvCaps = HF4_HMTelvCaps, .elv = HF4_HMTelv },
	{.br = 37, .refBr = 183, .cGma = gma2p15, .rTbl = HF4_A3_da_HMTrtbl37nit, .cTbl = HF4_A3_da_HMTctbl37nit, .aid = HF4_A3_HMTaid7999, .elvCaps = HF4_HMTelvCaps, .elv = HF4_HMTelv },
	{.br = 39, .refBr = 194, .cGma = gma2p15, .rTbl = HF4_A3_da_HMTrtbl39nit, .cTbl = HF4_A3_da_HMTctbl39nit, .aid = HF4_A3_HMTaid7999, .elvCaps = HF4_HMTelvCaps, .elv = HF4_HMTelv },
	{.br = 41, .refBr = 202, .cGma = gma2p15, .rTbl = HF4_A3_da_HMTrtbl41nit, .cTbl = HF4_A3_da_HMTctbl41nit, .aid = HF4_A3_HMTaid7999, .elvCaps = HF4_HMTelvCaps, .elv = HF4_HMTelv },
	{.br = 44, .refBr = 214, .cGma = gma2p15, .rTbl = HF4_A3_da_HMTrtbl44nit, .cTbl = HF4_A3_da_HMTctbl44nit, .aid = HF4_A3_HMTaid7999, .elvCaps = HF4_HMTelvCaps, .elv = HF4_HMTelv },
	{.br = 47, .refBr = 227, .cGma = gma2p15, .rTbl = HF4_A3_da_HMTrtbl47nit, .cTbl = HF4_A3_da_HMTctbl47nit, .aid = HF4_A3_HMTaid7999, .elvCaps = HF4_HMTelvCaps, .elv = HF4_HMTelv },
	{.br = 50, .refBr = 240, .cGma = gma2p15, .rTbl = HF4_A3_da_HMTrtbl50nit, .cTbl = HF4_A3_da_HMTctbl50nit, .aid = HF4_A3_HMTaid7999, .elvCaps = HF4_HMTelvCaps, .elv = HF4_HMTelv },
	{.br = 53, .refBr = 252, .cGma = gma2p15, .rTbl = HF4_A3_da_HMTrtbl53nit, .cTbl = HF4_A3_da_HMTctbl53nit, .aid = HF4_A3_HMTaid7999, .elvCaps = HF4_HMTelvCaps, .elv = HF4_HMTelv },
	{.br = 56, .refBr = 266, .cGma = gma2p15, .rTbl = HF4_A3_da_HMTrtbl56nit, .cTbl = HF4_A3_da_HMTctbl56nit, .aid = HF4_A3_HMTaid7999, .elvCaps = HF4_HMTelvCaps, .elv = HF4_HMTelv },
	{.br = 60, .refBr = 280, .cGma = gma2p15, .rTbl = HF4_A3_da_HMTrtbl60nit, .cTbl = HF4_A3_da_HMTctbl60nit, .aid = HF4_A3_HMTaid7999, .elvCaps = HF4_HMTelvCaps, .elv = HF4_HMTelv },
	{.br = 64, .refBr = 295, .cGma = gma2p15, .rTbl = HF4_A3_da_HMTrtbl64nit, .cTbl = HF4_A3_da_HMTctbl64nit, .aid = HF4_A3_HMTaid7999, .elvCaps = HF4_HMTelvCaps, .elv = HF4_HMTelv },
	{.br = 68, .refBr = 311, .cGma = gma2p15, .rTbl = HF4_A3_da_HMTrtbl68nit, .cTbl = HF4_A3_da_HMTctbl68nit, .aid = HF4_A3_HMTaid7999, .elvCaps = HF4_HMTelvCaps, .elv = HF4_HMTelv },
	{.br = 72, .refBr = 326, .cGma = gma2p15, .rTbl = HF4_A3_da_HMTrtbl72nit, .cTbl = HF4_A3_da_HMTctbl72nit, .aid = HF4_A3_HMTaid7999, .elvCaps = HF4_HMTelvCaps, .elv = HF4_HMTelv },
	{.br = 77, .refBr = 252, .cGma = gma2p15, .rTbl = HF4_A3_da_HMTrtbl77nit, .cTbl = HF4_A3_da_HMTctbl77nit, .aid = HF4_A3_HMTaid7001, .elvCaps = HF4_HMTelvCaps, .elv = HF4_HMTelv },
	{.br = 82, .refBr = 266, .cGma = gma2p15, .rTbl = HF4_A3_da_HMTrtbl82nit, .cTbl = HF4_A3_da_HMTctbl82nit, .aid = HF4_A3_HMTaid7001, .elvCaps = HF4_HMTelvCaps, .elv = HF4_HMTelv },
	{.br = 87, .refBr = 277, .cGma = gma2p15, .rTbl = HF4_A3_da_HMTrtbl87nit, .cTbl = HF4_A3_da_HMTctbl87nit, .aid = HF4_A3_HMTaid7001, .elvCaps = HF4_HMTelvCaps, .elv = HF4_HMTelv },
	{.br = 93, .refBr = 291, .cGma = gma2p15, .rTbl = HF4_A3_da_HMTrtbl93nit, .cTbl = HF4_A3_da_HMTctbl93nit, .aid = HF4_A3_HMTaid7001, .elvCaps = HF4_HMTelvCaps, .elv = HF4_HMTelv },
	{.br = 99, .refBr = 309, .cGma = gma2p15, .rTbl = HF4_A3_da_HMTrtbl99nit, .cTbl = HF4_A3_da_HMTctbl99nit, .aid = HF4_A3_HMTaid7001, .elvCaps = HF4_HMTelvCaps, .elv = HF4_HMTelv },
	{.br = 105, .refBr = 322, .cGma = gma2p15, .rTbl = HF4_A3_da_HMTrtbl105nit, .cTbl = HF4_A3_da_HMTctbl105nit, .aid = HF4_A3_HMTaid7001, .elvCaps = HF4_HMTelvCaps, .elv = HF4_HMTelv },
};


static int hf4_hmt_init_dimming(struct dsim_device *dsim, u8 * mtp)
{
	int i, j;
	int pos = 0;
	int ret = 0;
	short temp;
	static struct dim_data *dimming = NULL;
	struct panel_private *panel = &dsim->priv;
	struct SmtDimInfo *diminfo = NULL;

	if (dimming == NULL) {
		dimming = (struct dim_data *)kmalloc(sizeof(struct dim_data), GFP_KERNEL);
		if (!dimming) {
			dsim_err("failed to allocate memory for dim data\n");
			ret = -ENOMEM;
			goto error;
		}
	}
	dsim_info("%s init HMT dimming info for HF4 a3 daisy init panel\n", __func__);
	diminfo = (void *)hmt_dimming_info_HF4_A3_da;

	panel->hmt_dim_data = (void *)dimming;
	panel->hmt_dim_info = (void *)diminfo;
	panel->hmt_br_tbl = (unsigned int *)hmt_br_tbl;
	panel->hmt_on = panel->hmt_prev_status = 0;
	for (j = 0; j < CI_MAX; j++) {
		temp = ((mtp[pos] & 0x01) ? -1 : 1) * mtp[pos + 1];
		dimming->t_gamma[V255][j] = (int)center_gamma[V255][j] + temp;
		dimming->mtp[V255][j] = temp;
		pos += 2;
	}
	for (i = V203; i >= 0; i--) {
		for (j = 0; j < CI_MAX; j++) {
			temp = ((mtp[pos] & 0x80) ? -1 : 1) * (mtp[pos] & 0x7f);
			dimming->t_gamma[i][j] = (int)center_gamma[i][j] + temp;
			dimming->mtp[i][j] = temp;
			pos++;
		}
	}
	/* for vt */
	temp = (mtp[pos + 1]) << 8 | mtp[pos];
	for (i = 0; i < CI_MAX; i++)
		dimming->vt_mtp[i] = (temp >> (i * 4)) & 0x0f;

	ret = generate_volt_table(dimming);
	if (ret) {
		dimm_err("[ERR:%s] failed to generate volt table\n", __func__);
		goto error;
	}

	for (i = 0; i < HMT_MAX_BR_INFO; i++) {
		ret = cal_gamma_from_index(dimming, &diminfo[i]);
		if (ret) {
			dsim_err("failed to calculate gamma : index : %d\n", i);
			goto error;
		}
	}
error:
	return ret;

}

#endif

static void s6e3hf4_init_default_info(struct dsim_device *dsim)
{
	struct panel_private *panel = &dsim->priv;
	panel->elvss_len = S6E3HF4_ELVSS_LEN + 1;
	panel->elvss_start_offset = S6E3HF4_ELVSS_START;
	panel->elvss_temperature_offset = S6E3HF4_ELVSS_TEMPERATURE_POS;
	panel->elvss_tset_offset = S6E3HF4_ELVSS_TSET_POS;

	memset(panel->tset_set, 0, sizeof(panel->tset_set));
	panel->tset_len = 0;	// elvss set include tset

	memcpy(panel->aid_set, S6E3HF4_SEQ_AOR_CONTROL, S6E3HF4_AID_CMD_CNT);
	panel->aid_len = S6E3HF4_AID_LEN + 1;
	panel->aid_reg_offset = S6E3HF4_AID_LEN - 1;

	if (panel->panel_line == LCD_LINE_A2)
		memcpy(panel->vint_set, S6E3HF4_SEQ_VINT_SET, sizeof(S6E3HF4_SEQ_VINT_SET));
	else
		memcpy(panel->vint_set, S6E3HF4_SEQ_VINT_SET_A3, sizeof(S6E3HF4_SEQ_VINT_SET_A3));
	panel->vint_len = S6E3HF4_VINT_LEN + 1;

	memcpy(panel->vint_table, VINT_TABLE_HF4, sizeof(VINT_TABLE_HF4));
	memcpy(panel->vint_dim_table, VINT_DIM_TABLE, sizeof(VINT_DIM_TABLE));
	panel->vint_table_len = ARRAY_SIZE(VINT_DIM_TABLE);

	panel->acl_opr_tbl = (unsigned char **)ACL_OPR_TABLE_HF4;
	panel->acl_cutoff_tbl = (unsigned char **)ACL_CUTOFF_TABLE;

	pr_info("%s : init default value\n", __func__);
}

static int s6e3hf4_read_init_info(struct dsim_device *dsim, unsigned char *mtp, unsigned char *hbm)
{
	int i = 0;
	int ret;
	struct panel_private *panel = &dsim->priv;
	unsigned char bufForCoordi[S6E3HF4_COORDINATE_LEN] = { 0, };
	unsigned char buf[S6E3HF4_MTP_DATE_SIZE] = { 0, };
	unsigned char hbm_gamma[S6E3HF4_HBMGAMMA_LEN] = { 0, };

	dsim_info("MDD : %s was called\n", __func__);
	ret = dsim_write_hl_data(dsim, SEQ_TEST_KEY_ON_F0, ARRAY_SIZE(SEQ_TEST_KEY_ON_F0));
	if (ret < 0) {
		dsim_err("%s : fail to write CMD : SEQ_TEST_KEY_ON_F0\n", __func__);
	}

	// id
	ret = dsim_read_hl_data(dsim, S6E3HF4_ID_REG, S6E3HF4_ID_LEN, panel->id);
	if (ret != S6E3HF4_ID_LEN) {
		dsim_err("%s : can't find connected panel. check panel connection\n", __func__);
		panel->lcdConnected = PANEL_DISCONNEDTED;
		goto read_exit;
	}

	dsim_info("READ ID : ");
	for (i = 0; i < S6E3HF4_ID_LEN; i++)
		dsim_info("%02x, ", panel->id[i]);
	dsim_info("\n");

	dsim->priv.current_model = (dsim->priv.id[1] >> 4) & 0x03;
	dsim->priv.panel_rev = dsim->priv.id[2] & 0x0F;
	dsim->priv.panel_line = (dsim->priv.id[0] >> 6) & 0x03;
	dsim->priv.panel_material = dsim->priv.id[1] & 0x01;

	dsim_info("%s model is %d, panel rev : %d, panel line : %d panel material : %d\n",
		__func__, dsim->priv.current_model, dsim->priv.panel_rev, dsim->priv.panel_line, dsim->priv.panel_material);

	// mtp
	ret = dsim_read_hl_data(dsim, S6E3HF4_MTP_ADDR, S6E3HF4_MTP_DATE_SIZE, buf);
	if (ret != S6E3HF4_MTP_DATE_SIZE) {
		dsim_err("failed to read mtp, check panel connection\n");
		goto read_fail;
	}
	VT_RGB2GRB(buf + S6E3HF4_MTP_VT_ADDR);
	memcpy(mtp, buf, S6E3HF4_MTP_SIZE);
	memcpy(panel->date, &buf[40], ARRAY_SIZE(panel->date));
	dsim_info("READ MTP SIZE : %d\n", S6E3HF4_MTP_SIZE);
	dsim_info("=========== MTP INFO =========== \n");
	for (i = 0; i < S6E3HF4_MTP_SIZE; i++)
		dsim_info("MTP[%2d] : %2d : %2x\n", i, mtp[i], mtp[i]);

	// coordinate
	ret = dsim_read_hl_data(dsim, S6E3HF4_COORDINATE_REG, S6E3HF4_COORDINATE_LEN, bufForCoordi);
	if (ret != S6E3HF4_COORDINATE_LEN) {
		dsim_err("fail to read coordinate on command.\n");
		goto read_fail;
	}
	panel->coordinate[0] = bufForCoordi[0] << 8 | bufForCoordi[1];	/* X */
	panel->coordinate[1] = bufForCoordi[2] << 8 | bufForCoordi[3];	/* Y */
	dsim_info("READ coordi : ");
	for (i = 0; i < 2; i++)
		dsim_info("%d, ", dsim->priv.coordinate[i]);
	dsim_info("\n");


	// code
	ret = dsim_read_hl_data(dsim, S6E3HF4_CODE_REG, S6E3HF4_CODE_LEN, panel->code);
	if (ret != S6E3HF4_CODE_LEN) {
		dsim_err("fail to read code on command.\n");
		goto read_fail;
	}
	dsim_info("READ code : ");
	for (i = 0; i < S6E3HF4_CODE_LEN; i++)
		dsim_info("%x, ", panel->code[i]);
	dsim_info("\n");

	// elvss
	panel->elvss_set[0] = S6E3HF4_ELVSS_REG;
	ret = dsim_read_hl_data(dsim, S6E3HF4_ELVSS_REG, S6E3HF4_ELVSS_LEN, &(panel->elvss_set[1]));
	if (ret != S6E3HF4_ELVSS_LEN) {
		dsim_err("fail to read elvss on command.\n");
		goto read_fail;
	}
	dsim_info("READ elvss : ");
	for (i = 1; i <= S6E3HF4_ELVSS_LEN; i++)
		dsim_info("%x \n", dsim->priv.elvss_set[i]);

	ret = dsim_read_hl_data(dsim, S6E3HF4_HBMGAMMA_REG, S6E3HF4_HBMGAMMA_LEN, hbm_gamma);
	if (ret != S6E3HF4_HBMGAMMA_LEN) {
		dsim_err("fail to read elvss on command.\n");
		goto read_fail;
	}
	dsim_info("HBM Gamma : ");
	memcpy(hbm, hbm_gamma, S6E3HF4_HBMGAMMA_LEN);

	for (i = 50; i < S6E3HF4_HBMGAMMA_LEN; i++)
		dsim_info("hbm gamma[%d] : %x\n", i, hbm_gamma[i]);

	ret = dsim_write_hl_data(dsim, SEQ_TEST_KEY_OFF_F0, ARRAY_SIZE(SEQ_TEST_KEY_OFF_F0));
	if (ret < 0) {
		dsim_err("%s : fail to write CMD : SEQ_TEST_KEY_OFF_F0\n", __func__);
		goto read_fail;
	}
read_exit:
	return 0;

read_fail:
	return -ENODEV;

}

static int s6e3hf4_wqhd_dump(struct dsim_device *dsim)
{
	int ret = 0;
	int i;
	unsigned char id[S6E3HF4_ID_LEN];
	unsigned char rddpm[4];
	unsigned char rddsm[4];
	unsigned char err_buf[4];

	ret = dsim_write_hl_data(dsim, SEQ_TEST_KEY_ON_F0, ARRAY_SIZE(SEQ_TEST_KEY_ON_F0));
	if (ret < 0) {
		dsim_err("%s : fail to write CMD : SEQ_TEST_KEY_ON_F0\n", __func__);
	}

	ret = dsim_write_hl_data(dsim, SEQ_TEST_KEY_ON_FC, ARRAY_SIZE(SEQ_TEST_KEY_ON_FC));
	if (ret < 0) {
		dsim_err("%s : fail to write CMD : SEQ_TEST_KEY_ON_FC\n", __func__);
	}

	ret = dsim_read_hl_data(dsim, 0xEA, 3, err_buf);
	if (ret != 3) {
		dsim_err("%s : can't read Panel's EA Reg\n", __func__);
		goto dump_exit;
	}

	dsim_info("=== Panel's 0xEA Reg Value ===\n");
	dsim_info("* 0xEA : buf[0] = %x\n", err_buf[0]);
	dsim_info("* 0xEA : buf[1] = %x\n", err_buf[1]);

	ret = dsim_read_hl_data(dsim, S6E3HF4_RDDPM_ADDR, 3, rddpm);
	if (ret != 3) {
		dsim_err("%s : can't read RDDPM Reg\n", __func__);
		goto dump_exit;
	}

	dsim_info("=== Panel's RDDPM Reg Value : %x ===\n", rddpm[0]);

	if (rddpm[0] & 0x80)
		dsim_info("* Booster Voltage Status : ON\n");
	else
		dsim_info("* Booster Voltage Status : OFF\n");

	if (rddpm[0] & 0x40)
		dsim_info("* Idle Mode : On\n");
	else
		dsim_info("* Idle Mode : OFF\n");

	if (rddpm[0] & 0x20)
		dsim_info("* Partial Mode : On\n");
	else
		dsim_info("* Partial Mode : OFF\n");

	if (rddpm[0] & 0x10)
		dsim_info("* Sleep OUT and Working Ok\n");
	else
		dsim_info("* Sleep IN\n");

	if (rddpm[0] & 0x08)
		dsim_info("* Normal Mode On and Working Ok\n");
	else
		dsim_info("* Sleep IN\n");

	if (rddpm[0] & 0x04)
		dsim_info("* Display On and Working Ok\n");
	else
		dsim_info("* Display Off\n");

	ret = dsim_read_hl_data(dsim, S6E3HF4_RDDSM_ADDR, 3, rddsm);
	if (ret != 3) {
		dsim_err("%s : can't read RDDSM Reg\n", __func__);
		goto dump_exit;
	}

	dsim_info("=== Panel's RDDSM Reg Value : %x ===\n", rddsm[0]);

	if (rddsm[0] & 0x80)
		dsim_info("* TE On\n");
	else
		dsim_info("* TE OFF\n");

	if (rddsm[0] & 0x02)
		dsim_info("* S_DSI_ERR : Found\n");

	if (rddsm[0] & 0x01)
		dsim_info("* DSI_ERR : Found\n");

	ret = dsim_read_hl_data(dsim, S6E3HF4_ID_REG, S6E3HF4_ID_LEN, id);
	if (ret != S6E3HF4_ID_LEN) {
		dsim_err("%s : can't read panel id\n", __func__);
		goto dump_exit;
	}

	dsim_info("READ ID : ");
	for (i = 0; i < S6E3HF4_ID_LEN; i++)
		dsim_info("%02x, ", id[i]);
	dsim_info("\n");

	ret = dsim_write_hl_data(dsim, SEQ_TEST_KEY_OFF_FC, ARRAY_SIZE(SEQ_TEST_KEY_OFF_FC));
	if (ret < 0) {
		dsim_err("%s : fail to write CMD : SEQ_TEST_KEY_OFF_FC\n", __func__);
	}

	ret = dsim_write_hl_data(dsim, SEQ_TEST_KEY_OFF_F0, ARRAY_SIZE(SEQ_TEST_KEY_OFF_F0));
	if (ret < 0) {
		dsim_err("%s : fail to write CMD : SEQ_TEST_KEY_OFF_F0\n", __func__);
	}
dump_exit:
	return ret;

}

static int s6e3hf4_wqhd_probe(struct dsim_device *dsim)
{
	int ret = 0;
	struct panel_private *panel = &dsim->priv;
	unsigned char mtp[S6E3HF4_MTP_SIZE] = { 0, };
	unsigned char hbm[S6E3HF4_HBMGAMMA_LEN] = { 0, };

	dsim_info("MDD : %s was called\n", __func__);

	dsim->priv.panel_rev = 6;	// HF4 for grace is high rev

	panel->dim_data = (void *)NULL;
	panel->lcdConnected = PANEL_CONNECTED;

#ifdef CONFIG_LCD_ALPM
	mutex_init(&panel->alpm_lock);
#endif

	ret = s6e3hf4_read_init_info(dsim, mtp, hbm);
	if (panel->lcdConnected == PANEL_DISCONNEDTED) {
		dsim_err("dsim : %s lcd was not connected\n", __func__);
		goto probe_exit;
	}

	s6e3hf4_init_default_info(dsim);
#ifdef CONFIG_LCD_ALPM
	panel->alpm = 0;
	panel->current_alpm = 0;
/*	if((panel->current_model == MODEL_IS_HERO2) && (panel->panel_rev >= 3))
		panel->alpm_support = SUPPORT_LOWHZALPM;*/
#endif
	panel->alpm_support = SUPPORT_30HZALPM;

	if ((dsim->priv.current_model == 1) && (dsim->priv.panel_rev > 0))
		dsim->priv.esd_disable = 0;
	else
		dsim->priv.esd_disable = 1;
	dsim->priv.panel_type = dynamic_lcd_type;

#ifdef CONFIG_LCD_WEAKNESS_CCB
	panel->ccb_support = SUPPORT_CCB;
	panel->current_ccb = 0;
#endif

#ifdef CONFIG_EXYNOS_DECON_MDNIE_LITE
	panel->mdnie_support = (panel->current_model == MODEL_IS_HERO2) ? 1 : 0;
#endif

#ifdef CONFIG_PANEL_AID_DIMMING
	ret = hf4_init_dimming(dsim, mtp, hbm);
	if (ret) {
		dsim_err("%s : failed to generate gamma tablen\n", __func__);
	}
#endif

#ifdef CONFIG_LCD_HMT
	panel->hmt_support = SUPPORT_HMT;

	ret = hf4_hmt_init_dimming(dsim, mtp);
	if (ret) {
		dsim_err("%s : failed to generate gamma tablen\n", __func__);
	}
#endif

probe_exit:

	return ret;

}

static int s6e3hf4_wqhd_displayon(struct dsim_device *dsim)
{
	int ret = 0;
#ifdef CONFIG_LCD_ALPM
	struct panel_private *panel = &dsim->priv;
#endif

	dsim_info("MDD : %s was called\n", __func__);

#ifdef CONFIG_LCD_ALPM
	if (panel->current_alpm == panel->alpm && panel->current_alpm) {
		dsim_info("%s : ALPM mode\n", __func__);
		if ((panel->alpm_support == SUPPORT_LOWHZALPM) && (panel->current_alpm == ALPM_ON_40NIT)) {
			dsim_write_hl_data(dsim, SEQ_TEST_KEY_ON_F0, ARRAY_SIZE(SEQ_TEST_KEY_ON_F0));
			dsim_write_hl_data(dsim, SEQ_AOD_LOWHZ_OFF, ARRAY_SIZE(SEQ_AOD_LOWHZ_OFF));
			dsim_write_hl_data(dsim, SEQ_TEST_KEY_OFF_F0, ARRAY_SIZE(SEQ_TEST_KEY_OFF_F0));
			dsim_info("%s : ALPM LOW Hz off\n", __func__);
		}
	} else if (panel->current_alpm && panel->alpm == ALPM_OFF) {
		ret = alpm_set_mode(dsim, ALPM_OFF);
		if (ret) {
			dsim_err("failed to exit alpm.\n");
			goto displayon_err;
		}
	} else {
		if (panel->alpm) {
			ret = alpm_set_mode(dsim, panel->alpm);
			if (ret) {
				dsim_err("failed to initialize alpm.\n");
				goto displayon_err;
			}
		} else {
			ret = dsim_write_hl_data(dsim, SEQ_DISPLAY_ON, ARRAY_SIZE(SEQ_DISPLAY_ON));
			if (ret < 0) {
				dsim_err("%s : fail to write CMD : DISPLAY_ON\n", __func__);
				goto displayon_err;
			}
		}
	}
#else
	ret = dsim_write_hl_data(dsim, SEQ_DISPLAY_ON, ARRAY_SIZE(SEQ_DISPLAY_ON));
	if (ret < 0) {
		dsim_err("%s : fail to write CMD : DISPLAY_ON\n", __func__);
		goto displayon_err;
	}
#endif

displayon_err:
	return ret;

}

static int s6e3hf4_wqhd_exit(struct dsim_device *dsim)
{
	int ret = 0;
#ifdef CONFIG_LCD_ALPM
	struct panel_private *panel = &dsim->priv;
#endif
	dsim_info("MDD : %s was called\n", __func__);
#ifdef CONFIG_LCD_ALPM
	mutex_lock(&panel->alpm_lock);
	if (panel->current_alpm && panel->alpm) {
		dsim->alpm = panel->current_alpm;
		dsim_info("%s : ALPM mode\n", __func__);
		if ((panel->alpm_support == SUPPORT_LOWHZALPM) && (panel->current_alpm == ALPM_ON_40NIT)) {
			usleep_range(100000, 100000);
			dsim_write_hl_data(dsim, SEQ_TEST_KEY_ON_F0, ARRAY_SIZE(SEQ_TEST_KEY_ON_F0));
			dsim_write_hl_data(dsim, SEQ_AOD_LOWHZ_ON, ARRAY_SIZE(SEQ_AOD_LOWHZ_ON));
			dsim_write_hl_data(dsim, SEQ_TEST_KEY_OFF_F0, ARRAY_SIZE(SEQ_TEST_KEY_OFF_F0));
			dsim_info("%s : ALPM LOW Hz on\n", __func__);
		}
	} else {
		ret = dsim_write_hl_data(dsim, SEQ_DISPLAY_OFF, ARRAY_SIZE(SEQ_DISPLAY_OFF));
		if (ret < 0) {
			dsim_err("%s : fail to write CMD : DISPLAY_OFF\n", __func__);
			goto exit_err;
		}
		ret = dsim_write_hl_data(dsim, SEQ_SLEEP_IN, ARRAY_SIZE(SEQ_SLEEP_IN));
		if (ret < 0) {
			dsim_err("%s : fail to write CMD : SLEEP_IN\n", __func__);
			goto exit_err;
		}
		msleep(120);
	}

	dsim_info("MDD : %s was called unlock\n", __func__);
#else
	ret = dsim_write_hl_data(dsim, SEQ_DISPLAY_OFF, ARRAY_SIZE(SEQ_DISPLAY_OFF));
	if (ret < 0) {
		dsim_err("%s : fail to write CMD : DISPLAY_OFF\n", __func__);
		goto exit_err;
	}
	ret = dsim_write_hl_data(dsim, SEQ_SLEEP_IN, ARRAY_SIZE(SEQ_SLEEP_IN));
	if (ret < 0) {
		dsim_err("%s : fail to write CMD : SLEEP_IN\n", __func__);
		goto exit_err;
	}
	msleep(120);
#endif

exit_err:
#ifdef CONFIG_LCD_ALPM
	mutex_unlock(&panel->alpm_lock);
#endif

#if defined(CONFIG_FB_DSU) || defined(CONFIG_LCD_RES)
	last_dsc_enabled = false;
#endif

	return ret;
}

#ifdef CONFIG_FB_DSU
static const unsigned char S6E3HF4_SEQ_DDI_SCALER_PPS_00[] = {
	0xB0,
	0xB0
};

static const unsigned char S6E3HF4_SEQ_DDI_SCALER_PPS_01[] = {
	0xF2,
	0x2A
};
#endif

#if defined(CONFIG_FB_DSU) || defined(CONFIG_LCD_RES)
#undef CONFIG_HF4_CASET_PASET_CHECK
static int _s6e3hf4_wqhd_dsu_command(struct dsim_device *dsim, int xres, int yres)
{
	int ret = 0;
//	struct panel_private *panel = &dsim->priv;

#ifdef CONFIG_HF4_CASET_PASET_CHECK
	const unsigned char SEQ_HF4_CASET_PASET_GPARAM[] = { 0xB0, 0x0F };
	const unsigned char REG_HF4_CASET_PASET = 0xFB;
	const unsigned char size_hf4_caset_paset = 8;
	char buffer_caset_paset[size_hf4_caset_paset + 4];
	u16 *pint16;
	int i;
#endif
	ret = dsim_write_hl_data(dsim, SEQ_TEST_KEY_ON_F0, ARRAY_SIZE(SEQ_TEST_KEY_ON_F0));
	if (ret < 0) {
		dsim_err("%s : fail to write CMD : SEQ_TEST_KEY_ON_F0\n", __func__);
	}
	ret = dsim_write_hl_data(dsim, SEQ_TEST_KEY_ON_FC, ARRAY_SIZE(SEQ_TEST_KEY_ON_FC));
	if (ret < 0) {
		dsim_err("%s : fail to write CMD : SEQ_TEST_KEY_ON_FC\n", __func__);
	}

	pr_info( "%s.%d: (DSU) last_dsc=%d, dsc_enabled=%d\n", __func__, __LINE__, last_dsc_enabled, dsim->lcd_info.dsc_enabled );
	if (last_dsc_enabled != dsim->lcd_info.dsc_enabled) {
		last_dsc_enabled = dsim->lcd_info.dsc_enabled;
		if (dsim->lcd_info.dsc_enabled) {
			ret = dsim_write_data(dsim, MIPI_DSI_DSC_PRA, S6E3HF4_DATA_DSC_ENABLE[0], S6E3HF4_DATA_DSC_ENABLE[1]);
			if (ret < 0) {
				dsim_err("%s : fail to write CMD : S6E3HF4_SEQ_DSC_DISABLE\n", __func__);
			}
		} else {
			ret = dsim_write_data(dsim, MIPI_DSI_DSC_PRA, S6E3HF4_DATA_DSC_DISABLE[0], S6E3HF4_DATA_DSC_DISABLE[1]);
			if (ret < 0) {
				dsim_err("%s : fail to write CMD : S6E3HF4_SEQ_DSC_DISABLE\n", __func__);
			}
		}
	}

	switch (xres) {
	case 720:
		dsim_err("%s : xres=%d, yres=%d, dsc_enabled=%d : HD\n", __func__, xres, yres, dsim->lcd_info.dsc_enabled);
		if( dsim->lcd_info.dsc_enabled )
		{
			ret = dsim_write_data(dsim, MIPI_DSI_DSC_PPS, (unsigned long)S6E3HF4_SEQ_DDI_SCALER_HD_PPS, ARRAY_SIZE(S6E3HF4_SEQ_DDI_SCALER_HD_PPS));
			if (ret < 0) {
				dsim_err("%s : fail to write CMD : S6E3HF4_SEQ_DDI_SCALER_FHD_PPS\n", __func__);
			}
		}
		ret = dsim_write_hl_data(dsim, S6E3HF4_SEQ_DDI_SCALER_HD_00, ARRAY_SIZE(S6E3HF4_SEQ_DDI_SCALER_HD_00));
		if (ret < 0) {
			dsim_err("%s : fail to write CMD : S6E3HF4_SEQ_DDI_SCALER_FHD_01\n", __func__);
		}
//		if (panel->alpm_mode) {
			ret = dsim_write_hl_data(dsim, S6E3HF4_SEQ_DDI_SCALER_HD_01, ARRAY_SIZE(S6E3HF4_SEQ_DDI_SCALER_HD_01));
			if (ret < 0) {
				dsim_err("%s : fail to write CMD : S6E3HF4_SEQ_DDI_SCALER_HD_01\n", __func__);
			}
			ret = dsim_write_hl_data(dsim, S6E3HF4_SEQ_DDI_SCALER_HD_02, ARRAY_SIZE(S6E3HF4_SEQ_DDI_SCALER_HD_02));
			if (ret < 0) {
				dsim_err("%s : fail to write CMD : S6E3HF4_SEQ_DDI_SCALER_HD_02\n", __func__);
			}
//		}
		break;
	case 1080:
		dsim_err("%s : xres=%d, yres=%d, dsc_enabled=%d : FHD\n", __func__, xres, yres, dsim->lcd_info.dsc_enabled);
		ret = dsim_write_data(dsim, MIPI_DSI_DSC_PPS, (unsigned long)S6E3HF4_SEQ_DDI_SCALER_FHD_PPS, ARRAY_SIZE(S6E3HF4_SEQ_DDI_SCALER_FHD_PPS));
		if (ret < 0) {
			dsim_err("%s : fail to write CMD : S6E3HF4_SEQ_DDI_SCALER_FHD_PPS\n", __func__);
		}

		ret = dsim_write_hl_data(dsim, S6E3HF4_SEQ_DDI_SCALER_FHD_01, ARRAY_SIZE(S6E3HF4_SEQ_DDI_SCALER_FHD_01));
		if (ret < 0) {
			dsim_err("%s : fail to write CMD : S6E3HF4_SEQ_DDI_SCALER_FHD_01\n", __func__);
		}
//		if (panel->alpm_mode) {
			ret = dsim_write_hl_data(dsim, S6E3HF4_SEQ_DDI_SCALER_FHD_02, ARRAY_SIZE(S6E3HF4_SEQ_DDI_SCALER_FHD_02));
			if (ret < 0) {
				dsim_err("%s : fail to write CMD : S6E3HF4_SEQ_DDI_SCALER_FHD_02\n", __func__);
			}
			ret = dsim_write_hl_data(dsim, S6E3HF4_SEQ_DDI_SCALER_FHD_03, ARRAY_SIZE(S6E3HF4_SEQ_DDI_SCALER_FHD_03));
			if (ret < 0) {
				dsim_err("%s : fail to write CMD : S6E3HF4_SEQ_DDI_SCALER_FHD_03\n", __func__);
			}
//		}
		break;
	case 1440:
	default:
		dsim_err("%s : xres=%d, yres=%d, dsc_enabled=%d : WQHD\n", __func__, xres, yres, dsim->lcd_info.dsc_enabled);

		ret = dsim_write_data(dsim, MIPI_DSI_DSC_PPS, (unsigned long)S6E3HF4_SEQ_PPS_SLICE2, ARRAY_SIZE(S6E3HF4_SEQ_PPS_SLICE2));
		if (ret < 0) {
			dsim_err("%s : fail to write CMD : S6E3HF4_SEQ_PPS_SLICE4\n", __func__);
		}

		ret = dsim_write_hl_data(dsim, S6E3HF4_SEQ_DDI_SCALER_WQHD_01, ARRAY_SIZE(S6E3HF4_SEQ_DDI_SCALER_WQHD_01));
		if (ret < 0) {
			dsim_err("%s : fail to write CMD : S6E3HF4_SEQ_DDI_SCALER_WQHD_01\n", __func__);
		}

		break;
	}

#ifdef CONFIG_HF4_CASET_PASET_CHECK
	ret = dsim_write_hl_data(dsim, SEQ_HF4_CASET_PASET_GPARAM, ARRAY_SIZE(SEQ_HF4_CASET_PASET_GPARAM));
	ret = dsim_read_hl_data(dsim, REG_HF4_CASET_PASET, size_hf4_caset_paset, buffer_caset_paset);
	pint16 = (u16 *) buffer_caset_paset;
	dsim_info( "%s.%d (dsu) caset paset(%d) : %d, %d, %d, %d\n", __func__, __LINE__, ret, pint16[0], pint16[1], pint16[2], pint16[3] );
#endif

	return ret;
}
#endif

#ifdef CONFIG_FB_DSU
static int s6e3hf4_wqhd_dsu_command(struct dsim_device *dsim)
{
	int ret = 0;

	ret = _s6e3hf4_wqhd_dsu_command(dsim, dsim->dsu_xres, dsim->dsu_yres);

	dsim_write_hl_data(dsim, SEQ_TEST_KEY_OFF_F0, ARRAY_SIZE(SEQ_TEST_KEY_OFF_F0));
	dsim_write_hl_data(dsim, SEQ_TEST_KEY_OFF_FC, ARRAY_SIZE(SEQ_TEST_KEY_OFF_FC));

	return ret;
}
#endif

static int s6e3hf4_wqhd_init(struct dsim_device *dsim)
{
	int ret = 0;

	dsim_info("MDD : %s was called\n", __func__);

#ifdef CONFIG_LCD_ALPM
	if (dsim->priv.current_alpm) {
		dsim_info("%s : ALPM mode\n", __func__);

		return ret;
	}
#endif

	/* DSC setting */
	msleep(5);

#ifdef CONFIG_FB_DSU
	ret = dsim_write_hl_data(dsim, SEQ_TEST_KEY_ON_F0, ARRAY_SIZE(SEQ_TEST_KEY_ON_F0));
	if (ret < 0) {
		dsim_err("%s : fail to write CMD : SEQ_TEST_KEY_ON_F0\n", __func__);
	}

	ret = dsim_write_hl_data(dsim, S6E3HF4_SEQ_DDI_SCALER_UPDATE_TIMMING_00, ARRAY_SIZE(S6E3HF4_SEQ_DDI_SCALER_UPDATE_TIMMING_00));
	if (ret < 0) {
		dsim_err("%s : fail to write CMD : S6E3HF4_SEQ_DDI_SCALER_UPDATE_TIMMING_00\n", __func__);
	}

	ret = dsim_write_hl_data(dsim, S6E3HF4_SEQ_DDI_SCALER_UPDATE_TIMMING_01, ARRAY_SIZE(S6E3HF4_SEQ_DDI_SCALER_UPDATE_TIMMING_01));
	if (ret < 0) {
		dsim_err("%s : fail to write CMD : S6E3HF4_SEQ_DDI_SCALER_UPDATE_TIMMING_01\n", __func__);
	}
#endif

#ifdef CONFIG_FB_DSU
	ret = _s6e3hf4_wqhd_dsu_command(dsim, dsim->dsu_xres, dsim->dsu_yres);
#elif defined(CONFIG_LCD_RES)
	ret = _s6e3hf4_wqhd_dsu_command(dsim, dsim->priv.lcd_res, 0);
#else
	ret = dsim_write_data(dsim, MIPI_DSI_DSC_PRA, S6E3HF4_DATA_DSC_ENABLE[0], S6E3HF4_DATA_DSC_ENABLE[1]);
	if (ret < 0) {
		dsim_err("%s : fail to write CMD : DATA_SEC_ENABLE\n", __func__);
		goto init_exit;
	}

	ret = dsim_write_data(dsim, MIPI_DSI_DSC_PPS, (unsigned long)S6E3HF4_SEQ_PPS_SLICE2, ARRAY_SIZE(S6E3HF4_SEQ_PPS_SLICE2));
	if (ret < 0) {
		dsim_err("%s : fail to write CMD : SEQ_PPS_SLICE4\n", __func__);
		goto init_exit;
	}
#endif

	/* Sleep Out(11h) */
	ret = dsim_write_hl_data(dsim, SEQ_SLEEP_OUT, ARRAY_SIZE(SEQ_SLEEP_OUT));
	if (ret < 0) {
		dsim_err("%s : fail to write CMD : SEQ_SLEEP_OUT\n", __func__);
		goto init_exit;
	}
	msleep(120);

#ifdef CONFIG_ALWAYS_RELOAD_MTP_FACTORY_BUILD
	ret = lcd_reload_mtp(dynamic_lcd_type, dsim);
#endif

	/* Interface Setting */
	ret = dsim_write_hl_data(dsim, SEQ_TEST_KEY_ON_F0, ARRAY_SIZE(SEQ_TEST_KEY_ON_F0));
	if (ret < 0) {
		dsim_err("%s : fail to write CMD : SEQ_TEST_KEY_ON_F0\n", __func__);
		goto init_exit;
	}

	ret = dsim_write_hl_data(dsim, SEQ_TEST_KEY_ON_FC, ARRAY_SIZE(SEQ_TEST_KEY_ON_FC));
	if (ret < 0) {
		dsim_err("%s : fail to write CMD : SEQ_TEST_KEY_ON_F0\n", __func__);
		goto init_exit;
	}

	/* Common Setting */
	ret = dsim_write_hl_data(dsim, S6E3HF4_SEQ_TE_ON, ARRAY_SIZE(S6E3HF4_SEQ_TE_ON));
	if (ret < 0) {
		dsim_err("%s : fail to write CMD : SEQ_TE_ON\n", __func__);
		goto init_exit;
	}

	ret = dsim_write_hl_data(dsim, S6E3HF4_SEQ_ERR_FG_SETTING, ARRAY_SIZE(S6E3HF4_SEQ_ERR_FG_SETTING));
	if (ret < 0) {
		dsim_err("%s : fail to write CMD : SEQ_ERR_FG_SETTING\n", __func__);
		goto init_exit;
	}
	ret = dsim_write_hl_data(dsim, S6E3HF4_SEQ_TE_START_SETTING, ARRAY_SIZE(S6E3HF4_SEQ_TE_START_SETTING));
	if (ret < 0) {
		dsim_err("%s : fail to write CMD : SEQ_TE_START_SETTING\n", __func__);
		goto init_exit;
	}

#ifdef CONFIG_FB_DSU
	// do nothing
#else
	ret = dsim_write_hl_data(dsim, S6E3HF4_SEQ_FFC_SET, ARRAY_SIZE(S6E3HF4_SEQ_FFC_SET));
	if (ret < 0) {
		dsim_err("%s : fail to write CMD : SEQ_FFC_SET\n", __func__);
		goto init_exit;
	}
#endif

#ifndef CONFIG_PANEL_AID_DIMMING
	/* Brightness Setting */
	ret = dsim_write_hl_data(dsim, SEQ_GAMMA_CONDITION_SET, ARRAY_SIZE(SEQ_GAMMA_CONDITION_SET));
	if (ret < 0) {
		dsim_err(":%s fail to write CMD : SEQ_GAMMA_CONDITION_SET\n", __func__);
		goto init_exit;
	}
	ret = dsim_write_hl_data(dsim, SEQ_AOR_CONTROL, ARRAY_SIZE(SEQ_AOR_CONTROL));
	if (ret < 0) {
		dsim_err(":%s fail to write CMD : SEQ_AOR_CONTROL\n", __func__);
		goto init_exit;
	}
	ret = dsim_write_hl_data(dsim, SEQ_GAMMA_UPDATE, ARRAY_SIZE(SEQ_GAMMA_UPDATE));
	if (ret < 0) {
		dsim_err(":%s fail to write CMD : SEQ_GAMMA_UPDATE\n", __func__);
		goto init_exit;
	}

	/* elvss */
	ret = dsim_write_hl_data(dsim, SEQ_TSET_ELVSS_SET, ARRAY_SIZE(SEQ_TSET_ELVSS_SET));
	if (ret < 0) {
		dsim_err(":%s fail to write CMD : SEQ_TSET_ELVSS_SET\n", __func__);
		goto init_exit;
	}

	ret = dsim_write_hl_data(dsim, SEQ_VINT_SET, ARRAY_SIZE(SEQ_VINT_SET));
	if (ret < 0) {
		dsim_err(":%s fail to write CMD : SEQ_VINT_SET\n", __func__);
		goto init_exit;
	}
	/* ACL Setting */
	ret = dsim_write_hl_data(dsim, SEQ_ACL_OFF, ARRAY_SIZE(SEQ_ACL_OFF));
	if (ret < 0) {
		dsim_err(":%s fail to write CMD : SEQ_ACL_OFF\n", __func__);
		goto init_exit;
	}
	ret = dsim_write_hl_data(dsim, SEQ_ACL_OFF_OPR_AVR, ARRAY_SIZE(SEQ_ACL_OFF_OPR_AVR));
	if (ret < 0) {
		dsim_err(":%s fail to write CMD : SEQ_ACL_OFF_OPR\n", __func__);
		goto init_exit;
	}
#endif
	ret = dsim_write_hl_data(dsim, SEQ_TEST_KEY_OFF_FC, ARRAY_SIZE(SEQ_TEST_KEY_OFF_FC));
	if (ret < 0) {
		dsim_err("%s : fail to write CMD : SEQ_TEST_KEY_OFF_FC\n", __func__);
		goto init_exit;
	}

	ret = dsim_write_hl_data(dsim, SEQ_TEST_KEY_OFF_F0, ARRAY_SIZE(SEQ_TEST_KEY_OFF_F0));
	if (ret < 0) {
		dsim_err("%s : fail to write CMD : SEQ_TEST_KEY_OFF_F0\n", __func__);
		goto init_exit;
	}

init_exit:
	return ret;
}

#ifdef CONFIG_LCD_DOZE_MODE

int s6e3hf4_wqhd_setalpm(struct dsim_device *dsim, int mode)
{
	int ret = 0;

	struct panel_private *priv = &(dsim->priv);

	switch (mode) {
	case HLPM_ON_2NIT:
		if (priv->panel_rev >= 5) {
			dsim_write_hl_data(dsim, SEQ_SELECT_HLPM_2NIT_HF4, ARRAY_SIZE(SEQ_SELECT_HLPM_2NIT_HF4));
			dsim_write_hl_data(dsim, SEQ_2NIT_MODE_ON, ARRAY_SIZE(SEQ_2NIT_MODE_ON));
			dsim_write_hl_data(dsim, SEQ_MCLK_1SET_HF4, ARRAY_SIZE(SEQ_MCLK_1SET_HF4));
			}
			else {
			dsim_write_hl_data(dsim, SEQ_SELECT_HLPM_2NIT_HF4, ARRAY_SIZE(SEQ_SELECT_HLPM_2NIT_HF4));
			dsim_write_hl_data(dsim, SEQ_2NIT_MODE_ON, ARRAY_SIZE(SEQ_2NIT_MODE_ON));
		}
		pr_info("%s : HLPM_ON_2NIT !\n", __func__);
		break;
	case ALPM_ON_2NIT:
		if (priv->panel_rev >= 5) {
			dsim_write_hl_data(dsim, SEQ_SELECT_ALPM_2NIT_HF4, ARRAY_SIZE(SEQ_SELECT_ALPM_2NIT_HF4));
			dsim_write_hl_data(dsim, SEQ_2NIT_MODE_ON, ARRAY_SIZE(SEQ_2NIT_MODE_ON));
			dsim_write_hl_data(dsim, SEQ_MCLK_1SET_HF4, ARRAY_SIZE(SEQ_MCLK_1SET_HF4));
		}
		else {
			dsim_write_hl_data(dsim, SEQ_SELECT_ALPM_2NIT_HF4, ARRAY_SIZE(SEQ_SELECT_ALPM_2NIT_HF4));
			dsim_write_hl_data(dsim, SEQ_2NIT_MODE_ON, ARRAY_SIZE(SEQ_2NIT_MODE_ON));
		}
		pr_info("%s : ALPM_ON_2NIT !\n", __func__);
		break;
	case HLPM_ON_40NIT:
		if (priv->panel_rev >= 5) {
			//alpm_write_set(dsim, SEQ_HLPM_40NIT_ON_SET, ARRAY_SIZE(SEQ_HLPM_40NIT_ON_SET));
			dsim_write_hl_data(dsim, SEQ_SELECT_HLPM_40NIT_HF4, ARRAY_SIZE(SEQ_SELECT_HLPM_40NIT_HF4));
			dsim_write_hl_data(dsim, SEQ_40NIT_MODE_ON, ARRAY_SIZE(SEQ_40NIT_MODE_ON));
			dsim_write_hl_data(dsim, SEQ_MCLK_1SET_HF4, ARRAY_SIZE(SEQ_MCLK_1SET_HF4));
		}
		else {
			//alpm_write_set(dsim, SEQ_HLPM_40NIT_ON_SET_OLD, ARRAY_SIZE(SEQ_HLPM_40NIT_ON_SET_OLD));
			dsim_write_hl_data(dsim, SEQ_SELECT_HLPM_2NIT_HF4, ARRAY_SIZE(SEQ_SELECT_HLPM_2NIT_HF4));
			dsim_write_hl_data(dsim, SEQ_40NIT_MODE_ON, ARRAY_SIZE(SEQ_40NIT_MODE_ON));
		}
		pr_info("%s : HLPM_ON_40NIT !\n", __func__);
		break;
	case ALPM_ON_40NIT:
		if (priv->alpm_support == SUPPORT_LOWHZALPM) {
			dsim_write_hl_data(dsim, SEQ_2HZ_GPARA, ARRAY_SIZE(SEQ_2HZ_GPARA));
			dsim_write_hl_data(dsim, SEQ_2HZ_SET, ARRAY_SIZE(SEQ_2HZ_SET));
			dsim_write_hl_data(dsim, SEQ_AID_MOD_ON, ARRAY_SIZE(SEQ_AID_MOD_ON));
			pr_info("%s : Low hz support !\n", __func__);
		}
		if (priv->panel_rev >= 5) {
			//alpm_write_set(dsim, SEQ_ALPM_40NIT_ON_SET, ARRAY_SIZE(SEQ_ALPM_40NIT_ON_SET));
			dsim_write_hl_data(dsim, SEQ_SELECT_ALPM_40NIT_HF4, ARRAY_SIZE(SEQ_SELECT_ALPM_40NIT_HF4));
			dsim_write_hl_data(dsim, SEQ_40NIT_MODE_ON, ARRAY_SIZE(SEQ_40NIT_MODE_ON));
			dsim_write_hl_data(dsim, SEQ_MCLK_1SET_HF4, ARRAY_SIZE(SEQ_MCLK_1SET_HF4));
		}
		else {
			//alpm_write_set(dsim, SEQ_ALPM_40NIT_ON_SET_OLD, ARRAY_SIZE(SEQ_ALPM_40NIT_ON_SET_OLD));
			dsim_write_hl_data(dsim, SEQ_SELECT_ALPM_2NIT_HF4, ARRAY_SIZE(SEQ_SELECT_ALPM_2NIT_HF4));
			dsim_write_hl_data(dsim, SEQ_40NIT_MODE_ON, ARRAY_SIZE(SEQ_40NIT_MODE_ON));
		}
		pr_info("%s : ALPM_ON_40NIT !\n", __func__);
		break;
	default:
		pr_info("%s: input is out of range : %d \n", __func__, mode);
		break;
	}
	dsim_write_hl_data(dsim, HF4_A3_IRC_off, ARRAY_SIZE(HF4_A3_IRC_off));

	dsim_write_hl_data(dsim, SEQ_GAMMA_UPDATE, ARRAY_SIZE(SEQ_GAMMA_UPDATE));
	dsim_write_hl_data(dsim, SEQ_GAMMA_UPDATE_L, ARRAY_SIZE(SEQ_GAMMA_UPDATE_L));
	return ret;

}

static int s6e3hf4_wqhd_enteralpm(struct dsim_device *dsim)
{
	int ret = 0;
	struct panel_private *panel = &dsim->priv;

	dsim_info("%s was called\n", __func__);

	if (panel->state == PANEL_STATE_SUSPENED) {
		dsim_err("ERR:%s:panel is not active\n", __func__);
		return ret;
	}

	ret = dsim_write_hl_data(dsim, SEQ_TEST_KEY_ON_F0, ARRAY_SIZE(SEQ_TEST_KEY_ON_F0));
	if (ret < 0) {
		dsim_err("%s : fail to write CMD : SEQ_TEST_KEY_ON_F0\n", __func__);
	}
	ret = dsim_write_hl_data(dsim, SEQ_TEST_KEY_ON_FC, ARRAY_SIZE(SEQ_TEST_KEY_ON_FC));
	if (ret < 0) {
		dsim_err("%s : fail to write CMD : SEQ_TEST_KEY_ON_FC\n", __func__);
	}

	ret = dsim_write_hl_data(dsim, SEQ_DISPLAY_OFF, ARRAY_SIZE(SEQ_DISPLAY_OFF));
	if (ret < 0) {
		dsim_err("%s : fail to write CMD : SEQ_DISPLAY_OFF\n", __func__);
	}

	ret = s6e3hf4_wqhd_setalpm(dsim, panel->alpm_mode);
	if (ret < 0) {
		dsim_err("%s : failed to set alpm\n", __func__);
	}

	ret = dsim_write_hl_data(dsim, SEQ_TEST_KEY_OFF_F0, ARRAY_SIZE(SEQ_TEST_KEY_OFF_F0));
	if (ret < 0) {
		dsim_err("%s : fail to write CMD : SEQ_TEST_KEY_OFF_F0\n", __func__);
	}
	ret = dsim_write_hl_data(dsim, SEQ_TEST_KEY_OFF_FC, ARRAY_SIZE(SEQ_TEST_KEY_OFF_FC));
	if (ret < 0) {
		dsim_err("%s : fail to write CMD : SEQ_TEST_KEY_OFF_FC\n", __func__);
	}

//exit_enteralpm:
	return ret;
}

static int s6e3hf4_wqhd_exitalpm(struct dsim_device *dsim)
{
	int ret = 0;
	struct panel_private *panel = &dsim->priv;

	dsim_info("%s was called\n", __func__);

	if (panel->state == PANEL_STATE_SUSPENED) {
		dsim_err("ERR:%s:panel is not active\n", __func__);
		return ret;
	}

	ret = dsim_write_hl_data(dsim, SEQ_DISPLAY_OFF, ARRAY_SIZE(SEQ_DISPLAY_ON));
	if (ret < 0) {
		dsim_err("%s : fail to write CMD : DISPLAY_ON\n", __func__);
	}

	ret = dsim_write_hl_data(dsim, SEQ_TEST_KEY_ON_F0, ARRAY_SIZE(SEQ_TEST_KEY_ON_F0));
	if (ret < 0) {
		dsim_err("%s : fail to write CMD : SEQ_TEST_KEY_ON_F0\n", __func__);
	}
	ret = dsim_write_hl_data(dsim, SEQ_TEST_KEY_ON_FC, ARRAY_SIZE(SEQ_TEST_KEY_ON_FC));
	if (ret < 0) {
		dsim_err("%s : fail to write CMD : SEQ_TEST_KEY_ON_FC\n", __func__);
	}

	if (panel->panel_rev >= 5) {
		//alpm_write_set(dsim, SEQ_NORMAL_MODE_ON_SET, ARRAY_SIZE(SEQ_NORMAL_MODE_ON_SET));
		dsim_write_hl_data(dsim, SEQ_MCLK_3SET_HF4, ARRAY_SIZE(SEQ_MCLK_3SET_HF4));
		dsim_write_hl_data(dsim, SEQ_NORMAL_MODE_ON, ARRAY_SIZE(SEQ_NORMAL_MODE_ON));
	}
	else {
		//alpm_write_set(dsim, SEQ_NORMAL_MODE_ON_SET_OLD, ARRAY_SIZE(SEQ_NORMAL_MODE_ON_SET_OLD));
		dsim_write_hl_data(dsim, SEQ_NORMAL_MODE_ON, ARRAY_SIZE(SEQ_NORMAL_MODE_ON));
	}
	if ((panel->alpm_support == SUPPORT_LOWHZALPM) && (panel->alpm_mode == ALPM_ON_40NIT)) {
		dsim_write_hl_data(dsim, SEQ_AOD_LOWHZ_OFF, ARRAY_SIZE(SEQ_AOD_LOWHZ_OFF));
		dsim_write_hl_data(dsim, SEQ_AID_MOD_OFF, ARRAY_SIZE(SEQ_AID_MOD_OFF));
		pr_info("%s : Low hz support !\n", __func__);
	}

	dsim_write_hl_data(dsim, SEQ_TEST_KEY_OFF_F0, ARRAY_SIZE(SEQ_TEST_KEY_OFF_F0));
	dsim_write_hl_data(dsim, SEQ_TEST_KEY_OFF_FC, ARRAY_SIZE(SEQ_TEST_KEY_OFF_FC));

	return ret;
}
#endif

struct dsim_panel_ops s6e3hf4_panel_ops = {
	.probe = s6e3hf4_wqhd_probe,
	.displayon = s6e3hf4_wqhd_displayon,
	.exit = s6e3hf4_wqhd_exit,
	.init = s6e3hf4_wqhd_init,
	.dump = s6e3hf4_wqhd_dump,
#ifdef CONFIG_LCD_DOZE_MODE
	.enteralpm = s6e3hf4_wqhd_enteralpm,
	.exitalpm = s6e3hf4_wqhd_exitalpm,
#endif
#ifdef CONFIG_FB_DSU
	.dsu_cmd = s6e3hf4_wqhd_dsu_command,
#endif
};

/******************** HA5 ********************/


static unsigned int br_tbl_grace_420[EXTEND_BRIGHTNESS + 1] = { // 160504
	2, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 5,
	5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7,
	7, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13, 14,
	14, 15, 15, 16, 16, 17, 17, 19, 19, 20, 21, 21, 22, 22, 24, 24,
	25, 25, 27, 27, 29, 29, 30, 32, 32, 34, 34, 37, 37, 39, 39, 41,
	41, 44, 44, 47, 47, 50, 50, 53, 53, 56, 56, 60, 60, 64, 64, 68,
	68, 72, 72, 77, 77, 82, 82, 87, 87, 93, 93, 98, 98, 105, 105, 111,
	111, 119, 119, 126, 126, 134, 134, 143, 143, 152, 152, 162, 162, 172, 172, 183,
	183, 195, 195, 195, 195, 195, 195, 195, 207, 207, 207, 207, 207, 207, 207, 220,
	220, 220, 220, 220, 220, 220, 234, 234, 234, 234, 234, 234, 234, 234, 249, 249,
	249, 249, 249, 249, 249, 249, 265, 265, 265, 265, 265, 265, 265, 265, 265, 282,
	282, 282, 282, 282, 282, 282, 282, 282, 300, 300, 300, 300, 300, 300, 300, 300,
	300, 300, 316, 316, 316, 316, 316, 316, 316, 316, 333, 333, 333, 333, 333, 333,
	333, 333, 333, 350, 350, 350, 350, 350, 350, 350, 350, 350, 357, 357, 357, 357,
	365, 365, 365, 365, 372, 372, 372, 380, 380, 380, 380, 387, 387, 387, 387, 395,
	395, 395, 395, 403, 403, 403, 403, 412, 412, 412, 412, 420, 420, 420, 420, 420,
	[256 ... 281] = 420,
	[282 ... 295] = 465,
	[296 ... 309] = 488,
	[310 ... 323] = 510,
	[324 ... 336] = 533,
	[337 ... 350] = 555,
	[351 ... 364] = 578,
	[365 ... 365] = 600
};

static unsigned char inter_aor_tbl_ha5[512] = { // 160504
	0x90, 0xE2,   0x90, 0xE1,	0x90, 0xDF,   0x90, 0xDD,	0x90, 0xD3,   0x90, 0xD1,	0x90, 0xCF,   0x90, 0xCA,	0x90, 0xC3,   0x90, 0xC1,	0x90, 0xBF,   0x90, 0xBD,	0x90, 0xB3,   0x90, 0xB1,	0x90, 0xB0,   0x90, 0xAD,
	0x90, 0xA3,   0x90, 0xA1,	0x90, 0x9F,   0x90, 0x9D,	0x90, 0x93,   0x90, 0x92,	0x90, 0x91,   0x90, 0x8F,	0x90, 0x8D,   0x90, 0x83,	0x90, 0x81,   0x90, 0x7F,	0x90, 0x7D,   0x90, 0x73,	0x90, 0x71,   0x90, 0x6F,
	0x90, 0x6D,   0x90, 0x63,	0x90, 0x61,   0x90, 0x57,	0x90, 0x4D,   0x90, 0x3F,	0x90, 0x31,   0x90, 0x27,	0x90, 0x1D,   0x90, 0x0F,	0x90, 0x00,   0x80, 0xF7,	0x80, 0xEE,   0x80, 0xE0,	0x80, 0xD2,   0x80, 0xC8,
	0x80, 0xBE,   0x80, 0xB1,	0x80, 0xA3,   0x80, 0x99,	0x80, 0x8E,   0x80, 0x7F,	0x80, 0x71,   0x80, 0x5B,	0x80, 0x45,   0x80, 0x2F,	0x80, 0x21,   0x80, 0x13,	0x80, 0x05,   0x70, 0xF7,	0x70, 0xDF,   0x70, 0xC7,
	0x70, 0xBB,   0x70, 0xAE,	0x70, 0x96,   0x70, 0x7F,	0x70, 0x66,   0x70, 0x4D,	0x70, 0x2E,   0x70, 0x13,	0x60, 0xF7,   0x60, 0xE3,	0x60, 0xCF,   0x60, 0xA9,	0x60, 0x83,   0x60, 0x69,	0x60, 0x4F,   0x60, 0x34,
	0x60, 0x1A,   0x50, 0xF5,	0x50, 0xCF,   0x50, 0xA7,	0x50, 0x7E,   0x50, 0x57,	0x50, 0x30,   0x50, 0x08,	0x40, 0xE1,   0x40, 0xB8,	0x40, 0x8E,   0x40, 0x57,	0x40, 0x1F,   0x30, 0xE8,	0x30, 0xB1,   0x30, 0xD8,
	0x30, 0xA7,   0x30, 0xD5,	0x30, 0xA7,   0x30, 0xDD,	0x30, 0xA7,   0x30, 0xD9,	0x30, 0xA7,   0x30, 0xD6,	0x30, 0xA7,   0x30, 0xDC,	0x30, 0xA7,   0x30, 0xD1,	0x30, 0xA7,   0x30, 0xDE,	0x30, 0xA7,   0x30, 0xD4,
	0x30, 0xA7,   0x30, 0xDE,	0x30, 0xA7,   0x30, 0xD5,	0x30, 0xA7,   0x30, 0xD8,	0x30, 0xA7,   0x30, 0xDB,	0x30, 0xA7,   0x30, 0xD8,	0x30, 0xA7,   0x30, 0xDA,	0x30, 0xA7,   0x30, 0xD7,	0x30, 0xA7,   0x30, 0xD6,
	0x30, 0xA4,   0x30, 0x91,	0x30, 0x7E,   0x30, 0x6B,	0x30, 0x58,   0x30, 0x45,	0x30, 0x32,   0x30, 0x1F,	0x30, 0x0B,   0x20, 0xF6,	0x20, 0xE2,   0x20, 0xCE,	0x20, 0xBA,   0x20, 0xA5,	0x20, 0x91,   0x20, 0x7C,
	0x20, 0x68,   0x20, 0x53,	0x20, 0x3E,   0x20, 0x29,	0x20, 0x15,   0x20, 0x00,	0x10, 0xEA,   0x10, 0xD4,	0x10, 0xBE,   0x10, 0xA8,	0x10, 0x92,   0x10, 0x7C,	0x10, 0x66,   0x10, 0x50,	0x10, 0x7E,   0x10, 0x6C,
	0x10, 0x5B,   0x10, 0x49,	0x10, 0x38,   0x10, 0x26,	0x10, 0x15,   0x10, 0x03,	0x10, 0x80,   0x10, 0x70,	0x10, 0x61,   0x10, 0x51,	0x10, 0x41,   0x10, 0x32,	0x10, 0x22,   0x10, 0x13,	0x10, 0x03,   0x10, 0x80,
	0x10, 0x70,   0x10, 0x60,	0x10, 0x51,   0x10, 0x41,	0x10, 0x32,   0x10, 0x22,	0x10, 0x13,   0x10, 0x03,	0x10, 0x81,   0x10, 0x73,	0x10, 0x65,   0x10, 0x57,	0x10, 0x49,   0x10, 0x3B,	0x10, 0x2D,   0x10, 0x1F,
	0x10, 0x11,   0x10, 0x03,	0x10, 0x6A,   0x10, 0x5B,	0x10, 0x4D,   0x10, 0x3E,	0x10, 0x2F,   0x10, 0x20,	0x10, 0x12,   0x10, 0x03,	0x10, 0x6D,   0x10, 0x5F,	0x10, 0x52,   0x10, 0x45,	0x10, 0x38,   0x10, 0x2B,
	0x10, 0x1D,   0x10, 0x10,	0x10, 0x03,   0x10, 0x67,	0x10, 0x5B,   0x10, 0x4E,	0x10, 0x42,   0x10, 0x35,	0x10, 0x29,   0x10, 0x1C,	0x10, 0x10,   0x10, 0x03,	0x10, 0x25,   0x10, 0x1A,	0x10, 0x0E,   0x10, 0x03,
	0x10, 0x29,   0x10, 0x1C,	0x10, 0x10,   0x10, 0x03,	0x00, 0xEE,   0x00, 0xD8,	0x00, 0xC3,   0x00, 0xB5,	0x00, 0xA6,   0x00, 0x98,	0x00, 0x8A,   0x00, 0x7D,	0x00, 0x70,   0x00, 0x64,	0x00, 0x57,   0x00, 0x45,
	0x00, 0x33,   0x00, 0x22,	0x00, 0x10,   0x00, 0x32,	0x00, 0x26,   0x00, 0x19,	0x00, 0x0C,   0x00, 0x36,	0x00, 0x28,   0x00, 0x1A,	0x00, 0x0C,   0x00, 0x31,	0x00, 0x24,   0x00, 0x18,	0x00, 0x0C,   0x00, 0x0C,
};

static unsigned char irc_table_ha5_revA[EXTEND_BRIGHTNESS + 1][21] = { // 160504
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x02, 0x02, 0x02, 0x01, 0x01, 0x00, 0x00, 0x01, 0x01, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x02, 0x02, 0x02, 0x01, 0x01, 0x00, 0x00, 0x01, 0x01, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x02, 0x02, 0x02, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x02, 0x02, 0x02, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x02, 0x02, 0x02, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x02, 0x02, 0x02, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x03, 0x03, 0x02, 0x01, 0x01, 0x01, 0x00, 0x01, 0x01, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x03, 0x03, 0x02, 0x01, 0x01, 0x01, 0x00, 0x01, 0x01, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x03, 0x03, 0x03, 0x01, 0x01, 0x00, 0x01, 0x01, 0x01, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x03, 0x03, 0x03, 0x01, 0x01, 0x00, 0x01, 0x01, 0x01, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x03, 0x03, 0x03, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x03, 0x03, 0x03, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x03, 0x03, 0x03, 0x01, 0x02, 0x01, 0x01, 0x01, 0x00, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x03, 0x03, 0x03, 0x01, 0x02, 0x01, 0x01, 0x01, 0x00, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x04, 0x03, 0x03, 0x01, 0x02, 0x01, 0x01, 0x01, 0x01, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x04, 0x03, 0x03, 0x01, 0x02, 0x01, 0x01, 0x01, 0x01, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x04, 0x04, 0x03, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x04, 0x04, 0x03, 0x01, 0x01, 0x02, 0x01, 0x02, 0x00, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x04, 0x04, 0x04, 0x01, 0x02, 0x01, 0x01, 0x01, 0x01, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x04, 0x04, 0x04, 0x02, 0x02, 0x01, 0x01, 0x01, 0x01, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x04, 0x04, 0x04, 0x02, 0x02, 0x01, 0x01, 0x01, 0x01, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x05, 0x04, 0x04, 0x01, 0x02, 0x01, 0x01, 0x02, 0x01, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x05, 0x04, 0x04, 0x01, 0x02, 0x01, 0x01, 0x02, 0x01, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x05, 0x05, 0x04, 0x01, 0x02, 0x02, 0x01, 0x01, 0x00, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x05, 0x05, 0x04, 0x02, 0x02, 0x02, 0x01, 0x01, 0x01, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x05, 0x05, 0x05, 0x02, 0x02, 0x01, 0x01, 0x02, 0x01, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x05, 0x05, 0x05, 0x02, 0x02, 0x01, 0x01, 0x02, 0x01, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x05, 0x05, 0x05, 0x02, 0x02, 0x01, 0x01, 0x02, 0x01, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x06, 0x05, 0x05, 0x01, 0x03, 0x02, 0x02, 0x01, 0x01, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x06, 0x06, 0x05, 0x02, 0x02, 0x02, 0x01, 0x02, 0x01, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x06, 0x06, 0x05, 0x02, 0x02, 0x02, 0x01, 0x02, 0x01, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x06, 0x06, 0x06, 0x02, 0x03, 0x01, 0x02, 0x02, 0x01, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x06, 0x06, 0x06, 0x02, 0x03, 0x02, 0x02, 0x02, 0x01, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x07, 0x06, 0x06, 0x02, 0x03, 0x02, 0x01, 0x02, 0x01, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x07, 0x07, 0x06, 0x02, 0x03, 0x02, 0x02, 0x02, 0x01, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x07, 0x07, 0x06, 0x02, 0x03, 0x02, 0x02, 0x02, 0x01, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x07, 0x07, 0x07, 0x03, 0x03, 0x02, 0x02, 0x03, 0x01, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x08, 0x07, 0x07, 0x02, 0x04, 0x02, 0x02, 0x02, 0x01, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x08, 0x08, 0x07, 0x02, 0x03, 0x02, 0x02, 0x02, 0x02, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x08, 0x08, 0x07, 0x03, 0x03, 0x02, 0x02, 0x03, 0x02, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x08, 0x08, 0x07, 0x03, 0x04, 0x03, 0x02, 0x02, 0x01, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x08, 0x08, 0x08, 0x03, 0x04, 0x02, 0x02, 0x02, 0x01, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x09, 0x09, 0x08, 0x03, 0x03, 0x02, 0x02, 0x03, 0x02, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x09, 0x09, 0x08, 0x03, 0x04, 0x03, 0x02, 0x02, 0x01, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x0A, 0x09, 0x08, 0x02, 0x04, 0x03, 0x03, 0x03, 0x02, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x0A, 0x0A, 0x09, 0x03, 0x04, 0x02, 0x02, 0x02, 0x02, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x0A, 0x0A, 0x09, 0x03, 0x04, 0x03, 0x03, 0x03, 0x02, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x0A, 0x0A, 0x09, 0x04, 0x04, 0x03, 0x02, 0x04, 0x02, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x0B, 0x0B, 0x0A, 0x03, 0x04, 0x03, 0x03, 0x03, 0x01, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x0B, 0x0B, 0x0A, 0x03, 0x04, 0x03, 0x03, 0x04, 0x02, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x0B, 0x0B, 0x0A, 0x04, 0x05, 0x03, 0x03, 0x03, 0x02, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x0C, 0x0B, 0x0A, 0x03, 0x05, 0x04, 0x03, 0x04, 0x02, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x0C, 0x0C, 0x0B, 0x04, 0x05, 0x03, 0x03, 0x03, 0x02, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x0C, 0x0C, 0x0B, 0x04, 0x05, 0x04, 0x03, 0x04, 0x02, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x0D, 0x0D, 0x0B, 0x04, 0x05, 0x04, 0x03, 0x04, 0x02, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x0D, 0x0D, 0x0C, 0x04, 0x05, 0x04, 0x04, 0x04, 0x02, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x0E, 0x0D, 0x0C, 0x04, 0x06, 0x04, 0x03, 0x04, 0x02, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x0E, 0x0E, 0x0C, 0x04, 0x06, 0x05, 0x04, 0x04, 0x02, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x0F, 0x0E, 0x0D, 0x04, 0x06, 0x04, 0x04, 0x05, 0x03, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x0F, 0x0F, 0x0D, 0x05, 0x06, 0x04, 0x03, 0x04, 0x03, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x10, 0x0F, 0x0E, 0x04, 0x07, 0x04, 0x04, 0x04, 0x03, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x10, 0x10, 0x0E, 0x05, 0x06, 0x05, 0x04, 0x05, 0x02, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x11, 0x10, 0x0F, 0x05, 0x07, 0x04, 0x04, 0x05, 0x03, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x11, 0x11, 0x0F, 0x05, 0x07, 0x05, 0x05, 0x05, 0x03, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x12, 0x11, 0x10, 0x05, 0x07, 0x05, 0x05, 0x06, 0x03, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x12, 0x12, 0x10, 0x06, 0x07, 0x05, 0x04, 0x05, 0x03, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x13, 0x12, 0x11, 0x05, 0x08, 0x05, 0x05, 0x06, 0x03, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x13, 0x13, 0x11, 0x06, 0x08, 0x06, 0x05, 0x06, 0x03, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x14, 0x13, 0x12, 0x06, 0x09, 0x05, 0x05, 0x06, 0x04, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x14, 0x14, 0x12, 0x07, 0x08, 0x06, 0x05, 0x06, 0x03, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x15, 0x15, 0x13, 0x07, 0x08, 0x06, 0x05, 0x07, 0x03, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x16, 0x15, 0x13, 0x07, 0x09, 0x07, 0x05, 0x07, 0x03, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x16, 0x16, 0x14, 0x07, 0x09, 0x06, 0x06, 0x07, 0x04, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x17, 0x16, 0x14, 0x07, 0x0A, 0x07, 0x06, 0x07, 0x04, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x18, 0x17, 0x15, 0x07, 0x0A, 0x07, 0x06, 0x07, 0x04, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x19, 0x18, 0x16, 0x07, 0x0A, 0x07, 0x07, 0x08, 0x04, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x19, 0x19, 0x17, 0x08, 0x0A, 0x07, 0x07, 0x08, 0x04, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x1A, 0x1A, 0x17, 0x08, 0x0A, 0x08, 0x07, 0x08, 0x04, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x1B, 0x1A, 0x18, 0x08, 0x0B, 0x08, 0x07, 0x09, 0x04, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x1C, 0x1B, 0x19, 0x08, 0x0C, 0x08, 0x07, 0x08, 0x04, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x1D, 0x1C, 0x19, 0x09, 0x0C, 0x09, 0x07, 0x09, 0x05, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x1E, 0x1D, 0x1A, 0x09, 0x0C, 0x09, 0x07, 0x09, 0x05, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x1F, 0x1E, 0x1B, 0x09, 0x0D, 0x09, 0x08, 0x09, 0x05, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x1F, 0x1F, 0x1C, 0x0A, 0x0D, 0x09, 0x08, 0x09, 0x05, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x21, 0x20, 0x1D, 0x0A, 0x0D, 0x09, 0x08, 0x0A, 0x06, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x22, 0x21, 0x1E, 0x0A, 0x0E, 0x09, 0x08, 0x0A, 0x06, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x23, 0x22, 0x1F, 0x0A, 0x0E, 0x0A, 0x09, 0x0A, 0x06, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x24, 0x23, 0x20, 0x0B, 0x0F, 0x0A, 0x09, 0x0A, 0x06, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x25, 0x24, 0x21, 0x0B, 0x0F, 0x0A, 0x0A, 0x0B, 0x07, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x26, 0x25, 0x22, 0x0C, 0x10, 0x0A, 0x09, 0x0B, 0x07, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x26, 0x25, 0x22, 0x0C, 0x10, 0x0B, 0x0A, 0x0C, 0x07, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x27, 0x26, 0x22, 0x0B, 0x10, 0x0B, 0x0A, 0x0B, 0x07, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x27, 0x26, 0x22, 0x0C, 0x10, 0x0C, 0x0A, 0x0C, 0x06, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x27, 0x26, 0x23, 0x0D, 0x11, 0x0B, 0x0A, 0x0C, 0x07, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x28, 0x27, 0x23, 0x0C, 0x10, 0x0C, 0x0A, 0x0C, 0x06, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x28, 0x27, 0x23, 0x0C, 0x11, 0x0C, 0x0A, 0x0C, 0x07, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x28, 0x27, 0x24, 0x0D, 0x11, 0x0B, 0x0A, 0x0C, 0x07, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x29, 0x28, 0x24, 0x0C, 0x11, 0x0C, 0x0B, 0x0C, 0x07, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x29, 0x28, 0x24, 0x0D, 0x11, 0x0C, 0x0A, 0x0C, 0x07, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x29, 0x28, 0x25, 0x0D, 0x12, 0x0C, 0x0B, 0x0C, 0x07, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x2A, 0x29, 0x25, 0x0D, 0x11, 0x0C, 0x0A, 0x0D, 0x07, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x2A, 0x29, 0x25, 0x0D, 0x12, 0x0D, 0x0B, 0x0C, 0x07, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x2A, 0x29, 0x26, 0x0E, 0x12, 0x0C, 0x0A, 0x0D, 0x07, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x2B, 0x2A, 0x26, 0x0D, 0x12, 0x0C, 0x0B, 0x0C, 0x08, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x2B, 0x2A, 0x26, 0x0E, 0x12, 0x0D, 0x0B, 0x0D, 0x07, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x2C, 0x2B, 0x27, 0x0D, 0x12, 0x0C, 0x0B, 0x0D, 0x08, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x2C, 0x2B, 0x27, 0x0E, 0x12, 0x0D, 0x0B, 0x0E, 0x07, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x2C, 0x2B, 0x27, 0x0E, 0x13, 0x0D, 0x0B, 0x0D, 0x08, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x2D, 0x2C, 0x28, 0x0E, 0x12, 0x0C, 0x0B, 0x0E, 0x08, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x2D, 0x2C, 0x28, 0x0E, 0x13, 0x0D, 0x0C, 0x0D, 0x08, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x2E, 0x2D, 0x28, 0x0E, 0x12, 0x0D, 0x0B, 0x0E, 0x08, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x2E, 0x2D, 0x29, 0x0E, 0x13, 0x0D, 0x0C, 0x0E, 0x08, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x2E, 0x2D, 0x29, 0x0F, 0x14, 0x0D, 0x0C, 0x0D, 0x08, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x2F, 0x2E, 0x29, 0x0E, 0x13, 0x0E, 0x0C, 0x0E, 0x08, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x2F, 0x2E, 0x2A, 0x0F, 0x13, 0x0D, 0x0C, 0x0E, 0x08, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x2F, 0x2E, 0x2A, 0x0F, 0x14, 0x0E, 0x0C, 0x0E, 0x08, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x30, 0x2F, 0x2A, 0x0F, 0x14, 0x0E, 0x0C, 0x0E, 0x08, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x30, 0x2F, 0x2B, 0x0F, 0x14, 0x0D, 0x0C, 0x0E, 0x09, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x30, 0x2F, 0x2B, 0x10, 0x14, 0x0E, 0x0C, 0x0F, 0x08, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x31, 0x30, 0x2B, 0x0F, 0x14, 0x0E, 0x0C, 0x0F, 0x09, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x31, 0x30, 0x2C, 0x10, 0x15, 0x0E, 0x0C, 0x0E, 0x08, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x32, 0x31, 0x2C, 0x0F, 0x14, 0x0E, 0x0D, 0x0F, 0x09, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x32, 0x31, 0x2C, 0x10, 0x15, 0x0F, 0x0C, 0x0F, 0x08, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x32, 0x31, 0x2D, 0x10, 0x15, 0x0E, 0x0D, 0x0F, 0x09, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x33, 0x32, 0x2D, 0x10, 0x15, 0x0F, 0x0C, 0x0F, 0x08, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x33, 0x32, 0x2D, 0x10, 0x15, 0x0F, 0x0D, 0x0F, 0x09, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x34, 0x32, 0x2E, 0x10, 0x16, 0x0E, 0x0D, 0x0F, 0x09, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x34, 0x33, 0x2E, 0x10, 0x15, 0x0F, 0x0D, 0x10, 0x09, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x34, 0x33, 0x2E, 0x11, 0x16, 0x0F, 0x0D, 0x10, 0x09, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x35, 0x33, 0x2F, 0x10, 0x16, 0x0F, 0x0D, 0x10, 0x09, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x35, 0x34, 0x2F, 0x10, 0x16, 0x0F, 0x0E, 0x10, 0x09, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x35, 0x34, 0x2F, 0x11, 0x16, 0x10, 0x0E, 0x10, 0x09, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x36, 0x35, 0x30, 0x11, 0x16, 0x0F, 0x0D, 0x10, 0x09, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x36, 0x35, 0x30, 0x11, 0x16, 0x0F, 0x0E, 0x10, 0x0A, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x36, 0x35, 0x30, 0x11, 0x17, 0x10, 0x0E, 0x10, 0x09, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x37, 0x36, 0x31, 0x11, 0x16, 0x0F, 0x0E, 0x11, 0x0A, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x37, 0x36, 0x31, 0x11, 0x17, 0x10, 0x0E, 0x10, 0x09, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x38, 0x36, 0x31, 0x11, 0x17, 0x10, 0x0E, 0x11, 0x0A, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x38, 0x37, 0x32, 0x12, 0x17, 0x10, 0x0E, 0x11, 0x09, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x39, 0x37, 0x32, 0x11, 0x18, 0x10, 0x0E, 0x11, 0x0A, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x39, 0x37, 0x32, 0x11, 0x18, 0x11, 0x0F, 0x11, 0x09, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x39, 0x38, 0x33, 0x12, 0x18, 0x10, 0x0E, 0x11, 0x0A, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x3A, 0x38, 0x33, 0x11, 0x18, 0x11, 0x0F, 0x11, 0x09, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x3A, 0x39, 0x33, 0x12, 0x18, 0x11, 0x0F, 0x11, 0x0A, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x3A, 0x39, 0x34, 0x13, 0x18, 0x10, 0x0E, 0x12, 0x0B, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x3B, 0x39, 0x34, 0x12, 0x19, 0x11, 0x0F, 0x11, 0x0A, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x3B, 0x3A, 0x34, 0x13, 0x18, 0x11, 0x0F, 0x12, 0x0B, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x3B, 0x3A, 0x35, 0x13, 0x19, 0x11, 0x0F, 0x11, 0x0A, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x3C, 0x3A, 0x35, 0x12, 0x19, 0x11, 0x10, 0x12, 0x0B, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x3C, 0x3B, 0x35, 0x13, 0x19, 0x12, 0x0F, 0x12, 0x0A, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x3D, 0x3B, 0x36, 0x13, 0x19, 0x11, 0x0F, 0x13, 0x0B, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x3D, 0x3C, 0x36, 0x13, 0x19, 0x12, 0x10, 0x12, 0x0A, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x3D, 0x3C, 0x36, 0x13, 0x19, 0x12, 0x10, 0x13, 0x0A, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x3E, 0x3C, 0x37, 0x13, 0x1A, 0x11, 0x0F, 0x12, 0x0B, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x3E, 0x3D, 0x37, 0x13, 0x19, 0x12, 0x10, 0x13, 0x0B, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x3F, 0x3D, 0x37, 0x13, 0x1A, 0x12, 0x10, 0x13, 0x0B, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x3F, 0x3E, 0x38, 0x14, 0x1A, 0x12, 0x0F, 0x12, 0x0B, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x3F, 0x3E, 0x38, 0x14, 0x1A, 0x12, 0x10, 0x13, 0x0B, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x40, 0x3E, 0x38, 0x14, 0x1B, 0x13, 0x10, 0x13, 0x0B, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x40, 0x3F, 0x39, 0x14, 0x1A, 0x12, 0x10, 0x14, 0x0B, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x41, 0x3F, 0x39, 0x14, 0x1B, 0x13, 0x10, 0x13, 0x0B, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x41, 0x40, 0x3A, 0x14, 0x1A, 0x12, 0x11, 0x14, 0x0B, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x41, 0x40, 0x3A, 0x15, 0x1B, 0x13, 0x10, 0x14, 0x0B, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x42, 0x40, 0x3A, 0x14, 0x1C, 0x13, 0x11, 0x13, 0x0C, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x42, 0x41, 0x3B, 0x15, 0x1B, 0x13, 0x11, 0x14, 0x0B, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x43, 0x41, 0x3B, 0x14, 0x1C, 0x13, 0x11, 0x14, 0x0C, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x43, 0x42, 0x3B, 0x15, 0x1B, 0x14, 0x11, 0x14, 0x0B, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x43, 0x42, 0x3C, 0x15, 0x1C, 0x13, 0x11, 0x14, 0x0C, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x44, 0x42, 0x3C, 0x15, 0x1C, 0x13, 0x11, 0x14, 0x0C, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x44, 0x43, 0x3C, 0x15, 0x1C, 0x14, 0x12, 0x14, 0x0C, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x45, 0x43, 0x3D, 0x15, 0x1C, 0x13, 0x11, 0x15, 0x0C, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x45, 0x43, 0x3D, 0x15, 0x1D, 0x14, 0x12, 0x15, 0x0C, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x45, 0x44, 0x3D, 0x16, 0x1D, 0x14, 0x11, 0x14, 0x0C, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x46, 0x44, 0x3E, 0x15, 0x1D, 0x14, 0x12, 0x15, 0x0C, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x46, 0x45, 0x3E, 0x16, 0x1D, 0x14, 0x12, 0x15, 0x0C, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x47, 0x45, 0x3F, 0x16, 0x1D, 0x14, 0x11, 0x15, 0x0C, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x47, 0x45, 0x3F, 0x16, 0x1E, 0x14, 0x12, 0x15, 0x0C, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x47, 0x46, 0x3F, 0x16, 0x1D, 0x15, 0x12, 0x15, 0x0C, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x48, 0x46, 0x3F, 0x16, 0x1E, 0x15, 0x12, 0x15, 0x0C, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x48, 0x46, 0x40, 0x16, 0x1E, 0x15, 0x13, 0x16, 0x0C, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x49, 0x47, 0x40, 0x16, 0x1E, 0x15, 0x12, 0x16, 0x0D, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x49, 0x47, 0x41, 0x17, 0x1E, 0x14, 0x12, 0x16, 0x0D, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x49, 0x48, 0x41, 0x17, 0x1E, 0x15, 0x13, 0x16, 0x0D, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x4A, 0x48, 0x41, 0x16, 0x1E, 0x15, 0x13, 0x16, 0x0D, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x4A, 0x48, 0x41, 0x17, 0x1F, 0x16, 0x13, 0x16, 0x0C, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x4A, 0x49, 0x42, 0x17, 0x1E, 0x15, 0x13, 0x17, 0x0D, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x4B, 0x49, 0x42, 0x17, 0x1F, 0x16, 0x13, 0x16, 0x0D, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x4B, 0x49, 0x43, 0x18, 0x20, 0x15, 0x13, 0x16, 0x0D, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x4C, 0x4A, 0x43, 0x17, 0x1F, 0x16, 0x13, 0x17, 0x0D, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x4C, 0x4A, 0x43, 0x18, 0x20, 0x16, 0x13, 0x16, 0x0D, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x4D, 0x4B, 0x44, 0x17, 0x20, 0x16, 0x14, 0x17, 0x0D, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x4D, 0x4B, 0x44, 0x18, 0x20, 0x16, 0x13, 0x17, 0x0E, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x4D, 0x4C, 0x45, 0x19, 0x20, 0x16, 0x13, 0x17, 0x0D, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x4E, 0x4C, 0x45, 0x18, 0x20, 0x16, 0x14, 0x18, 0x0E, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x4E, 0x4D, 0x45, 0x19, 0x20, 0x17, 0x13, 0x17, 0x0D, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x4F, 0x4D, 0x46, 0x18, 0x20, 0x16, 0x14, 0x18, 0x0E, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x4F, 0x4D, 0x46, 0x19, 0x21, 0x17, 0x14, 0x18, 0x0D, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x50, 0x4E, 0x46, 0x18, 0x21, 0x17, 0x14, 0x17, 0x0E, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x50, 0x4E, 0x47, 0x19, 0x21, 0x17, 0x14, 0x18, 0x0D, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x50, 0x4E, 0x47, 0x19, 0x21, 0x17, 0x14, 0x18, 0x0E, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x51, 0x4F, 0x47, 0x19, 0x21, 0x17, 0x14, 0x18, 0x0E, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x51, 0x4F, 0x48, 0x19, 0x22, 0x17, 0x15, 0x18, 0x0E, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x51, 0x50, 0x48, 0x1A, 0x21, 0x17, 0x14, 0x19, 0x0E, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x52, 0x50, 0x48, 0x19, 0x22, 0x18, 0x15, 0x18, 0x0E, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x52, 0x50, 0x49, 0x1A, 0x22, 0x17, 0x15, 0x19, 0x0F, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x53, 0x51, 0x49, 0x19, 0x22, 0x18, 0x15, 0x19, 0x0E, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x53, 0x51, 0x4A, 0x1A, 0x23, 0x17, 0x15, 0x18, 0x0F, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x53, 0x52, 0x4A, 0x1A, 0x22, 0x18, 0x15, 0x19, 0x0E, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x54, 0x52, 0x4A, 0x1A, 0x23, 0x18, 0x15, 0x19, 0x0F, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x55, 0x53, 0x4B, 0x1A, 0x23, 0x18, 0x15, 0x19, 0x0F, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x55, 0x53, 0x4B, 0x1A, 0x23, 0x19, 0x16, 0x1A, 0x0E, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x55, 0x53, 0x4C, 0x1B, 0x24, 0x18, 0x15, 0x19, 0x0F, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x56, 0x54, 0x4C, 0x1A, 0x23, 0x19, 0x16, 0x1A, 0x0E, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x56, 0x54, 0x4C, 0x1B, 0x24, 0x19, 0x15, 0x19, 0x0F, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x56, 0x54, 0x4C, 0x1B, 0x24, 0x19, 0x16, 0x1A, 0x0F, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x57, 0x55, 0x4D, 0x1B, 0x24, 0x19, 0x16, 0x1A, 0x0F, },
	{0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x57, 0x55, 0x4D, 0x1B, 0x24, 0x19, 0x16, 0x1A, 0x0F, },
	[256 ... 281] = {0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x5C, 0x5A, 0x51, 0x1C, 0x26, 0x1B, 0x17, 0x1B, 0x0F, },
	[282 ... 295] = {0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x60, 0x5E, 0x55, 0x1E, 0x28, 0x1C, 0x19, 0x1D, 0x11, },
	[296 ... 309] = {0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x65, 0x63, 0x59, 0x1F, 0x2A, 0x1E, 0x1A, 0x1E, 0x11, },
	[310 ... 323] = {0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x6A, 0x67, 0x5E, 0x20, 0x2C, 0x1E, 0x1B, 0x20, 0x12, },
	[324 ... 336] = {0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x6E, 0x6C, 0x62, 0x23, 0x2E, 0x1F, 0x1C, 0x21, 0x13, },
	[337 ... 350] = {0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x73, 0x70, 0x66, 0x24, 0x30, 0x21, 0x1D, 0x22, 0x14, },
	[351 ... 364] = {0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x78, 0x75, 0x6A, 0x25, 0x32, 0x22, 0x1E, 0x23, 0x15, },
	[365 ... 365] = {0xB8, 0x6D, 0xB0, 0x48, 0x4C, 0x95, 0xDA, 0x33, 0x69, 0x12, 0x7A, 0xDA, 0x7C, 0x79, 0x6E, 0x27, 0x34, 0x24, 0x1F, 0x25, 0x15, },
};


struct SmtDimInfo dimming_info_HA5_RevA[MAX_BR_INFO] = {
	{ .br = 2, .refBr = 119, .cGma = gma2p15, .rTbl = ha5_revA_rtbl2nit, .cTbl = ha5_revA_ctbl2nit, .aid = aid9791, .elvCaps = elvCaps19h, .elv = elv19h, .way = W1, .elvss_offset = ha5_elvss_offset2},
	{ .br = 3, .refBr = 119, .cGma = gma2p15, .rTbl = ha5_revA_rtbl3nit, .cTbl = ha5_revA_ctbl3nit, .aid = aid9698, .elvCaps = elvCaps19h, .elv = elv19h, .way = W1, .elvss_offset = ha5_elvss_offset3},
	{ .br = 4, .refBr = 119, .cGma = gma2p15, .rTbl = ha5_revA_rtbl4nit, .cTbl = ha5_revA_ctbl4nit, .aid = aid9598, .elvCaps = elvCaps19h, .elv = elv19h, .way = W1, .elvss_offset = ha5_elvss_offset4},
	{ .br = 5, .refBr = 119, .cGma = gma2p15, .rTbl = ha5_revA_rtbl5nit, .cTbl = ha5_revA_ctbl5nit, .aid = aid9481, .elvCaps = elvCaps19h, .elv = elv19h, .way = W1, .elvss_offset = ha5_elvss_offset5},
	{ .br = 6, .refBr = 119, .cGma = gma2p15, .rTbl = ha5_revA_rtbl6nit, .cTbl = ha5_revA_ctbl6nit, .aid = aid9400, .elvCaps = elvCaps19h, .elv = elv19h, .way = W1, .elvss_offset = elvss_offset_otp},
	{ .br = 7, .refBr = 119, .cGma = gma2p15, .rTbl = ha5_revA_rtbl7nit, .cTbl = ha5_revA_ctbl7nit, .aid = aid9292, .elvCaps = elvCaps19h, .elv = elv19h, .way = W1, .elvss_offset = elvss_offset_otp},
	{ .br = 8, .refBr = 119, .cGma = gma2p15, .rTbl = ha5_revA_rtbl8nit, .cTbl = ha5_revA_ctbl8nit, .aid = aid9214, .elvCaps = elvCaps19h, .elv = elv19h, .way = W1, .elvss_offset = elvss_offset_otp},
	{ .br = 9, .refBr = 119, .cGma = gma2p15, .rTbl = ha5_revA_rtbl9nit, .cTbl = ha5_revA_ctbl9nit, .aid = aid9106, .elvCaps = elvCaps19h, .elv = elv19h, .way = W1, .elvss_offset = elvss_offset_otp},
	{ .br = 10, .refBr = 119, .cGma = gma2p15, .rTbl = ha5_revA_rtbl10nit, .cTbl = ha5_revA_ctbl10nit, .aid = aid9029, .elvCaps = elvCaps19h, .elv = elv19h, .way = W1, .elvss_offset = elvss_offset_otp},
	{ .br = 11, .refBr = 119, .cGma = gma2p15, .rTbl = ha5_revA_rtbl11nit, .cTbl = ha5_revA_ctbl11nit, .aid = aid8916, .elvCaps = elvCaps19h, .elv = elv19h, .way = W1, .elvss_offset = elvss_offset_otp},
	{ .br = 12, .refBr = 119, .cGma = gma2p15, .rTbl = ha5_revA_rtbl12nit, .cTbl = ha5_revA_ctbl12nit, .aid = aid8847, .elvCaps = elvCaps19h, .elv = elv19h, .way = W1, .elvss_offset = elvss_offset_otp},
	{ .br = 13, .refBr = 119, .cGma = gma2p15, .rTbl = ha5_revA_rtbl13nit, .cTbl = ha5_revA_ctbl13nit, .aid = aid8738, .elvCaps = elvCaps19h, .elv = elv19h, .way = W1, .elvss_offset = elvss_offset_otp},
	{ .br = 14, .refBr = 119, .cGma = gma2p15, .rTbl = ha5_revA_rtbl14nit, .cTbl = ha5_revA_ctbl14nit, .aid = aid8661, .elvCaps = elvCaps19h, .elv = elv19h, .way = W1, .elvss_offset = elvss_offset_otp},
	{ .br = 15, .refBr = 119, .cGma = gma2p15, .rTbl = ha5_revA_rtbl15nit, .cTbl = ha5_revA_ctbl15nit, .aid = aid8557, .elvCaps = elvCaps19h, .elv = elv19h, .way = W1, .elvss_offset = elvss_offset_otp},
	{ .br = 16, .refBr = 119, .cGma = gma2p15, .rTbl = ha5_revA_rtbl16nit, .cTbl = ha5_revA_ctbl16nit, .aid = aid8475, .elvCaps = elvCaps19h, .elv = elv19h, .way = W1, .elvss_offset = elvss_offset_otp},
	{ .br = 17, .refBr = 119, .cGma = gma2p15, .rTbl = ha5_revA_rtbl17nit, .cTbl = ha5_revA_ctbl17nit, .aid = aid8363, .elvCaps = elvCaps19h, .elv = elv19h, .way = W1, .elvss_offset = elvss_offset_otp},
	{ .br = 19, .refBr = 119, .cGma = gma2p15, .rTbl = ha5_revA_rtbl19nit, .cTbl = ha5_revA_ctbl19nit, .aid = aid8193, .elvCaps = elvCaps19h, .elv = elv19h, .way = W1, .elvss_offset = elvss_offset_otp},
	{ .br = 20, .refBr = 119, .cGma = gma2p15, .rTbl = ha5_revA_rtbl20nit, .cTbl = ha5_revA_ctbl20nit, .aid = aid8108, .elvCaps = elvCaps19h, .elv = elv19h, .way = W1, .elvss_offset = elvss_offset_otp},
	{ .br = 21, .refBr = 119, .cGma = gma2p15, .rTbl = ha5_revA_rtbl21nit, .cTbl = ha5_revA_ctbl21nit, .aid = aid7999, .elvCaps = elvCaps19h, .elv = elv19h, .way = W1, .elvss_offset = elvss_offset_otp},
	{ .br = 22, .refBr = 119, .cGma = gma2p15, .rTbl = ha5_revA_rtbl22nit, .cTbl = ha5_revA_ctbl22nit, .aid = aid7891, .elvCaps = elvCaps19h, .elv = elv19h, .way = W1, .elvss_offset = elvss_offset_otp},
	{ .br = 24, .refBr = 119, .cGma = gma2p15, .rTbl = ha5_revA_rtbl24nit, .cTbl = ha5_revA_ctbl24nit, .aid = aid7705, .elvCaps = elvCaps19h, .elv = elv19h, .way = W1, .elvss_offset = elvss_offset_otp},
	{ .br = 25, .refBr = 119, .cGma = gma2p15, .rTbl = ha5_revA_rtbl25nit, .cTbl = ha5_revA_ctbl25nit, .aid = aid7608, .elvCaps = elvCaps19h, .elv = elv19h, .way = W1, .elvss_offset = elvss_offset_otp},
	{ .br = 27, .refBr = 119, .cGma = gma2p15, .rTbl = ha5_revA_rtbl27nit, .cTbl = ha5_revA_ctbl27nit, .aid = aid7426, .elvCaps = elvCaps19h, .elv = elv19h, .way = W1, .elvss_offset = elvss_offset_otp},
	{ .br = 29, .refBr = 119, .cGma = gma2p15, .rTbl = ha5_revA_rtbl29nit, .cTbl = ha5_revA_ctbl29nit, .aid = aid7233, .elvCaps = elvCaps19h, .elv = elv19h, .way = W1, .elvss_offset = elvss_offset_otp},
	{ .br = 30, .refBr = 119, .cGma = gma2p15, .rTbl = ha5_revA_rtbl30nit, .cTbl = ha5_revA_ctbl30nit, .aid = aid7113, .elvCaps = elvCaps19h, .elv = elv19h, .way = W1, .elvss_offset = elvss_offset_otp},
	{ .br = 32, .refBr = 119, .cGma = gma2p15, .rTbl = ha5_revA_rtbl32nit, .cTbl = ha5_revA_ctbl32nit, .aid = aid6900, .elvCaps = elvCaps19h, .elv = elv19h, .way = W1, .elvss_offset = elvss_offset_otp},
	{ .br = 34, .refBr = 119, .cGma = gma2p15, .rTbl = ha5_revA_rtbl34nit, .cTbl = ha5_revA_ctbl34nit, .aid = aid6745, .elvCaps = elvCaps19h, .elv = elv19h, .way = W1, .elvss_offset = elvss_offset_otp},
	{ .br = 37, .refBr = 119, .cGma = gma2p15, .rTbl = ha5_revA_rtbl37nit, .cTbl = ha5_revA_ctbl37nit, .aid = aid6451, .elvCaps = elvCaps19h, .elv = elv19h, .way = W1, .elvss_offset = elvss_offset_otp},
	{ .br = 39, .refBr = 119, .cGma = gma2p15, .rTbl = ha5_revA_rtbl39nit, .cTbl = ha5_revA_ctbl39nit, .aid = aid6250, .elvCaps = elvCaps19h, .elv = elv19h, .way = W1, .elvss_offset = elvss_offset_otp},
	{ .br = 41, .refBr = 119, .cGma = gma2p15, .rTbl = ha5_revA_rtbl41nit, .cTbl = ha5_revA_ctbl41nit, .aid = aid6045, .elvCaps = elvCaps19h, .elv = elv19h, .way = W1, .elvss_offset = elvss_offset_otp},
	{ .br = 44, .refBr = 119, .cGma = gma2p15, .rTbl = ha5_revA_rtbl44nit, .cTbl = ha5_revA_ctbl44nit, .aid = aid5755, .elvCaps = elvCaps19h, .elv = elv19h, .way = W1, .elvss_offset = elvss_offset_otp},
	{ .br = 47, .refBr = 119, .cGma = gma2p15, .rTbl = ha5_revA_rtbl47nit, .cTbl = ha5_revA_ctbl47nit, .aid = aid5441, .elvCaps = elvCaps19h, .elv = elv19h, .way = W1, .elvss_offset = elvss_offset_otp},
	{ .br = 50, .refBr = 119, .cGma = gma2p15, .rTbl = ha5_revA_rtbl50nit, .cTbl = ha5_revA_ctbl50nit, .aid = aid5139, .elvCaps = elvCaps19h, .elv = elv19h, .way = W1, .elvss_offset = elvss_offset_otp},
	{ .br = 53, .refBr = 119, .cGma = gma2p15, .rTbl = ha5_revA_rtbl53nit, .cTbl = ha5_revA_ctbl53nit, .aid = aid4834, .elvCaps = elvCaps19h, .elv = elv19h, .way = W1, .elvss_offset = elvss_offset_otp},
	{ .br = 56, .refBr = 119, .cGma = gma2p15, .rTbl = ha5_revA_rtbl56nit, .cTbl = ha5_revA_ctbl56nit, .aid = aid4512, .elvCaps = elvCaps19h, .elv = elv19h, .way = W1, .elvss_offset = elvss_offset_otp},
	{ .br = 60, .refBr = 119, .cGma = gma2p15, .rTbl = ha5_revA_rtbl60nit, .cTbl = ha5_revA_ctbl60nit, .aid = aid4083, .elvCaps = elvCaps19h, .elv = elv19h, .way = W1, .elvss_offset = elvss_offset_otp},
	{ .br = 64, .refBr = 119, .cGma = gma2p15, .rTbl = ha5_revA_rtbl64nit, .cTbl = ha5_revA_ctbl64nit, .aid = aid3657, .elvCaps = elvCaps19h, .elv = elv19h, .way = W1, .elvss_offset = elvss_offset_otp},
	{ .br = 68, .refBr = 125, .cGma = gma2p15, .rTbl = ha5_revA_rtbl68nit, .cTbl = ha5_revA_ctbl68nit, .aid = aid3618, .elvCaps = elvCaps19h, .elv = elv19h, .way = W1, .elvss_offset = elvss_offset_otp},
	{ .br = 72, .refBr = 131, .cGma = gma2p15, .rTbl = ha5_revA_rtbl72nit, .cTbl = ha5_revA_ctbl72nit, .aid = aid3618, .elvCaps = elvCaps19h, .elv = elv19h, .way = W1, .elvss_offset = elvss_offset_otp},
	{ .br = 77, .refBr = 140, .cGma = gma2p15, .rTbl = ha5_revA_rtbl77nit, .cTbl = ha5_revA_ctbl77nit, .aid = aid3618, .elvCaps = elvCaps19h, .elv = elv19h, .way = W1, .elvss_offset = elvss_offset_otp},
	{ .br = 82, .refBr = 147, .cGma = gma2p15, .rTbl = ha5_revA_rtbl82nit, .cTbl = ha5_revA_ctbl82nit, .aid = aid3618, .elvCaps = elvCaps18h, .elv = elv18h, .way = W1, .elvss_offset = elvss_offset_otp},
	{ .br = 87, .refBr = 157, .cGma = gma2p15, .rTbl = ha5_revA_rtbl87nit, .cTbl = ha5_revA_ctbl87nit, .aid = aid3618, .elvCaps = elvCaps18h, .elv = elv18h, .way = W1, .elvss_offset = elvss_offset_otp},
	{ .br = 93, .refBr = 166, .cGma = gma2p15, .rTbl = ha5_revA_rtbl93nit, .cTbl = ha5_revA_ctbl93nit, .aid = aid3618, .elvCaps = elvCaps17h, .elv = elv17h, .way = W1, .elvss_offset = elvss_offset_otp},
	{ .br = 98, .refBr = 174, .cGma = gma2p15, .rTbl = ha5_revA_rtbl98nit, .cTbl = ha5_revA_ctbl98nit, .aid = aid3618, .elvCaps = elvCaps17h, .elv = elv17h, .way = W1, .elvss_offset = elvss_offset_otp},
	{ .br = 105, .refBr = 186, .cGma = gma2p15, .rTbl = ha5_revA_rtbl105nit, .cTbl = ha5_revA_ctbl105nit, .aid = aid3618, .elvCaps = elvCaps17h, .elv = elv17h, .way = W1, .elvss_offset = elvss_offset_otp},
	{ .br = 111, .refBr = 195, .cGma = gma2p15, .rTbl = ha5_revA_rtbl111nit, .cTbl = ha5_revA_ctbl111nit, .aid = aid3618, .elvCaps = elvCaps16h, .elv = elv16h, .way = W1, .elvss_offset = elvss_offset_otp},
	{ .br = 119, .refBr = 207, .cGma = gma2p15, .rTbl = ha5_revA_rtbl119nit, .cTbl = ha5_revA_ctbl119nit, .aid = aid3618, .elvCaps = elvCaps16h, .elv = elv16h, .way = W1, .elvss_offset = elvss_offset_otp},
	{ .br = 126, .refBr = 217, .cGma = gma2p15, .rTbl = ha5_revA_rtbl126nit, .cTbl = ha5_revA_ctbl126nit, .aid = aid3618, .elvCaps = elvCaps15h, .elv = elv15h, .way = W1, .elvss_offset = elvss_offset_otp},
	{ .br = 134, .refBr = 228, .cGma = gma2p15, .rTbl = ha5_revA_rtbl134nit, .cTbl = ha5_revA_ctbl134nit, .aid = aid3618, .elvCaps = elvCaps15h, .elv = elv15h, .way = W1, .elvss_offset = elvss_offset_otp},
	{ .br = 143, .refBr = 241, .cGma = gma2p15, .rTbl = ha5_revA_rtbl143nit, .cTbl = ha5_revA_ctbl143nit, .aid = aid3618, .elvCaps = elvCaps14h, .elv = elv14h, .way = W1, .elvss_offset = elvss_offset_otp},
	{ .br = 152, .refBr = 252, .cGma = gma2p15, .rTbl = ha5_revA_rtbl152nit, .cTbl = ha5_revA_ctbl152nit, .aid = aid3618, .elvCaps = elvCaps13h, .elv = elv13h, .way = W1, .elvss_offset = elvss_offset_otp},
	{ .br = 162, .refBr = 268, .cGma = gma2p15, .rTbl = ha5_revA_rtbl162nit, .cTbl = ha5_revA_ctbl162nit, .aid = aid3618, .elvCaps = elvCaps13h, .elv = elv13h, .way = W1, .elvss_offset = elvss_offset_otp},
	{ .br = 172, .refBr = 280, .cGma = gma2p15, .rTbl = ha5_revA_rtbl172nit, .cTbl = ha5_revA_ctbl172nit, .aid = aid3618, .elvCaps = elvCaps13h, .elv = elv13h, .way = W1, .elvss_offset = elvss_offset_otp},
	{ .br = 183, .refBr = 292, .cGma = gma2p15, .rTbl = ha5_revA_rtbl183nit, .cTbl = ha5_revA_ctbl183nit, .aid = aid3607, .elvCaps = elvCaps12h, .elv = elv12h, .way = W1, .elvss_offset = elvss_offset_otp},
	{ .br = 195, .refBr = 292, .cGma = gma2p15, .rTbl = ha5_revA_rtbl195nit, .cTbl = ha5_revA_ctbl195nit, .aid = aid3092, .elvCaps = elvCaps12h, .elv = elv12h, .way = W1, .elvss_offset = elvss_offset_otp},
	{ .br = 207, .refBr = 292, .cGma = gma2p15, .rTbl = ha5_revA_rtbl207nit, .cTbl = ha5_revA_ctbl207nit, .aid = aid2543, .elvCaps = elvCaps12h, .elv = elv12h, .way = W1, .elvss_offset = elvss_offset_otp},
	{ .br = 220, .refBr = 292, .cGma = gma2p15, .rTbl = ha5_revA_rtbl220nit, .cTbl = ha5_revA_ctbl220nit, .aid = aid1981, .elvCaps = elvCaps12h, .elv = elv12h, .way = W1, .elvss_offset = elvss_offset_otp},
	{ .br = 234, .refBr = 292, .cGma = gma2p15, .rTbl = ha5_revA_rtbl234nit, .cTbl = ha5_revA_ctbl234nit, .aid = aid1300, .elvCaps = elvCaps11h, .elv = elv11h, .way = W1, .elvss_offset = elvss_offset_otp},
	{ .br = 249, .refBr = 300, .cGma = gma2p15, .rTbl = ha5_revA_rtbl249nit, .cTbl = ha5_revA_ctbl249nit, .aid = aid1002, .elvCaps = elvCaps12h, .elv = elv12h, .way = W1, .elvss_offset = elvss_offset_otp},
	{ .br = 265, .refBr = 316, .cGma = gma2p15, .rTbl = ha5_revA_rtbl265nit, .cTbl = ha5_revA_ctbl265nit, .aid = aid1002, .elvCaps = elvCaps11h, .elv = elv11h, .way = W1, .elvss_offset = elvss_offset_otp},
	{ .br = 282, .refBr = 328, .cGma = gma2p15, .rTbl = ha5_revA_rtbl282nit, .cTbl = ha5_revA_ctbl282nit, .aid = aid1002, .elvCaps = elvCaps10h, .elv = elv10h, .way = W1, .elvss_offset = elvss_offset_otp},
	{ .br = 300, .refBr = 344, .cGma = gma2p15, .rTbl = ha5_revA_rtbl300nit, .cTbl = ha5_revA_ctbl300nit, .aid = aid1002, .elvCaps = elvCaps0Fh, .elv = elv0Fh, .way = W1, .elvss_offset = elvss_offset_otp},
	{ .br = 316, .refBr = 358, .cGma = gma2p15, .rTbl = ha5_revA_rtbl316nit, .cTbl = ha5_revA_ctbl316nit, .aid = aid1002, .elvCaps = elvCaps0Eh, .elv = elv0Eh, .way = W1, .elvss_offset = elvss_offset_otp},
	{ .br = 333, .refBr = 374, .cGma = gma2p15, .rTbl = ha5_revA_rtbl333nit, .cTbl = ha5_revA_ctbl333nit, .aid = aid1002, .elvCaps = elvCaps0Dh, .elv = elv0Dh, .way = W1, .elvss_offset = elvss_offset_otp},
	{ .br = 350, .refBr = 392, .cGma = gma2p15, .rTbl = ha5_revA_rtbl350nit, .cTbl = ha5_revA_ctbl350nit, .aid = aid1002, .elvCaps = elvCaps0Dh, .elv = elv0Dh, .way = W1, .elvss_offset = elvss_offset_otp},
	{ .br = 357, .refBr = 395, .cGma = gma2p15, .rTbl = ha5_revA_rtbl357nit, .cTbl = ha5_revA_ctbl357nit, .aid = aid1002, .elvCaps = elvCaps0Ch, .elv = elv0Ch, .way = W1, .elvss_offset = elvss_offset_otp},
	{ .br = 365, .refBr = 402, .cGma = gma2p15, .rTbl = ha5_revA_rtbl365nit, .cTbl = ha5_revA_ctbl365nit, .aid = aid1002, .elvCaps = elvCaps0Ch, .elv = elv0Ch, .way = W1, .elvss_offset = elvss_offset_otp},
	{ .br = 372, .refBr = 402, .cGma = gma2p15, .rTbl = ha5_revA_rtbl372nit, .cTbl = ha5_revA_ctbl372nit, .aid = aid755, .elvCaps = elvCaps0Ch, .elv = elv0Ch, .way = W1, .elvss_offset = elvss_offset_otp},
	{ .br = 380, .refBr = 402, .cGma = gma2p15, .rTbl = ha5_revA_rtbl380nit, .cTbl = ha5_revA_ctbl380nit, .aid = aid534, .elvCaps = elvCaps0Ch, .elv = elv0Ch, .way = W1, .elvss_offset = elvss_offset_otp},
	{ .br = 387, .refBr = 402, .cGma = gma2p15, .rTbl = ha5_revA_rtbl387nit, .cTbl = ha5_revA_ctbl387nit, .aid = aid337, .elvCaps = elvCaps0Bh, .elv = elv0Bh, .way = W1, .elvss_offset = elvss_offset_otp},
	{ .br = 395, .refBr = 402, .cGma = gma2p15, .rTbl = ha5_revA_rtbl395nit, .cTbl = ha5_revA_ctbl395nit, .aid = aid62, .elvCaps = elvCaps0Bh, .elv = elv0Bh, .way = W1, .elvss_offset = elvss_offset_otp},
	{ .br = 403, .refBr = 409, .cGma = gma2p15, .rTbl = ha5_revA_rtbl403nit, .cTbl = ha5_revA_ctbl403nit, .aid = aid46, .elvCaps = elvCaps0Bh, .elv = elv0Bh, .way = W1, .elvss_offset = elvss_offset_otp},
	{ .br = 412, .refBr = 416, .cGma = gma2p15, .rTbl = ha5_revA_rtbl412nit, .cTbl = ha5_revA_ctbl412nit, .aid = aid46, .elvCaps = elvCaps0Ah, .elv = elv0Ah, .way = W1, .elvss_offset = elvss_offset_otp},
	{ .br = 420, .refBr = 420, .cGma = gma2p20, .rTbl = ha5_revA_rtbl420nit, .cTbl = ha5_revA_ctbl420nit, .aid = aid46, .elvCaps = elvCaps0Ah, .elv = elv0Ah, .way = W2, .elvss_offset = elvss_offset_otp},
	/*hbm interpolation */
	{ .br = 443, .refBr = 443, .cGma = gma2p20, .rTbl = ha5_revA_rtbl420nit, .cTbl = ha5_revA_ctbl420nit, .aid = aid46, .elvCaps = elvCaps13h, .elv = elv13h, .way = W3, .elvss_offset = elvss_offset_para},
	{ .br = 465, .refBr = 465, .cGma = gma2p20, .rTbl = ha5_revA_rtbl420nit, .cTbl = ha5_revA_ctbl420nit, .aid = aid46, .elvCaps = elvCaps11h, .elv = elv11h, .way = W3, .elvss_offset = elvss_offset_para},
	{ .br = 488, .refBr = 488, .cGma = gma2p20, .rTbl = ha5_revA_rtbl420nit, .cTbl = ha5_revA_ctbl420nit, .aid = aid46, .elvCaps = elvCaps10h, .elv = elv10h, .way = W3, .elvss_offset = elvss_offset_para},
	{ .br = 510, .refBr = 510, .cGma = gma2p20, .rTbl = ha5_revA_rtbl420nit, .cTbl = ha5_revA_ctbl420nit, .aid = aid46, .elvCaps = elvCaps0Fh, .elv = elv0Fh, .way = W3, .elvss_offset = elvss_offset_para},
	{ .br = 533, .refBr = 533, .cGma = gma2p20, .rTbl = ha5_revA_rtbl420nit, .cTbl = ha5_revA_ctbl420nit, .aid = aid46, .elvCaps = elvCaps0Eh, .elv = elv0Eh, .way = W3, .elvss_offset = elvss_offset_para},
	{ .br = 555, .refBr = 555, .cGma = gma2p20, .rTbl = ha5_revA_rtbl420nit, .cTbl = ha5_revA_ctbl420nit, .aid = aid46, .elvCaps = elvCaps0Ch, .elv = elv0Ch, .way = W3, .elvss_offset = elvss_offset_para},
	{ .br = 578, .refBr = 578, .cGma = gma2p20, .rTbl = ha5_revA_rtbl420nit, .cTbl = ha5_revA_ctbl420nit, .aid = aid46, .elvCaps = elvCaps0Bh, .elv = elv0Bh, .way = W3, .elvss_offset = elvss_offset_para},
	/* hbm */
	{ .br = 600, .refBr = 600, .cGma = gma2p20, .rTbl = ha5_revA_rtbl420nit, .cTbl = ha5_revA_ctbl420nit, .aid = aid46, .elvCaps = elvCaps0Ah, .elv = elv0Ah, .way = W4, .elvss_offset = elvss_offset_para},
};

static int s6e3ha5_set_gamma_to_hbm(struct SmtDimInfo *brInfo, struct dim_data *dimData, u8 * hbm)
{
	int ret = 0;
	unsigned int index = 0;
	unsigned char *result = brInfo->gamma;
	int i = 0;
	memset(result, 0, OLED_CMD_GAMMA_CNT);

	result[index++] = OLED_CMD_GAMMA;
	result[index++] = (hbm[2] & 0x04) >> 2;	// reg 3rd of op-manul is same as reg[2]
	result[index++] = hbm[3];
	result[index++] = (hbm[2] & 0x02) >> 1;
	result[index++] = hbm[4];
	result[index++] = (hbm[2] & 0x01);
	result[index++] = hbm[5];

	for (i = 6; i < 35; i++) {
		result[index++] = hbm[i];
	}

	for (i = 0; i < OLED_CMD_GAMMA_CNT; i++)
		dsim_info("%d : %d\n", i + 1, result[i]);
	return ret;
}


static int ha5_init_dimming(struct dsim_device *dsim, u8 * mtp, u8 * hbm)
{
	int i, j;
	int pos = 0;
	int ret = 0;
	short temp;
	int method = 0;
	static struct dim_data *dimming = NULL;

	struct panel_private *panel = &dsim->priv;
	struct SmtDimInfo *diminfo = NULL;
	int string_offset;
	char string_buf[1024];

	if (dimming == NULL) {
		dimming = (struct dim_data *)kmalloc(sizeof(struct dim_data), GFP_KERNEL);
		if (!dimming) {
			dsim_err("failed to allocate memory for dim data\n");
			ret = -ENOMEM;
			goto error;
		}
	}
	memset(panel->irc_table, 0x00, sizeof(panel->irc_table));

	panel->br_tbl = (unsigned int *)br_tbl_grace_420;
	dsim_info("%s init dimming info for HA5 panel\n", __func__);
	diminfo = (void *)dimming_info_HA5_RevA;
	panel->inter_aor_tbl = inter_aor_tbl_ha5;
	memcpy(panel->irc_table, irc_table_ha5_revA, sizeof(irc_table_ha5_revA));

	panel->dim_data = (void *)dimming;
	panel->dim_info = (void *)diminfo;

	for (j = 0; j < CI_MAX; j++) {
		temp = ((mtp[pos] & 0x01) ? -1 : 1) * mtp[pos + 1];
		dimming->t_gamma[V255][j] = (int)center_gamma[V255][j] + temp;
		dimming->mtp[V255][j] = temp;
		pos += 2;
	}

	for (i = V203; i >= 0; i--) {
		for (j = 0; j < CI_MAX; j++) {
			temp = ((mtp[pos] & 0x80) ? -1 : 1) * (mtp[pos] & 0x7f);
			dimming->t_gamma[i][j] = (int)center_gamma[i][j] + temp;
			dimming->mtp[i][j] = temp;
			pos++;
		}
	}
	/* for vt */
	temp = (mtp[pos + 1]) << 8 | mtp[pos];

	for (i = 0; i < CI_MAX; i++)
		dimming->vt_mtp[i] = (temp >> (i * 4)) & 0x0f;
#ifdef SMART_DIMMING_DEBUG
	dimm_info("Center Gamma Info : \n");
	for (i = 0; i < VMAX; i++) {
		dsim_info("Gamma : %3d %3d %3d : %3x %3x %3x\n",
			  dimming->t_gamma[i][CI_RED], dimming->t_gamma[i][CI_GREEN], dimming->t_gamma[i][CI_BLUE],
			  dimming->t_gamma[i][CI_RED], dimming->t_gamma[i][CI_GREEN], dimming->t_gamma[i][CI_BLUE]);
	}
#endif
	dimm_info("VT MTP : \n");
	dimm_info("Gamma : %3d %3d %3d : %3x %3x %3x\n",
		  dimming->vt_mtp[CI_RED], dimming->vt_mtp[CI_GREEN], dimming->vt_mtp[CI_BLUE],
		  dimming->vt_mtp[CI_RED], dimming->vt_mtp[CI_GREEN], dimming->vt_mtp[CI_BLUE]);

	dimm_info("MTP Info : \n");
	for (i = 0; i < VMAX; i++) {
		dimm_info("Gamma : %3d %3d %3d : %3x %3x %3x\n",
			  dimming->mtp[i][CI_RED], dimming->mtp[i][CI_GREEN], dimming->mtp[i][CI_BLUE],
			  dimming->mtp[i][CI_RED], dimming->mtp[i][CI_GREEN], dimming->mtp[i][CI_BLUE]);
	}

	ret = generate_volt_table(dimming);
	if (ret) {
		dimm_err("[ERR:%s] failed to generate volt table\n", __func__);
		goto error;
	}

	for (i = 0; i < MAX_BR_INFO; i++) {
		method = diminfo[i].way;

		if (method == DIMMING_METHOD_FILL_CENTER) {
			ret = set_gamma_to_center(&diminfo[i]);
			if (ret) {
				dsim_err("%s : failed to get center gamma\n", __func__);
				goto error;
			}
		}
		else if (method == DIMMING_METHOD_FILL_HBM) {
			ret = s6e3ha5_set_gamma_to_hbm(&diminfo[i], dimming, hbm);
			if (ret) {
				dsim_err("%s : failed to get hbm gamma\n", __func__);
				goto error;
			}
		}
	}

	for (i = 0; i < MAX_BR_INFO; i++) {
		method = diminfo[i].way;
		if (method == DIMMING_METHOD_AID) {
			ret = cal_gamma_from_index(dimming, &diminfo[i]);
			if (ret) {
				dsim_err("%s : failed to calculate gamma : index : %d\n", __func__, i);
				goto error;
			}
		}
		if (method == DIMMING_METHOD_INTERPOLATION) {
			ret = interpolation_gamma_to_hbm(diminfo, i);
			if (ret) {
				dsim_err("%s : failed to calculate gamma : index : %d\n", __func__, i);
				goto error;
			}
		}
	}

	for (i = 0; i < MAX_BR_INFO; i++) {
		memset(string_buf, 0, sizeof(string_buf));
		string_offset = sprintf(string_buf, "gamma[%3d] : ", diminfo[i].br);

		for (j = 0; j < GAMMA_CMD_CNT; j++)
			string_offset += sprintf(string_buf + string_offset, "%02x ", diminfo[i].gamma[j]);

		dsim_info("%s\n", string_buf);
	}

error:
	return ret;

}

#ifdef CONFIG_LCD_HMT
struct SmtDimInfo hmt_dimming_info_HA5[HMT_MAX_BR_INFO] = {
	{.br = 10, .refBr = 48, .cGma = gma2p15, .rTbl = ha5_revA_HMTrtbl10nit, .cTbl = ha5_revA_HMTctbl10nit, .aid = ha5_revA_HMTaid7999, .elvCaps = ha5_HMTelvCaps, .elv = ha5_HMTelv },
	{.br = 11, .refBr = 53, .cGma = gma2p15, .rTbl = ha5_revA_HMTrtbl11nit, .cTbl = ha5_revA_HMTctbl11nit, .aid = ha5_revA_HMTaid7999, .elvCaps = ha5_HMTelvCaps, .elv = ha5_HMTelv },
	{.br = 12, .refBr = 59, .cGma = gma2p15, .rTbl = ha5_revA_HMTrtbl12nit, .cTbl = ha5_revA_HMTctbl12nit, .aid = ha5_revA_HMTaid7999, .elvCaps = ha5_HMTelvCaps, .elv = ha5_HMTelv },
	{.br = 13, .refBr = 64, .cGma = gma2p15, .rTbl = ha5_revA_HMTrtbl13nit, .cTbl = ha5_revA_HMTctbl13nit, .aid = ha5_revA_HMTaid7999, .elvCaps = ha5_HMTelvCaps, .elv = ha5_HMTelv },
	{.br = 14, .refBr = 69, .cGma = gma2p15, .rTbl = ha5_revA_HMTrtbl14nit, .cTbl = ha5_revA_HMTctbl14nit, .aid = ha5_revA_HMTaid7999, .elvCaps = ha5_HMTelvCaps, .elv = ha5_HMTelv },
	{.br = 15, .refBr = 73, .cGma = gma2p15, .rTbl = ha5_revA_HMTrtbl15nit, .cTbl = ha5_revA_HMTctbl15nit, .aid = ha5_revA_HMTaid7999, .elvCaps = ha5_HMTelvCaps, .elv = ha5_HMTelv },
	{.br = 16, .refBr = 78, .cGma = gma2p15, .rTbl = ha5_revA_HMTrtbl16nit, .cTbl = ha5_revA_HMTctbl16nit, .aid = ha5_revA_HMTaid7999, .elvCaps = ha5_HMTelvCaps, .elv = ha5_HMTelv },
	{.br = 17, .refBr = 83, .cGma = gma2p15, .rTbl = ha5_revA_HMTrtbl17nit, .cTbl = ha5_revA_HMTctbl17nit, .aid = ha5_revA_HMTaid7999, .elvCaps = ha5_HMTelvCaps, .elv = ha5_HMTelv },
	{.br = 19, .refBr = 92, .cGma = gma2p15, .rTbl = ha5_revA_HMTrtbl19nit, .cTbl = ha5_revA_HMTctbl19nit, .aid = ha5_revA_HMTaid7999, .elvCaps = ha5_HMTelvCaps, .elv = ha5_HMTelv },
	{.br = 20, .refBr = 95, .cGma = gma2p15, .rTbl = ha5_revA_HMTrtbl20nit, .cTbl = ha5_revA_HMTctbl20nit, .aid = ha5_revA_HMTaid7999, .elvCaps = ha5_HMTelvCaps, .elv = ha5_HMTelv },
	{.br = 21, .refBr = 100, .cGma = gma2p15, .rTbl = ha5_revA_HMTrtbl21nit, .cTbl = ha5_revA_HMTctbl21nit, .aid = ha5_revA_HMTaid7999, .elvCaps = ha5_HMTelvCaps, .elv = ha5_HMTelv },
	{.br = 22, .refBr = 104, .cGma = gma2p15, .rTbl = ha5_revA_HMTrtbl22nit, .cTbl = ha5_revA_HMTctbl22nit, .aid = ha5_revA_HMTaid7999, .elvCaps = ha5_HMTelvCaps, .elv = ha5_HMTelv },
	{.br = 23, .refBr = 108, .cGma = gma2p15, .rTbl = ha5_revA_HMTrtbl23nit, .cTbl = ha5_revA_HMTctbl23nit, .aid = ha5_revA_HMTaid7999, .elvCaps = ha5_HMTelvCaps, .elv = ha5_HMTelv },
	{.br = 25, .refBr = 117, .cGma = gma2p15, .rTbl = ha5_revA_HMTrtbl25nit, .cTbl = ha5_revA_HMTctbl25nit, .aid = ha5_revA_HMTaid7999, .elvCaps = ha5_HMTelvCaps, .elv = ha5_HMTelv },
	{.br = 27, .refBr = 125, .cGma = gma2p15, .rTbl = ha5_revA_HMTrtbl27nit, .cTbl = ha5_revA_HMTctbl27nit, .aid = ha5_revA_HMTaid7999, .elvCaps = ha5_HMTelvCaps, .elv = ha5_HMTelv },
	{.br = 29, .refBr = 132, .cGma = gma2p15, .rTbl = ha5_revA_HMTrtbl29nit, .cTbl = ha5_revA_HMTctbl29nit, .aid = ha5_revA_HMTaid7999, .elvCaps = ha5_HMTelvCaps, .elv = ha5_HMTelv },
	{.br = 31, .refBr = 143, .cGma = gma2p15, .rTbl = ha5_revA_HMTrtbl31nit, .cTbl = ha5_revA_HMTctbl31nit, .aid = ha5_revA_HMTaid7999, .elvCaps = ha5_HMTelvCaps, .elv = ha5_HMTelv },
	{.br = 33, .refBr = 151, .cGma = gma2p15, .rTbl = ha5_revA_HMTrtbl33nit, .cTbl = ha5_revA_HMTctbl33nit, .aid = ha5_revA_HMTaid7999, .elvCaps = ha5_HMTelvCaps, .elv = ha5_HMTelv },
	{.br = 35, .refBr = 160, .cGma = gma2p15, .rTbl = ha5_revA_HMTrtbl35nit, .cTbl = ha5_revA_HMTctbl35nit, .aid = ha5_revA_HMTaid7999, .elvCaps = ha5_HMTelvCaps, .elv = ha5_HMTelv },
	{.br = 37, .refBr = 167, .cGma = gma2p15, .rTbl = ha5_revA_HMTrtbl37nit, .cTbl = ha5_revA_HMTctbl37nit, .aid = ha5_revA_HMTaid7999, .elvCaps = ha5_HMTelvCaps, .elv = ha5_HMTelv },
	{.br = 39, .refBr = 177, .cGma = gma2p15, .rTbl = ha5_revA_HMTrtbl39nit, .cTbl = ha5_revA_HMTctbl39nit, .aid = ha5_revA_HMTaid7999, .elvCaps = ha5_HMTelvCaps, .elv = ha5_HMTelv },
	{.br = 41, .refBr = 183, .cGma = gma2p15, .rTbl = ha5_revA_HMTrtbl41nit, .cTbl = ha5_revA_HMTctbl41nit, .aid = ha5_revA_HMTaid7999, .elvCaps = ha5_HMTelvCaps, .elv = ha5_HMTelv },
	{.br = 44, .refBr = 195, .cGma = gma2p15, .rTbl = ha5_revA_HMTrtbl44nit, .cTbl = ha5_revA_HMTctbl44nit, .aid = ha5_revA_HMTaid7999, .elvCaps = ha5_HMTelvCaps, .elv = ha5_HMTelv },
	{.br = 47, .refBr = 208, .cGma = gma2p15, .rTbl = ha5_revA_HMTrtbl47nit, .cTbl = ha5_revA_HMTctbl47nit, .aid = ha5_revA_HMTaid7999, .elvCaps = ha5_HMTelvCaps, .elv = ha5_HMTelv },
	{.br = 50, .refBr = 220, .cGma = gma2p15, .rTbl = ha5_revA_HMTrtbl50nit, .cTbl = ha5_revA_HMTctbl50nit, .aid = ha5_revA_HMTaid7999, .elvCaps = ha5_HMTelvCaps, .elv = ha5_HMTelv },
	{.br = 53, .refBr = 229, .cGma = gma2p15, .rTbl = ha5_revA_HMTrtbl53nit, .cTbl = ha5_revA_HMTctbl53nit, .aid = ha5_revA_HMTaid7999, .elvCaps = ha5_HMTelvCaps, .elv = ha5_HMTelv },
	{.br = 56, .refBr = 244, .cGma = gma2p15, .rTbl = ha5_revA_HMTrtbl56nit, .cTbl = ha5_revA_HMTctbl56nit, .aid = ha5_revA_HMTaid7999, .elvCaps = ha5_HMTelvCaps, .elv = ha5_HMTelv },
	{.br = 60, .refBr = 256, .cGma = gma2p15, .rTbl = ha5_revA_HMTrtbl60nit, .cTbl = ha5_revA_HMTctbl60nit, .aid = ha5_revA_HMTaid7999, .elvCaps = ha5_HMTelvCaps, .elv = ha5_HMTelv },
	{.br = 64, .refBr = 267, .cGma = gma2p15, .rTbl = ha5_revA_HMTrtbl64nit, .cTbl = ha5_revA_HMTctbl64nit, .aid = ha5_revA_HMTaid7999, .elvCaps = ha5_HMTelvCaps, .elv = ha5_HMTelv },
	{.br = 68, .refBr = 281, .cGma = gma2p15, .rTbl = ha5_revA_HMTrtbl68nit, .cTbl = ha5_revA_HMTctbl68nit, .aid = ha5_revA_HMTaid7999, .elvCaps = ha5_HMTelvCaps, .elv = ha5_HMTelv },
	{.br = 72, .refBr = 291, .cGma = gma2p15, .rTbl = ha5_revA_HMTrtbl72nit, .cTbl = ha5_revA_HMTctbl72nit, .aid = ha5_revA_HMTaid7999, .elvCaps = ha5_HMTelvCaps, .elv = ha5_HMTelv },
	{.br = 77, .refBr = 229, .cGma = gma2p15, .rTbl = ha5_revA_HMTrtbl77nit, .cTbl = ha5_revA_HMTctbl77nit, .aid = ha5_revA_HMTaid7001, .elvCaps = ha5_HMTelvCaps, .elv = ha5_HMTelv },
	{.br = 82, .refBr = 242, .cGma = gma2p15, .rTbl = ha5_revA_HMTrtbl82nit, .cTbl = ha5_revA_HMTctbl82nit, .aid = ha5_revA_HMTaid7001, .elvCaps = ha5_HMTelvCaps, .elv = ha5_HMTelv },
	{.br = 87, .refBr = 255, .cGma = gma2p15, .rTbl = ha5_revA_HMTrtbl87nit, .cTbl = ha5_revA_HMTctbl87nit, .aid = ha5_revA_HMTaid7001, .elvCaps = ha5_HMTelvCaps, .elv = ha5_HMTelv },
	{.br = 93, .refBr = 268, .cGma = gma2p15, .rTbl = ha5_revA_HMTrtbl93nit, .cTbl = ha5_revA_HMTctbl93nit, .aid = ha5_revA_HMTaid7001, .elvCaps = ha5_HMTelvCaps, .elv = ha5_HMTelv },
	{.br = 99, .refBr = 281, .cGma = gma2p15, .rTbl = ha5_revA_HMTrtbl99nit, .cTbl = ha5_revA_HMTctbl99nit, .aid = ha5_revA_HMTaid7001, .elvCaps = ha5_HMTelvCaps, .elv = ha5_HMTelv },
	{.br = 105, .refBr = 295, .cGma = gma2p15, .rTbl = ha5_revA_HMTrtbl105nit, .cTbl = ha5_revA_HMTctbl105nit, .aid = ha5_revA_HMTaid7001, .elvCaps = ha5_HMTelvCaps, .elv = ha5_HMTelv },
};

static int ha5_hmt_init_dimming(struct dsim_device *dsim, u8 * mtp)
{
	int i, j;
	int pos = 0;
	int ret = 0;
	short temp;
	static struct dim_data *dimming = NULL;
	struct panel_private *panel = &dsim->priv;
	struct SmtDimInfo *diminfo = NULL;

	if (dimming == NULL) {
		dimming = (struct dim_data *)kmalloc(sizeof(struct dim_data), GFP_KERNEL);
		if (!dimming) {
			dsim_err("failed to allocate memory for dim data\n");
			ret = -ENOMEM;
			goto error;
		}
	}
	switch (dynamic_lcd_type) {
	case LCD_TYPE_S6E3HA5_WQHD:
		dsim_info("%s init HMT dimming info for HA5 init panel\n", __func__);
		diminfo = (void *)hmt_dimming_info_HA5;
		break;
	default:
		dsim_info("%s init HMT dimming info for (UNKNOWN) HA5 panel\n", __func__);
		diminfo = (void *)hmt_dimming_info_HA5;
		break;
	}

	panel->hmt_dim_data = (void *)dimming;
	panel->hmt_dim_info = (void *)diminfo;
	panel->hmt_br_tbl = (unsigned int *)hmt_br_tbl;
	panel->hmt_on = panel->hmt_prev_status = 0;
	for (j = 0; j < CI_MAX; j++) {
		temp = ((mtp[pos] & 0x01) ? -1 : 1) * mtp[pos + 1];
		dimming->t_gamma[V255][j] = (int)center_gamma[V255][j] + temp;
		dimming->mtp[V255][j] = temp;
		pos += 2;
	}
	for (i = V203; i >= 0; i--) {
		for (j = 0; j < CI_MAX; j++) {
			temp = ((mtp[pos] & 0x80) ? -1 : 1) * (mtp[pos] & 0x7f);
			dimming->t_gamma[i][j] = (int)center_gamma[i][j] + temp;
			dimming->mtp[i][j] = temp;
			pos++;
		}
	}
	/* for vt */
	temp = (mtp[pos + 1]) << 8 | mtp[pos];
	for (i = 0; i < CI_MAX; i++)
		dimming->vt_mtp[i] = (temp >> (i * 4)) & 0x0f;

	ret = generate_volt_table(dimming);
	if (ret) {
		dimm_err("[ERR:%s] failed to generate volt table\n", __func__);
		goto error;
	}

	for (i = 0; i < HMT_MAX_BR_INFO; i++) {
		ret = cal_gamma_from_index(dimming, &diminfo[i]);
		if (ret) {
			dsim_err("failed to calculate gamma : index : %d\n", i);
			goto error;
		}
	}
error:
	return ret;

}

#endif

static void s6e3ha5_init_default_info(struct dsim_device *dsim)
{
	struct panel_private *panel = &dsim->priv;
	panel->elvss_len = S6E3HA5_ELVSS_LEN + 1;
	panel->elvss_start_offset = S6E3HA5_ELVSS_START;
	panel->elvss_temperature_offset = S6E3HA5_ELVSS_TEMPERATURE_POS;
	panel->elvss_tset_offset = S6E3HA5_ELVSS_TSET_POS;

	memset(panel->tset_set, 0, sizeof(panel->tset_set));
	panel->tset_len = 0;	// elvss set include tset

	memcpy(panel->aid_set, S6E3HA5_SEQ_AOR_CONTROL, S6E3HA5_AID_CMD_CNT);
	panel->aid_len = S6E3HA5_AID_LEN + 1;
	panel->aid_reg_offset = S6E3HA5_AID_LEN - 1;

	memcpy(panel->vint_set, S6E3HA5_SEQ_VINT_SET, sizeof(S6E3HA5_SEQ_VINT_SET));
	panel->vint_len = S6E3HA5_VINT_LEN + 1;

	memcpy(panel->vint_table, VINT_TABLE_HA5, sizeof(VINT_TABLE_HA5));
	memcpy(panel->vint_dim_table, VINT_DIM_TABLE, sizeof(VINT_DIM_TABLE));
	panel->vint_table_len = ARRAY_SIZE(VINT_DIM_TABLE);

	panel->acl_opr_tbl = (unsigned char **)ACL_OPR_TABLE_HA5;
	panel->acl_cutoff_tbl = (unsigned char **)ACL_CUTOFF_TABLE;

	pr_info("%s : init default value\n", __func__);
}

static int s6e3ha5_read_init_info(struct dsim_device *dsim, unsigned char *mtp, unsigned char *hbm)
{
	int i = 0;
	int ret;
	struct panel_private *panel = &dsim->priv;
	unsigned char bufForCoordi[S6E3HA5_COORDINATE_LEN] = { 0, };
	unsigned char buf[S6E3HA5_MTP_DATE_SIZE] = { 0, };
	unsigned char hbm_gamma[S6E3HA5_HBMGAMMA_READ_END + 4] = { 0, };

	dsim_info("MDD : %s was called\n", __func__);
	ret = dsim_write_hl_data(dsim, SEQ_TEST_KEY_ON_F0, ARRAY_SIZE(SEQ_TEST_KEY_ON_F0));
	if (ret < 0) {
		dsim_err("%s : fail to write CMD : SEQ_TEST_KEY_ON_F0\n", __func__);
	}

	// id
	ret = dsim_read_hl_data(dsim, S6E3HA5_ID_REG, S6E3HA5_ID_LEN, panel->id);
	if (ret != S6E3HA5_ID_LEN) {
		dsim_err("%s : can't find connected panel. check panel connection\n", __func__);
		panel->lcdConnected = PANEL_DISCONNEDTED;
		goto read_exit;
	}

	dsim_info("READ ID : ");
	for (i = 0; i < S6E3HA5_ID_LEN; i++)
		dsim_info("%02x, ", panel->id[i]);
	dsim_info("\n");

	dsim->priv.current_model = (dsim->priv.id[1] >> 4) & 0x03;
	dsim->priv.panel_rev = dsim->priv.id[2] & 0x0F;
	dsim->priv.panel_line = (dsim->priv.id[0] >> 6) & 0x03;
	dsim->priv.panel_material = dsim->priv.id[1] & 0x01;

	dsim_info("%s model is %d, panel rev : %d, panel line : %d panel material : %d\n",
		__func__, dsim->priv.current_model, dsim->priv.panel_rev, dsim->priv.panel_line, dsim->priv.panel_material);

	// mtp
	ret = dsim_read_hl_data(dsim, S6E3HA5_MTP_ADDR, S6E3HA5_MTP_DATE_SIZE, buf);
	if (ret != S6E3HA5_MTP_DATE_SIZE) {
		dsim_err("failed to read mtp, check panel connection\n");
		goto read_fail;
	}
	VT_RGB2GRB(buf + S6E3HA5_MTP_VT_ADDR);
	memcpy(mtp, buf, S6E3HA5_MTP_SIZE);
	memcpy(panel->date, &buf[40], ARRAY_SIZE(panel->date));
	dsim_info("READ MTP SIZE : %d\n", S6E3HA5_MTP_SIZE);
	dsim_info("=========== MTP INFO =========== \n");
	for (i = 0; i < S6E3HA5_MTP_SIZE; i++)
		dsim_info("MTP[%2d] : %2d : %2x\n", i, mtp[i], mtp[i]);

	// coordinate
	ret = dsim_read_hl_data(dsim, S6E3HA5_COORDINATE_REG, S6E3HA5_COORDINATE_LEN, bufForCoordi);
	if (ret != S6E3HA5_COORDINATE_LEN) {
		dsim_err("fail to read coordinate on command.\n");
		goto read_fail;
	}
	panel->coordinate[0] = bufForCoordi[0] << 8 | bufForCoordi[1];	/* X */
	panel->coordinate[1] = bufForCoordi[2] << 8 | bufForCoordi[3];	/* Y */
	dsim_info("READ coordi : ");
	for (i = 0; i < 2; i++)
		dsim_info("%d, ", dsim->priv.coordinate[i]);
	dsim_info("\n");

	// code
	ret = dsim_read_hl_data(dsim, S6E3HA5_CODE_REG, S6E3HA5_CODE_LEN, panel->code);
	if (ret != S6E3HA5_CODE_LEN) {
		dsim_err("fail to read code on command.\n");
		goto read_fail;
	}
	dsim_info("READ code : ");
	for (i = 0; i < S6E3HA5_CODE_LEN; i++)
		dsim_info("%x, ", panel->code[i]);
	dsim_info("\n");

	// elvss
	panel->elvss_set[0] = S6E3HA5_ELVSS_REG;
	ret = dsim_read_hl_data(dsim, S6E3HA5_ELVSS_REG, S6E3HA5_ELVSS_LEN, &(panel->elvss_set[1]));
	if (ret != S6E3HA5_ELVSS_LEN) {
		dsim_err("fail to read elvss on command.\n");
		goto read_fail;
	}
	dsim_info("READ elvss : ");
	for (i = 1; i <= S6E3HA5_ELVSS_LEN; i++)
		dsim_info("%x \n", dsim->priv.elvss_set[i]);

	ret = dsim_read_hl_data(dsim, S6E3HA5_HBMGAMMA_REG, S6E3HA5_HBMGAMMA_READ_END, hbm_gamma);
	if (ret != S6E3HA5_HBMGAMMA_READ_END) {
		dsim_err("fail to read hbm_gamma.\n");
		goto read_fail;
	}
	memcpy(hbm, hbm_gamma, S6E3HA5_HBMGAMMA_LEN);

	dsim_info("HBM Gamma : ");
	for (i = 50; i < S6E3HA5_HBMGAMMA_LEN; i++)
		dsim_info("hbm gamma[%d] : %x\n", i, hbm_gamma[i]);

#ifdef CONFIG_CHECK_OCTA_CHIP_ID
		// octa_id
	ret = dsim_read_hl_data(dsim, S6E3HA5_OCTAID_REG, S6E3HA5_OCTAID_LEN, panel->octa_id);
	if (ret != S6E3HA5_OCTAID_LEN) {
		dsim_err("fail to read octa_id command.\n");
		goto read_fail;
	}
	dsim_info("READ octa_id : ");
	for(i = 1; i < S6E3HA5_OCTAID_LEN; i++)
		dsim_info("%x, ", dsim->priv.octa_id[i]);
	dsim_info("\n");
#endif

	ret = dsim_write_hl_data(dsim, SEQ_TEST_KEY_OFF_F0, ARRAY_SIZE(SEQ_TEST_KEY_OFF_F0));
	if (ret < 0) {
		dsim_err("%s : fail to write CMD : SEQ_TEST_KEY_OFF_F0\n", __func__);
		goto read_fail;
	}
read_exit:
	return 0;

read_fail:
	return -ENODEV;

}

static int s6e3ha5_wqhd_dump(struct dsim_device *dsim)
{
	int ret = 0;
	int i;
	unsigned char id[S6E3HA5_ID_LEN];
	unsigned char rddpm[4];
	unsigned char rddsm[4];
	unsigned char err_buf[4];

	ret = dsim_write_hl_data(dsim, SEQ_TEST_KEY_ON_F0, ARRAY_SIZE(SEQ_TEST_KEY_ON_F0));
	if (ret < 0) {
		dsim_err("%s : fail to write CMD : SEQ_TEST_KEY_ON_F0\n", __func__);
	}

	ret = dsim_write_hl_data(dsim, SEQ_TEST_KEY_ON_FC, ARRAY_SIZE(SEQ_TEST_KEY_ON_FC));
	if (ret < 0) {
		dsim_err("%s : fail to write CMD : SEQ_TEST_KEY_ON_FC\n", __func__);
	}

	ret = dsim_read_hl_data(dsim, 0xEA, 3, err_buf);
	if (ret != 3) {
		dsim_err("%s : can't read Panel's EA Reg\n", __func__);
		goto dump_exit;
	}

	dsim_info("=== Panel's 0xEA Reg Value ===\n");
	dsim_info("* 0xEA : buf[0] = %x\n", err_buf[0]);
	dsim_info("* 0xEA : buf[1] = %x\n", err_buf[1]);

	ret = dsim_read_hl_data(dsim, S6E3HA5_RDDPM_ADDR, 3, rddpm);
	if (ret != 3) {
		dsim_err("%s : can't read RDDPM Reg\n", __func__);
		goto dump_exit;
	}

	dsim_info("=== Panel's RDDPM Reg Value : %x ===\n", rddpm[0]);

	if (rddpm[0] & 0x80)
		dsim_info("* Booster Voltage Status : ON\n");
	else
		dsim_info("* Booster Voltage Status : OFF\n");

	if (rddpm[0] & 0x40)
		dsim_info("* Idle Mode : On\n");
	else
		dsim_info("* Idle Mode : OFF\n");

	if (rddpm[0] & 0x20)
		dsim_info("* Partial Mode : On\n");
	else
		dsim_info("* Partial Mode : OFF\n");

	if (rddpm[0] & 0x10)
		dsim_info("* Sleep OUT and Working Ok\n");
	else
		dsim_info("* Sleep IN\n");

	if (rddpm[0] & 0x08)
		dsim_info("* Normal Mode On and Working Ok\n");
	else
		dsim_info("* Sleep IN\n");

	if (rddpm[0] & 0x04)
		dsim_info("* Display On and Working Ok\n");
	else
		dsim_info("* Display Off\n");

	ret = dsim_read_hl_data(dsim, S6E3HA5_RDDSM_ADDR, 3, rddsm);
	if (ret != 3) {
		dsim_err("%s : can't read RDDSM Reg\n", __func__);
		goto dump_exit;
	}

	dsim_info("=== Panel's RDDSM Reg Value : %x ===\n", rddsm[0]);

	if (rddsm[0] & 0x80)
		dsim_info("* TE On\n");
	else
		dsim_info("* TE OFF\n");

	if (rddsm[0] & 0x02)
		dsim_info("* S_DSI_ERR : Found\n");

	if (rddsm[0] & 0x01)
		dsim_info("* DSI_ERR : Found\n");

	ret = dsim_read_hl_data(dsim, S6E3HA5_ID_REG, S6E3HA5_ID_LEN, id);
	if (ret != S6E3HA5_ID_LEN) {
		dsim_err("%s : can't read panel id\n", __func__);
		goto dump_exit;
	}

	dsim_info("READ ID : ");
	for (i = 0; i < S6E3HA5_ID_LEN; i++)
		dsim_info("%02x, ", id[i]);
	dsim_info("\n");

	ret = dsim_write_hl_data(dsim, SEQ_TEST_KEY_OFF_FC, ARRAY_SIZE(SEQ_TEST_KEY_OFF_FC));
	if (ret < 0) {
		dsim_err("%s : fail to write CMD : SEQ_TEST_KEY_OFF_FC\n", __func__);
	}

	ret = dsim_write_hl_data(dsim, SEQ_TEST_KEY_OFF_F0, ARRAY_SIZE(SEQ_TEST_KEY_OFF_F0));
	if (ret < 0) {
		dsim_err("%s : fail to write CMD : SEQ_TEST_KEY_OFF_F0\n", __func__);
	}
dump_exit:
	return ret;

}

static int s6e3ha5_wqhd_probe(struct dsim_device *dsim)
{
	int ret = 0;
	struct panel_private *panel = &dsim->priv;
	unsigned char mtp[S6E3HA5_MTP_SIZE] = { 0, };
	unsigned char hbm[S6E3HA5_HBMGAMMA_LEN] = { 0, };

	dsim_info("MDD : %s was called\n", __func__);

	panel->dim_data = (void *)NULL;
	panel->lcdConnected = PANEL_CONNECTED;

#ifdef CONFIG_LCD_ALPM
	mutex_init(&panel->alpm_lock);
#endif

	ret = s6e3ha5_read_init_info(dsim, mtp, hbm);
	if (panel->lcdConnected == PANEL_DISCONNEDTED) {
		dsim_err("dsim : %s lcd was not connected\n", __func__);
		goto probe_exit;
	}

	s6e3ha5_init_default_info(dsim);
#ifdef CONFIG_LCD_ALPM
	panel->alpm = 0;
	panel->current_alpm = 0;
/*	if((panel->current_model == MODEL_IS_HERO2) && (panel->panel_rev >= 3))	panel->alpm_support = SUPPORT_LOWHZALPM;*/
#endif
	panel->alpm_support = SUPPORT_30HZALPM;


	if ((dsim->priv.current_model == 1) && (dsim->priv.panel_rev > 0))
		dsim->priv.esd_disable = 0;
	else
		dsim->priv.esd_disable = 1;
	dsim->priv.panel_type = dynamic_lcd_type;

#ifdef CONFIG_LCD_WEAKNESS_CCB
	panel->ccb_support = SUPPORT_CCB;
	panel->current_ccb = 0;
#endif

#ifdef CONFIG_EXYNOS_DECON_MDNIE_LITE
	panel->mdnie_support = true;
#endif

#ifdef CONFIG_PANEL_AID_DIMMING
	ret = ha5_init_dimming(dsim, mtp, hbm);
	if (ret) {
		dsim_err("%s : failed to generate gamma tablen\n", __func__);
	}
#endif

#ifdef CONFIG_LCD_HMT
	panel->hmt_support = SUPPORT_HMT;

	ret = ha5_hmt_init_dimming(dsim, mtp);
	if (ret) {
		dsim_err("%s : failed to generate gamma tablen\n", __func__);
	}
#endif

probe_exit:
	return ret;

}

static int s6e3ha5_wqhd_displayon(struct dsim_device *dsim)
{
	int ret = 0;
#ifdef CONFIG_LCD_ALPM
	struct panel_private *panel = &dsim->priv;
#endif

	dsim_info("MDD : %s was called\n", __func__);

#ifdef CONFIG_LCD_ALPM
	if (panel->current_alpm == panel->alpm && panel->current_alpm) {
		dsim_info("%s : ALPM mode\n", __func__);
		if ((panel->alpm_support == SUPPORT_LOWHZALPM) && (panel->current_alpm == ALPM_ON_40NIT)) {
			dsim_write_hl_data(dsim, SEQ_TEST_KEY_ON_F0, ARRAY_SIZE(SEQ_TEST_KEY_ON_F0));
			dsim_write_hl_data(dsim, SEQ_AOD_LOWHZ_OFF, ARRAY_SIZE(SEQ_AOD_LOWHZ_OFF));
			dsim_write_hl_data(dsim, SEQ_TEST_KEY_OFF_F0, ARRAY_SIZE(SEQ_TEST_KEY_OFF_F0));
			dsim_info("%s : ALPM LOW Hz off\n", __func__);
		}
	} else if (panel->current_alpm && panel->alpm == ALPM_OFF) {
		ret = alpm_set_mode(dsim, ALPM_OFF);
		if (ret) {
			dsim_err("failed to exit alpm.\n");
			goto displayon_err;
		}
	} else {
		if (panel->alpm) {
			ret = alpm_set_mode(dsim, panel->alpm);
			if (ret) {
				dsim_err("failed to initialize alpm.\n");
				goto displayon_err;
			}
		} else {
			ret = dsim_write_hl_data(dsim, SEQ_DISPLAY_ON, ARRAY_SIZE(SEQ_DISPLAY_ON));
			if (ret < 0) {
				dsim_err("%s : fail to write CMD : DISPLAY_ON\n", __func__);
				goto displayon_err;
			}
		}
	}
#else
	ret = dsim_write_hl_data(dsim, SEQ_DISPLAY_ON, ARRAY_SIZE(SEQ_DISPLAY_ON));
	if (ret < 0) {
		dsim_err("%s : fail to write CMD : DISPLAY_ON\n", __func__);
		goto displayon_err;
	}
#endif

displayon_err:
	return ret;

}

static int s6e3ha5_wqhd_exit(struct dsim_device *dsim)
{
	int ret = 0;
#ifdef CONFIG_LCD_ALPM
	struct panel_private *panel = &dsim->priv;
#endif
	dsim_info("MDD : %s was called\n", __func__);
#ifdef CONFIG_LCD_ALPM
	mutex_lock(&panel->alpm_lock);
	if (panel->current_alpm && panel->alpm) {
		dsim->alpm = panel->current_alpm;
		dsim_info("%s : ALPM mode\n", __func__);
		if ((panel->alpm_support == SUPPORT_LOWHZALPM) && (panel->current_alpm == ALPM_ON_40NIT)) {
			usleep_range(100000, 100000);
			dsim_write_hl_data(dsim, SEQ_TEST_KEY_ON_F0, ARRAY_SIZE(SEQ_TEST_KEY_ON_F0));
			dsim_write_hl_data(dsim, SEQ_AOD_LOWHZ_ON, ARRAY_SIZE(SEQ_AOD_LOWHZ_ON));
			dsim_write_hl_data(dsim, SEQ_TEST_KEY_OFF_F0, ARRAY_SIZE(SEQ_TEST_KEY_OFF_F0));
			dsim_info("%s : ALPM LOW Hz on\n", __func__);
		}
	} else {
		ret = dsim_write_hl_data(dsim, SEQ_DISPLAY_OFF, ARRAY_SIZE(SEQ_DISPLAY_OFF));
		if (ret < 0) {
			dsim_err("%s : fail to write CMD : DISPLAY_OFF\n", __func__);
			goto exit_err;
		}
		ret = dsim_write_hl_data(dsim, SEQ_SLEEP_IN, ARRAY_SIZE(SEQ_SLEEP_IN));
		if (ret < 0) {
			dsim_err("%s : fail to write CMD : SLEEP_IN\n", __func__);
			goto exit_err;
		}
		msleep(120);
	}

	dsim_info("MDD : %s was called unlock\n", __func__);
#else
	ret = dsim_write_hl_data(dsim, SEQ_DISPLAY_OFF, ARRAY_SIZE(SEQ_DISPLAY_OFF));
	if (ret < 0) {
		dsim_err("%s : fail to write CMD : DISPLAY_OFF\n", __func__);
		goto exit_err;
	}
	ret = dsim_write_hl_data(dsim, SEQ_SLEEP_IN, ARRAY_SIZE(SEQ_SLEEP_IN));
	if (ret < 0) {
		dsim_err("%s : fail to write CMD : SLEEP_IN\n", __func__);
		goto exit_err;
	}
	msleep(120);
#endif

exit_err:
#ifdef CONFIG_LCD_ALPM
	mutex_unlock(&panel->alpm_lock);
#endif

#if defined(CONFIG_FB_DSU) || defined(CONFIG_LCD_RES)
	last_dsc_enabled = false;
#endif

	return ret;
}

#ifdef CONFIG_FB_DSU
static const unsigned char S6E3HA5_SEQ_DDI_SCALER_PPS_00[] = {
	0xB0,
	0xB0
};

static const unsigned char S6E3HA5_SEQ_DDI_SCALER_PPS_01[] = {
	0xF2,
	0x2A
};
#endif

#if defined(CONFIG_FB_DSU) || defined(CONFIG_LCD_RES)
#undef CONFIG_HA5_CASET_PASET_CHECK
static int _s6e3ha5_wqhd_dsu_command(struct dsim_device *dsim, int xres, int yres)
{
	int ret = 0;
//	struct panel_private *panel = &dsim->priv;
	static int last_xres = 1440;

#ifdef CONFIG_HA5_CASET_PASET_CHECK
	const unsigned char SEQ_HA5_CASET_PASET_GPARAM[] = { 0xB0, 0x0F };
	const unsigned char REG_HA5_CASET_PASET = 0xFB;
	const unsigned char size_ha5_caset_paset = 8;
	char buffer_caset_paset[size_ha5_caset_paset + 4];
	u16 *pint16;
	int i;
#endif

	pr_info( "%s.%d: (DSU) last_dsc=%d, dsc_enabled=%d\n", __func__, __LINE__, last_dsc_enabled, dsim->lcd_info.dsc_enabled );
	if (last_dsc_enabled != dsim->lcd_info.dsc_enabled) {
		last_dsc_enabled = dsim->lcd_info.dsc_enabled;
		if( dsim->lcd_info.dsc_enabled )
		{
			ret = dsim_write_data(dsim, MIPI_DSI_DSC_PRA, S6E3HA5_DATA_DSC_ENABLE[0], S6E3HA5_DATA_DSC_ENABLE[1]);
			if (ret < 0) dsim_err("%s : fail to write CMD : S6E3HA5_DATA_DSC_ENABLE\n", __func__);
		} else {
			ret = dsim_write_data(dsim, MIPI_DSI_DSC_PRA, S6E3HA5_DATA_DSC_DISABLE[0], S6E3HA5_DATA_DSC_DISABLE[1]);
			if (ret < 0) dsim_err("%s : fail to write CMD : S6E3HA5_DATA_DSC_DISABLE\n", __func__);
		}
	}

	switch (xres) {
	case 720:
		dsim_err("%s : xres=%d, yres=%d, dsc_enabled=%d : HD\n", __func__, xres, yres, dsim->lcd_info.dsc_enabled);

		ret = dsim_write_hl_data(dsim, S6E3HA5_SEQ_DDI_SCALER_HD_XRES, ARRAY_SIZE(S6E3HA5_SEQ_DDI_SCALER_HD_XRES));
		if (ret < 0) dsim_err("%s : fail to write CMD : S6E3HA5_SEQ_DDI_SCALER_HD_XRES\n", __func__);
		ret = dsim_write_hl_data(dsim, S6E3HA5_SEQ_DDI_SCALER_HD_YRES, ARRAY_SIZE(S6E3HA5_SEQ_DDI_SCALER_HD_YRES));
		if (ret < 0) dsim_err("%s : fail to write CMD : S6E3HA5_SEQ_DDI_SCALER_HD_YRES\n", __func__);
		ret = dsim_write_hl_data(dsim, S6E3HA5_SEQ_DDI_SCALER_HD_SEAMLESS_00, ARRAY_SIZE(S6E3HA5_SEQ_DDI_SCALER_HD_SEAMLESS_00));
		if (ret < 0) dsim_err("%s : fail to write CMD : S6E3HA5_SEQ_DDI_SCALER_HD_SEAMLESS_00\n", __func__);
		ret = dsim_write_hl_data(dsim, S6E3HA5_SEQ_DDI_SCALER_HD_SEAMLESS_01, ARRAY_SIZE(S6E3HA5_SEQ_DDI_SCALER_HD_SEAMLESS_01));
		if (ret < 0) dsim_err("%s : fail to write CMD : S6E3HA5_SEQ_DDI_SCALER_HD_SEAMLESS_01\n", __func__);

		if( dsim->lcd_info.dsc_enabled )
		{
			ret = dsim_write_data(dsim, MIPI_DSI_DSC_PPS, (unsigned long)S6E3HA5_SEQ_DDI_SCALER_HD_PPS_SLICE2, ARRAY_SIZE(S6E3HA5_SEQ_DDI_SCALER_HD_PPS_SLICE2));
			if (ret < 0) dsim_err("%s : fail to write CMD : S6E3HA5_SEQ_DDI_SCALER_HD_PPS_SLICE2\n", __func__);
			}

		ret = dsim_write_hl_data(dsim, S6E3HA5_SEQ_DDI_SCALER_HD, ARRAY_SIZE(S6E3HA5_SEQ_DDI_SCALER_HD));
		if (ret < 0) dsim_err("%s : fail to write CMD : S6E3HA5_SEQ_DDI_SCALER_HD\n", __func__);

		break;
	case 1080:
		dsim_err("%s : xres=%d, yres=%d, dsc_enabled=%d : FHD\n", __func__, xres, yres, dsim->lcd_info.dsc_enabled);

		ret = dsim_write_hl_data(dsim, S6E3HA5_SEQ_DDI_SCALER_FHD_XRES, ARRAY_SIZE(S6E3HA5_SEQ_DDI_SCALER_FHD_XRES));
		if (ret < 0) dsim_err("%s : fail to write CMD : S6E3HA5_SEQ_DDI_SCALER_FHD_XRES\n", __func__);
		ret = dsim_write_hl_data(dsim, S6E3HA5_SEQ_DDI_SCALER_FHD_YRES, ARRAY_SIZE(S6E3HA5_SEQ_DDI_SCALER_FHD_YRES));
		if (ret < 0) dsim_err("%s : fail to write CMD : S6E3HA5_SEQ_DDI_SCALER_FHD_YRES\n", __func__);
		ret = dsim_write_hl_data(dsim, S6E3HA5_SEQ_DDI_SCALER_FHD_SEAMLESS_00, ARRAY_SIZE(S6E3HA5_SEQ_DDI_SCALER_FHD_SEAMLESS_00));
		if (ret < 0) dsim_err("%s : fail to write CMD : S6E3HA5_SEQ_DDI_SCALER_FHD_SEAMLESS_00\n", __func__);
		ret = dsim_write_hl_data(dsim, S6E3HA5_SEQ_DDI_SCALER_FHD_SEAMLESS_01, ARRAY_SIZE(S6E3HA5_SEQ_DDI_SCALER_FHD_SEAMLESS_01));
		if (ret < 0) dsim_err("%s : fail to write CMD : S6E3HA5_SEQ_DDI_SCALER_FHD_SEAMLESS_01\n", __func__);

		if( dsim->lcd_info.dsc_enabled )
		{
		ret = dsim_write_data(dsim, MIPI_DSI_DSC_PPS, (unsigned long)S6E3HA5_SEQ_DDI_SCALER_FHD_PPS_SLICE2, ARRAY_SIZE(S6E3HA5_SEQ_DDI_SCALER_FHD_PPS_SLICE2));
			if (ret < 0) dsim_err("%s : fail to write CMD : S6E3HA5_SEQ_DDI_SCALER_FHD_PPS_SLICE2\n", __func__);
		}

		ret = dsim_write_hl_data(dsim, S6E3HA5_SEQ_DDI_SCALER_FHD, ARRAY_SIZE(S6E3HA5_SEQ_DDI_SCALER_FHD));
		if (ret < 0) dsim_err("%s : fail to write CMD : S6E3HA5_SEQ_DDI_SCALER_FHD\n", __func__);
		break;
	case 1440:
	default:
		dsim_err("%s : xres=%d, yres=%d, dsc_enabled=%d : WQHD\n", __func__, xres, yres, dsim->lcd_info.dsc_enabled);

		ret = dsim_write_hl_data(dsim, S6E3HA5_SEQ_DDI_SCALER_WQHD_XRES, ARRAY_SIZE(S6E3HA5_SEQ_DDI_SCALER_WQHD_XRES));
		if (ret < 0) dsim_err("%s : fail to write CMD : S6E3HA5_SEQ_DDI_SCALER_WQHD_XRES\n", __func__);
		ret = dsim_write_hl_data(dsim, S6E3HA5_SEQ_DDI_SCALER_WQHD_YRES, ARRAY_SIZE(S6E3HA5_SEQ_DDI_SCALER_WQHD_YRES));
		if (ret < 0) dsim_err("%s : fail to write CMD : S6E3HA5_SEQ_DDI_SCALER_WQHD_YRES\n", __func__);
		ret = dsim_write_hl_data(dsim, S6E3HA5_SEQ_DDI_SCALER_WQHD_SEAMLESS_00, ARRAY_SIZE(S6E3HA5_SEQ_DDI_SCALER_WQHD_SEAMLESS_00));
		if (ret < 0) dsim_err("%s : fail to write CMD : S6E3HA5_SEQ_DDI_SCALER_WQHD_SEAMLESS_00\n", __func__);
		ret = dsim_write_hl_data(dsim, S6E3HA5_SEQ_DDI_SCALER_WQHD_SEAMLESS_01, ARRAY_SIZE(S6E3HA5_SEQ_DDI_SCALER_WQHD_SEAMLESS_01));
		if (ret < 0) dsim_err("%s : fail to write CMD : S6E3HA5_SEQ_DDI_SCALER_WQHD_SEAMLESS_01\n", __func__);

		if( dsim->lcd_info.dsc_enabled )
		{
		ret = dsim_write_data(dsim, MIPI_DSI_DSC_PPS, (unsigned long)S6E3HA5_SEQ_PPS_WQHD_SLICE2, ARRAY_SIZE(S6E3HA5_SEQ_PPS_WQHD_SLICE2));
			if (ret < 0) dsim_err("%s : fail to write CMD : S6E3HA5_SEQ_PPS_WQHD_SLICE2\n", __func__);
		}

		if( last_xres == 720 ) {
			ret = dsim_write_hl_data(dsim, S6E3HA5_SEQ_DDI_SCALER_HD_2_WQHD, ARRAY_SIZE(S6E3HA5_SEQ_DDI_SCALER_HD_2_WQHD));
			if (ret < 0) dsim_err("%s : fail to write CMD : S6E3HA5_SEQ_DDI_SCALER_HD_2_WQHD\n", __func__);
		} else if( last_xres == 1080 ) {
			ret = dsim_write_hl_data(dsim, S6E3HA5_SEQ_DDI_SCALER_FHD_2_WQHD, ARRAY_SIZE(S6E3HA5_SEQ_DDI_SCALER_FHD_2_WQHD));
			if (ret < 0) dsim_err("%s : fail to write CMD : S6E3HA5_SEQ_DDI_SCALER_FHD_2_WQHD\n", __func__);
		} else {
			ret = dsim_write_hl_data(dsim, S6E3HA5_SEQ_DDI_SCALER_FHD_2_WQHD, ARRAY_SIZE(S6E3HA5_SEQ_DDI_SCALER_FHD_2_WQHD));
			if (ret < 0) dsim_err("%s : fail to write CMD : S6E3HA5_SEQ_DDI_SCALER_FHD_2_WQHD\n", __func__);
		}

		break;
	}

#ifdef CONFIG_HA5_CASET_PASET_CHECK
	ret = dsim_write_hl_data(dsim, SEQ_HA5_CASET_PASET_GPARAM, ARRAY_SIZE(SEQ_HA5_CASET_PASET_GPARAM));
	ret = dsim_read_hl_data(dsim, REG_HA5_CASET_PASET, size_ha5_caset_paset, buffer_caset_paset);
	pint16 = (u16 *) buffer_caset_paset;
//	for (i = 0; i < size_ha5_caset_paset / sizeof(pint16[0]); i++) {
//		pint16[i] = ((pint16[i] & 0xF0) >> 4) + ((pint16[i] & 0x0F) << 4);
//	}
	dsim_info( "%s.%d (dsu) caset paset(%d) : %04x, %04x, %04x, %04x\n", __func__, __LINE__, ret, pint16[0], pint16[1], pint16[2], pint16[3] );
#endif

	last_xres = xres;
	return ret;
}
#endif

#ifdef CONFIG_FB_DSU
static int s6e3ha5_wqhd_dsu_command(struct dsim_device *dsim)
{
	struct panel_private *panel = &dsim->priv;
	int ret = 0;

	mutex_lock(&panel->lock);
	ret = dsim_write_hl_data(dsim, SEQ_TEST_KEY_ON_F0, ARRAY_SIZE(SEQ_TEST_KEY_ON_F0));
	if (ret < 0) dsim_err("%s : fail to write CMD : SEQ_TEST_KEY_ON_F0\n", __func__);

	ret = _s6e3ha5_wqhd_dsu_command(dsim, dsim->dsu_xres, dsim->dsu_yres);

	dsim_write_hl_data(dsim, SEQ_TEST_KEY_OFF_F0, ARRAY_SIZE(SEQ_TEST_KEY_OFF_F0));
	mutex_unlock(&panel->lock);

	return ret;
}
#endif

static int s6e3ha5_wqhd_init(struct dsim_device *dsim)
{
	int ret = 0;

	dsim_info("MDD : %s was called\n", __func__);

#ifdef CONFIG_LCD_ALPM
	if (dsim->priv.current_alpm) {
		dsim_info("%s : ALPM mode\n", __func__);

		return ret;
	}
#endif
	/* DSC setting */
	msleep(5);

#ifdef CONFIG_FB_DSU
	ret = dsim_write_hl_data(dsim, SEQ_TEST_KEY_ON_F0, ARRAY_SIZE(SEQ_TEST_KEY_ON_F0));
	if (ret < 0) dsim_err("%s : fail to write CMD : SEQ_TEST_KEY_ON_F0\n", __func__);

	ret = _s6e3ha5_wqhd_dsu_command(dsim, dsim->dsu_xres, dsim->dsu_yres);
#elif defined(CONFIG_LCD_RES)
	ret = dsim_write_hl_data(dsim, SEQ_TEST_KEY_ON_F0, ARRAY_SIZE(SEQ_TEST_KEY_ON_F0));
	if (ret < 0) dsim_err("%s : fail to write CMD : SEQ_TEST_KEY_ON_F0\n", __func__);

	ret = _s6e3ha5_wqhd_dsu_command(dsim, dsim->priv.lcd_res, 0);
#else
	ret = dsim_write_data(dsim, MIPI_DSI_DSC_PRA, S6E3HA5_DATA_DSC_ENABLE[0], S6E3HA5_DATA_DSC_ENABLE[1]);
	if (ret < 0) {
		dsim_err("%s : fail to write CMD : DATA_SEC_ENABLE\n", __func__);
		goto init_exit;
	}

	ret = dsim_write_data(dsim, MIPI_DSI_DSC_PPS, (unsigned long)S6E3HA5_SEQ_PPS_WQHD_SLICE2, ARRAY_SIZE(S6E3HA5_SEQ_PPS_WQHD_SLICE2));
	if (ret < 0) {
		dsim_err("%s : fail to write CMD : SEQ_PPS_SLICE4\n", __func__);
		goto init_exit;
	}
#endif

	/* Sleep Out(11h) */
	ret = dsim_write_hl_data(dsim, SEQ_SLEEP_OUT, ARRAY_SIZE(SEQ_SLEEP_OUT));
	if (ret < 0) {
		dsim_err("%s : fail to write CMD : SEQ_SLEEP_OUT\n", __func__);
		goto init_exit;
	}
	msleep(120);

#ifdef CONFIG_ALWAYS_RELOAD_MTP_FACTORY_BUILD
	ret = lcd_reload_mtp(dynamic_lcd_type, dsim);
#endif

	/* Interface Setting */
	ret = dsim_write_hl_data(dsim, SEQ_TEST_KEY_ON_F0, ARRAY_SIZE(SEQ_TEST_KEY_ON_F0));
	if (ret < 0) {
		dsim_err("%s : fail to write CMD : SEQ_TEST_KEY_ON_F0\n", __func__);
		goto init_exit;
	}

	ret = dsim_write_hl_data(dsim, SEQ_TEST_KEY_ON_FC, ARRAY_SIZE(SEQ_TEST_KEY_ON_FC));
	if (ret < 0) {
		dsim_err("%s : fail to write CMD : SEQ_TEST_KEY_ON_F0\n", __func__);
		goto init_exit;
	}

	/* Common Setting */
	ret = dsim_write_hl_data(dsim, S6E3HA5_SEQ_TE_ON, ARRAY_SIZE(S6E3HA5_SEQ_TE_ON));
	if (ret < 0) {
		dsim_err("%s : fail to write CMD : SEQ_TE_ON\n", __func__);
		goto init_exit;
	}

	ret = dsim_write_hl_data(dsim, S6E3HA5_SEQ_ERR_FG_SETTING, ARRAY_SIZE(S6E3HA5_SEQ_ERR_FG_SETTING));
	if (ret < 0) {
		dsim_err("%s : fail to write CMD : SEQ_ERR_FG_SETTING\n", __func__);
		goto init_exit;
	}
	ret = dsim_write_hl_data(dsim, S6E3HA5_SEQ_TE_START_HSYNC_SETTING, ARRAY_SIZE(S6E3HA5_SEQ_TE_START_HSYNC_SETTING));
	if (ret < 0) {
		dsim_err("%s : fail to write CMD : SEQ_TE_START_SETTING\n", __func__);
		goto init_exit;
	}

	ret = dsim_write_hl_data(dsim, S6E3HA5_SEQ_FFC_SET, ARRAY_SIZE(S6E3HA5_SEQ_FFC_SET));
	if (ret < 0) {
		dsim_err("%s : fail to write CMD : S6E3HA5_SEQ_FFC_SET\n", __func__);
		goto init_exit;
	}

#ifndef CONFIG_PANEL_AID_DIMMING
	/* Brightness Setting */
	ret = dsim_write_hl_data(dsim, S6E3HA5_SEQ_GAMMA_CONDITION_SET, ARRAY_SIZE(S6E3HA5_SEQ_GAMMA_CONDITION_SET));
	if (ret < 0) {
		dsim_err(":%s fail to write CMD : SEQ_GAMMA_CONDITION_SET\n", __func__);
		goto init_exit;
	}
	ret = dsim_write_hl_data(dsim, S6E3HA5_SEQ_AOR_CONTROL, ARRAY_SIZE(S6E3HA5_SEQ_AOR_CONTROL));
	if (ret < 0) {
		dsim_err(":%s fail to write CMD : SEQ_AOR_CONTROL\n", __func__);
		goto init_exit;
	}
	ret = dsim_write_hl_data(dsim, SEQ_GAMMA_UPDATE, ARRAY_SIZE(SEQ_GAMMA_UPDATE));
	if (ret < 0) {
		dsim_err(":%s fail to write CMD : SEQ_GAMMA_UPDATE\n", __func__);
		goto init_exit;
	}

	/* elvss */
	ret = dsim_write_hl_data(dsim, S6E3HA5_SEQ_TSET_ELVSS_SET, ARRAY_SIZE(S6E3HA5_SEQ_TSET_ELVSS_SET));
	if (ret < 0) {
		dsim_err(":%s fail to write CMD : SEQ_TSET_ELVSS_SET\n", __func__);
		goto init_exit;
	}

	ret = dsim_write_hl_data(dsim, S6E3HA5_SEQ_VINT_SET, ARRAY_SIZE(S6E3HA5_SEQ_VINT_SET));
	if (ret < 0) {
		dsim_err(":%s fail to write CMD : SEQ_VINT_SET\n", __func__);
		goto init_exit;
	}
	/* ACL Setting */
	ret = dsim_write_hl_data(dsim, SEQ_ACL_OFF, ARRAY_SIZE(SEQ_ACL_OFF));
	if (ret < 0) {
		dsim_err(":%s fail to write CMD : SEQ_ACL_OFF\n", __func__);
		goto init_exit;
	}
	ret = dsim_write_hl_data(dsim, SEQ_ACL_OFF_OPR_AVR, ARRAY_SIZE(SEQ_ACL_OFF_OPR_AVR));
	if (ret < 0) {
		dsim_err(":%s fail to write CMD : SEQ_ACL_OFF_OPR\n", __func__);
		goto init_exit;
	}
#endif
	ret = dsim_write_hl_data(dsim, SEQ_TEST_KEY_OFF_FC, ARRAY_SIZE(SEQ_TEST_KEY_OFF_FC));
	if (ret < 0) {
		dsim_err("%s : fail to write CMD : SEQ_TEST_KEY_OFF_FC\n", __func__);
		goto init_exit;
	}

	ret = dsim_write_hl_data(dsim, SEQ_TEST_KEY_OFF_F0, ARRAY_SIZE(SEQ_TEST_KEY_OFF_F0));
	if (ret < 0) {
		dsim_err("%s : fail to write CMD : SEQ_TEST_KEY_OFF_F0\n", __func__);
		goto init_exit;
	}

init_exit:
	return ret;
}

#ifdef CONFIG_LCD_DOZE_MODE

int s6e3ha5_wqhd_setalpm(struct dsim_device *dsim, int mode)
{
	int ret = 0;

	struct panel_private *priv = &(dsim->priv);

	u8 seq_eq0[] = { 0xBB, 0x0C, 0x70, 0x0C, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07 };
	const u8 seq_eq1[] = { 0xF6, 0x43, 0x03 };

	switch (mode) {
	case HLPM_ON_2NIT:
		dsim_write_hl_data(dsim, SEQ_SELECT_xLPM_NIT_GPARAM, ARRAY_SIZE(SEQ_SELECT_xLPM_NIT_GPARAM));
//		dsim_write_hl_data(dsim, SEQ_SELECT_HLPM_2NIT_HA5, ARRAY_SIZE(SEQ_SELECT_HLPM_2NIT_HA5));

		seq_eq0[1] = SEQ_SELECT_HLPM_2NIT_HA5[1];
		dsim_write_hl_data(dsim, seq_eq0, ARRAY_SIZE(seq_eq0));
		dsim_write_hl_data(dsim, seq_eq1, ARRAY_SIZE(seq_eq1));

		dsim_write_hl_data(dsim, SEQ_2NIT_MODE_ON, ARRAY_SIZE(SEQ_2NIT_MODE_ON));
		pr_info("%s : HLPM_ON_2NIT !\n", __func__);
		break;
	case ALPM_ON_2NIT:
		dsim_write_hl_data(dsim, SEQ_SELECT_xLPM_NIT_GPARAM, ARRAY_SIZE(SEQ_SELECT_xLPM_NIT_GPARAM));
//		dsim_write_hl_data(dsim, SEQ_SELECT_ALPM_2NIT_HA5, ARRAY_SIZE(SEQ_SELECT_ALPM_2NIT_HA5));

		seq_eq0[1] = SEQ_SELECT_ALPM_2NIT_HA5[1];
		dsim_write_hl_data(dsim, seq_eq0, ARRAY_SIZE(seq_eq0));
		dsim_write_hl_data(dsim, seq_eq1, ARRAY_SIZE(seq_eq1));

		dsim_write_hl_data(dsim, SEQ_2NIT_MODE_ON, ARRAY_SIZE(SEQ_2NIT_MODE_ON));
		pr_info("%s : ALPM_ON_2NIT !\n", __func__);
		break;
	case HLPM_ON_40NIT:
		dsim_write_hl_data(dsim, SEQ_SELECT_xLPM_NIT_GPARAM, ARRAY_SIZE(SEQ_SELECT_xLPM_NIT_GPARAM));
//		dsim_write_hl_data(dsim, SEQ_SELECT_HLPM_60NIT_HA5, ARRAY_SIZE(SEQ_SELECT_HLPM_60NIT_HA5));

		seq_eq0[1] = SEQ_SELECT_HLPM_60NIT_HA5[1];
		dsim_write_hl_data(dsim, seq_eq0, ARRAY_SIZE(seq_eq0));
		dsim_write_hl_data(dsim, seq_eq1, ARRAY_SIZE(seq_eq1));

		dsim_write_hl_data(dsim, SEQ_60NIT_MODE_ON, ARRAY_SIZE(SEQ_60NIT_MODE_ON));
		pr_info("%s : HLPM_ON_60NIT !\n", __func__);
		break;
	case ALPM_ON_40NIT:
		if (priv->alpm_support == SUPPORT_LOWHZALPM) {
			dsim_write_hl_data(dsim, SEQ_2HZ_GPARA, ARRAY_SIZE(SEQ_2HZ_GPARA));
			dsim_write_hl_data(dsim, SEQ_2HZ_SET, ARRAY_SIZE(SEQ_2HZ_SET));
			dsim_write_hl_data(dsim, SEQ_AID_MOD_ON, ARRAY_SIZE(SEQ_AID_MOD_ON));
			pr_info("%s : Low hz support !\n", __func__);
		}

		dsim_write_hl_data(dsim, SEQ_SELECT_xLPM_NIT_GPARAM, ARRAY_SIZE(SEQ_SELECT_xLPM_NIT_GPARAM));
//		dsim_write_hl_data(dsim, SEQ_SELECT_ALPM_60NIT_HA5, ARRAY_SIZE(SEQ_SELECT_ALPM_60NIT_HA5));

		seq_eq0[1] = SEQ_SELECT_ALPM_60NIT_HA5[1];
		dsim_write_hl_data(dsim, seq_eq0, ARRAY_SIZE(seq_eq0));
		dsim_write_hl_data(dsim, seq_eq1, ARRAY_SIZE(seq_eq1));

		dsim_write_hl_data(dsim, SEQ_60NIT_MODE_ON, ARRAY_SIZE(SEQ_60NIT_MODE_ON));
		pr_info("%s : ALPM_ON_60NIT !\n", __func__);
		break;
	default:
		pr_info("%s: input is out of range : %d \n", __func__, mode);
		break;
	}
	dsim_write_hl_data(dsim, HA5_A3_IRC_off, ARRAY_SIZE(HA5_A3_IRC_off));

	dsim_write_hl_data(dsim, SEQ_GAMMA_UPDATE, ARRAY_SIZE(SEQ_GAMMA_UPDATE));
	dsim_write_hl_data(dsim, SEQ_GAMMA_UPDATE_L, ARRAY_SIZE(SEQ_GAMMA_UPDATE_L));
	return ret;

}

static int s6e3ha5_wqhd_enteralpm(struct dsim_device *dsim)
{
	int ret = 0;
	struct panel_private *panel = &dsim->priv;

	dsim_info("%s was called\n", __func__);

	if (panel->state == PANEL_STATE_SUSPENED) {
		dsim_err("ERR:%s:panel is not active\n", __func__);
		return ret;
	}

	ret = dsim_write_hl_data(dsim, SEQ_TEST_KEY_ON_F0, ARRAY_SIZE(SEQ_TEST_KEY_ON_F0));
	if (ret < 0) {
		dsim_err("%s : fail to write CMD : SEQ_TEST_KEY_ON_F0\n", __func__);
	}
	ret = dsim_write_hl_data(dsim, SEQ_TEST_KEY_ON_FC, ARRAY_SIZE(SEQ_TEST_KEY_ON_FC));
	if (ret < 0) {
		dsim_err("%s : fail to write CMD : SEQ_TEST_KEY_ON_FC\n", __func__);
	}

	ret = dsim_write_hl_data(dsim, SEQ_DISPLAY_OFF, ARRAY_SIZE(SEQ_DISPLAY_OFF));
	if (ret < 0) {
		dsim_err("%s : fail to write CMD : SEQ_DISPLAY_OFF\n", __func__);
	}

	ret = s6e3ha5_wqhd_setalpm(dsim, panel->alpm_mode);
	if (ret < 0) {
		dsim_err("%s : failed to set alpm\n", __func__);
	}

	ret = dsim_write_hl_data(dsim, SEQ_TEST_KEY_OFF_F0, ARRAY_SIZE(SEQ_TEST_KEY_OFF_F0));
	if (ret < 0) {
		dsim_err("%s : fail to write CMD : SEQ_TEST_KEY_OFF_F0\n", __func__);
	}
	ret = dsim_write_hl_data(dsim, SEQ_TEST_KEY_OFF_FC, ARRAY_SIZE(SEQ_TEST_KEY_OFF_FC));
	if (ret < 0) {
		dsim_err("%s : fail to write CMD : SEQ_TEST_KEY_OFF_FC\n", __func__);
	}

//exit_enteralpm:
	return ret;
}

static int s6e3ha5_wqhd_exitalpm(struct dsim_device *dsim)
{
	int ret = 0;
	struct panel_private *panel = &dsim->priv;

	if (panel->state == PANEL_STATE_SUSPENED) {
		dsim_err("ERR:%s:panel is not active\n", __func__);
		return ret;
	}

	dsim_info("%s++\n", __func__);
	mutex_lock(&panel->lock);

	ret = dsim_write_hl_data(dsim, SEQ_DISPLAY_OFF, ARRAY_SIZE(SEQ_DISPLAY_ON));
	if (ret < 0) {
		dsim_err("%s : fail to write CMD : DISPLAY_ON\n", __func__);
	}

	ret = dsim_write_hl_data(dsim, SEQ_TEST_KEY_ON_F0, ARRAY_SIZE(SEQ_TEST_KEY_ON_F0));
	if (ret < 0) {
		dsim_err("%s : fail to write CMD : SEQ_TEST_KEY_ON_F0\n", __func__);
	}
	ret = dsim_write_hl_data(dsim, SEQ_TEST_KEY_ON_FC, ARRAY_SIZE(SEQ_TEST_KEY_ON_FC));
	if (ret < 0) {
		dsim_err("%s : fail to write CMD : SEQ_TEST_KEY_ON_FC\n", __func__);
	}

	dsim_write_hl_data(dsim, SEQ_NORMAL_MODE_ON, ARRAY_SIZE(SEQ_NORMAL_MODE_ON));
	usleep_range(35000, 35000);
	// 16.04.28 delay of OP-manual is 34msec(2 frame), experimental test result = 25ms
	// 16.06.01 one sample has problem of this. delay increased to 35ms.

	if ((panel->alpm_support == SUPPORT_LOWHZALPM) && (panel->alpm_mode == ALPM_ON_40NIT)) {
		dsim_write_hl_data(dsim, SEQ_AOD_LOWHZ_OFF, ARRAY_SIZE(SEQ_AOD_LOWHZ_OFF));
		dsim_write_hl_data(dsim, SEQ_AID_MOD_OFF, ARRAY_SIZE(SEQ_AID_MOD_OFF));
		pr_info("%s : Low hz support !\n", __func__);
	}

	dsim_write_hl_data(dsim, SEQ_TEST_KEY_OFF_F0, ARRAY_SIZE(SEQ_TEST_KEY_OFF_F0));
	dsim_write_hl_data(dsim, SEQ_TEST_KEY_OFF_FC, ARRAY_SIZE(SEQ_TEST_KEY_OFF_FC));

	mutex_unlock(&panel->lock);
	dsim_info("%s--\n", __func__);

	return ret;
}
#endif

struct dsim_panel_ops s6e3ha5_panel_ops = {
	.probe = s6e3ha5_wqhd_probe,
	.displayon = s6e3ha5_wqhd_displayon,
	.exit = s6e3ha5_wqhd_exit,
	.init = s6e3ha5_wqhd_init,
	.dump = s6e3ha5_wqhd_dump,
#ifdef CONFIG_LCD_DOZE_MODE
	.enteralpm = s6e3ha5_wqhd_enteralpm,
	.exitalpm = s6e3ha5_wqhd_exitalpm,
#endif
#ifdef CONFIG_FB_DSU
	.dsu_cmd = s6e3ha5_wqhd_dsu_command,
#endif
};

/******************** COMMON ********************/

#ifdef CONFIG_ALWAYS_RELOAD_MTP_FACTORY_BUILD
static int lcd_reload_mtp(int lcd_type, struct dsim_device *dsim)
{
	int i, ret;
	unsigned char mtp[S6E3HF4_MTP_SIZE] = { 0, };	// all value is 35
	unsigned char hbm[S6E3HF4_HBMGAMMA_LEN] = { 0, };	// maximum value is S6E3HF4_HBMGAMMA_LEN

	// retry 3 times
	for (i = 0; i < 3; i++) {
		switch (lcd_type) {
		case LCD_TYPE_S6E3HF4_WQHD:
			ret = s6e3hf4_read_init_info(dsim, mtp, hbm);
			s6e3hf4_init_default_info(dsim);
			dsim_info("%s : load MTP of s6e3hf4\n", __func__);
			break;
		case LCD_TYPE_S6E3HA5_WQHD:
			ret = s6e3ha5_read_init_info(dsim, mtp, hbm);
			s6e3ha5_init_default_info(dsim);
			dsim_info("%s : load MTP of s6e3ha5\n", __func__);
			break;
		default:
			ret = s6e3ha5_read_init_info(dsim, mtp, hbm);
			s6e3ha5_init_default_info(dsim);
			dsim_info("%s : load MTP of s6e3ha5(default)\n", __func__);
			break;
		}
		if( ret == 0 ) break;
	}
	if( ret != 0 ) return -EIO;

	update_mdnie_coordinate(dsim->priv.coordinate[0], dsim->priv.coordinate[1]);

#ifdef CONFIG_PANEL_AID_DIMMING
	switch (lcd_type) {
	case LCD_TYPE_S6E3HF4_WQHD:
		ret = hf4_init_dimming(dsim, mtp, hbm);
		break;
	case LCD_TYPE_S6E3HA5_WQHD:
		ret = ha5_init_dimming(dsim, mtp, hbm);
		break;
	default:
		ret = ha5_init_dimming(dsim, mtp, hbm);
		break;
	}
	if (ret) {
		dsim_err("%s : failed to generate gamma tablen\n", __func__);
	}
#endif

	return 0;		//success
}
#endif // CONFIG_ALWAYS_RELOAD_MTP_FACTORY_BUILD

struct dsim_panel_ops *dsim_panel_get_priv_ops(struct dsim_device *dsim)
{
	//struct device *dev = dsim->dev;
	int model_id, ddi_id;

	model_id = (lcdtype & 0x003000) >> 12;
	ddi_id = (lcdtype & 0x0000C0) >> 6;
	dsim_info("hw_rev=%d, model_id=%d, ddi_id=%d\n", hw_rev, model_id, ddi_id);

	switch (ddi_id) {
	case 2:
		dynamic_lcd_type = LCD_TYPE_S6E3HA5_WQHD;
		break;
	case 0:
		dynamic_lcd_type = LCD_TYPE_S6E3HF4_WQHD;
		break;
	default:
		dynamic_lcd_type = LCD_TYPE_S6E3HA5_WQHD;
		break;
	}

#if 0				// lcd check by LCD_ID
	/* TODO : check LCD ID */
	if( hw_rev == 0 ) dynamic_lcd_type = LCD_TYPE_S6E3HF4_WQHD;
	else dynamic_lcd_type = LCD_TYPE_S6E3HA5_WQHD;
#endif
	dsim->priv.panel_type = dynamic_lcd_type;

	switch (dynamic_lcd_type) {
	case LCD_TYPE_S6E3HF4_WQHD:
		dsim_info("LCD TYPE : S6E3HF4 (WQHD) : %d type %x\n", dynamic_lcd_type, lcdtype);
		return &s6e3hf4_panel_ops;
	case LCD_TYPE_S6E3HA5_WQHD:
		dsim_info("LCD TYPE : S6E3HA5 (WQHD) : %d type %x\n", dynamic_lcd_type, lcdtype);
		return &s6e3ha5_panel_ops;
	default:
		dsim_info("LCD TYPE : S6E3HA5 (default.WQHD) : %d type %x\n", dynamic_lcd_type, lcdtype);
		return &s6e3ha5_panel_ops;
	}
}


static int __init get_lcd_type(char *arg)
{
	get_option(&arg, &lcdtype);

	dsim_info("parse LCDTYPE : %x\n", lcdtype);

	return 0;
}

early_param("lcdtype", get_lcd_type);

static int __init get_hw_rev(char *arg)
{
	get_option(&arg, &hw_rev);
	dsim_info("hw_rev : %d\n", hw_rev);

	return 0;
}
early_param("androidboot.hw_rev", get_hw_rev);

#pragma once
#include <pebble.h>
#include "nodes.h"
#include "settings.h"

#define WINDOW_WIDTH 144
#define WINDOW_HEIGHT 168

#define graphics_draw_text_border(ctx, text, font, x, y, width, height, border) { \
	graphics_draw_text((ctx), (text), (font), GRect((x), (y), (width), (height)), GTextOverflowModeFill, GTextAlignmentCenter, NULL); \
	graphics_draw_text((ctx), (text), (font), GRect((x), (y) + (border) * 2, (width), (height)), GTextOverflowModeFill, GTextAlignmentCenter, NULL); \
	graphics_draw_text((ctx), (text), (font), GRect((x) + (border) * 2, (y), (width), (height)), GTextOverflowModeFill, GTextAlignmentCenter, NULL); \
	graphics_draw_text((ctx), (text), (font), GRect((x) + (border) * 2, (y) + (border) * 2, (width), (height)), GTextOverflowModeFill, GTextAlignmentCenter, NULL); }

enum COLOR_PALETTE {C_BACKGROUND, C_TEXT, C_BORDER, C_PATTERN_1, C_PATTERN_2};	
	
void update_date(struct tm *tick_time);
void update_battery(bool show);
void tick_handler(struct tm *tick_time, TimeUnits units_changed);
void background_layer_mark_dirty();
void configure_inactivity_alert();
void randomise_color_palette(GColor8 *color_palette);

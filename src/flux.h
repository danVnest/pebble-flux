#pragma once
#include <pebble.h>
#include "nodes.h"
#include "settings.h"

enum COLOR_PALETTE {C_BACKGROUND, C_TEXT, C_BORDER, C_PATTERN_1, C_PATTERN_2};	
	
void update_date(struct tm *tick_time);
void update_battery(bool show);
void tick_handler(struct tm *tick_time, TimeUnits units_changed);
void background_layer_mark_dirty();
void configure_inactivity_alert();
void randomise_color_palette(GColor8 *color_palette);

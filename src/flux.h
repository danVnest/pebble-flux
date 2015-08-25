#pragma once
#include <pebble.h>
#include "nodes.h"
#include "settings.h"
	
void update_date(struct tm *tick_time);
void tick_handler(struct tm *tick_time, TimeUnits units_changed);
void background_layer_mark_dirty();
void enable_animations();
void disable_animations();

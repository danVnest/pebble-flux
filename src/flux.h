#pragma once
#include <pebble.h>
#include "nodes.h"
#include "settings.h"
	
void update_date(struct tm *tick_time);
void update_battery(bool show);
void tick_handler(struct tm *tick_time, TimeUnits units_changed);
void background_layer_mark_dirty();
void configure_inactivity_alert();

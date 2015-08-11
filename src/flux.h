#pragma once
#include <pebble.h>
#include "nodes.h"
#include "settings.h"
	
void update_date(struct tm *tick_time);
void background_layer_mark_dirty();
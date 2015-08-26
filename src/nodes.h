#pragma once
#include <pebble.h>
#include "flux.h"
	
void create_nodes();
void begin_startup_animation();
void animation_tick_handler(struct tm *tick_time);
void configure_frames_per_animation();
void configure_animation_frequency();
void configure_low_power_mode();
void draw_nodes(Layer *layer, GContext* ctx);

#pragma once
#include <pebble.h>
#include "flux.h"
	
void create_nodes();
void sync_animations();
void set_frames_per_animation(uint8_t animation_duration_ten);
void set_animation_frequency(uint16_t seconds);
void draw_nodes(Layer *layer, GContext* ctx);

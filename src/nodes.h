#pragma once
#include <pebble.h>
#include "flux.h"
	
void create_nodes();
void draw_nodes(Layer *layer, GContext* ctx);
void begin_animating_nodes();
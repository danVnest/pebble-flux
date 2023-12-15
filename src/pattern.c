#include "pattern.h"

static Layer *pattern_layer;
static AppTimer *animation_timer = NULL;
static uint8_t frame_number;
static uint8_t frames_per_animation;
static uint32_t animation_interval;
static uint8_t animation_interval_unit;
static bool animations_enabled;

static initialisePatternFunction initialise_current_pattern = initialise_nodes;
static animateFunction animate_current_pattern = animate_nodes;
static animationStepFunction current_pattern_animation_step = nodes_animation_step;
static drawPatternFunction draw_current_pattern = draw_nodes;

// TODO: define functions as xxx(void) or xxx()?
static void check_animation_status();
static void animate_pattern();
static void pattern_animation_step();

void initialise_pattern() {
	configure_frames_per_animation();
	configure_animation_frequency();
	randomise_color_palette(color_palette);
	initialise_current_pattern();
	pattern_layer = layer_create(GRect(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT));
	layer_set_update_proc(pattern_layer, draw_pattern);
}

void startup_animation() { }

static void check_animation_status() {
	if ((frames_per_animation == 0) || (animation_interval == 0) || low_power_mode_time_enabled || low_power_mode_threshold_enabled) {
		animations_enabled = false;
		tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
   	}
	else {
		if (animation_interval == 0xFF) {
			animate_nodes();
			animations_enabled = false;
		}
		else animations_enabled = true;
		tick_timer_service_subscribe(animation_interval_unit, tick_handler);
	}
}

void configure_frames_per_animation() {
	frames_per_animation = FRAMES_PER_SECOND * get_setting(SETTING_ANIMATIONS_DURATION) / 10;
	check_animation_status();
}

void configure_animation_frequency() {
	switch (get_setting(SETTING_ANIMATIONS_FREQUENCY)) {
		case 0: animation_interval = 0xFF; animation_interval_unit = MINUTE_UNIT; break;
		case 1: animation_interval = 1; animation_interval_unit = SECOND_UNIT; break;
		case 2: animation_interval = 2; animation_interval_unit = SECOND_UNIT; break;
		case 3: animation_interval = 3; animation_interval_unit = SECOND_UNIT; break;
		case 4: animation_interval = 5; animation_interval_unit = SECOND_UNIT; break;
		case 5: animation_interval = 10; animation_interval_unit = SECOND_UNIT; break;
		case 6: animation_interval = 15; animation_interval_unit = SECOND_UNIT; break;
		case 7: animation_interval = 30; animation_interval_unit = SECOND_UNIT; break;
		case 8: animation_interval = 1; animation_interval_unit = MINUTE_UNIT; break;
		case 9: animation_interval = 15; animation_interval_unit = MINUTE_UNIT; break;
		case 10: animation_interval = 30; animation_interval_unit = MINUTE_UNIT; break;
		case 11: animation_interval = 60; animation_interval_unit = MINUTE_UNIT; break;
		case 12: animation_interval = 0; animation_interval_unit = MINUTE_UNIT; break;
	}
	check_animation_status();
}

void animation_tick_handler(struct tm *tick_time) {
	if (animations_enabled) {
		if (((animation_interval_unit == SECOND_UNIT) && (tick_time->tm_sec % animation_interval == 0)) || ((animation_interval_unit == MINUTE_UNIT) && (tick_time->tm_min % animation_interval == 0))) {
			randomise_color_palette(color_palette);
			animate_pattern();
		}
	}
}

static void animate_pattern() {
	animation_timer = app_timer_register(FRAME_DURATION, pattern_animation_step, NULL);
	animate_current_pattern();
	frame_number = 0;
	layer_mark_dirty(pattern_layer);
}

static void pattern_animation_step() {
	if (++frame_number < frames_per_animation) animation_timer = app_timer_register(FRAME_DURATION, pattern_animation_step, NULL);
	else if ((animation_interval == 0xFF) && (animations_enabled)) animation_timer = app_timer_register(FRAME_DURATION, animate_pattern, NULL);
	current_pattern_animation_step();
	layer_mark_dirty(pattern_layer);
}

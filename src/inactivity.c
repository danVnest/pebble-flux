#include "inactivity.h"

static Layer *inactivity_layer;
static bool show_inactivity = false;
static bool inactivity_alert_enabled;
static uint32_t activity_this_minute = 0;
static uint32_t activity_window_sum = 0;
static uint32_t activity_window[INACTIVITY_WINDOW_SIZE] = {0};
static uint8_t current_activity_index = 0;
static uint8_t inactivity_period = 0;
static uint8_t inactivity_period_max;
static uint8_t inactivity_period_repeat;
static uint32_t activity_threshold;

static void draw_inactivity(Layer *layer, GContext *ctx);

void initialise_inactivity_alert(void) {
	inactivity_layer = layer_create(GRect(INACTIVITY_POS_X, INACTIVITY_POS_Y, INACTIVITY_WIDTH, INACTIVITY_HEIGHT));
	layer_set_update_proc(inactivity_layer, draw_inactivity);
	configure_inactivity_alert();
if (inactivity_alert_enabled && !is_low_power_mode_enabled()) {
		if (time(NULL) < persist_read_int(STORAGE_STATE_TIME) + 60 * 60) {
			show_inactivity = persist_read_bool(STORAGE_INACTIVITY_SHOWN);
			inactivity_period = persist_read_int(STORAGE_INACTIVITY_PERIOD);
			persist_read_data(STORAGE_ACTIVITY_WINDOW, activity_window, sizeof(activity_window));
			for (uint8_t i = 0; i < INACTIVITY_WINDOW_SIZE; i++) activity_window_sum += activity_window[i];
		}
	}
	review_inactivity_alert_state();
}

void configure_inactivity_alert(void) {
	switch (get_setting(SETTING_INACTIVITY_PERIOD)) {
		case 0:	inactivity_period_max = 15; break;
		case 1:	inactivity_period_max = 30; break;
		case 2:	inactivity_period_max = 60; break;
		case 3:	inactivity_period_max = 120; break;
		case 4:	inactivity_period_max = 180; break;
		case 5:	inactivity_period_max = 0; break;
	}
	switch (get_setting(SETTING_INACTIVITY_REPEAT)) {
		case 0:	inactivity_period_repeat = 1; break;
		case 1:	inactivity_period_repeat = 5; break;
		case 2:	inactivity_period_repeat = 10; break;
		case 3:	inactivity_period_repeat = 15; break;
		case 4:	inactivity_period_repeat = 30; break;
		case 5:	inactivity_period_repeat = 60; break;
		case 6:	inactivity_period_repeat = inactivity_period_max; break;
	}
	switch (get_setting(SETTING_INACTIVITY_THRESHOLD)) {
		case 0:	activity_threshold = 1000000; break;
		case 1:	activity_threshold = 2000000; break;
		case 2:	activity_threshold = 3000000; break;
		case 3:	activity_threshold = 4000000; break;
	}
	if (inactivity_period_repeat > 0) inactivity_period_repeat = inactivity_period_max;
}

void review_inactivity_alert_state(void) {
	if ((inactivity_period_max != 0) && (show_battery == false)) {
		if (inactivity_alert_enabled == false) {
			accel_raw_data_service_subscribe(25, activity_handler);
			accel_service_set_sampling_rate(ACCEL_SAMPLING_10HZ);
			inactivity_alert_enabled = true;
		}
	}
	else if (inactivity_alert_enabled == true) {
		accel_data_service_unsubscribe();
		inactivity_alert_enabled = false;
	}
	layer_mark_dirty(inactivity_layer);
}

bool is_inactivity_alert_enabled(void) {
	return inactivity_alert_enabled; // TODO: check battery too?
}

void activity_handler(AccelRawData *data, uint32_t num_samples, uint64_t timestamp) {
	// TODO: if num_samples is constant, unroll for loop, use macros
	for (uint32_t i = 1; i < num_samples; i++) {
		int32_t diff_x = data[i].x - data[i-1].x;
		int32_t diff_y = data[i].y - data[i-1].y;
		int32_t diff_z = data[i].z - data[i-1].z;
#define abs(x) ((x) < 0 ? -(x) : (x))
		activity_this_minute += abs(diff_x) + abs(diff_y) + abs(diff_z);
	}
}

void analyse_activity(void) {
	if (++current_activity_index >= INACTIVITY_WINDOW_SIZE) current_activity_index = 0;
	activity_window_sum += activity_this_minute - activity_window[current_activity_index];
	activity_window[current_activity_index] = activity_this_minute;
	activity_this_minute = 0;
	if (activity_window_sum >= activity_threshold) {
		inactivity_period = 0;
		if (show_inactivity) {
			show_inactivity = false;
			layer_mark_dirty(inactivity_layer);
		}
	}
	else if (++inactivity_period >= inactivity_period_max) {
		vibes_short_pulse();
		inactivity_period = inactivity_period_max - inactivity_period_repeat;
		show_inactivity = true;
	}
	if (show_inactivity) layer_mark_dirty(inactivity_layer);
}

static void draw_inactivity(Layer *layer, GContext *ctx) {
	if (show_inactivity) { 
		int32_t activity_level = activity_window_sum - activity_threshold / INACTIVITY_DISPLAY_THRESHOLD;
		if (activity_level < 0) activity_level = 0;
		activity_level = activity_level * INACTIVITY_WIDTH * INACTIVITY_DISPLAY_THRESHOLD / (INACTIVITY_DISPLAY_THRESHOLD - 1) / activity_threshold;
		graphics_context_set_fill_color(ctx, color_palette[C_BORDER]);
		graphics_fill_rect(ctx, GRect(0, INACTIVITY_OFFSET_Y, INACTIVITY_WIDTH, INACTIVITY_BAR_THICKNESS + INACTIVITY_BORDER * 2), 0, GCornerNone);
		if (activity_level > 0) graphics_fill_rect(ctx, GRect(0, 0, INACTIVITY_BORDER * 2 + activity_level, INACTIVITY_BORDER * 2 + INACTIVITY_LEVEL_THICKNESS), 0, GCornerNone);
		graphics_context_set_fill_color(ctx, color_palette[C_TEXT]);
		graphics_fill_rect(ctx, GRect(INACTIVITY_BORDER, INACTIVITY_OFFSET_Y + INACTIVITY_BORDER, INACTIVITY_WIDTH_NB, INACTIVITY_BAR_THICKNESS), 0, GCornerNone);
		graphics_fill_rect(ctx, GRect(INACTIVITY_BORDER, INACTIVITY_BORDER, activity_level, INACTIVITY_LEVEL_THICKNESS), 0, GCornerNone);
		graphics_context_set_fill_color(ctx, color_palette[C_BORDER]);
		graphics_fill_rect(ctx, GRect(INACTIVITY_WEIGHT_OFFSET_X, INACTIVITY_WEIGHT_OFFSET_Y - INACTIVITY_WEIGHT_SMALL_HEIGHT / 2, INACTIVITY_WEIGHT_WIDTH, INACTIVITY_WEIGHT_SMALL_HEIGHT), 0, GCornerNone);
		graphics_fill_rect(ctx, GRect(INACTIVITY_WEIGHT_OFFSET_X + INACTIVITY_WEIGHT_WIDTH + INACTIVITY_WEIGHT_GAP, INACTIVITY_WEIGHT_OFFSET_Y - INACTIVITY_WEIGHT_LARGE_HEIGHT / 2, INACTIVITY_WEIGHT_WIDTH, INACTIVITY_WEIGHT_LARGE_HEIGHT), 0, GCornerNone);
		graphics_fill_rect(ctx, GRect(INACTIVITY_WIDTH - INACTIVITY_WEIGHT_OFFSET_X - INACTIVITY_WEIGHT_WIDTH * 2 - INACTIVITY_WEIGHT_GAP, INACTIVITY_WEIGHT_OFFSET_Y - INACTIVITY_WEIGHT_LARGE_HEIGHT / 2, INACTIVITY_WEIGHT_WIDTH, INACTIVITY_WEIGHT_LARGE_HEIGHT), 0, GCornerNone);
		graphics_fill_rect(ctx, GRect(INACTIVITY_WIDTH - INACTIVITY_WEIGHT_OFFSET_X - INACTIVITY_WEIGHT_WIDTH, INACTIVITY_WEIGHT_OFFSET_Y - INACTIVITY_WEIGHT_SMALL_HEIGHT / 2, INACTIVITY_WEIGHT_WIDTH, INACTIVITY_WEIGHT_SMALL_HEIGHT), 0, GCornerNone);
		graphics_context_set_fill_color(ctx, color_palette[C_TEXT]);
		graphics_fill_rect(ctx, GRect(INACTIVITY_WEIGHT_OFFSET_X + INACTIVITY_BORDER, INACTIVITY_WEIGHT_OFFSET_Y - INACTIVITY_WEIGHT_SMALL_HEIGHT / 2 + INACTIVITY_BORDER, INACTIVITY_WEIGHT_WIDTH_NB, INACTIVITY_WEIGHT_SMALL_HEIGHT - INACTIVITY_BORDER * 2), 0, GCornerNone);
		graphics_fill_rect(ctx, GRect(INACTIVITY_WEIGHT_OFFSET_X + INACTIVITY_WEIGHT_WIDTH + INACTIVITY_WEIGHT_GAP + INACTIVITY_BORDER, INACTIVITY_WEIGHT_OFFSET_Y - INACTIVITY_WEIGHT_LARGE_HEIGHT / 2 + INACTIVITY_BORDER, INACTIVITY_WEIGHT_WIDTH_NB, INACTIVITY_WEIGHT_LARGE_HEIGHT - INACTIVITY_BORDER * 2), 0, GCornerNone);
		graphics_fill_rect(ctx, GRect(INACTIVITY_WIDTH - INACTIVITY_WEIGHT_OFFSET_X - INACTIVITY_WEIGHT_WIDTH * 2 - INACTIVITY_WEIGHT_GAP + INACTIVITY_BORDER, INACTIVITY_WEIGHT_OFFSET_Y - INACTIVITY_WEIGHT_LARGE_HEIGHT / 2 + INACTIVITY_BORDER, INACTIVITY_WEIGHT_WIDTH_NB, INACTIVITY_WEIGHT_LARGE_HEIGHT - INACTIVITY_BORDER * 2), 0, GCornerNone);
		graphics_fill_rect(ctx, GRect(INACTIVITY_WIDTH - INACTIVITY_WEIGHT_OFFSET_X - INACTIVITY_WEIGHT_WIDTH + INACTIVITY_BORDER, INACTIVITY_WEIGHT_OFFSET_Y - INACTIVITY_WEIGHT_SMALL_HEIGHT / 2 + INACTIVITY_BORDER, INACTIVITY_WEIGHT_WIDTH_NB, INACTIVITY_WEIGHT_SMALL_HEIGHT - INACTIVITY_BORDER * 2), 0, GCornerNone);
	}
}

void save_activity_state(void) {
	if ((inactivity_alert_enabled == true) && (show_battery == false)) {
		persist_write_int(STORAGE_STATE_TIME, time(NULL));
		persist_write_bool(STORAGE_INACTIVITY_SHOWN, true);
		persist_write_int(STORAGE_INACTIVITY_PERIOD, inactivity_period);
		persist_write_data(STORAGE_ACTIVITY_WINDOW, activity_window, sizeof(activity_window));
	}
}

#include "flux.h"

#define ACTIVITY_WINDOW_SIZE 5
	
static Window *window;
static Layer *background_layer;
static Layer *text_layer;
static Layer *battery_layer;
static Layer *bluetooth_layer;
static Layer *inactivity_layer;
static char time_text[] = "00:00";
static char date_text[] = "MON 11 JAN";
static GFont time_font;
static GFont date_font;
static bool show_battery = false;
static bool show_bluetooth = false;
static bool show_inactivity = false;
static bool inactivity_alert_enabled;
static uint32_t activity_this_minute = 0;
static uint32_t activity_window_sum = 0;
static uint32_t activity_window[ACTIVITY_WINDOW_SIZE] = {0};
static uint8_t current_activity_index = 0;
static uint8_t inactivity_period = 0;
static uint8_t inactivity_period_max;
static uint8_t inactivity_period_repeat;
static uint32_t activity_threshold;

static void update_time(struct tm *tick_time);
static void window_load(Window *window);
static void window_unload(Window *window);
static void draw_battery(Layer *layer, GContext *ctx);
static void draw_bluetooth(Layer *layer, GContext *ctx);
static void draw_inactivity(Layer *layer, GContext *ctx);
static void activity_handler(AccelRawData *data, uint32_t num_samples, uint64_t timestamp);
static void initialise();
static void cleanup();	

static void update_time(struct tm *tick_time) {
	if(clock_is_24h_style() == false) strftime(time_text, sizeof("00:00"), "%l:%M", tick_time);
	else strftime(time_text, sizeof("00:00"), "%k:%M", tick_time);
	layer_mark_dirty(text_layer);
}

void update_date(struct tm *tick_time) {
	if (get_setting(SETTING_DISPLAY_DATE) == 1) strftime(date_text, sizeof("MON 11 JAN"), "%a %e %b", tick_time);
	else if (get_setting(SETTING_DISPLAY_DATE) == 2) strftime(date_text, sizeof("MON JAN 11"), "%a %b %e", tick_time);
	else date_text[0] = '!';
}

void update_battery(bool show) {
	if (show_battery != show) {
		configure_inactivity_alert();
		show_battery = show;
	}
	layer_mark_dirty(battery_layer);
}

void draw_text(Layer *layer, GContext *ctx) {
	graphics_context_set_text_color(ctx, GColorBlack);
	graphics_draw_text(ctx, time_text, time_font, GRect(0, 54 - 2, 140, 40), GTextOverflowModeFill, GTextAlignmentCenter, NULL);
	graphics_draw_text(ctx, time_text, time_font, GRect(0, 54 + 2, 140, 40), GTextOverflowModeFill, GTextAlignmentCenter, NULL);
	graphics_draw_text(ctx, time_text, time_font, GRect(4, 54 - 2, 140, 40), GTextOverflowModeFill, GTextAlignmentCenter, NULL);
	graphics_draw_text(ctx, time_text, time_font, GRect(4, 54 + 2, 140, 40), GTextOverflowModeFill, GTextAlignmentCenter, NULL);
	if (date_text[0] != '!') {
		graphics_draw_text(ctx, date_text, date_font, GRect(0, 140 - 1, 142, 20), GTextOverflowModeFill, GTextAlignmentCenter, NULL);
		graphics_draw_text(ctx, date_text, date_font, GRect(0, 140 + 1, 142, 20), GTextOverflowModeFill, GTextAlignmentCenter, NULL);
		graphics_draw_text(ctx, date_text, date_font, GRect(2, 140 - 1, 142, 20), GTextOverflowModeFill, GTextAlignmentCenter, NULL);
		graphics_draw_text(ctx, date_text, date_font, GRect(2, 140 + 1, 142, 20), GTextOverflowModeFill, GTextAlignmentCenter, NULL);
	}	
	graphics_context_set_text_color(ctx, GColorWhite);
	graphics_draw_text(ctx, time_text, time_font, GRect(2, 54, 140, 40), GTextOverflowModeFill, GTextAlignmentCenter, NULL);
	if (date_text[0] != '!') graphics_draw_text(ctx, date_text, date_font, GRect(1, 140, 142, 20), GTextOverflowModeFill, GTextAlignmentCenter, NULL);
}

static void draw_battery(Layer *layer, GContext *ctx) {
	if (show_battery) { 
		uint8_t percent = battery_state_service_peek().charge_percent;
		char percent_text[] = "10%";		
		if (percent == 100) percent_text[2] = '0';
		else {
			percent_text[0] = percent / 10 + '0';
			percent_text[1] = percent % 10 + '0';
		}
		graphics_context_set_fill_color(ctx, GColorBlack);
		graphics_fill_rect(ctx, GRect(0, 0, 48, 21), 0, GCornerNone);
		graphics_fill_rect(ctx, GRect(48, 6, 2, 9), 0, GCornerNone);
		graphics_context_set_fill_color(ctx, GColorWhite);
		graphics_fill_rect(ctx, GRect(1, 1, 46, 19), 0, GCornerNone);
		graphics_fill_rect(ctx, GRect(47, 7, 2, 7), 0, GCornerNone);
		graphics_context_set_fill_color(ctx, GColorBlack);
		graphics_fill_rect(ctx, GRect(3, 3, 43 * (100 - percent) / 100, 15), 0, GCornerNone);
		graphics_context_set_text_color(ctx, GColorBlack);
		graphics_draw_text(ctx, percent_text, date_font, GRect(3, -3, 44, 20), GTextOverflowModeFill, GTextAlignmentCenter, NULL);
		graphics_draw_text(ctx, percent_text, date_font, GRect(3, -1, 44, 20), GTextOverflowModeFill, GTextAlignmentCenter, NULL);
		graphics_draw_text(ctx, percent_text, date_font, GRect(5, -3, 44, 20), GTextOverflowModeFill, GTextAlignmentCenter, NULL);
		graphics_draw_text(ctx, percent_text, date_font, GRect(5, -1, 44, 20), GTextOverflowModeFill, GTextAlignmentCenter, NULL);
		graphics_context_set_text_color(ctx, GColorWhite);
		graphics_draw_text(ctx, percent_text, date_font, GRect(4, -2, 44, 20), GTextOverflowModeFill, GTextAlignmentCenter, NULL);
	}
}

static void draw_bluetooth(Layer *layer, GContext *ctx) {
	if (show_bluetooth) { 
		graphics_context_set_fill_color(ctx, GColorBlack);
		graphics_fill_rect(ctx, GRect(0, 4, 21, 40), 0, GCornerNone);
		graphics_context_set_fill_color(ctx, GColorWhite);
		graphics_fill_rect(ctx, GRect(1, 5, 19, 38), 0, GCornerNone);
		graphics_context_set_fill_color(ctx, GColorBlack);
		graphics_fill_rect(ctx, GRect(3, 11, 15, 26), 0, GCornerNone);
		graphics_fill_rect(ctx, GRect(9, 39, 3, 2), 0, GCornerNone);
		graphics_context_set_antialiased(ctx, false);
		graphics_context_set_stroke_color(ctx, GColorBlack);
		graphics_context_set_stroke_width(ctx, 5);
		graphics_draw_line(ctx, GPoint(2, 2), GPoint(19, 46));
		graphics_context_set_stroke_color(ctx, GColorWhite);
		graphics_context_set_stroke_width(ctx, 1);
		graphics_draw_line(ctx, GPoint(2, 2), GPoint(19, 46));
	}
}

static void draw_inactivity(Layer *layer, GContext *ctx) {
	if (show_inactivity) { 
		int32_t activity_level = activity_window_sum - activity_threshold / 3;
		if (activity_level < 0) activity_level = 0;
		activity_level = activity_level * 60 * 3 / 2 / activity_threshold;
		graphics_context_set_fill_color(ctx, GColorBlack);
		graphics_fill_rect(ctx, GRect(0, 10, 62, 4), 0, GCornerNone);
		graphics_fill_rect(ctx, GRect(0, 0, 2 + activity_level, 3), 0, GCornerNone);
		graphics_context_set_fill_color(ctx, GColorWhite);
		graphics_fill_rect(ctx, GRect(1, 11, 60, 2), 0, GCornerNone);
		graphics_fill_rect(ctx, GRect(1, 1, activity_level, 1), 0, GCornerNone);
		graphics_context_set_fill_color(ctx, GColorBlack);
		graphics_fill_rect(ctx, GRect(2, 7, 5, 10), 0, GCornerNone);
		graphics_fill_rect(ctx, GRect(6, 5, 5, 14), 0, GCornerNone);
		graphics_fill_rect(ctx, GRect(51, 5, 5, 14), 0, GCornerNone);
		graphics_fill_rect(ctx, GRect(55, 7, 5, 10), 0, GCornerNone);
		graphics_context_set_fill_color(ctx, GColorWhite);
		graphics_fill_rect(ctx, GRect(3, 8, 3, 8), 0, GCornerNone);
		graphics_fill_rect(ctx, GRect(7, 6, 3, 12), 0, GCornerNone);
		graphics_fill_rect(ctx, GRect(52, 6, 3, 12), 0, GCornerNone);
		graphics_fill_rect(ctx, GRect(56, 8, 3, 8), 0, GCornerNone);
	}
}

void configure_inactivity_alert() {
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
	if (inactivity_period_repeat > 0) inactivity_period_repeat = inactivity_period_max;
	layer_mark_dirty(inactivity_layer);
}

static void check_activity() {
	if (++current_activity_index >= ACTIVITY_WINDOW_SIZE) current_activity_index = 0;
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

static void activity_handler(AccelRawData *data, uint32_t num_samples, uint64_t timestamp) {
	if (num_samples != 26) APP_LOG(APP_LOG_LEVEL_ERROR, "samples: %d", (int)num_samples);
	// TODO: if num_samples is constant, unroll for loop, use macros
	for (uint32_t i = 1; i < num_samples; i++) {
		int32_t diff_x = data[i].x - data[i-1].x;
		int32_t diff_y = data[i].y - data[i-1].y;
		int32_t diff_z = data[i].z - data[i-1].z;
#define abs(x) ((x) < 0 ? -(x) : (x))
		activity_this_minute += abs(diff_x) + abs(diff_y) + abs(diff_z);
	}
}

void bluetooth_handler(bool connected) {
	show_bluetooth = !connected && get_setting(SETTING_BLUETOOTH_ICON);
	if (!connected && get_setting(SETTING_BLUETOOTH_VIBRATE)) vibes_short_pulse();
	layer_mark_dirty(bluetooth_layer);
}

void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
	animation_tick_handler(tick_time);
	if (units_changed & DAY_UNIT) update_date(tick_time);
	if (units_changed & MINUTE_UNIT) {
	   	update_time(tick_time);
		if (inactivity_alert_enabled) check_activity();
	}
}

void background_layer_mark_dirty() {
	layer_mark_dirty(background_layer);
}

static void window_load(Window *window) {
	window_set_background_color(window, GColorBlack);
	background_layer = layer_create(GRect(0, 0, 144, 168));
	text_layer = layer_create(GRect(0, 0, 144, 168));
	battery_layer = layer_create(GRect(8, 8, 50, 21));
	bluetooth_layer = layer_create(GRect(144 - 21 - 8, 4, 21, 50));
	inactivity_layer = layer_create(GRect((144 - 62) / 2, 110, 62, 19));
	create_nodes();
	time_t raw_time = time(NULL);
	struct tm *tick_time = localtime(&raw_time);
	update_time(tick_time);
	update_date(tick_time);
	time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_IMAGINE_38));
	date_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_IMAGINE_18));
	layer_set_update_proc(background_layer, draw_nodes);
	layer_set_update_proc(text_layer, draw_text);
	layer_set_update_proc(battery_layer, draw_battery);
	layer_set_update_proc(bluetooth_layer, draw_bluetooth);
	layer_set_update_proc(inactivity_layer, draw_inactivity);
	layer_add_child(window_get_root_layer(window), background_layer);
	layer_add_child(window_get_root_layer(window), text_layer);
	layer_add_child(window_get_root_layer(window), battery_layer);
	layer_add_child(window_get_root_layer(window), bluetooth_layer);
	layer_add_child(window_get_root_layer(window), inactivity_layer);
}

static void window_unload(Window *window) {
	layer_destroy(background_layer);
	layer_destroy(text_layer);
	layer_destroy(battery_layer);
	layer_destroy(bluetooth_layer);
	layer_destroy(inactivity_layer);
	fonts_unload_custom_font(time_font);
	fonts_unload_custom_font(date_font);
}

static void load_state() {
	if ((inactivity_alert_enabled == true) && (show_battery == false)) {
		if (time(NULL) < persist_read_int(STORAGE_STATE_TIME) + 60 * 60) {
			show_inactivity = persist_read_bool(STORAGE_INACTIVITY_SHOWN);
			inactivity_period = persist_read_int(STORAGE_INACTIVITY_PERIOD);
			persist_read_data(STORAGE_ACTIVITY_WINDOW, activity_window, sizeof(activity_window));
			for (uint8_t i = 0; i < ACTIVITY_WINDOW_SIZE; i++) activity_window_sum += activity_window[i];
		}
	}
}

static void save_state() {
	if ((inactivity_alert_enabled == true) && (show_battery == false)) {
		persist_write_int(STORAGE_STATE_TIME, time(NULL));
		persist_write_bool(STORAGE_INACTIVITY_SHOWN, true);
		persist_write_int(STORAGE_INACTIVITY_PERIOD, inactivity_period);
		persist_write_data(STORAGE_ACTIVITY_WINDOW, activity_window, sizeof(activity_window));
	}
}

static void initialise() {
	sync_settings();
	window = window_create();
	window_set_window_handlers(window, (WindowHandlers) {
		.load = window_load,
		.unload = window_unload
	});
	window_stack_push(window, true);
	bluetooth_connection_service_subscribe(bluetooth_handler);
	configure_inactivity_alert();
	begin_startup_animation();
	load_state();
}

static void cleanup() {
	wakeup_cancel_all();
	tick_timer_service_unsubscribe();
	battery_state_service_unsubscribe();
	bluetooth_connection_service_unsubscribe();
	window_destroy(window);
	save_settings();
	save_state();
}

int main(void) {
	initialise();
	app_event_loop();
	cleanup();
}

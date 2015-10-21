#include "flux.h"

#define WINDOW_WIDTH 144
#define WINDOW_HEIGHT 168
#define TIME_POS_X 0
#define TIME_POS_Y 50
#define TIME_WIDTH WINDOW_WIDTH
#define TIME_HEIGHT 44
#define TIME_BORDER 2
#define DATE_POS_X 0
#define DATE_POS_Y 139
#define DATE_WIDTH 142
#define DATE_HEIGHT 20
#define DATE_BORDER 1
#define BATTERY_POS_X 8
#define BATTERY_POS_Y 8
#define BATTERY_WIDTH 46
#define BATTERY_HEIGHT 21
#define BATTERY_THICKNESS 2
#define BATTERY_BUMP_POS_Y 6
#define BATTERY_BUMP_WIDTH 2
#define BATTERY_BUMP_HEIGHT 9
#define BATTERY_BORDER 1
#define BATTERY_TEXT_POS_Y -3
#define BATTERY_TEXT_HEIGHT 20
#define BATTERY_TEXT_BORDER 1
#define BATTERY_WIDTH_NB (BATTERY_WIDTH - BATTERY_BORDER * 2)
#define BATTERY_HEIGHT_NB (BATTERY_HEIGHT - BATTERY_BORDER * 2)
#define BATTERY_BUMP_HEIGHT_NB (BATTERY_BUMP_HEIGHT - BATTERY_BORDER * 2)
#define BLUETOOTH_POS_Y 4
#define BLUETOOTH_WIDTH 21
#define BLUETOOTH_HEIGHT 50
#define BLUETOOTH_OFFSET_Y 4
#define BLUETOOTH_THICKNESS_X 2
#define BLUETOOTH_THICKNESS_Y 6
#define BLUETOOTH_BORDER 1
#define BLUETOOTH_BUTTON_WIDTH 3
#define BLUETOOTH_BUTTON_HEIGHT 2
#define BLUETOOTH_LINE_BORDER 5
#define BLUETOOTH_LINE_STROKE 1
#define BLUETOOTH_LINE_OFFSET 2
#define BLUETOOTH_POS_X (WINDOW_WIDTH - BLUETOOTH_WIDTH - BLUETOOTH_POS_Y * 2)
#define BLUETOOTH_BUTTON_POS_X ((BLUETOOTH_WIDTH - BLUETOOTH_BUTTON_WIDTH) / 2)
#define BLUETOOTH_BUTTON_POS_Y (BLUETOOTH_HEIGHT - BLUETOOTH_BORDER - BLUETOOTH_OFFSET_Y - BLUETOOTH_THICKNESS_Y / 2 - BLUETOOTH_BUTTON_HEIGHT / 2)
#define BLUETOOTH_WIDTH_NB (BLUETOOTH_WIDTH - BLUETOOTH_BORDER * 2)
#define BLUETOOTH_HEIGHT_NBO (BLUETOOTH_HEIGHT - BLUETOOTH_BORDER * 2 - BLUETOOTH_OFFSET_Y * 2)
#define ACTIVITY_WINDOW_SIZE 5
#define ACTIVITY_DISPLAY_THRESHOLD 3
#define ACTIVITY_POS_Y 110
#define ACTIVITY_WIDTH 62
#define ACTIVITY_HEIGHT 19
#define ACTIVITY_OFFSET_Y 10
#define ACTIVITY_BAR_THICKNESS 2
#define ACTIVITY_BORDER 1
#define ACTIVITY_LEVEL_THICKNESS 1
#define ACTIVITY_WEIGHT_SMALL_HEIGHT 10
#define ACTIVITY_WEIGHT_LARGE_HEIGHT 14
#define ACTIVITY_WEIGHT_WIDTH 5
#define ACTIVITY_WEIGHT_GAP -1
#define ACTIVITY_POS_X ((WINDOW_WIDTH - ACTIVITY_WIDTH) / 2)
#define ACTIVITY_WEIGHT_OFFSET_X (1 + ACTIVITY_BORDER)
#define ACTIVITY_WEIGHT_OFFSET_Y (ACTIVITY_OFFSET_Y + ACTIVITY_BAR_THICKNESS)
#define ACTIVITY_WIDTH_NB (ACTIVITY_WIDTH - ACTIVITY_BORDER * 2)
#define ACTIVITY_WEIGHT_WIDTH_NB (ACTIVITY_WEIGHT_WIDTH - ACTIVITY_BORDER * 2)
#define graphics_draw_text_border(ctx, text, font, x, y, width, height, border) { \
	graphics_draw_text((ctx), (text), (font), GRect((x), (y), (width), (height)), GTextOverflowModeFill, GTextAlignmentCenter, NULL); \
	graphics_draw_text((ctx), (text), (font), GRect((x), (y) + (border) * 2, (width), (height)), GTextOverflowModeFill, GTextAlignmentCenter, NULL); \
	graphics_draw_text((ctx), (text), (font), GRect((x) + (border) * 2, (y), (width), (height)), GTextOverflowModeFill, GTextAlignmentCenter, NULL); \
	graphics_draw_text((ctx), (text), (font), GRect((x) + (border) * 2, (y) + (border) * 2, (width), (height)), GTextOverflowModeFill, GTextAlignmentCenter, NULL); }
	
static Window *window;
static Layer *background_layer;
static Layer *time_layer;
static Layer *date_layer;
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
	layer_mark_dirty(time_layer);
}

void update_date(struct tm *tick_time) {
	if (get_setting(SETTING_DISPLAY_DATE) == 1) strftime(date_text, sizeof("MON 11 JAN"), "%a %e %b", tick_time);
	else if (get_setting(SETTING_DISPLAY_DATE) == 2) strftime(date_text, sizeof("MON JAN 11"), "%a %b %e", tick_time);
	else date_text[0] = '!';
	layer_mark_dirty(date_layer);
}

void update_battery(bool show) {
	if (show_battery != show) {
		configure_inactivity_alert();
		show_battery = show;
	}
	layer_mark_dirty(battery_layer);
}

void draw_time(Layer *layer, GContext *ctx) {
	graphics_context_set_text_color(ctx, GColorBlack);
	graphics_draw_text_border(ctx, time_text, time_font, 0, 0, TIME_WIDTH, TIME_HEIGHT, TIME_BORDER);
	graphics_context_set_text_color(ctx, GColorWhite);
	graphics_draw_text(ctx, time_text, time_font, GRect(TIME_BORDER, TIME_BORDER, TIME_WIDTH, TIME_HEIGHT), GTextOverflowModeFill, GTextAlignmentCenter, NULL);
}

void draw_date(Layer *layer, GContext *ctx) {
	if (date_text[0] != '!') {
		graphics_context_set_text_color(ctx, GColorBlack);
		graphics_draw_text_border(ctx, date_text, date_font, 0, 0, DATE_WIDTH, DATE_HEIGHT, DATE_BORDER);
		graphics_context_set_text_color(ctx, GColorWhite);
		graphics_draw_text(ctx, date_text, date_font, GRect(DATE_BORDER, DATE_BORDER, DATE_WIDTH, DATE_HEIGHT), GTextOverflowModeFill, GTextAlignmentCenter, NULL);
	}
}

static void draw_battery(Layer *layer, GContext *ctx) {
	if (show_battery) { 
		uint8_t percent = battery_state_service_peek().charge_percent;
		char battery_text[] = "10%";		
		if (percent == 100) battery_text[2] = '0';
		else {
			battery_text[0] = percent / 10 + '0';
			battery_text[1] = percent % 10 + '0';
		}
		graphics_context_set_fill_color(ctx, GColorBlack);
		graphics_fill_rect(ctx, GRect(0, 0, BATTERY_WIDTH, BATTERY_HEIGHT), 0, GCornerNone);
		graphics_fill_rect(ctx, GRect(BATTERY_WIDTH, BATTERY_BUMP_POS_Y, BATTERY_BUMP_WIDTH, BATTERY_BUMP_HEIGHT), 0, GCornerNone);
		graphics_context_set_fill_color(ctx, GColorWhite);
		graphics_fill_rect(ctx, GRect(BATTERY_BORDER, BATTERY_BORDER, BATTERY_WIDTH_NB, BATTERY_HEIGHT_NB), 0, GCornerNone);
		graphics_fill_rect(ctx, GRect(BATTERY_WIDTH - BATTERY_BORDER, BATTERY_BUMP_POS_Y + BATTERY_BORDER, BATTERY_BUMP_WIDTH, BATTERY_BUMP_HEIGHT_NB), 0, GCornerNone);
		graphics_context_set_fill_color(ctx, GColorBlack);
		graphics_fill_rect(ctx, GRect(BATTERY_BORDER + BATTERY_THICKNESS, BATTERY_BORDER + BATTERY_THICKNESS, (BATTERY_WIDTH_NB - BATTERY_THICKNESS * 2) * (100 - percent) / 100, BATTERY_HEIGHT_NB - BATTERY_THICKNESS * 2), 0, GCornerNone);
		graphics_context_set_text_color(ctx, GColorBlack);
		graphics_draw_text_border(ctx, battery_text, date_font, 0, BATTERY_TEXT_POS_Y, BATTERY_WIDTH, BATTERY_TEXT_HEIGHT, BATTERY_TEXT_BORDER);
		graphics_context_set_text_color(ctx, GColorWhite);
		graphics_draw_text(ctx, battery_text, date_font, GRect(BATTERY_TEXT_BORDER, BATTERY_TEXT_POS_Y + BATTERY_TEXT_BORDER, BATTERY_WIDTH, BATTERY_TEXT_HEIGHT), GTextOverflowModeFill, GTextAlignmentCenter, NULL);
	}
}

static void draw_bluetooth(Layer *layer, GContext *ctx) {
	if (show_bluetooth) { 
		graphics_context_set_fill_color(ctx, GColorBlack);
		graphics_fill_rect(ctx, GRect(0, BLUETOOTH_OFFSET_Y, BLUETOOTH_WIDTH, BLUETOOTH_HEIGHT - BLUETOOTH_OFFSET_Y * 2), 0, GCornerNone);
		graphics_context_set_fill_color(ctx, GColorWhite);
		graphics_fill_rect(ctx, GRect(BLUETOOTH_BORDER, BLUETOOTH_OFFSET_Y + BLUETOOTH_BORDER, BLUETOOTH_WIDTH_NB, BLUETOOTH_HEIGHT_NBO), 0, GCornerNone);
		graphics_context_set_fill_color(ctx, GColorBlack);
		graphics_fill_rect(ctx, GRect(BLUETOOTH_BORDER + BLUETOOTH_THICKNESS_X, BLUETOOTH_OFFSET_Y + BLUETOOTH_BORDER + BLUETOOTH_THICKNESS_Y, BLUETOOTH_WIDTH_NB - BLUETOOTH_THICKNESS_X * 2, BLUETOOTH_HEIGHT_NBO - BLUETOOTH_THICKNESS_Y * 2), 0, GCornerNone);
		graphics_fill_rect(ctx, GRect(BLUETOOTH_BUTTON_POS_X, BLUETOOTH_BUTTON_POS_Y, BLUETOOTH_BUTTON_WIDTH, BLUETOOTH_BUTTON_HEIGHT), 0, GCornerNone);
		graphics_context_set_antialiased(ctx, false);
		graphics_context_set_stroke_color(ctx, GColorBlack);
		graphics_context_set_stroke_width(ctx, BLUETOOTH_LINE_BORDER);
		graphics_draw_line(ctx, GPoint(BLUETOOTH_LINE_OFFSET, BLUETOOTH_LINE_OFFSET), GPoint(BLUETOOTH_WIDTH - BLUETOOTH_LINE_OFFSET * 2 + BLUETOOTH_LINE_STROKE, BLUETOOTH_HEIGHT - BLUETOOTH_LINE_OFFSET * 2 + BLUETOOTH_LINE_STROKE));
		graphics_context_set_stroke_color(ctx, GColorWhite);
		graphics_context_set_stroke_width(ctx, BLUETOOTH_LINE_STROKE);
		graphics_draw_line(ctx, GPoint(BLUETOOTH_LINE_OFFSET, BLUETOOTH_LINE_OFFSET), GPoint(BLUETOOTH_WIDTH - BLUETOOTH_LINE_OFFSET * 2 + BLUETOOTH_LINE_STROKE, BLUETOOTH_HEIGHT - BLUETOOTH_LINE_OFFSET * 2 + BLUETOOTH_LINE_STROKE));
	}
}

static void draw_inactivity(Layer *layer, GContext *ctx) {
	if (show_inactivity) { 
		int32_t activity_level = activity_window_sum - activity_threshold / ACTIVITY_DISPLAY_THRESHOLD;
		if (activity_level < 0) activity_level = 0;
		activity_level = activity_level * ACTIVITY_WIDTH * ACTIVITY_DISPLAY_THRESHOLD / (ACTIVITY_DISPLAY_THRESHOLD - 1) / activity_threshold;
		graphics_context_set_fill_color(ctx, GColorBlack);
		graphics_fill_rect(ctx, GRect(0, ACTIVITY_OFFSET_Y, ACTIVITY_WIDTH, ACTIVITY_BAR_THICKNESS + ACTIVITY_BORDER * 2), 0, GCornerNone);
		if (activity_level > 0) graphics_fill_rect(ctx, GRect(0, 0, ACTIVITY_BORDER * 2 + activity_level, ACTIVITY_BORDER * 2 + ACTIVITY_LEVEL_THICKNESS), 0, GCornerNone);
		graphics_context_set_fill_color(ctx, GColorWhite);
		graphics_fill_rect(ctx, GRect(ACTIVITY_BORDER, ACTIVITY_OFFSET_Y + ACTIVITY_BORDER, ACTIVITY_WIDTH_NB, ACTIVITY_BAR_THICKNESS), 0, GCornerNone);
		graphics_fill_rect(ctx, GRect(ACTIVITY_BORDER, ACTIVITY_BORDER, activity_level, ACTIVITY_LEVEL_THICKNESS), 0, GCornerNone);
		graphics_context_set_fill_color(ctx, GColorBlack);
		graphics_fill_rect(ctx, GRect(ACTIVITY_WEIGHT_OFFSET_X, ACTIVITY_WEIGHT_OFFSET_Y - ACTIVITY_WEIGHT_SMALL_HEIGHT / 2, ACTIVITY_WEIGHT_WIDTH, ACTIVITY_WEIGHT_SMALL_HEIGHT), 0, GCornerNone);
		graphics_fill_rect(ctx, GRect(ACTIVITY_WEIGHT_OFFSET_X + ACTIVITY_WEIGHT_WIDTH + ACTIVITY_WEIGHT_GAP, ACTIVITY_WEIGHT_OFFSET_Y - ACTIVITY_WEIGHT_LARGE_HEIGHT / 2, ACTIVITY_WEIGHT_WIDTH, ACTIVITY_WEIGHT_LARGE_HEIGHT), 0, GCornerNone);
		graphics_fill_rect(ctx, GRect(ACTIVITY_WIDTH - ACTIVITY_WEIGHT_OFFSET_X - ACTIVITY_WEIGHT_WIDTH * 2 - ACTIVITY_WEIGHT_GAP, ACTIVITY_WEIGHT_OFFSET_Y - ACTIVITY_WEIGHT_LARGE_HEIGHT / 2, ACTIVITY_WEIGHT_WIDTH, ACTIVITY_WEIGHT_LARGE_HEIGHT), 0, GCornerNone);
		graphics_fill_rect(ctx, GRect(ACTIVITY_WIDTH - ACTIVITY_WEIGHT_OFFSET_X - ACTIVITY_WEIGHT_WIDTH, ACTIVITY_WEIGHT_OFFSET_Y - ACTIVITY_WEIGHT_SMALL_HEIGHT / 2, ACTIVITY_WEIGHT_WIDTH, ACTIVITY_WEIGHT_SMALL_HEIGHT), 0, GCornerNone);
		graphics_context_set_fill_color(ctx, GColorWhite);
		graphics_fill_rect(ctx, GRect(ACTIVITY_WEIGHT_OFFSET_X + ACTIVITY_BORDER, ACTIVITY_WEIGHT_OFFSET_Y - ACTIVITY_WEIGHT_SMALL_HEIGHT / 2 + ACTIVITY_BORDER, ACTIVITY_WEIGHT_WIDTH_NB, ACTIVITY_WEIGHT_SMALL_HEIGHT - ACTIVITY_BORDER * 2), 0, GCornerNone);
		graphics_fill_rect(ctx, GRect(ACTIVITY_WEIGHT_OFFSET_X + ACTIVITY_WEIGHT_WIDTH + ACTIVITY_WEIGHT_GAP + ACTIVITY_BORDER, ACTIVITY_WEIGHT_OFFSET_Y - ACTIVITY_WEIGHT_LARGE_HEIGHT / 2 + ACTIVITY_BORDER, ACTIVITY_WEIGHT_WIDTH_NB, ACTIVITY_WEIGHT_LARGE_HEIGHT - ACTIVITY_BORDER * 2), 0, GCornerNone);
		graphics_fill_rect(ctx, GRect(ACTIVITY_WIDTH - ACTIVITY_WEIGHT_OFFSET_X - ACTIVITY_WEIGHT_WIDTH * 2 - ACTIVITY_WEIGHT_GAP + ACTIVITY_BORDER, ACTIVITY_WEIGHT_OFFSET_Y - ACTIVITY_WEIGHT_LARGE_HEIGHT / 2 + ACTIVITY_BORDER, ACTIVITY_WEIGHT_WIDTH_NB, ACTIVITY_WEIGHT_LARGE_HEIGHT - ACTIVITY_BORDER * 2), 0, GCornerNone);
		graphics_fill_rect(ctx, GRect(ACTIVITY_WIDTH - ACTIVITY_WEIGHT_OFFSET_X - ACTIVITY_WEIGHT_WIDTH + ACTIVITY_BORDER, ACTIVITY_WEIGHT_OFFSET_Y - ACTIVITY_WEIGHT_SMALL_HEIGHT / 2 + ACTIVITY_BORDER, ACTIVITY_WEIGHT_WIDTH_NB, ACTIVITY_WEIGHT_SMALL_HEIGHT - ACTIVITY_BORDER * 2), 0, GCornerNone);
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
	background_layer = layer_create(GRect(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT));
	time_layer = layer_create(GRect(0, TIME_POS_Y, WINDOW_WIDTH, WINDOW_HEIGHT));
	date_layer = layer_create(GRect(0, DATE_POS_Y, WINDOW_WIDTH, WINDOW_HEIGHT));
	battery_layer = layer_create(GRect(BATTERY_POS_X, BATTERY_POS_Y, BATTERY_WIDTH + BATTERY_BUMP_WIDTH, BATTERY_HEIGHT));
	bluetooth_layer = layer_create(GRect(BLUETOOTH_POS_X, BLUETOOTH_POS_Y, BLUETOOTH_WIDTH, BLUETOOTH_HEIGHT));
	inactivity_layer = layer_create(GRect(ACTIVITY_POS_X, ACTIVITY_POS_Y, ACTIVITY_WIDTH, ACTIVITY_HEIGHT));
	create_nodes();
	time_t raw_time = time(NULL);
	struct tm *tick_time = localtime(&raw_time);
	update_time(tick_time);
	update_date(tick_time);
	time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_IMAGINE_38));
	date_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_IMAGINE_18));
	layer_set_update_proc(background_layer, draw_nodes);
	layer_set_update_proc(time_layer, draw_time);
	layer_set_update_proc(date_layer, draw_date);
	layer_set_update_proc(battery_layer, draw_battery);
	layer_set_update_proc(bluetooth_layer, draw_bluetooth);
	layer_set_update_proc(inactivity_layer, draw_inactivity);
	layer_add_child(window_get_root_layer(window), background_layer);
	layer_add_child(window_get_root_layer(window), time_layer);
	layer_add_child(window_get_root_layer(window), date_layer);
	layer_add_child(window_get_root_layer(window), battery_layer);
	layer_add_child(window_get_root_layer(window), bluetooth_layer);
	layer_add_child(window_get_root_layer(window), inactivity_layer);
}

static void window_unload(Window *window) {
	layer_destroy(background_layer);
	layer_destroy(time_layer);
	layer_destroy(date_layer);
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

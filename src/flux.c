#include "flux.h"

static Window *window;
static TextLayer *time_layer;
static TextLayer *date_layer;
static GFont time_font;
static GFont date_font;

static void update_time(struct tm *tick_time);
static void window_load(Window *window);
static void window_unload(Window *window);
static void initialise();
static void cleanup();	
	
static void update_time(struct tm *tick_time) {
	static char time_buffer[] = "00:00";
	if(clock_is_24h_style() == false) strftime(time_buffer, sizeof("00:00"), "%l:%M", tick_time);
	else strftime(time_buffer, sizeof("00:00"), "%k:%M", tick_time);
	text_layer_set_text(time_layer, time_buffer);
}

void update_date(struct tm *tick_time) {
	static char date_buffer[] = "MON 11 JAN";
	uint8_t date_setting = get_setting(SETTING_DATE);
	if (date_setting == 0) text_layer_set_text(date_layer, " ");
	else {
		if (date_setting == 1) strftime(date_buffer, sizeof("MON 11 JAN"), "%a %e %b", tick_time);
		else strftime(date_buffer, sizeof("MON JAN 11"), "%a %b %e", tick_time);
		text_layer_set_text(date_layer, date_buffer);
	}
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
	update_time(tick_time);
	if (units_changed & DAY_UNIT) update_date(tick_time);
}

static void window_load(Window *window) {
	window_set_background_color(window, GColorBlack);
	time_layer = text_layer_create(GRect(0, 54, 144, 40));
	date_layer = text_layer_create(GRect(0, 94, 144, 20));
	time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_IMAGINE_38));
	date_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_IMAGINE_18));
	text_layer_set_font(time_layer, time_font);
	text_layer_set_font(date_layer, date_font);
	text_layer_set_text_color(time_layer, GColorWhite);
	text_layer_set_text_color(date_layer, GColorWhite);
	text_layer_set_background_color(time_layer, GColorClear);
	text_layer_set_background_color(date_layer, GColorClear);
	text_layer_set_text_alignment(time_layer, GTextAlignmentCenter);
	text_layer_set_text_alignment(date_layer, GTextAlignmentCenter);
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(time_layer));
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(date_layer));
	time_t raw_time = time(NULL);
	struct tm *tick_time = localtime(&raw_time);
	update_time(tick_time);
	update_date(tick_time);
}

static void window_unload(Window *window) {
	text_layer_destroy(time_layer);
	text_layer_destroy(date_layer);
	fonts_unload_custom_font(time_font);
	fonts_unload_custom_font(date_font);
}

static void initialise() {
	sync_settings();
	window = window_create();
	window_set_window_handlers(window, (WindowHandlers) {
		.load = window_load,
		.unload = window_unload
	});
	tick_timer_service_subscribe(MINUTE_UNIT || DAY_UNIT, tick_handler);
	window_stack_push(window, true);
}

static void cleanup() {
	tick_timer_service_unsubscribe();
	window_destroy(window);
	save_settings();
}

int main(void) {
	initialise();
	app_event_loop();
	cleanup();
}
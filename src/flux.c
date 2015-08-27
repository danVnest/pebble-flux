#include "flux.h"
	
static Window *window;
static Layer *background_layer;
static Layer *text_layer;
static Layer *battery_layer;
static Layer *bluetooth_layer;
static char time_text[] = "00:00";
static char date_text[] = "MON 11 JAN";
static GFont time_font;
static GFont date_font;
static bool show_battery;
static bool show_bluetooth;
static void update_time(struct tm *tick_time);
static void window_load(Window *window);
static void window_unload(Window *window);
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
	update_time(tick_time);
}

void update_battery(bool show) {
	show_battery = show;
	layer_mark_dirty(battery_layer);
}

void draw_text(Layer *layer, GContext *ctx) {
	graphics_context_set_text_color(ctx, GColorBlack);
	graphics_draw_text(ctx, time_text, time_font, GRect(0, 54 - 2, 140, 40), GTextOverflowModeFill, GTextAlignmentCenter, NULL);
	graphics_draw_text(ctx, time_text, time_font, GRect(0, 54 + 2, 140, 40), GTextOverflowModeFill, GTextAlignmentCenter, NULL);
	graphics_draw_text(ctx, time_text, time_font, GRect(4, 54 - 2, 140, 40), GTextOverflowModeFill, GTextAlignmentCenter, NULL);
	graphics_draw_text(ctx, time_text, time_font, GRect(4, 54 + 2, 140, 40), GTextOverflowModeFill, GTextAlignmentCenter, NULL);
	if (get_setting(SETTING_DISPLAY_DATE) != 0) {
		graphics_draw_text(ctx, date_text, date_font, GRect(0, 140 - 1, 142, 20), GTextOverflowModeFill, GTextAlignmentCenter, NULL);
		graphics_draw_text(ctx, date_text, date_font, GRect(0, 140 + 1, 142, 20), GTextOverflowModeFill, GTextAlignmentCenter, NULL);
		graphics_draw_text(ctx, date_text, date_font, GRect(2, 140 - 1, 142, 20), GTextOverflowModeFill, GTextAlignmentCenter, NULL);
		graphics_draw_text(ctx, date_text, date_font, GRect(2, 140 + 1, 142, 20), GTextOverflowModeFill, GTextAlignmentCenter, NULL);
	}	
	graphics_context_set_text_color(ctx, GColorWhite);
	graphics_draw_text(ctx, time_text, time_font, GRect(2, 54, 140, 40), GTextOverflowModeFill, GTextAlignmentCenter, NULL);
	if (get_setting(SETTING_DISPLAY_DATE) != 0) graphics_draw_text(ctx, date_text, date_font, GRect(1, 140, 142, 20), GTextOverflowModeFill, GTextAlignmentCenter, NULL);
}

void draw_battery(Layer *layer, GContext *ctx) {
	if (show_battery) { 
		uint8_t percent = battery_state_service_peek().charge_percent;
		char percent_text[] = "00%";		
		percent_text[0] = percent / 10 + '0';
		percent_text[1] = percent % 10 + '0';
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

void draw_bluetooth(Layer *layer, GContext *ctx) {
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

void bluetooth_handler(bool connected) {
	show_bluetooth = !connected && get_setting(SETTING_BLUETOOTH_ICON);
	if (!connected && get_setting(SETTING_BLUETOOTH_VIBRATE)) vibes_short_pulse();
	layer_mark_dirty(bluetooth_layer);
}

void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
	animation_tick_handler(tick_time);
	if (units_changed & DAY_UNIT) update_date(tick_time);
	else if (units_changed & MINUTE_UNIT) update_time(tick_time);
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
	layer_add_child(window_get_root_layer(window), background_layer);
	layer_add_child(window_get_root_layer(window), text_layer);
	layer_add_child(window_get_root_layer(window), battery_layer);
	layer_add_child(window_get_root_layer(window), bluetooth_layer);
}

static void window_unload(Window *window) {
	layer_destroy(background_layer);
	layer_destroy(text_layer);
	layer_destroy(battery_layer);
	layer_destroy(bluetooth_layer);
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
	window_stack_push(window, true);
	bluetooth_connection_service_subscribe(bluetooth_handler);
	begin_startup_animation();
}

static void cleanup() {
	wakeup_cancel_all();
	tick_timer_service_unsubscribe();
	battery_state_service_unsubscribe();
	bluetooth_connection_service_unsubscribe();
	window_destroy(window);
	save_settings();
}

int main(void) {
	initialise();
	app_event_loop();
	cleanup();
}

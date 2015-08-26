#include "flux.h"
	
static Window *window;
static Layer *background_layer;
static Layer *text_layer;
static char time_text[] = "00:00";
static char date_text[] = "MON 11 JAN";
static GFont time_font;
static GFont date_font;
bool low_power_mode_enabled = false;

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

void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
	if (low_power_mode_enabled == false) animation_tick_handler(tick_time);
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
	create_nodes();
	time_t raw_time = time(NULL);
	struct tm *tick_time = localtime(&raw_time);
	update_time(tick_time);
	update_date(tick_time);
	time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_IMAGINE_38));
	date_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_IMAGINE_18));
	layer_set_update_proc(background_layer, draw_nodes);
	layer_set_update_proc(text_layer, draw_text);
	layer_add_child(window_get_root_layer(window), background_layer);
	layer_add_child(window_get_root_layer(window), text_layer);
}

static void window_unload(Window *window) {
	layer_destroy(background_layer);
	layer_destroy(text_layer);
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
	//configure_low_power_mode();
	//begin_startup_animation(low_power_mode_enabled);
	begin_startup_animation();
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

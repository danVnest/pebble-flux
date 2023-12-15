#include "clock.h"

static Layer *time_layer;
static Layer *date_layer;
static char time_text[] = "00:00";
static char date_text[] = "MON 11 JAN";
static GFont time_font;
static GFont date_font;

static void draw_time(Layer *layer, GContext *ctx);
static void draw_date(Layer *layer, GContext *ctx);

void initialise_clock(void) {
	time_layer = layer_create(GRect(0, TIME_POS_Y, WINDOW_WIDTH, WINDOW_HEIGHT));
	date_layer = layer_create(GRect(0, DATE_POS_Y, WINDOW_WIDTH, WINDOW_HEIGHT));
	layer_set_update_proc(time_layer, draw_time);
	layer_set_update_proc(date_layer, draw_date);
	time_t raw_time = time(NULL);
	struct tm *tick_time = localtime(&raw_time);
	update_time(tick_time);
	update_date(tick_time);
	time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_IMAGINE_38));
	date_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_IMAGINE_18));
}

Layer* get_time_layer(void) { return time_layer; }
Layer* get_date_layer(void) { return date_layer; }

void update_time(struct tm *tick_time) {
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

static void draw_time(Layer *layer, GContext *ctx) {
	graphics_context_set_text_color(ctx, color_palette[C_BORDER]);
	graphics_draw_text_border(ctx, time_text, time_font, 0, 0, TIME_WIDTH, TIME_HEIGHT, TIME_BORDER);
	graphics_context_set_text_color(ctx, color_palette[C_TEXT]);
	graphics_draw_text(ctx, time_text, time_font, GRect(TIME_BORDER, TIME_BORDER, TIME_WIDTH, TIME_HEIGHT), GTextOverflowModeFill, GTextAlignmentCenter, NULL);
}

static void draw_date(Layer *layer, GContext *ctx) {
	if (date_text[0] != '!') {
		graphics_context_set_text_color(ctx, color_palette[C_BORDER]);
		graphics_draw_text_border(ctx, date_text, date_font, 0, 0, DATE_WIDTH, DATE_HEIGHT, DATE_BORDER);
		graphics_context_set_text_color(ctx, color_palette[C_TEXT]);
		graphics_draw_text(ctx, date_text, date_font, GRect(DATE_BORDER, DATE_BORDER, DATE_WIDTH, DATE_HEIGHT), GTextOverflowModeFill, GTextAlignmentCenter, NULL);
	}
}

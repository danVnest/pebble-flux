#include "bluetooth.h"

static Layer *bluetooth_layer;
static bool show_bluetooth = false;

static void draw_bluetooth(Layer *layer, GContext *ctx);

void initialise_bluetooth(void) {
	bluetooth_layer = layer_create(GRect(BLUETOOTH_POS_X, BLUETOOTH_POS_Y, BLUETOOTH_WIDTH, BLUETOOTH_HEIGHT));
	layer_set_update_proc(bluetooth_layer, draw_bluetooth);
}

void bluetooth_handler(bool connected) {
	show_bluetooth = !connected && get_setting(SETTING_BLUETOOTH_ICON);
	if (!connected && get_setting(SETTING_BLUETOOTH_VIBRATE)) vibes_short_pulse();
	layer_mark_dirty(bluetooth_layer);
}

static void draw_bluetooth(Layer *layer, GContext *ctx) {
	if (show_bluetooth) { 
		graphics_context_set_fill_color(ctx, color_palette[C_BORDER]);
		graphics_fill_rect(ctx, GRect(0, BLUETOOTH_OFFSET_Y, BLUETOOTH_WIDTH, BLUETOOTH_HEIGHT - BLUETOOTH_OFFSET_Y * 2), 0, GCornerNone);
		graphics_context_set_fill_color(ctx, color_palette[C_TEXT]);
		graphics_fill_rect(ctx, GRect(BLUETOOTH_BORDER, BLUETOOTH_OFFSET_Y + BLUETOOTH_BORDER, BLUETOOTH_WIDTH_NB, BLUETOOTH_HEIGHT_NBO), 0, GCornerNone);
		graphics_context_set_fill_color(ctx, color_palette[C_BORDER]);
		graphics_fill_rect(ctx, GRect(BLUETOOTH_BORDER + BLUETOOTH_THICKNESS_X, BLUETOOTH_OFFSET_Y + BLUETOOTH_BORDER + BLUETOOTH_THICKNESS_Y, BLUETOOTH_WIDTH_NB - BLUETOOTH_THICKNESS_X * 2, BLUETOOTH_HEIGHT_NBO - BLUETOOTH_THICKNESS_Y * 2), 0, GCornerNone);
		graphics_fill_rect(ctx, GRect(BLUETOOTH_BUTTON_POS_X, BLUETOOTH_BUTTON_POS_Y, BLUETOOTH_BUTTON_WIDTH, BLUETOOTH_BUTTON_HEIGHT), 0, GCornerNone);
		graphics_context_set_antialiased(ctx, false);
		graphics_context_set_stroke_color(ctx, color_palette[C_BORDER]);
		graphics_context_set_stroke_width(ctx, BLUETOOTH_LINE_BORDER);
		graphics_draw_line(ctx, GPoint(BLUETOOTH_LINE_OFFSET, BLUETOOTH_LINE_OFFSET), GPoint(BLUETOOTH_WIDTH - BLUETOOTH_LINE_OFFSET * 2 + BLUETOOTH_LINE_STROKE, BLUETOOTH_HEIGHT - BLUETOOTH_LINE_OFFSET * 2 + BLUETOOTH_LINE_STROKE));
		graphics_context_set_stroke_color(ctx, color_palette[C_TEXT]);
		graphics_context_set_stroke_width(ctx, BLUETOOTH_LINE_STROKE);
		graphics_draw_line(ctx, GPoint(BLUETOOTH_LINE_OFFSET, BLUETOOTH_LINE_OFFSET), GPoint(BLUETOOTH_WIDTH - BLUETOOTH_LINE_OFFSET * 2 + BLUETOOTH_LINE_STROKE, BLUETOOTH_HEIGHT - BLUETOOTH_LINE_OFFSET * 2 + BLUETOOTH_LINE_STROKE));
	}
}

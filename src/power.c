#include "power_mode.h"

static Layer *battery_layer;
static bool show_battery = false;

static void low_power_threshold_handler(BatteryChargeState charge_state);
static void low_power_time_handler(WakeupId id, int32_t reason);
static void draw_battery(Layer *layer, GContext *ctx);

void initialise_power_mode(void) {
	battery_layer = layer_create(GRect(BATTERY_POS_X, BATTERY_POS_Y, BATTERY_WIDTH + BATTERY_BUMP_WIDTH, BATTERY_HEIGHT));
	layer_set_update_proc(battery_layer, draw_battery);

}

void update_battery(bool show) {
	if (show_battery != show) {
		check_inactivity_alert();
		show_battery = show;
	}
	layer_mark_dirty(battery_layer);
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
		graphics_context_set_fill_color(ctx, color_palette[C_BORDER]);
		graphics_fill_rect(ctx, GRect(0, 0, BATTERY_WIDTH, BATTERY_HEIGHT), 0, GCornerNone);
		graphics_fill_rect(ctx, GRect(BATTERY_WIDTH, BATTERY_BUMP_POS_Y, BATTERY_BUMP_WIDTH, BATTERY_BUMP_HEIGHT), 0, GCornerNone);
		graphics_context_set_fill_color(ctx, color_palette[C_TEXT]);
		graphics_fill_rect(ctx, GRect(BATTERY_BORDER, BATTERY_BORDER, BATTERY_WIDTH_NB, BATTERY_HEIGHT_NB), 0, GCornerNone);
		graphics_fill_rect(ctx, GRect(BATTERY_WIDTH - BATTERY_BORDER, BATTERY_BUMP_POS_Y + BATTERY_BORDER, BATTERY_BUMP_WIDTH, BATTERY_BUMP_HEIGHT_NB), 0, GCornerNone);
		graphics_context_set_fill_color(ctx, color_palette[C_BORDER]);
		graphics_fill_rect(ctx, GRect(BATTERY_BORDER + BATTERY_THICKNESS, BATTERY_BORDER + BATTERY_THICKNESS, (BATTERY_WIDTH_NB - BATTERY_THICKNESS * 2) * (100 - percent) / 100, BATTERY_HEIGHT_NB - BATTERY_THICKNESS * 2), 0, GCornerNone);
		graphics_context_set_text_color(ctx, color_palette[C_BORDER]);
		graphics_draw_text_border(ctx, battery_text, date_font, 0, BATTERY_TEXT_POS_Y, BATTERY_WIDTH, BATTERY_TEXT_HEIGHT, BATTERY_TEXT_BORDER);
		graphics_context_set_text_color(ctx, color_palette[C_TEXT]);
		graphics_draw_text(ctx, battery_text, date_font, GRect(BATTERY_TEXT_BORDER, BATTERY_TEXT_POS_Y + BATTERY_TEXT_BORDER, BATTERY_WIDTH, BATTERY_TEXT_HEIGHT), GTextOverflowModeFill, GTextAlignmentCenter, NULL);
	}
}

void configure_low_power_mode() {
	wakeup_cancel_all();
	time_t raw_time = time(NULL);
	struct tm *current_time = localtime(&raw_time);
	struct tm start_time = *current_time;
	struct tm end_time = *current_time;
	start_time.tm_hour = get_setting(SETTING_POWER_START_HOUR);
	start_time.tm_min = get_setting(SETTING_POWER_START_MINUTE);
	end_time.tm_hour = get_setting(SETTING_POWER_END_HOUR);
	end_time.tm_min = get_setting(SETTING_POWER_END_MINUTE);
	int current_minutes = current_time->tm_hour * 60 + current_time->tm_min;
	int start_minutes = start_time.tm_hour * 60 + start_time.tm_min;
	int end_minutes = end_time.tm_hour * 60 + end_time.tm_min;
	if (start_minutes == end_minutes) low_power_mode_time_enabled = true;
	else {	
		if (start_minutes > end_minutes) { end_time.tm_mday++; end_minutes += 60*60*24; }
		if ((current_minutes >= start_minutes) && (end_minutes > current_minutes)) low_power_mode_time_enabled = true;
		else low_power_mode_time_enabled = false;
		if (current_minutes >= start_minutes) start_time.tm_mday++;
		if (current_minutes >= end_minutes) end_time.tm_mday++;
		WakeupId startid = wakeup_schedule(mktime(&start_time), LOW_POWER_MODE_START, false);
		WakeupId endid = wakeup_schedule(mktime(&end_time), LOW_POWER_MODE_END, false); // TODO: check return values to ensure wakeup has been set
	}
	BatteryChargeState charge_state = battery_state_service_peek();
	if ((charge_state.charge_percent <= get_setting(SETTING_POWER_THRESHOLD)) && (charge_state.is_charging == false)) low_power_mode_threshold_enabled = true;
	else low_power_mode_threshold_enabled = false;
	update_battery((low_power_mode_time_enabled || low_power_mode_threshold_enabled));
	check_animation_status();
}

bool is_lower_power_mode_enabled(void) {
	return low_power_mode_threshold_enabled || low_power_mode_time_enabled;
}

static void low_power_threshold_handler(BatteryChargeState charge_state) {
	if ((charge_state.charge_percent <= get_setting(SETTING_POWER_THRESHOLD)) && (charge_state.is_charging == false)) low_power_mode_threshold_enabled = true;
	else low_power_mode_threshold_enabled = false;
	update_battery((low_power_mode_time_enabled || low_power_mode_threshold_enabled));
	check_animation_status();
}

static void low_power_time_handler(WakeupId id, int32_t reason) {
	time_t raw_time;
	wakeup_query(id, &raw_time);
	struct tm *time = localtime(&raw_time);
	time->tm_yday++;
	if (reason == LOW_POWER_MODE_START) {
		low_power_mode_time_enabled = true;
		wakeup_schedule(mktime(time), LOW_POWER_MODE_START, false);
	}
	else {
		low_power_mode_time_enabled = false;
		wakeup_schedule(mktime(time), LOW_POWER_MODE_END, false);
	}
	update_battery((low_power_mode_time_enabled || low_power_mode_threshold_enabled));
	check_animation_status();
}

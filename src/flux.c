#include "flux.h"

static Window *window;

static void tick_handler(struct tm *tick_time, TimeUnits units_changed);
static void initialise();
static void cleanup();	
static void window_load(Window *window);
static void window_unload(Window *window);

int main(void) {
	initialise();
	app_event_loop();
	cleanup();
}

void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
	animation_tick_handler(tick_time);
	if (units_changed & DAY_UNIT) update_date(tick_time);
	if (units_changed & MINUTE_UNIT) {
	   	update_time(tick_time);
		if (inactivity_alert_enabled) analyse_activity();
	}
}

static void initialise() {
	initialise_pattern();
	initialise_clock();
	initialise_icons();
	initialise_inactivity_alert();
	configure_low_power_mode();
	window = window_create();
	window_set_window_handlers(window, (WindowHandlers) { .load = window_load, .unload = window_unload });
	window_stack_push(window, true);
	startup_animation();
	bluetooth_connection_service_subscribe(bluetooth_handler);
	battery_state_service_subscribe(low_power_threshold_handler);
	wakeup_service_subscribe(low_power_time_handler);
	sync_settings();
}

static void cleanup() {
	wakeup_cancel_all();
	tick_timer_service_unsubscribe();
	battery_state_service_unsubscribe();
	bluetooth_connection_service_unsubscribe();
	save_state();
	save_settings();
	window_destroy(window);
	// TODO: move to clock cleanup function:
	// TODO: figure out what the js folder should have in it
	fonts_unload_custom_font(time_font);
	fonts_unload_custom_font(date_font);
}

static void window_load(Window *window) {
	layer_add_child(window_get_root_layer(window), get_pattern_layer());
	layer_add_child(window_get_root_layer(window), get_time_layer());
	layer_add_child(window_get_root_layer(window), get_date_layer());
	layer_add_child(window_get_root_layer(window), get_battery_layer());
	layer_add_child(window_get_root_layer(window), get_bluetooth_layer());
	layer_add_child(window_get_root_layer(window), get_inactivity_layer());
}

static void window_unload(Window *window) {
	layer_destroy(get_pattern_layer());
	layer_destroy(get_time_layer());
	layer_destroy(get_date_layer());
	layer_destroy(get_battery_layer());
	layer_destroy(get_bluetooth_layer());
	layer_destroy(get_inactivity_layer());
}

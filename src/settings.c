#include "settings.h"

static uint8_t settings[SETTING_KEY_COUNT];
static AppSync app_sync;
static uint8_t sync_buffer[SETTING_KEY_COUNT * (3 + 8) + 1]; // tuplet count * (3 byte key + 8 byte integer) + 1 byte header

static void sync_error_handler(DictionaryResult dict_error, AppMessageResult app_message_error, void *context);
static void sync_changed_handler(const uint32_t key, const Tuple *new_tuple, const Tuple *old_tuple, void *context);

uint8_t get_setting(uint8_t key) {
	return settings[key];
}
	
void sync_settings() {
	size_t stored_byte_count = persist_get_size(STORAGE_SETTINGS);
	if (stored_byte_count == sizeof(settings)) persist_read_data(STORAGE_SETTINGS, settings, stored_byte_count);
	else {
		settings[SETTING_PATTERN_CHANGE] = 8;
		settings[SETTING_PATTERN_NODES] = 1;
		settings[SETTING_PATTERN_FRACTAL] = 1;
		settings[SETTING_ANIMATIONS_FREQUENCY] = 8;
		settings[SETTING_ANIMATIONS_DURATION] = 3;
		settings[SETTING_COLOURS_BRIGHT] = 1;
		settings[SETTING_COLOURS_SOFT] = 1;
		settings[SETTING_POWER_START_HOUR] = 22;
		settings[SETTING_POWER_START_MINUTE] = 0;
		settings[SETTING_POWER_END_HOUR] = 7;
		settings[SETTING_POWER_END_MINUTE] = 0;
		settings[SETTING_POWER_THRESHOLD] = 30;
		settings[SETTING_BLUETOOTH_ICON] = 1;
		settings[SETTING_BLUETOOTH_VIBRATE] = 1;
		settings[SETTING_INACTIVITY_PERIOD] = 2;
		settings[SETTING_INACTIVITY_REPEAT] = 1;
		settings[SETTING_INACTIVITY_THRESHOLD] = 1;
		settings[SETTING_DISPLAY_DATE] = 1;
	}
	app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
	Tuplet settings_sync[] = {
		TupletInteger(SETTING_PATTERN_CHANGE, settings[SETTING_PATTERN_CHANGE]),
		TupletInteger(SETTING_PATTERN_NODES, settings[SETTING_PATTERN_NODES]),
		TupletInteger(SETTING_PATTERN_FRACTAL, settings[SETTING_PATTERN_FRACTAL]),
		TupletInteger(SETTING_ANIMATIONS_FREQUENCY, settings[SETTING_ANIMATIONS_FREQUENCY]),
		TupletInteger(SETTING_ANIMATIONS_DURATION, settings[SETTING_ANIMATIONS_DURATION]),
		TupletInteger(SETTING_COLOURS_BRIGHT, settings[SETTING_COLOURS_BRIGHT]),
		TupletInteger(SETTING_COLOURS_SOFT, settings[SETTING_COLOURS_SOFT]),
		TupletInteger(SETTING_POWER_START_HOUR, settings[SETTING_POWER_START_HOUR]),
		TupletInteger(SETTING_POWER_START_MINUTE, settings[SETTING_POWER_START_MINUTE]),
		TupletInteger(SETTING_POWER_END_HOUR, settings[SETTING_POWER_END_HOUR]),
		TupletInteger(SETTING_POWER_END_MINUTE, settings[SETTING_POWER_END_MINUTE]),
		TupletInteger(SETTING_POWER_THRESHOLD, settings[SETTING_POWER_THRESHOLD]),
		TupletInteger(SETTING_BLUETOOTH_ICON, settings[SETTING_BLUETOOTH_ICON]),
		TupletInteger(SETTING_BLUETOOTH_VIBRATE, settings[SETTING_BLUETOOTH_VIBRATE]),
		TupletInteger(SETTING_INACTIVITY_PERIOD, settings[SETTING_INACTIVITY_PERIOD]),
		TupletInteger(SETTING_INACTIVITY_REPEAT, settings[SETTING_INACTIVITY_REPEAT]),
		TupletInteger(SETTING_INACTIVITY_THRESHOLD, settings[SETTING_INACTIVITY_THRESHOLD]),
		TupletInteger(SETTING_DISPLAY_DATE, settings[SETTING_DISPLAY_DATE])
	};
	app_sync_init(&app_sync, sync_buffer, sizeof(sync_buffer), settings_sync, ARRAY_LENGTH(settings_sync), sync_changed_handler, sync_error_handler, NULL);
}

void save_settings() {
	persist_write_data(STORAGE_SETTINGS, settings, sizeof(settings));
	app_sync_deinit(&app_sync);
}

static void sync_changed_handler(const uint32_t key, const Tuple *new_tuple, const Tuple *old_tuple, void *context) {
	if (settings[key] != new_tuple->value->uint8) {
		// APP_LOG(APP_LOG_LEVEL_DEBUG, "key: %d; value: %d; old: %d", (int)key, new_tuple->value->unit8, settings[key]);
		settings[key] = new_tuple->value->uint8;
		if (key == SETTING_ANIMATIONS_FREQUENCY) configure_animation_frequency();
		else if (key == SETTING_ANIMATIONS_DURATION) configure_frames_per_animation();
		else if ((key >= SETTING_POWER_START_HOUR) && (key <= SETTING_POWER_THRESHOLD)) configure_low_power_mode();
		else if ((key >= SETTING_INACTIVITY_PERIOD) && (key <= SETTING_INACTIVITY_THRESHOLD)) configure_inactivity_alert();
		else if (key == SETTING_DISPLAY_DATE) {
			time_t raw_time = time(NULL);
			struct tm *tick_time = localtime(&raw_time);
			update_date(tick_time);
		}
	}
}

static void sync_error_handler(DictionaryResult dict_error, AppMessageResult app_message_error, void *context) {
	APP_LOG(APP_LOG_LEVEL_ERROR, "sync error: %d %d", dict_error, app_message_error);
}

static GColor8 random_color() {
	  return (GColor8) { .argb = ((rand() % 0b00111111) + 0b11000000) };
}

void randomise_color_palette(GColor8 *palette) {
	palette[C_BACKGROUND] = random_color();
	palette[C_TEXT] = gcolor_legible_over(palette[C_BACKGROUND]);
	palette[C_BORDER] = (GColor8) { .argb = palette[C_BACKGROUND].argb & (palette[C_TEXT].argb ^ 0b00101010)};
	palette[C_PATTERN_1] = random_color();
	palette[C_PATTERN_2] = random_color();
	window_set_background_color(window, palette[C_BACKGROUND]);
}

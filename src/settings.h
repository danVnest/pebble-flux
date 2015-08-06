#pragma once
#include <pebble.h>
#include "flux.h"

enum STORAGE_KEYS {
	STORAGE_SETTINGS
};
enum SETTING_KEYS {
	SETTING_DATE,
	SETTING_KEY_COUNT
};

uint8_t get_setting(int key);
void sync_settings();
void save_settings();
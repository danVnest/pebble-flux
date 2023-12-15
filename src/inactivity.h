#pragma once
#include <pebble.h>
#include "power_mode.h"
#include "settings.h"

#define INACTIVITY_WINDOW_SIZE 5
#define INACTIVITY_DISPLAY_THRESHOLD 3
#define INACTIVITY_POS_Y 110
#define INACTIVITY_WIDTH 62
#define INACTIVITY_HEIGHT 19
#define INACTIVITY_OFFSET_Y 10
#define INACTIVITY_BAR_THICKNESS 2
#define INACTIVITY_BORDER 1
#define INACTIVITY_LEVEL_THICKNESS 1
#define INACTIVITY_WEIGHT_SMALL_HEIGHT 10
#define INACTIVITY_WEIGHT_LARGE_HEIGHT 14
#define INACTIVITY_WEIGHT_WIDTH 5
#define INACTIVITY_WEIGHT_GAP -1
#define INACTIVITY_POS_X ((WINDOW_WIDTH - INACTIVITY_WIDTH) / 2)
#define INACTIVITY_WEIGHT_OFFSET_X (1 + INACTIVITY_BORDER)
#define INACTIVITY_WEIGHT_OFFSET_Y (INACTIVITY_OFFSET_Y + INACTIVITY_BAR_THICKNESS)
#define INACTIVITY_WIDTH_NB (INACTIVITY_WIDTH - INACTIVITY_BORDER * 2)
#define INACTIVITY_WEIGHT_WIDTH_NB (INACTIVITY_WEIGHT_WIDTH - INACTIVITY_BORDER * 2)

void initialise_inactivity_alert(void);
void configure_inactivity_alert(void);
void review_inactivity_alert_state(void);
bool is_inactivity_alert_enabled(void);
void activity_handler(AccelRawData *data, uint32_t num_samples, uint64_t timestamp);
void analyse_activity(void);
void save_activity_state(void);

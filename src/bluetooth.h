#pragma once
#include <pebble.h>

#define BLUETOOTH_POS_Y 4
#define BLUETOOTH_WIDTH 21
#define BLUETOOTH_HEIGHT 50
#define BLUETOOTH_OFFSET_Y 4
#define BLUETOOTH_THICKNESS_X 2
#define BLUETOOTH_THICKNESS_Y 6
#define BLUETOOTH_BORDER 1
#define BLUETOOTH_BUTTON_WIDTH 3
#define BLUETOOTH_BUTTON_HEIGHT 2
#define BLUETOOTH_LINE_BORDER 5
#define BLUETOOTH_LINE_STROKE 1
#define BLUETOOTH_LINE_OFFSET 2
#define BLUETOOTH_POS_X (WINDOW_WIDTH - BLUETOOTH_WIDTH - BLUETOOTH_POS_Y * 2)
#define BLUETOOTH_BUTTON_POS_X ((BLUETOOTH_WIDTH - BLUETOOTH_BUTTON_WIDTH) / 2)
#define BLUETOOTH_BUTTON_POS_Y (BLUETOOTH_HEIGHT - BLUETOOTH_BORDER - BLUETOOTH_OFFSET_Y - BLUETOOTH_THICKNESS_Y / 2 - BLUETOOTH_BUTTON_HEIGHT / 2)
#define BLUETOOTH_WIDTH_NB (BLUETOOTH_WIDTH - BLUETOOTH_BORDER * 2)
#define BLUETOOTH_HEIGHT_NBO (BLUETOOTH_HEIGHT - BLUETOOTH_BORDER * 2 - BLUETOOTH_OFFSET_Y * 2)

void initialise_bluetooth(void);
void bluetooth_handler(bool connected);

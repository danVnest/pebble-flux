#pragma once
#include <pebble.h>

#define TIME_POS_X 0
#define TIME_POS_Y 50
#define TIME_WIDTH WINDOW_WIDTH
#define TIME_HEIGHT 44
#define TIME_BORDER 2
#define DATE_POS_X 0
#define DATE_POS_Y 139
#define DATE_WIDTH 142
#define DATE_HEIGHT 20
#define DATE_BORDER 1

void initialise_clock(void);
Layer* get_time_layer(void);
Layer* get_date_layer(void);
void update_time(struct tm *tick_time);
void update_date(struct tm *tick_time);

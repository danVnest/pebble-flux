#include "nodes.h"
	
#define NODE_COUNT 10
#define INT_PRECISION 1000
#define X_MAX (144 * INT_PRECISION)
#define Y_MAX (168 * INT_PRECISION)
#define PIXELS_MAX 8
#define PIXELS_MIN 4
#define NODE_SIZE_MAX (2 * INT_PRECISION)
#define FRAMES_PER_SECOND 20
#define FRAME_DURATION (1000 / FRAMES_PER_SECOND)
#define LOW_POWER_MODE_START 0
#define LOW_POWER_MODE_END 1

struct Node {
	int x;
	int y;
	int size;
};

static struct Node nodes[NODE_COUNT] = {{0}};
static struct Node node_changes[NODE_COUNT] = {{0}};
static AppTimer *animation_timer = NULL;
static uint8_t frame_number;
static uint8_t frames_per_animation;
static uint32_t animation_interval;
static uint8_t animation_interval_unit;
static bool animations_enabled;
bool low_power_mode_time_enabled = false;

static void check_animation_status();
static void begin_animating_nodes();
static void animate_nodes();

static void check_animation_status() {
	if ((frames_per_animation == 0) || (animation_interval == 0) || low_power_mode_time_enabled) {
		animations_enabled = false;
		tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
   	}
	else {
		if (animation_interval == 0xFF) {
			begin_animating_nodes();
			animations_enabled = false;
		}
		else animations_enabled = true;
		tick_timer_service_subscribe(animation_interval_unit, tick_handler);
	}
}

void configure_frames_per_animation() {
	frames_per_animation = FRAMES_PER_SECOND * get_setting(SETTING_ANIMATIONS_DURATION) / 10;
	check_animation_status();
}

void configure_animation_frequency() {
	switch (get_setting(SETTING_ANIMATIONS_FREQUENCY)) {
		case 0: animation_interval = 0xFF; animation_interval_unit = MINUTE_UNIT; break;
		case 1: animation_interval = 1; animation_interval_unit = SECOND_UNIT; break;
		case 2: animation_interval = 2; animation_interval_unit = SECOND_UNIT; break;
		case 3: animation_interval = 3; animation_interval_unit = SECOND_UNIT; break;
		case 4: animation_interval = 5; animation_interval_unit = SECOND_UNIT; break;
		case 5: animation_interval = 10; animation_interval_unit = SECOND_UNIT; break;
		case 6: animation_interval = 15; animation_interval_unit = SECOND_UNIT; break;
		case 7: animation_interval = 30; animation_interval_unit = SECOND_UNIT; break;
		case 8: animation_interval = 1; animation_interval_unit = MINUTE_UNIT; break;
		case 9: animation_interval = 15; animation_interval_unit = MINUTE_UNIT; break;
		case 10: animation_interval = 30; animation_interval_unit = MINUTE_UNIT; break;
		case 11: animation_interval = 60; animation_interval_unit = MINUTE_UNIT; break;
		case 12: animation_interval = 0; animation_interval_unit = MINUTE_UNIT; break;
	}
	check_animation_status();
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
	check_animation_status();
}

void begin_startup_animation() {
	// TODO: implement actual startup animation
	frames_per_animation = FRAMES_PER_SECOND * get_setting(SETTING_ANIMATIONS_DURATION) / 10;
	configure_animation_frequency();
	wakeup_service_subscribe(low_power_time_handler);
	configure_low_power_mode();
}

void animation_tick_handler(struct tm *tick_time) {
	if (animations_enabled) {
		if (((animation_interval_unit == SECOND_UNIT) && (tick_time->tm_sec % animation_interval == 0)) || ((animation_interval_unit == MINUTE_UNIT) && (tick_time->tm_min % animation_interval == 0))) {
			begin_animating_nodes();
		}
	}
}

static void begin_animating_nodes() {
	animation_timer = app_timer_register(FRAME_DURATION, animate_nodes, NULL);
	for (int i = 0; i < NODE_COUNT; i++) {
		node_changes[i].size = ((rand() % NODE_SIZE_MAX) - nodes[i].size) / frames_per_animation;
		node_changes[i].x = ((rand() % (X_MAX / 5)) - X_MAX / 10) / frames_per_animation;
		node_changes[i].y = ((rand() % (Y_MAX / 5)) - Y_MAX / 10) / frames_per_animation;
		nodes[i].size += node_changes[i].size;
		nodes[i].x += node_changes[i].x - node_changes[i].size / 2;
		nodes[i].y += node_changes[i].y - node_changes[i].size / 2;
	}
	frame_number = 0;
	background_layer_mark_dirty();
}

static void animate_nodes() {
	if (frame_number++ < frames_per_animation) animation_timer = app_timer_register(FRAME_DURATION, animate_nodes, NULL);
	else if ((animation_interval == 0xFF) && (animations_enabled)) animation_timer = app_timer_register(FRAME_DURATION, begin_animating_nodes, NULL);
	for (int i = 0; i < NODE_COUNT; i++) {
		if ((nodes[i].x < 0) && (node_changes[i].x < 0)) node_changes[i].x *= -1;
		else if ((nodes[i].x > X_MAX) && (node_changes[i].x > 0)) node_changes[i].x *= -1;
		if ((nodes[i].y < 0) && (node_changes[i].y < 0)) node_changes[i].y *= -1;
		else if ((nodes[i].y > Y_MAX) && (node_changes[i].y > 0)) node_changes[i].y *= -1;
		nodes[i].size += node_changes[i].size;
		nodes[i].x += node_changes[i].x - node_changes[i].size / 2;
		nodes[i].y += node_changes[i].y - node_changes[i].size / 2;
	}
	background_layer_mark_dirty();
}

void draw_nodes(Layer *layer, GContext* ctx) {
	graphics_context_set_fill_color(ctx, GColorWhite);
	graphics_context_set_stroke_color(ctx, GColorBlack);
	graphics_context_set_stroke_width(ctx, 1);
	for(int i = 0; i < NODE_COUNT; i++) {
		int node_x = nodes[i].x / INT_PRECISION;
		int node_y = nodes[i].y / INT_PRECISION;
		int node_size = nodes[i].size * PIXELS_MAX / NODE_SIZE_MAX + PIXELS_MIN;
		graphics_draw_rect(ctx, GRect(node_x - 1, node_y - 1, node_size + 2, node_size + 2));
		graphics_fill_rect(ctx, GRect(node_x, node_y, node_size, node_size), 0, GCornerNone);
	}
}

void create_nodes() {
	check_animation_status();
	for (int i = 0; i < NODE_COUNT; i++) {
		nodes[i].x = rand() % X_MAX;
		nodes[i].y = rand() % Y_MAX;
		nodes[i].size = rand() % NODE_SIZE_MAX;
	}
}

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
static uint32_t animation_frequency;
static bool animations_enabled;

static void check_animation_status();
static void begin_animating_nodes();
static void animate_nodes();

void create_nodes() {
	check_animation_status();
	for (int i = 0; i < NODE_COUNT; i++) {
		nodes[i].x = rand() % X_MAX;
		nodes[i].y = rand() % Y_MAX;
		nodes[i].size = rand() % NODE_SIZE_MAX;
	}
}

void begin_startup_animation() {
	// TODO: implement actual startup animation
	configure_frames_per_animation();
	configure_animation_frequency();
}

void sync_animations() {
	if (animations_enabled == false) return;
	if (animation_timer != NULL) app_timer_cancel(animation_timer);
	if (animation_frequency == FRAME_DURATION * 2) begin_animating_nodes();
	else {
		time_t raw_time;
		uint16_t ms = time_ms(&raw_time, NULL);
		struct tm *time = localtime(&raw_time);
		uint32_t next = 0;
		switch (animation_frequency) {
			case 1000: next = 1000 - ms;
			case 2000: next = (2 - time->tm_sec % 2) * 1000 - ms;
		else if (animation_frequency == 2000) next = 1000 - ms;
		else if (animation_frequency == 15000) next = (15 - time->tm_sec % 15) * 1000 - ms;
		else if (animation_frequency == 30000) next = (30 - time->tm_sec % 30) * 1000 - ms;
		else if (animation_frequency == 60000) next = (60 - time->tm_sec) * 1000 - ms;
		else if (animation_frequency == 900000) next = (15 - time->tm_min % 15) * 60000 - time->tm_sec * 1000 - ms;
		else if (animation_frequency == 1800000) next = (30 - time->tm_min % 30) * 60000 - time->tm_sec * 1000 - ms;
		else if (animation_frequency == 3600000) next = (60 - time->tm_min) * 60000 - time->tm_sec * 1000 - ms;
		animation_timer = app_timer_register(next, begin_animating_nodes, NULL);
		APP_LOG(APP_LOG_LEVEL_DEBUG, "freq: %u; next: %u", (unsigned int)animation_frequency, (unsigned int)next);
	}
}

static void check_animation_status() {
	if ((frames_per_animation == 0) || (animation_frequency == 0)) { animations_enabled = false; return; }
	if (animation_frequency < frames_per_animation * 1000 / FRAMES_PER_SECOND) frames_per_animation = FRAMES_PER_SECOND * animation_frequency / 1000;
	animations_enabled = true;
}

void configure_frames_per_animation() {
	frames_per_animation = FRAMES_PER_SECOND * get_setting(SETTING_ANIMATIONS_DURATION) / 10;
	check_animation_status();
	sync_animations();
	tick_timer_service_subscribe(get_timer_unit_setting(), tick_handler);
}

void configure_animation_frequency() {
	uint16_t seconds;
	switch (get_setting(SETTING_ANIMATIONS_FREQUENCY)) {
		case 0: animation_frequency = FRAME_DURATION * 2; break;
		case 1: animation_frequency = 1000; break;
		case 2: animation_frequency = 15000; break;
		case 3: animation_frequency = 30000; break;
		case 4: animation_frequency = 60000; break;
		case 5: animation_frequency = 900000; break;
		case 6: animation_frequency = 180000; break;
		case 7: animation_frequency = 360000; break;
		case 8: animation_frequency = 0; break;
	}
	check_animation_status();
	sync_animations();
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

static void begin_animating_nodes() {
	for (int i = 0; i < NODE_COUNT; i++) {
		node_changes[i].size = ((rand() % NODE_SIZE_MAX) - nodes[i].size) / frames_per_animation;
		node_changes[i].x = ((rand() % (X_MAX / 5)) - X_MAX / 10) / frames_per_animation;
		node_changes[i].y = ((rand() % (Y_MAX / 5)) - X_MAX / 10) / frames_per_animation;
		nodes[i].size += node_changes[i].size;
		nodes[i].x += node_changes[i].x - node_changes[i].size;
		nodes[i].y += node_changes[i].y - node_changes[i].size;
	}
	frame_number = 0;
	animation_timer = app_timer_register(FRAME_DURATION, animate_nodes, NULL);
}

static void animate_nodes() {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "freq: %u; fpa: %u", (unsigned int)animation_frequency, (unsigned int)frames_per_animation);
	if (frame_number++ < frames_per_animation) animation_timer = app_timer_register(FRAME_DURATION, animate_nodes, NULL);
	else if (animations_enabled) animation_timer = app_timer_register(animation_frequency - FRAME_DURATION * frames_per_animation, begin_animating_nodes, NULL);
	else animation_timer = NULL;
	for (int i = 0; i < NODE_COUNT; i++) {
		nodes[i].size += node_changes[i].size;
		nodes[i].x += node_changes[i].x - node_changes[i].size;
		nodes[i].y += node_changes[i].y - node_changes[i].size;
		if ((nodes[i].x < 0) && (node_changes[i].x < 0)) node_changes[i].x *= -1;
		else if ((nodes[i].x > X_MAX) && (node_changes[i].x > 0)) node_changes[i].x *= -1;
		if ((nodes[i].y < 0) && (node_changes[i].y < 0)) node_changes[i].y *= -1;
		else if ((nodes[i].y > Y_MAX) && (node_changes[i].y > 0)) node_changes[i].y *= -1;
	}
	background_layer_mark_dirty();
}

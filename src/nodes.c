#include "nodes.h"
	
#define NODE_COUNT 10
#define INT_PRECISION 1000
#define X_MAX (144 * INT_PRECISION)
#define Y_MAX (168 * INT_PRECISION)
#define PIXELS_MAX 8
#define PIXELS_MIN 4
#define NODE_SIZE_MAX (2 * INT_PRECISION)
#define FRAME_RATE 30 //FPS
#define FRAME_DURATION (1000 / FRAME_RATE)

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
static bool animations_enabled = true;

static void set_animation_status();
static void begin_animating_nodes();
static void animate_nodes();

void create_nodes() {
	for (int i = 0; i < NODE_COUNT; i++) {
		nodes[i].x = rand() % X_MAX;
		nodes[i].y = rand() % Y_MAX;
		nodes[i].size = rand() % NODE_SIZE_MAX;
	}
}

void sync_animations() {
	if (animation_frequency == 0) return;
	else if (animation_frequency == FRAME_DURATION) begin_animating_nodes();
	else {
		time_t raw_time;
		uint16_t ms = time_ms(&raw_time, NULL);
		struct tm *time = localtime(&raw_time);
		uint32_t next = 0;
		if (animation_frequency == 1000) next = 1000 - ms;
		else if (animation_frequency == 15000) next = (15 - time->tm_sec % 15) * 1000 - ms;
		else if (animation_frequency == 30000) next = (30 - time->tm_sec % 30) * 1000 - ms;
		else if (animation_frequency == 60000) next = (60 - time->tm_sec) * 1000 - ms;
		else if (animation_frequency == 900000) next = (15 - time->tm_min % 15) * 60000 - time->tm_sec * 1000 - ms;
		else if (animation_frequency == 1800000) next = (30 - time->tm_min % 30) * 60000 - time->tm_sec * 1000 - ms;
		else if (animation_frequency == 3600000) next = (60 - time->tm_min) * 60000 - time->tm_sec * 1000 - ms;
		animation_timer = app_timer_register(next, begin_animating_nodes, NULL);
	}
}

void set_frames_per_animation(uint8_t animation_duration_ten) {
	frames_per_animation = FRAME_RATE * animation_duration_ten / 10;
	set_animation_status();
}

void set_animation_frequency(uint16_t seconds) {
	if (seconds == 0xFFFF) animation_frequency = FRAME_DURATION;
	else animation_frequency = seconds * 1000;
	set_animation_status();
}

static void set_animation_status() {
	if ((frames_per_animation != 0) || (animation_frequency != 0)) animations_enabled = true;
	else animations_enabled = false;
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
	for (int i = 0; i < NODE_COUNT; i++) {
		nodes[i].size += node_changes[i].size;
		nodes[i].x += node_changes[i].x - node_changes[i].size;
		nodes[i].y += node_changes[i].y - node_changes[i].size;
		if ((nodes[i].x < 0) && (node_changes[i].x < 0)) node_changes[i].x *= -1;
		else if ((nodes[i].x > X_MAX) && (node_changes[i].x > 0)) node_changes[i].x *= -1;
		if ((nodes[i].y < 0) && (node_changes[i].y < 0)) node_changes[i].y *= -1;
		else if ((nodes[i].y > Y_MAX) && (node_changes[i].y > 0)) node_changes[i].y *= -1;
	}
	if (frame_number++ < frames_per_animation) animation_timer = app_timer_register(FRAME_DURATION, animate_nodes, NULL);
	else if (animations_enabled) animation_timer = app_timer_register(animation_frequency - FRAME_DURATION * frames_per_animation, begin_animating_nodes, NULL);
	background_layer_mark_dirty();
}

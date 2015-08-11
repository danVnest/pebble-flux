#include "nodes.h"
	
#define NODE_COUNT 10
#define INT_PRECISION 1000
#define X_MAX (144 * INT_PRECISION)
#define Y_MAX (168 * INT_PRECISION)
#define PIXELS_MAX 8
#define PIXELS_MIN 4
#define NODE_SIZE_MAX (2 * INT_PRECISION)
#define FRAMES_PER_ANIMATION 30

struct Node {
	int x;
	int y;
	int size;
};

static struct Node nodes[NODE_COUNT] = {{0}};
static struct Node node_changes[NODE_COUNT] = {{0}};
static AppTimer *animation_timer = NULL;
static uint32_t delta = 40;
static uint8_t frameNumber;

static void animate_nodes();

void create_nodes() {
	for (int i = 0; i < NODE_COUNT; i++) {
		nodes[i].x = rand() % X_MAX;
		nodes[i].y = rand() % Y_MAX;
		nodes[i].size = rand() % NODE_SIZE_MAX;
	}
}

void draw_nodes(Layer *layer, GContext* ctx) {
	graphics_context_set_fill_color(ctx, GColorWhite);
	graphics_context_set_stroke_color(ctx, GColorBlack);
	graphics_context_set_stroke_width(ctx, 3);
	for(int i = 0; i < NODE_COUNT; i++) {
		int node_x = nodes[i].x / INT_PRECISION;
		int node_y = nodes[i].y / INT_PRECISION;
		int node_size = nodes[i].size * PIXELS_MAX / NODE_SIZE_MAX + PIXELS_MIN;
		graphics_draw_rect(ctx, GRect(node_x, node_y, node_size, node_size));
		graphics_fill_rect(ctx, GRect(node_x, node_y, node_size, node_size), 0, GCornerNone);
	}
}

void begin_animating_nodes() {
	for (int i = 0; i < NODE_COUNT; i++) {
		node_changes[i].size = ((rand() % NODE_SIZE_MAX) - nodes[i].size) / FRAMES_PER_ANIMATION;
		node_changes[i].x = ((rand() % (X_MAX / 5)) - X_MAX / 10) / FRAMES_PER_ANIMATION;
		node_changes[i].y = ((rand() % (Y_MAX / 5)) - X_MAX / 10) / FRAMES_PER_ANIMATION;
		nodes[i].size += node_changes[i].size;
		nodes[i].x += node_changes[i].x - node_changes[i].size;
		nodes[i].y += node_changes[i].y - node_changes[i].size;
	}
	frameNumber = 0;
	animation_timer = app_timer_register(delta, animate_nodes, NULL);
}

static void animate_nodes() {
	for (int i = 0; i < NODE_COUNT; i++) {
		nodes[i].size += node_changes[i].size;
		nodes[i].x += node_changes[i].x - node_changes[i].size;
		nodes[i].y += node_changes[i].y - node_changes[i].size;
	}
	if (frameNumber++ < FRAMES_PER_ANIMATION) animation_timer = app_timer_register(delta, animate_nodes, NULL);
	background_layer_mark_dirty();
}
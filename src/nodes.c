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
static GColor8 color_palette[5];
bool low_power_mode_time_enabled = false;
bool low_power_mode_threshold_enabled = false;

static void check_animation_status();

void animate_nodes() {
	for (int i = 0; i < NODE_COUNT; i++) {
		node_changes[i].size = ((rand() % NODE_SIZE_MAX) - nodes[i].size) / frames_per_animation;
		node_changes[i].x = ((rand() % (X_MAX / 5)) - X_MAX / 10) / frames_per_animation;
		node_changes[i].y = ((rand() % (Y_MAX / 5)) - Y_MAX / 10) / frames_per_animation;
		node_animation_step(i);
	}
}

void nodes_animation_step() {
	for (int i = 0; i < NODE_COUNT; i++) {
		if ((nodes[i].x < 0) && (node_changes[i].x < 0)) node_changes[i].x *= -1;
		else if ((nodes[i].x > X_MAX) && (node_changes[i].x > 0)) node_changes[i].x *= -1;
		if ((nodes[i].y < 0) && (node_changes[i].y < 0)) node_changes[i].y *= -1;
		else if ((nodes[i].y > Y_MAX) && (node_changes[i].y > 0)) node_changes[i].y *= -1;
		node_animation_step(i);
	}
}

static inline void node_animation_step(int node_index) {
	nodes[node_index].size += node_changes[node_index].size;
	nodes[node_index].x += node_changes[node_index].x - node_changes[node_index].size / 2;
	nodes[node_index].y += node_changes[node_index].y - node_changes[node_index].size / 2;
}

void draw_nodes(Layer *layer, GContext* ctx) {
	graphics_context_set_fill_color(ctx, color_palette[C_PATTERN_1]);
	graphics_context_set_stroke_color(ctx, color_palette[C_PATTERN_2]);
	graphics_context_set_stroke_width(ctx, 1);
	for(int i = 0; i < NODE_COUNT; i++) {
		int node_x = nodes[i].x / INT_PRECISION;
		int node_y = nodes[i].y / INT_PRECISION;
		int node_size = nodes[i].size * PIXELS_MAX / NODE_SIZE_MAX + PIXELS_MIN;
		graphics_draw_rect(ctx, GRect(node_x - 1, node_y - 1, node_size + 2, node_size + 2));
		graphics_fill_rect(ctx, GRect(node_x, node_y, node_size, node_size), 0, GCornerNone);
	}
}

void initialise_nodes() {
	for (int i = 0; i < NODE_COUNT; i++) {
		nodes[i].x = rand() % X_MAX;
		nodes[i].y = rand() % Y_MAX;
		nodes[i].size = rand() % NODE_SIZE_MAX;
	}
}

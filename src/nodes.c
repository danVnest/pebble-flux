#include "nodes.h"
	
#define NODE_COUNT 10
#define INT_PRECISION 1000
#define X_MAX (144 * INT_PRECISION)
#define Y_MAX (168 * INT_PRECISION)
#define PIXELS_MAX 8
#define PIXELS_MIN 4
#define NODE_SIZE_MAX (2 * INT_PRECISION)

struct Node {
	int x;
	int y;
	int size;
};

static struct Node nodes[NODE_COUNT] = {{0}};

void create_nodes() {
	for (int i = 0; i < NODE_COUNT; i++) {
		nodes[i].size = rand() % NODE_SIZE_MAX;
		nodes[i].x = rand() % X_MAX;
		nodes[i].y = rand() % Y_MAX;
	}
}

void draw_nodes(Layer *layer, GContext* ctx) {
	graphics_context_set_fill_color(ctx, GColorWhite);
	for(int i = 0; i < NODE_COUNT; i++) {
		int node_size = nodes[i].size * PIXELS_MAX / NODE_SIZE_MAX + PIXELS_MIN;
		graphics_fill_rect(ctx, GRect(nodes[i].x / INT_PRECISION, nodes[i].y / INT_PRECISION, node_size, node_size), 0, GCornerNone);
	}
}
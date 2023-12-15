#pragma once
#include <pebble.h>

typedef void (*animationStepFunction)(void);
typedef void (*animateFunction)(void);
typedef void (*drawPatternFunction)(Layer *layer, GContext* ctx);
typedef void (*initialisePatternFunction)(void);

void initialise_pattern();
void startup_animation();
void configure_frames_per_animation();
void configure_animation_frequency();
void animation_tick_handler(struct tm *tick_time);

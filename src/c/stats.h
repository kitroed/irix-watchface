#include <pebble.h>

#ifndef STATS_H
#define STATS_H

void bluetooth_callback_stats(bool connected);
void tick_stats();
void load_stats(Window* window);
void unload_stats(Window* window);

#endif

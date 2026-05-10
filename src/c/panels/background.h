#include <pebble.h>

#ifndef BACKGROUND_H
#define BACKGROUND_H

void update_clock_bg();
void redraw_bg();
void load_bg(Window* window);
void unload_bg(Window* window);

#endif

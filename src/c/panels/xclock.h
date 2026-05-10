#include <pebble.h>

#ifndef XCLOCK_H
#define XCLOCK_H

void redraw_xclock();
void tick_xclock();
void load_xclock(Window* window);
void unload_xclock(Window* window);

#endif

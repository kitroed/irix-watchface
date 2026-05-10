#include <pebble.h>

#ifndef PBLVIEW_H
#define PBLVIEW_H

void redraw_pblview(GContext* ctx);
void tick_pblview();
void load_pblview(Window* window);
void unload_pblview(Window* window);

#endif

#include <pebble.h>

#ifndef MESSAGING_H
#define MESSAGING_H

void inbox_received(DictionaryIterator *iterator, void *context);
void inbox_dropped(AppMessageResult reason, void *context);

#endif

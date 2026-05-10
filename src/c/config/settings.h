#include <pebble.h>

#ifndef SETTINGS_H
#define SETTINGS_H

enum ClockDisplayMode {
    BOTH, DIGITAL, ANALOG
};

enum DistanceUnit {
    METRIC, IMPERIAL
};

struct Settings {
    GColor bg_color, clock_hands_color;
    enum ClockDisplayMode clock_display_mode;
    enum DistanceUnit distance_unit;
};

GColor get_bg_color();
GColor get_clock_hands_color();
enum ClockDisplayMode get_clock_display_mode();
enum DistanceUnit get_distance_unit();

void set_bg_color(GColor color);
void set_clock_hands_color(GColor color);
void set_clock_display_mode(enum ClockDisplayMode clock_display_mode);
void set_distance_unit(enum DistanceUnit distance_unit);

void load_settings();
void save_settings();

#endif

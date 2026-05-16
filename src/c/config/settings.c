#include "settings.h"

#define SETTINGS_KEY 1

static struct Settings settings;

GColor get_bg_color() { return settings.bg_color; }

GColor get_clock_hands_color() { return settings.clock_hands_color; }

enum ClockDisplayMode get_clock_display_mode() {
    return settings.clock_display_mode;
}

enum DistanceUnit get_distance_unit() { return settings.distance_unit; }

enum HourlyChimeMode get_hourly_chime_mode() {
    return settings.hourly_chime_mode;
}

void set_bg_color(GColor color) { settings.bg_color = color; }

void set_clock_hands_color(GColor color) { settings.clock_hands_color = color; }

void set_clock_display_mode(enum ClockDisplayMode mode) {
    settings.clock_display_mode = mode;
}

void set_distance_unit(enum DistanceUnit distance_unit) {
    settings.distance_unit = distance_unit;
}

void set_hourly_chime_mode(enum HourlyChimeMode mode) {
    settings.hourly_chime_mode = mode;
}

void load_settings() {
    settings.bg_color = GColorVividCerulean;
    settings.clock_hands_color = GColorVividCerulean;
    settings.clock_display_mode = BOTH;
    settings.distance_unit = IMPERIAL;
    settings.hourly_chime_mode = CHIME_DOUBLE_PULSE;

    persist_read_data(SETTINGS_KEY, &settings, sizeof(settings));

    APP_LOG(APP_LOG_LEVEL_DEBUG, "Copied settings from memory.");
    APP_LOG(APP_LOG_LEVEL_DEBUG, "setting: bg_color = %d", settings.bg_color);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "setting: clock_hands_color = %d",
            settings.clock_hands_color);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "setting: clock_display_mode = %d",
            settings.clock_display_mode);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "setting: distance_unit = %d",
            settings.distance_unit);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "setting: hourly_chime_mode = %d",
            settings.hourly_chime_mode);
}

void save_settings() {
    int bytes = persist_write_data(SETTINGS_KEY, &settings, sizeof(settings));
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Wrote %d bytes to memory for settings.",
            bytes);
}

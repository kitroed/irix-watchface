#include "toolchest.h"

#include "../config/settings.h"
#include "../utils.h"

static TextLayer *steps_text_layer, *heartrate_text_layer, *distance_temp_layer,
    *bat_text_layer;

static char steps_buffer[16], bpm_buffer[16], distance_buffer[16],
    bat_buffer[16];

#define CELL_X_OFFSET 10
#define CELL_WIDTH 68

static int CELL_Y_OFFSETS[4] = {32, 58, 84, 110};

void tick_toolchest() {
    int step_count = get_health_metric_value(HealthMetricStepCount);
    int bpm = get_health_metric_value(HealthMetricHeartRateBPM);
    int distance = get_health_metric_value(HealthMetricWalkedDistanceMeters);

    if (step_count <= 0) {
        snprintf(steps_buffer, sizeof(steps_buffer), "%s", "N/A");
    } else {
        snprintf(steps_buffer, sizeof(steps_buffer), "%d", step_count);
    }

    if (bpm <= 0) {
        snprintf(bpm_buffer, sizeof(bpm_buffer), "%s", "N/A");
    } else {
        snprintf(bpm_buffer, sizeof(bpm_buffer), "%d", bpm);
    }

    if (distance <= 0) {
        snprintf(distance_buffer, sizeof(distance_buffer), "%s", "N/A");
    } else {
        enum DistanceUnit distance_unit = get_distance_unit();

        APP_LOG(APP_LOG_LEVEL_DEBUG, "distance = %d", distance);

        if (distance_unit == IMPERIAL) {
            print_float(distance_buffer, sizeof(distance_buffer),
                        distance / 1609.344, 2);
            strcat(distance_buffer, "mi");
        } else if (distance < 1000) {
            snprintf(distance_buffer, sizeof(distance_buffer), "%dm", distance);
        } else {
            print_float(distance_buffer, sizeof(distance_buffer),
                        distance / 1000.0, 2);
            strcat(distance_buffer, "km");
        }
    }

    snprintf(bat_buffer, sizeof(bat_buffer), "%d%%",
             battery_state_service_peek().charge_percent);

    text_layer_set_text(steps_text_layer, steps_buffer);
    text_layer_set_text(heartrate_text_layer, bpm_buffer);
    text_layer_set_text(distance_temp_layer, distance_buffer);
    text_layer_set_text(bat_text_layer, bat_buffer);

    APP_LOG(APP_LOG_LEVEL_INFO,
            "Toolchest ticked, step_count = %d, bpm = %d, distance = %d",
            step_count, bpm, distance);
}

void load_toolchest(Window* window) {
    Layer* window_layer = window_get_root_layer(window);

    steps_text_layer =
        text_layer_create(GRect(CELL_X_OFFSET, CELL_Y_OFFSETS[0], 50, 20));
    heartrate_text_layer =
        text_layer_create(GRect(CELL_X_OFFSET, CELL_Y_OFFSETS[1], 50, 20));
    distance_temp_layer =
        text_layer_create(GRect(CELL_X_OFFSET, CELL_Y_OFFSETS[2], 50, 20));
    bat_text_layer =
        text_layer_create(GRect(CELL_X_OFFSET, CELL_Y_OFFSETS[3], 50, 20));

    text_layer_set_text(steps_text_layer, "N/A");
    text_layer_set_text(heartrate_text_layer, "N/A");
    text_layer_set_text(distance_temp_layer, "N/A");
    text_layer_set_text(bat_text_layer, "N/A");

    text_layer_set_background_color(steps_text_layer, GColorClear);
    text_layer_set_text_color(steps_text_layer, GColorBlack);
    text_layer_set_font(steps_text_layer,
                        fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
    text_layer_set_text_alignment(steps_text_layer, GTextAlignmentLeft);

    text_layer_set_background_color(heartrate_text_layer, GColorClear);
    text_layer_set_text_color(heartrate_text_layer, GColorBlack);
    text_layer_set_font(heartrate_text_layer,
                        fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
    text_layer_set_text_alignment(heartrate_text_layer, GTextAlignmentLeft);

    text_layer_set_background_color(distance_temp_layer, GColorClear);
    text_layer_set_text_color(distance_temp_layer, GColorBlack);
    text_layer_set_font(distance_temp_layer,
                        fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
    text_layer_set_text_alignment(distance_temp_layer, GTextAlignmentLeft);

    text_layer_set_background_color(bat_text_layer, GColorClear);
    text_layer_set_text_color(bat_text_layer, GColorBlack);
    text_layer_set_font(bat_text_layer,
                        fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
    text_layer_set_text_alignment(bat_text_layer, GTextAlignmentLeft);

    layer_add_child(window_layer, text_layer_get_layer(steps_text_layer));
    layer_add_child(window_layer, text_layer_get_layer(heartrate_text_layer));
    layer_add_child(window_layer, text_layer_get_layer(distance_temp_layer));
    layer_add_child(window_layer, text_layer_get_layer(bat_text_layer));

    tick_toolchest();
}

void unload_toolchest(Window* window) {
    text_layer_destroy(steps_text_layer);
    text_layer_destroy(heartrate_text_layer);
    text_layer_destroy(distance_temp_layer);
    text_layer_destroy(bat_text_layer);
}

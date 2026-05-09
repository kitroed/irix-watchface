#include <pebble.h>

#include "stats.h"
#include "pblview.h"
#include "toolchest.h"
#include "xclock.h"

static Window* s_window;

static BitmapLayer *bg_track_layer;
static GBitmap *bg_bitmap;
static Layer* canvas;

static void battery_state_handler(BatteryChargeState charge) {
    tick_toolchest();
    tick_stats();
}

static void update_proc(Layer* layer, GContext* ctx) {
    graphics_context_set_compositing_mode(ctx, GCompOpSet);

    // BMC TODO Introduce the ability to configure the background color.
    redraw_xclock(ctx);
    redraw_pblview(ctx);
}

static void tick_handler(struct tm* tick_time, TimeUnits units_changed) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "tick_handler called.");

    tick_toolchest();
    tick_xclock();
    tick_pblview();

    layer_mark_dirty(canvas);

    time_t now = time(NULL);
    struct tm* local_time = localtime(&now);

    bool time_to_chime = local_time->tm_min == 0;
    bool can_chime = persist_read_bool(MESSAGE_KEY_CAN_CHIME_FLAG);

    APP_LOG(APP_LOG_LEVEL_INFO, "before: can_chime_flag = %d", can_chime);

    if (time_to_chime && can_chime) {
        vibes_double_pulse();
        can_chime = 0;
        APP_LOG(APP_LOG_LEVEL_INFO,
                "Ringing hourly chime and setting can_chime to false.");
    } else if (!time_to_chime) {
        can_chime = 1;
        APP_LOG(APP_LOG_LEVEL_INFO, "Cleared can_chime flag for next hour.");
    }

    persist_write_bool(MESSAGE_KEY_CAN_CHIME_FLAG, can_chime);
}

static void prv_window_load(Window* window) {
    Layer* window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);

    canvas = layer_create(GRect(0, 0, bounds.size.w, bounds.size.h));
    layer_set_update_proc(canvas, update_proc);

    bg_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BG);
    bg_track_layer = bitmap_layer_create(bounds);
    bitmap_layer_set_bitmap(bg_track_layer, bg_bitmap);

    layer_add_child(window_layer, bitmap_layer_get_layer(bg_track_layer));
    layer_add_child(window_layer, canvas);

    load_toolchest(window);
    load_xclock(window);
    load_pblview(window);
    load_stats(window);
}

static void prv_window_unload(Window* window) {
    layer_destroy(canvas);
    gbitmap_destroy(bg_bitmap);
    bitmap_layer_destroy(bg_track_layer);

    unload_toolchest(window);
    unload_xclock(window);
    unload_pblview(window);
    unload_stats(window);
}

static void prv_init(void) {
    s_window = window_create();
    window_set_window_handlers(s_window, (WindowHandlers){
                                             .load = prv_window_load,
                                             .unload = prv_window_unload,
                                         });
    const bool animated = true;
    window_stack_push(s_window, animated);

    connection_service_subscribe((ConnectionHandlers){
        .pebble_app_connection_handler = bluetooth_callback_stats});

    battery_state_service_subscribe(battery_state_handler);

    tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}

static void prv_deinit(void) { window_destroy(s_window); }

int main(void) {
    prv_init();

    APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p",
            s_window);

    app_event_loop();
    prv_deinit();
}

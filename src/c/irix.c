#include <pebble.h>

#include "config/settings.h"
#include "messaging/messaging.h"
#include "panels/pblview.h"
#include "panels/stats.h"
#include "panels/toolchest.h"
#include "panels/xclock.h"
#include "panels/background.h"

static Window* s_window;

static void battery_state_handler(BatteryChargeState charge) {
    tick_toolchest();
    tick_stats();
}

static void tick_handler(struct tm* tick_time, TimeUnits units_changed) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "tick_handler called.");

    tick_toolchest();
    tick_xclock();
    tick_pblview();

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
    load_bg(window);
    load_toolchest(window);
    load_xclock(window);
    load_pblview(window);
    load_stats(window);
}

static void prv_window_unload(Window* window) {
    unload_bg(window);
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

    app_message_register_inbox_received(inbox_received);
    app_message_register_inbox_dropped(inbox_dropped);
    app_message_open(128, 128);
}

static void prv_deinit(void) { window_destroy(s_window); }

int main(void) {
    load_settings();

    prv_init();

    APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p",
            s_window);

    app_event_loop();
    prv_deinit();
}

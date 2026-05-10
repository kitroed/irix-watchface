#include "messaging.h"

#include "../config/settings.h"
#include "../panels/xclock.h"
#include "../panels/toolchest.h"

void inbox_received(DictionaryIterator* iter, void* context) {
    Tuple* bg_color = dict_find(iter, MESSAGE_KEY_BG_COLOR);
    Tuple* clock_hands_color = dict_find(iter, MESSAGE_KEY_CLOCK_HANDS_COLOR);
    Tuple* xclock_disp_mode = dict_find(iter, MESSAGE_KEY_XCLOCK_MODE);
    Tuple* distance_unit = dict_find(iter, MESSAGE_KEY_DISTANCE_UNIT);

    if (distance_unit) {
        set_distance_unit(atoi(distance_unit->value->cstring));
        APP_LOG(APP_LOG_LEVEL_DEBUG, "distance_unit = %d", get_distance_unit());
        tick_toolchest();
    }

    if (bg_color) {
        set_bg_color(GColorFromHEX(bg_color->value->int32));
        APP_LOG(APP_LOG_LEVEL_DEBUG, "bg_color = %d", get_bg_color());
    }

    if (clock_hands_color) {
        set_clock_hands_color(GColorFromHEX(clock_hands_color->value->int32));
        APP_LOG(APP_LOG_LEVEL_DEBUG, "clock_hands_color = %d",
                get_clock_hands_color());

        redraw_xclock();
        light_enable_interaction();
    }

    if (xclock_disp_mode) {
        set_clock_display_mode(atoi(xclock_disp_mode->value->cstring));
        APP_LOG(APP_LOG_LEVEL_DEBUG, "clock_display_mode = %d",
                get_clock_display_mode());
    }

    save_settings();
}

void inbox_dropped(AppMessageResult reason, void* context) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped. Reason: %d", (int)reason);
}

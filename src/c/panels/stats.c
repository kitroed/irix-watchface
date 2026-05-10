#include "stats.h"

static const VibePattern BT_STATE_PATTERN = {
    .durations = (uint32_t[]){100, 100, 100, 100, 100, 100, 100, 100},
    .num_segments = 8,
};

static BitmapLayer *chrg_bitmap_layer, *bt_bmp_layer;
static GBitmap *chrg_bitmap, *bt_bitmap;

void bluetooth_callback_stats(bool connected) {
    layer_set_hidden(bitmap_layer_get_layer(bt_bmp_layer), !connected);
    vibes_enqueue_custom_pattern(BT_STATE_PATTERN);

    APP_LOG(APP_LOG_LEVEL_INFO,
            "bluetooth connection state has changed, connected=%d", connected);
}

void tick_stats() {
    layer_set_hidden(bitmap_layer_get_layer(chrg_bitmap_layer),
                     !battery_state_service_peek().is_charging);
}

void load_stats(Window* window) {
    Layer* window_layer = window_get_root_layer(window);

    chrg_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_CHARGING);
    chrg_bitmap_layer = bitmap_layer_create(GRect(25, 138, 25, 25));
    bitmap_layer_set_bitmap(chrg_bitmap_layer, chrg_bitmap);
    bitmap_layer_set_compositing_mode(chrg_bitmap_layer, GCompOpSet);

    layer_set_hidden(bitmap_layer_get_layer(chrg_bitmap_layer),
                     !battery_state_service_peek().is_charging);

    bt_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BT);
    bt_bmp_layer = bitmap_layer_create(GRect(2, 138, 25, 25));
    bitmap_layer_set_bitmap(bt_bmp_layer, bt_bitmap);
    bitmap_layer_set_compositing_mode(bt_bmp_layer, GCompOpSet);

    layer_set_hidden(bitmap_layer_get_layer(bt_bmp_layer),
                     !connection_service_peek_pebble_app_connection());

    layer_add_child(window_layer, bitmap_layer_get_layer(bt_bmp_layer));
    layer_add_child(window_layer, bitmap_layer_get_layer(chrg_bitmap_layer));

    tick_stats();
}

void unload_stats(Window* window) {
    gbitmap_destroy(chrg_bitmap);
    gbitmap_destroy(bt_bitmap);

    bitmap_layer_destroy(chrg_bitmap_layer);
    bitmap_layer_destroy(bt_bmp_layer);
}

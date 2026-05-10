#include "background.h"

#include "../config/settings.h"

static BitmapLayer* bg_track_layer;
static GBitmap *bg_bitmap, *bg_analog_bitmap, *bg_digital_bitmap;
static Layer* canvas;

void update_bg_canvas(Layer* layer, GContext* ctx) {
    GRect bounds = layer_get_bounds(layer);
    graphics_context_set_fill_color(ctx, get_bg_color());
    graphics_fill_rect(ctx, bounds, 0, GCornerNone);
}

void update_clock_bg() {
    switch (get_clock_display_mode()) {
        case DIGITAL:
            bitmap_layer_set_bitmap(bg_track_layer, bg_digital_bitmap);
            break;
        case ANALOG:
            bitmap_layer_set_bitmap(bg_track_layer, bg_analog_bitmap);
            break;
        default:
            bitmap_layer_set_bitmap(bg_track_layer, bg_bitmap);
            break;
    }
}

void redraw_bg() { layer_mark_dirty(canvas); }

void load_bg(Window* window) {
    Layer* window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);

    canvas = layer_create(GRect(0, 0, bounds.size.w, bounds.size.h));
    layer_set_update_proc(canvas, update_bg_canvas);

    bg_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BG);
    bg_digital_bitmap =
        gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BG_DIGITAL);
    bg_analog_bitmap =
        gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BG_ANALOG);

    bg_track_layer = bitmap_layer_create(bounds);

    update_clock_bg();

    bitmap_layer_set_compositing_mode(bg_track_layer, GCompOpSet);

    layer_add_child(window_layer, canvas);
    layer_add_child(window_layer, bitmap_layer_get_layer(bg_track_layer));
}

void unload_bg(Window* window) {
    layer_destroy(canvas);
    gbitmap_destroy(bg_bitmap);
    gbitmap_destroy(bg_digital_bitmap);
    gbitmap_destroy(bg_analog_bitmap);

    bitmap_layer_destroy(bg_track_layer);
}
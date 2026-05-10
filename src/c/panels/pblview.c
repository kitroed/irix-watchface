#include "pblview.h"

#define BAR_HEIGHT 10
#define BAR_WIDTH 126
#define BAR_X 55
#define BAR_Y 200
#define METRIC_WIDTH 3

static Layer* canvas;
static TextLayer* cpu_text_layer;

void update_pblview_canvas(Layer* layer, GContext* ctx) {
    graphics_context_set_stroke_color(ctx, GColorBlack);
    graphics_draw_rect(ctx, GRect(BAR_X, BAR_Y, BAR_WIDTH + 2, BAR_HEIGHT + 2));

    for (int x = 0; x < BAR_WIDTH; x += METRIC_WIDTH) {
        int g_h = (rand() % BAR_HEIGHT) + 1;
        int y_h = g_h < 10 ? (rand() % (BAR_HEIGHT - g_h)) + 1 : 0;
        int r_h = (g_h + y_h) < 10 ? BAR_HEIGHT - g_h - y_h : 0;

        graphics_context_set_fill_color(ctx, GColorIslamicGreen);
        GRect g_bar = GRect(BAR_X + 1 + x, BAR_Y + 1, METRIC_WIDTH, g_h);
        graphics_fill_rect(ctx, g_bar, 0, GCornerNone);

        graphics_context_set_fill_color(ctx, GColorIcterine);
        GRect y_bar = GRect(BAR_X + 1 + x, BAR_Y + 1 + g_h, METRIC_WIDTH, y_h);
        graphics_fill_rect(ctx, y_bar, 0, GCornerNone);

        graphics_context_set_fill_color(ctx, GColorRed);
        GRect r_bar =
            GRect(BAR_X + 1 + x, BAR_Y + 1 + g_h + y_h, METRIC_WIDTH, r_h);
        graphics_fill_rect(ctx, r_bar, 0, GCornerNone);
    }
}

void tick_pblview() {}

void load_pblview(Window* window) {
    Layer* window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);

    cpu_text_layer = text_layer_create(GRect(12 + 4, 193, 50, 20));

    text_layer_set_text(cpu_text_layer, "CPU0");

    text_layer_set_background_color(cpu_text_layer, GColorClear);
    text_layer_set_text_color(cpu_text_layer, GColorBlack);
    text_layer_set_font(cpu_text_layer,
                        fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
    text_layer_set_text_alignment(cpu_text_layer, GTextAlignmentLeft);

    canvas = layer_create(GRect(0, 0, bounds.size.w, bounds.size.h));
    layer_set_update_proc(canvas, update_pblview_canvas);

    layer_add_child(window_layer, canvas);
    layer_add_child(window_layer, text_layer_get_layer(cpu_text_layer));

    tick_pblview();
}

void unload_pblview(Window* window) {
    layer_destroy(canvas);
    text_layer_destroy(cpu_text_layer);
}

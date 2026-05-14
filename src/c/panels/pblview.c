#include "pblview.h"

#define BAR_HEIGHT 10
#define BAR_WIDTH 126
#define BAR_X 55
#define BAR_Y 200
#define METRIC_WIDTH 3
#define NUM_COLUMNS (BAR_WIDTH / METRIC_WIDTH)  // 42

// How far back to scan for HR samples. Sized so that even at the watch's
// slowest "every 10 minutes" sampling cadence we can comfortably overfill
// the 42-column bar. Bumping this further is cheap (12 bytes per minute,
// sits in .bss).
#define LOOKBACK_MINUTES 600  // 10 hours

static Layer* canvas;
static TextLayer *cpu_text_layer, *title_text_layer;

// Most-recent BPM samples, right-aligned: hr_history[NUM_COLUMNS - 1] is the
// freshest minute. 0 means "no reading" (column draws empty).
static uint8_t hr_history[NUM_COLUMNS];

// Buffer for the dynamic title "pblview (min/avg/max)". Sized for 3-digit
// BPM values: "pblview (255/255/255)\0" = 22 chars; 32 leaves headroom.
static char title_buffer[32];

// Map a percentage [0..100] of the observed BPM range to stacked segment
// heights. The bar is autoscaled against the current history's min/max so
// the visualization is always informative regardless of resting/active
// baseline. Layout (bottom-up: red, then yellow, then green):
//
//     0% -> full green column     (no yellow, no red)
//    25% -> 5px yellow / 5px green (yellow fills bottom half)
//    50% -> full yellow column    (no green, no red)
//    75% -> 5px red / 5px yellow  (red fills bottom half)
//   100% -> full red column       (no yellow, no green)
//
// 0-50% scales yellow up from the floor, displacing green.
// 50-100% scales red up from the floor, displacing yellow.
// Total filled height is always BAR_HEIGHT.
static void pct_to_segments(int pct, int* g_h, int* y_h, int* r_h) {
    if (pct < 0) pct = 0;
    if (pct > 100) pct = 100;

    if (pct <= 50) {
        *y_h = pct * BAR_HEIGHT / 50;
        *g_h = BAR_HEIGHT - *y_h;
        *r_h = 0;
    } else {
        *r_h = (pct - 50) * BAR_HEIGHT / 50;
        *y_h = BAR_HEIGHT - *r_h;
        *g_h = 0;
    }
}

void update_pblview_canvas(Layer* layer, GContext* ctx) {
    graphics_context_set_stroke_color(ctx, GColorBlack);
    graphics_draw_rect(ctx, GRect(BAR_X, BAR_Y, BAR_WIDTH + 2, BAR_HEIGHT + 2));

    // Autoscale: find the min and max BPM among the non-zero samples in
    // the current history. The lowest observed reading anchors 0% (all
    // green), the highest anchors 100% (all red).
    uint8_t min_bpm = 255;
    uint8_t max_bpm = 0;
    for (int i = 0; i < NUM_COLUMNS; i++) {
        uint8_t v = hr_history[i];
        if (v == 0) continue;
        if (v < min_bpm) min_bpm = v;
        if (v > max_bpm) max_bpm = v;
    }
    // If we have no samples, or every sample is identical, there's no
    // range to scale against — treat every reading as 0% (all green).
    const bool has_range = max_bpm > min_bpm;

    // Interior runs from y = BAR_Y + 1 (top) to y = BAR_Y + 1 + BAR_HEIGHT
    // (exclusive bottom). Columns are drawn bottom-up: red at the floor,
    // yellow above red, green filling the rest to the top.
    const int bottom_y = BAR_Y + 1 + BAR_HEIGHT;

    for (int i = 0; i < NUM_COLUMNS; i++) {
        uint8_t bpm = hr_history[i];
        int g_h = 0, y_h = 0, r_h = 0;

        if (bpm > 0) {
            int pct = has_range
                          ? (bpm - min_bpm) * 100 / (max_bpm - min_bpm)
                          : 0;
            pct_to_segments(pct, &g_h, &y_h, &r_h);
        }

        int x = BAR_X + 1 + i * METRIC_WIDTH;

        graphics_context_set_fill_color(ctx, GColorRed);
        graphics_fill_rect(ctx, GRect(x, bottom_y - r_h, METRIC_WIDTH, r_h), 0,
                           GCornerNone);

        graphics_context_set_fill_color(ctx, GColorIcterine);
        graphics_fill_rect(
            ctx, GRect(x, bottom_y - r_h - y_h, METRIC_WIDTH, y_h), 0,
            GCornerNone);

        graphics_context_set_fill_color(ctx, GColorIslamicGreen);
        graphics_fill_rect(
            ctx, GRect(x, bottom_y - r_h - y_h - g_h, METRIC_WIDTH, g_h), 0,
            GCornerNone);
    }
}

void tick_pblview() {
    // Refresh the entire window from Pebble's stored minute history each
    // tick. The minute-history API is the source of truth, so we don't
    // keep our own ring buffer.
    //
    // The watch can be configured to sample HR as rarely as once every 10
    // minutes, which means most minute records have heart_rate_bpm == 0
    // ("no reading"). We scan a wide window (LOOKBACK_MINUTES) and keep
    // only the most-recent NUM_COLUMNS valid+nonzero readings, packed
    // right-aligned so the freshest reading lands in the last column.
    //
    // The buffer is `static` so it lives in .bss instead of blowing the
    // tiny app stack — ~5.7 KB at LOOKBACK_MINUTES = 480.
    static HealthMinuteData minute_data[LOOKBACK_MINUTES];

    time_t end = time(NULL);
    time_t start = end - (LOOKBACK_MINUTES * 60);

    for (int i = 0; i < NUM_COLUMNS; i++) hr_history[i] = 0;

    // Note: deliberately *not* gating on health_service_metric_accessible.
    // That accessibility check is documented as a precondition for the
    // `health_service_sum` family, and for HR + wide windows it tends to
    // return NotAvailable whenever the watch had any gap in coverage —
    // which left the bar completely blank. The minute-history call is
    // safe to make unconditionally and will return 0 records if there's
    // genuinely nothing to read.
    uint32_t num_records = health_service_get_minute_history(
        minute_data, LOOKBACK_MINUTES, &start, &end);

    if (num_records == 0) {
        APP_LOG(APP_LOG_LEVEL_INFO,
                "pblview: minute_history returned 0 records "
                "(start=%ld, end=%ld).",
                (long)start, (long)end);
        return;
    }

    // Walk newest-first, taking up to NUM_COLUMNS valid samples. Place
    // them right-aligned in hr_history so the newest reading lands in
    // the rightmost column.
    int filled = 0;
    for (int i = (int)num_records - 1; i >= 0 && filled < NUM_COLUMNS; i--) {
        if (minute_data[i].is_invalid) continue;
        if (minute_data[i].heart_rate_bpm == 0) continue;
        hr_history[NUM_COLUMNS - 1 - filled] = minute_data[i].heart_rate_bpm;
        filled++;
    }

    // Summarize the displayed history into the title bar. Walks the same
    // hr_history we just wrote so the title matches exactly what the bar
    // is showing.
    uint8_t min_bpm = 255, max_bpm = 0;
    uint32_t sum = 0;
    int count = 0;
    for (int i = 0; i < NUM_COLUMNS; i++) {
        uint8_t v = hr_history[i];
        if (v == 0) continue;
        if (v < min_bpm) min_bpm = v;
        if (v > max_bpm) max_bpm = v;
        sum += v;
        count++;
    }
    if (count == 0) {
        title_buffer[0] = '\0';
    } else {
        int avg = (int)(sum / (uint32_t)count);
        snprintf(title_buffer, sizeof(title_buffer), "(%u/%d/%u)",
                 (unsigned)min_bpm, avg, (unsigned)max_bpm);
    }
    text_layer_set_text(title_text_layer, title_buffer);

    APP_LOG(APP_LOG_LEVEL_DEBUG,
            "pblview: scanned %lu minute records, kept %d valid HR samples "
            "(newest BPM=%u, title=\"%s\").",
            (unsigned long)num_records, filled, hr_history[NUM_COLUMNS - 1],
            title_buffer);
}

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

    // Render the dynamic (min/avg/max) summary to the right of the chrome's
    // static "pblview" title. Transparent background so the title-bar
    // stripes show through.
    title_text_layer = text_layer_create(GRect(91, 172, 70, 16));
    text_layer_set_text(title_text_layer, "");
    text_layer_set_background_color(title_text_layer, GColorClear);
    text_layer_set_text_color(title_text_layer, GColorBlack);
    text_layer_set_font(title_text_layer,
                        fonts_get_system_font(FONT_KEY_GOTHIC_14));
    text_layer_set_text_alignment(title_text_layer, GTextAlignmentLeft);

    canvas = layer_create(GRect(0, 0, bounds.size.w, bounds.size.h));
    layer_set_update_proc(canvas, update_pblview_canvas);

    layer_add_child(window_layer, canvas);
    layer_add_child(window_layer, text_layer_get_layer(cpu_text_layer));
    layer_add_child(window_layer, text_layer_get_layer(title_text_layer));

    tick_pblview();
}

void unload_pblview(Window* window) {
    layer_destroy(canvas);
    text_layer_destroy(cpu_text_layer);
    text_layer_destroy(title_text_layer);
}

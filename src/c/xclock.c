#include "xclock.h"
#include "utils.h"

#define CLOCK_X_OFFSET 85
#define CLOCK_Y_OFFSET 32

static TextLayer *date_text_layer, *clock_text_layer;
static char date_buffer[16], clock_buffer[32];

static float hour_hand_angle = 0, minute_hand_angle = 0;

static GSize minute_hand_rect = GSize(70, 70);
static GSize hour_hand_rect = GSize(51, 51);
static GPoint clock_center = GPoint(136, 110);
static GRect minute_hand_bounds, hour_hand_bounds;

static int local_hour = 0;

static int addntl_offsets[12][2] = {
    {0, 0},  // 12
    {1, 0},  // 1
    {0, 0},  // 2
    {0, 1},  // 3
    {0, 0},  // 4
    {1, 0},  // 5
    {1, 0},  // 6
    {1, 0},  // 7
    {0, 1},  // 8
    {0, 1},  // 9
    {0, 0},  // 10
    {0, 0}   // 11
};

void redraw_xclock(GContext* ctx) {
    // BMC TODO, this could probably use some refactoring.
    
    // Calcuate the point along the clock's circle where the tip of the hand
    // should point.
    GPoint minute_hand_point =
        gpoint_from_polar(minute_hand_bounds, GOvalScaleModeFitCircle,
                          DEG_TO_TRIGANGLE(minute_hand_angle));
    GPoint hour_hand_point =
        gpoint_from_polar(hour_hand_bounds, GOvalScaleModeFitCircle,
                          DEG_TO_TRIGANGLE(hour_hand_angle));

    GPoint hour_hand_center = GPoint(clock_center.x, clock_center.y);
    hour_hand_center.x += addntl_offsets[local_hour][0];
    hour_hand_center.y += addntl_offsets[local_hour][1];

    // Calculate the magnitude of each hand.
    int minute_dx = minute_hand_point.x - clock_center.x;
    int minute_dy = minute_hand_point.y - clock_center.y;
    int hour_dx = hour_hand_point.x - clock_center.x;
    int hour_dy = hour_hand_point.y - clock_center.y;

    float minute_mag = my_sqrt(minute_dx * minute_dx + minute_dy * minute_dy);
    float hour_mag = my_sqrt(hour_dx * hour_dx + hour_dy * hour_dy);

    // Half of the triangle's base.
    float half_width = 6.5f;

    // Calculate the offset we need to apply so that the normal vector's create
    // a triangle with the base size we want.
    int minute_x = (int)((-minute_dy / minute_mag) * half_width);
    int minute_y = (int)((minute_dx / minute_mag) * half_width);
    int hour_x = (int)((-hour_dy / hour_mag) * half_width);
    int hour_y = (int)((hour_dx / hour_mag) * half_width);

    // Calculate an offset to move the center from the bottom of the triangle.
    float pivot_offset = 2.0f;
    int minute_ox = (int)(minute_dx / minute_mag * pivot_offset);
    int minute_oy = (int)(minute_dy / minute_mag * pivot_offset);
    int hour_ox = (int)(hour_dx / hour_mag * pivot_offset);
    int hour_oy = (int)(hour_dy / hour_mag * pivot_offset);

    // Construct the path info containing the three points of each triangle.
    GPathInfo minute_path_info = {
        .num_points = 3,
        .points = (GPoint[]){GPoint(clock_center.x - minute_x - minute_ox,
                                    clock_center.y - minute_y - minute_oy),
                             GPoint(clock_center.x + minute_x - minute_ox,
                                    clock_center.y + minute_y - minute_oy),
                             GPoint(minute_hand_point.x - minute_ox,
                                    minute_hand_point.y - minute_oy)}};

    GPathInfo hour_path_info = {
        .num_points = 3,
        .points = (GPoint[]){
            GPoint(clock_center.x - hour_x - hour_ox,
                   clock_center.y - hour_y - hour_oy),
            GPoint(clock_center.x + hour_x - hour_ox,
                   clock_center.y + hour_y - hour_oy),
            GPoint(hour_hand_point.x - hour_ox, hour_hand_point.y - hour_oy)}};

    GPath* minute_hand = gpath_create(&minute_path_info);
    GPath* hour_hand = gpath_create(&hour_path_info);

    graphics_context_set_fill_color(ctx, GColorVividCerulean);
    gpath_draw_filled(ctx, minute_hand);
    gpath_draw_filled(ctx, hour_hand);

    graphics_context_set_stroke_color(ctx, GColorBlack);
    gpath_draw_outline(ctx, minute_hand);
    gpath_draw_outline(ctx, hour_hand);

    gpath_destroy(minute_hand);
    gpath_destroy(hour_hand);
}

void tick_xclock() {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Ticked xclock.");

    time_t now = time(NULL);
    struct tm* local_time = localtime(&now);

    minute_hand_angle = local_time->tm_min * (360.0f / 60.0f);
    hour_hand_angle = (local_time->tm_hour * (360.0f / 12.0f)) +
                      (local_time->tm_min * (30.0f / 60.0f));
    local_hour = my_abs(local_time->tm_hour - 12);

    APP_LOG(APP_LOG_LEVEL_INFO,
            "clock info: minutes = %d, minute_hand_angle = %d, hour_hand_angle "
            "= %d",
            local_time->tm_min, (int)minute_hand_angle, (int)hour_hand_angle);

    if (clock_is_24h_style()) {
        strftime(clock_buffer, sizeof(clock_buffer), "%H:%M ", local_time);
    } else {
        strftime(clock_buffer, sizeof(clock_buffer), "%I:%M %p ", local_time);
    }

    strcat(clock_buffer, local_time->tm_isdst > 0 ? "DST" : "");

    strftime(date_buffer, sizeof(date_buffer), "%a %m/%d", local_time);
    text_layer_set_text(clock_text_layer, clock_buffer);
    text_layer_set_text(date_text_layer, date_buffer);
}

void load_xclock(Window* window) {
    minute_hand_bounds = GRect(clock_center.x - (minute_hand_rect.w / 2),
                               clock_center.y - (minute_hand_rect.h / 2),
                               minute_hand_rect.w, minute_hand_rect.h);

    hour_hand_bounds = GRect(clock_center.x - (hour_hand_rect.w / 2),
                             clock_center.y - (hour_hand_rect.h / 2),
                             hour_hand_rect.w, hour_hand_rect.h);

    Layer* window_layer = window_get_root_layer(window);

    clock_text_layer =
        text_layer_create(GRect(CLOCK_X_OFFSET, CLOCK_Y_OFFSET, 103, 20));
    date_text_layer =
        text_layer_create(GRect(CLOCK_X_OFFSET, CLOCK_Y_OFFSET + 13, 103, 20));

    text_layer_set_text(clock_text_layer, "N/A");
    text_layer_set_text(date_text_layer, "N/A");

    text_layer_set_background_color(clock_text_layer, GColorClear);
    text_layer_set_text_color(clock_text_layer, GColorBlack);
    text_layer_set_font(clock_text_layer,
                        fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
    text_layer_set_text_alignment(clock_text_layer, GTextAlignmentCenter);

    text_layer_set_background_color(date_text_layer, GColorClear);
    text_layer_set_text_color(date_text_layer, GColorBlack);
    text_layer_set_font(date_text_layer,
                        fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
    text_layer_set_text_alignment(date_text_layer, GTextAlignmentCenter);

    layer_add_child(window_layer, text_layer_get_layer(clock_text_layer));
    layer_add_child(window_layer, text_layer_get_layer(date_text_layer));

    tick_xclock();
}

void unload_xclock(Window* window) {
    text_layer_destroy(clock_text_layer);
    text_layer_destroy(date_text_layer);
}

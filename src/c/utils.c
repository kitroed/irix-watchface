#include "utils.h"

HealthValue get_health_metric_value(HealthMetric metric) {
    if (metric == HealthMetricHeartRateBPM) {
        return health_service_peek_current_value(metric);
    }

    time_t then = time_start_of_today();
    time_t now = time(NULL);

    if (!(health_service_metric_accessible(metric, then, now) &
          HealthServiceAccessibilityMaskAvailable))
        return -1;

    return (int)health_service_sum_today(metric);
}

double my_abs(double val) { return val < 0 ? -val : val; }

double my_sqrt(double val) {
    if (val < 0) return 0;
    if (val == 0) return 0;

    float guess = val / 2.0f;
    float precision = 0.000001f;

    while (1) {
        float next_guess = (guess + val / guess) / 2.0f;
        if (my_abs(guess - next_guess) < precision) {
            return next_guess;
        }

        guess = next_guess;
    }
}

void print_float(char* buffer, int buffer_size, float value, int precision) {
    char whole_buffer[8], part_buffer[8];
    memset(part_buffer, 0, sizeof(part_buffer));

    value = my_abs(value);

    size_t whole = (size_t)value;
    float part = (value - whole);

    snprintf(whole_buffer, sizeof(whole_buffer), "%d", whole);

    char tmp[8];

    for (int i = 0; i < precision; ++i) {
        part *= 10;
        size_t digit = (size_t)part;

        snprintf(tmp, sizeof(tmp), "%d", digit);
        strcat(part_buffer, tmp);

        part -= digit;
    }

    memset(buffer, 0, buffer_size);
    strcat(buffer, whole_buffer);
    strcat(buffer, ".");
    strcat(buffer, part_buffer);

    APP_LOG(APP_LOG_LEVEL_DEBUG, "buffer = %s", buffer);
}

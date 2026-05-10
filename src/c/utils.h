#include <pebble.h>

#ifndef UTILS_H
#define UTILS_H

HealthValue get_health_metric_value(HealthMetric metric);
double my_abs(double val);
double my_sqrt(double val);
void print_float(char* buffer, int buffer_size, float value, int precision);

#endif

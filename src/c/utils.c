#include "utils.h"

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

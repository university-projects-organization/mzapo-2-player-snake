#include <stdint.h>
#include <stdio.h>

#include "apo_library/mzapo_regs.h"
#include "knobs_control.h"


int8_t knobRotated(uint8_t actualValue, uint8_t *previousValue, int8_t positions) {
    int16_t difference = ((actualValue - *previousValue) >> 2);
    difference = difference == 63 ? -1 : difference == -63 ? 1 : difference;
    int8_t step = difference % positions;
    *previousValue = actualValue;
    return step;
}

int8_t knobRotated2(uint32_t actualValue, uint32_t *previousValue, int8_t knob, int8_t positions) {
    int coef;
    switch (knob) {
        case BLUE:
            coef = 4;
            break;
        case GREEN:
            coef = 1024;
            break;
        default:
            coef = 262144;
            break;
    }
    int16_t difference = (actualValue - *previousValue) / coef;
    difference = difference == 63 ? -1 : difference == -63 ? 1 : difference;
    int8_t step = difference % positions;
    // printf("step - %d\n", step);
    *previousValue = actualValue;
    return step;
}
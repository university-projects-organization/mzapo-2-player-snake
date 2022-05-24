#include <stdint.h>
#include <stdio.h>

#include "apo_library/mzapo_regs.h"
#include "knobs_control.h"


int8_t knobRotated(uint8_t actualValue, uint8_t *previousValue, int8_t positions) {
    int16_t difference = ((actualValue - *previousValue) >> 2);
    difference = difference == 63 ? -1 : difference == -63 ? 1 : difference;
    int8_t step;
    if (positions != 0) {
        step = difference % positions;
    } else {
        step = difference;
    }
    *previousValue = actualValue;
    return step;
}

void knobUnpressed(volatile void *spiled_reg_base) {
    while (1) {
        uint32_t actualKnobs = *(volatile uint32_t *) (spiled_reg_base + SPILED_REG_KNOBS_8BIT_o);
        uint8_t knobUnpressed = (uint8_t) (actualKnobs >> 24);
        if (knobUnpressed == 0) {
            break;
        }
    }
}
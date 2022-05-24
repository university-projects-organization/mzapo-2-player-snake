#ifndef KNOBS_CONTROL_H
#define KNOBS_CONTROL_H

#include <stdint.h>

int8_t knobRotated(uint8_t actualValue, uint8_t *previousValue, int8_t positions);

void knobUnpressed(volatile void *spiled_reg_base);

#endif
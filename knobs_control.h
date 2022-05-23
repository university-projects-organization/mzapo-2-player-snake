#ifndef KNOBS_CONTROL_H
#define KNOBS_CONTROL_H

#include <stdint.h>

#define BLUEPRESSED 16777216
#define GREENPRESSED 16777216 * 2
#define REDPRESSED 16777216 * 4
#define BLUE 0
#define GREEN 1
#define RED 2

int8_t knobRotated(uint32_t actualValue, uint32_t *previousValue, int8_t knob, int8_t positions);

_Bool knobPressed(uint32_t actualKnob, uint32_t knobDigit, uint32_t knob, volatile void *spiled_reg_base);

#endif
#ifndef KNOBS_CONTROL_H
#define KNOBS_CONTROL_H

#include <stdint.h>

int8_t knobRotated(uint32_t actualValue, uint32_t *previousValue, int8_t knob, int8_t positions);

_Bool knobPressed(uint32_t actualKnob, uint32_t knobDigit, uint32_t knob, volatile void *spiled_reg_base);

void menuPosition(uint32_t actual, uint32_t *previous, int knob, settingsParameter** settingsParameters, int8_t index, int8_t numOptions);

#endif
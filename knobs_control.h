#ifndef KNOBS_CONTROL_H
#define KNOBS_CONTROL_H

#include <stdint.h>

<<<<<<< HEAD
#define BLUEPRESSED 16777216
#define GREENPRESSED 16777216 * 2
#define REDPRESSED 16777216 * 4
#define BLUE 0
#define GREEN 1
#define RED 2

=======
>>>>>>> b878198bbf26b6f78952c51c8218db35dc7948cb
int8_t knobRotated(uint32_t actualValue, uint32_t *previousValue, int8_t knob, int8_t positions);

_Bool knobPressed(uint32_t actualKnob, uint32_t knobDigit, uint32_t knob, volatile void *spiled_reg_base);

<<<<<<< HEAD
=======
void menuPosition(uint32_t actual, uint32_t *previous, int knob, settingsParameter** settingsParameters, int8_t index, int8_t numOptions);

>>>>>>> b878198bbf26b6f78952c51c8218db35dc7948cb
#endif
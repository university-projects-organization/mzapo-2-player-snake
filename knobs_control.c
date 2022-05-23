
#include <stdint.h>

#define BLUEPRESSED 16777216
#define GREENPRESSED 16777216 * 2
#define REDPRESSED 16777216 * 4
#define BLUE 0
#define GREEN 1
#define RED 2

int8_t knobRotated(uint32_t actualValue, uint32_t *previousValue, int8_t knob, int8_t positions) {
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
    *previousValue = actualValue;
    return step;
}

_Bool knobPressed(uint32_t actualKnob, uint32_t knobDigit, uint32_t knob, volatile void *spiled_reg_base) {
    uint32_t act = *(volatile uint32_t *) (spiled_reg_base + SPILED_REG_KNOBS_8BIT_o);
    if (actualKnob - knobDigit == knob) {
        while (act == actualKnob) {
            act = *(volatile uint32_t *) (spiled_reg_base + SPILED_REG_KNOBS_8BIT_o);
        }
        return 1;
    }
    return 0;

}


void menuPosition(uint32_t actual, uint32_t *previous, int knob, settingsParameter** settingsParameters, int8_t index, int8_t numOptions) {
    int8_t step = knobRotated(actual, previous, knob, numOptions);
    settingsParameters[index]->position = (settingsParameters[index]->position + step + numOptions) % numOptions;
}
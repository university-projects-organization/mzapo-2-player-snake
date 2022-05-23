#ifndef SETTINGS_MENU_H
#define SETTINGS_MENU_H

#include <stdint.h>
#include "pixel.h"
#include "settings_struct.h"

void loadSettingsMenu(union pixel **background, union pixel **screen, settingsParameter** settingsParameters, unsigned char *selector, unsigned char *parlcd_reg_base, uint16_t selectorY);

void menuPosition(uint32_t actual, uint32_t *previous, int knob, settingsParameter** settingsParameters, int8_t index, int8_t numOptions);

void settingsMenu(union pixel **screen, volatile void *spiled_reg_base, unsigned char *parlcd_reg_base, uint8_t *settings, uint32_t* actualG);

#endif


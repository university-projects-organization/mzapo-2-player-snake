#ifndef SETTINGS_ALLOCATE_H
#define SETTINGS_ALLOCATE_H

#include <stdint.h>
#include "settings_structure.h"

settingsParameter **allocateSettingsParameter(uint8_t *settings);

void freeSettingsParameter(settingsParameter **settingsParameters);

#endif
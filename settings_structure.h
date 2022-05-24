#ifndef SETTINGS_STRUCT_H
#define SETTINGS_STRUCT_H

#include <stdint.h>
#include "wordbuffer_structure.h"

typedef struct {
    wordBuffer **string;
    uint16_t position;
} settingsParameter;

#endif
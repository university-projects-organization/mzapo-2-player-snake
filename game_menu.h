#ifndef GAME_MENU_H
#define GAME_MENU_H

#include "pixel.h"
#include <stdint.h>

void endGame(union pixel **screen, union pixel **background, unsigned char *selector, uint8_t *settings);

_Bool gameMenu(union pixel **screen, uint8_t *settings, volatile void *spiled_reg_base, unsigned char *parlcd_reg_base);

#endif
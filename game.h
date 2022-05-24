#ifndef GAME_H
#define GAME_H

#include <stdint.h>
#include "snake.h"
#include "screen.h"

void setUpGame(uint8_t *settings, uint16_t *color1, uint16_t *color2, _Bool *foodOwner, uint8_t *foodAmount, _Bool *vs,
               uint8_t *speed, uint8_t *boost);

void
freeAllocatedMemory(union pixel **screen, union pixel **background, snake_t *snake1, snake_t *snake2, food_t *food1,
                    food_t *food2);

void gameOver(snake_t *snake, uint16_t color, union pixel **screen, volatile void *spiled_reg_base,
              unsigned char *parlcd_reg_base, int8_t winner, uint32_t *colorLed1, uint32_t *colorLed2);

void game(uint8_t *settings, volatile void *spiled_reg_base, unsigned char *parlcd_reg_base, union pixel **screen, uint32_t *colorLed1, uint32_t *colorLed2);

#endif
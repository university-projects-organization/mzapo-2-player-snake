#ifndef SNAKE_H
#define SNAKE_H

#include <stdint.h>
#include "food_structure.h"
#include "snake_structure.h"
#include "pixel.h"

void setTile(snake_t *snake, uint16_t x, uint16_t y, uint8_t index, uint8_t direction);

snake_t *
allocateSnake(unsigned char *imageTail, unsigned char *imageBody, unsigned char *imageHead, unsigned char *color1,
              uint16_t x, uint16_t y, int8_t direction);

void freeSnake(snake_t *snake);

void moveForward(snake_t *snake);

void up(tile_t tile, unsigned char *image, union pixel **screen);

void down(tile_t tile, unsigned char *image, union pixel **screen);

void left(tile_t tile, unsigned char *image, union pixel **screen);

void right(tile_t tile, unsigned char *image, union pixel **screen);

void switchDirection(tile_t tile, unsigned char *image, union pixel **screen);

void setHead(snake_t *snake, union pixel **screen);

void setBody(snake_t *snake, union pixel **screen);

void setTail(snake_t *snake, union pixel **screen);

void setSnake(snake_t *snake, union pixel **screen);

void chooseDirection(snake_t *snake, int8_t step);

snake_t *chooseColor(uint16_t color, food_t *food, uint16_t x, uint16_t y, int8_t direction);

#endif
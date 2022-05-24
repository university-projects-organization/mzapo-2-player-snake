#ifndef COLLISIONS_H
#define COLLISIONS_H

#include "snake.h"

_Bool checkWalls(snake_t *snake);

_Bool checkApple(tile_t tile, food_t *food);

_Bool checkSnakeCollision(snake_t *snake, snake_t *target, _Bool sameSnake);

void checkFood(snake_t *snake1, snake_t *snake2, food_t *food1, food_t *food2, _Bool foodOwner);

#endif

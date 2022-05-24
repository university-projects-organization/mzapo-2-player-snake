#ifndef FOOD_H
#define FOOD_H

#include <stdint.h>
#include "screen.h"
#include "food_structure.h"
#include "snake_structure.h"


void setFoodImage(food_t *food, unsigned char *image);

void freeFood (food_t *food);

void generateApple(snake_t *snake1, snake_t *snake2, food_t *food1, food_t *food2);

void setFood(union pixel **screen, food_t *food);

food_t *allocateFood();

void setAppleColor(apple_t apple, int8_t color);


#endif
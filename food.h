#ifndef FOOD_H
#define FOOD_H

#include <stdint.h>
#include "screen.h"
#include "food_structure.h"
#include "snake_structure.h"



void setFoodImage(food_t *food, unsigned char *image);

void generateApple(snake_t *snake1, snake_t *snake2, food_t *food1, food_t *food2);

void setFood(union pixel **screen, food_t *food);

//apple_t *allocateFood(snake_t *snake1, snake_t *snake2, uint16_t foodX, uint16_t foodY);

void freeFood(apple_t *food);

void setAppleColor(apple_t apple, int8_t color);


#endif
#ifndef FOOD_STRUCTURE_H
#define FOOD_STRUCTURE_H

#include <stdint.h>

typedef struct apple_t {
    uint16_t x;
    uint16_t y;
} apple_t;

typedef struct food_t {
    uint8_t length;
    unsigned char *image;
    apple_t *array;
} food_t;

food_t *allocateFood();

#endif
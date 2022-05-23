#ifndef SNAKE_STRUCTURE_H
#define SNAKE_STRUCTURE_H

#include <stdint.h>

typedef struct tile_t {
    uint16_t x;
    uint16_t y;
    uint8_t direction;
} tile_t;

typedef struct snake_t {
    uint16_t length;
    int8_t direction;
    int8_t speed;
    unsigned char *imageTail;
    unsigned char *imageBody;
    unsigned char *imageHead;
    unsigned char *color;

    tile_t *tiles;
    tile_t lastTile;
} snake_t;

#endif
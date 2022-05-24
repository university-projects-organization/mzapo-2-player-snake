#ifndef PIXEL_STRUCTURE_H
#define PIXEL_STRUCTURE_H

#include <stdint.h>

union pixel {
    struct {
        unsigned b: 5;
        unsigned g: 6;
        unsigned r: 5;
    };
    uint16_t d;
};

#endif
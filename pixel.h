#ifndef PIXEL_H
#define PIXEL_H

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
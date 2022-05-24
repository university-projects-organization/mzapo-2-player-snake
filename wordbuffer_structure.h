#ifndef WORD_BUFFER_H
#define WORD_BUFFER_H

#include <stdint.h>

typedef struct {
    uint16_t **array;
    uint16_t height;
    uint16_t width;
} wordBuffer;

#endif
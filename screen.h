#ifndef SCREEN_H
#define SCREEN_H

#define HEIGHT 320
#define WIDTH 480

#include <stdint.h>
#include <stdlib.h>
#include "pixel.h"

union pixel **allocateScreen(void);

void freeScreen(union pixel **screen);

void imageToPixelArray(unsigned char *image, union pixel **pixelArray);

void setBackground(union pixel **background, union pixel **screen);

void setSelector(const unsigned char *selector, union pixel **screen, const size_t offsetY, const size_t offsetX);

void loadScreen(union pixel **screen, unsigned char *parlcd_reg_base);

#endif
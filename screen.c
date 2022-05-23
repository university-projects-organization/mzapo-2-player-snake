#include <stdlib.h>
#include "apo_library/mzapo_parlcd.h"
#include "pixel.h"
#include "screen.h"

union pixel **allocateScreen(void) {
    union pixel **screen = (union pixel **) malloc(sizeof(union pixel *) * HEIGHT);
    for (size_t i = 0; i < HEIGHT; ++i) {
        screen[i] = (union pixel *) malloc(sizeof(union pixel) * WIDTH);
    }
    return screen;
}

void freeScreen(union pixel **screen) {
    for (size_t i = 0; i < HEIGHT; i++) {
        free(screen[i]);
    }
    free(screen);
}


void imageToPixelArray(unsigned char *image, union pixel **pixelArray) {
    size_t i = 0;
    for (size_t y = 0; y < HEIGHT; y++) {
        for (size_t x = 0; x < WIDTH; x++) {
            pixelArray[y][x].r = image[i] >> 3;
            pixelArray[y][x].g = image[i + 1] >> 2;
            pixelArray[y][x].b = image[i + 2] >> 3;
            i += 3;
        }
    }
}

void setBackground(union pixel **background, union pixel **screen) {
    for (size_t y = 0; y < HEIGHT; y++) {
        for (size_t x = 0; x < WIDTH; x++) {
            screen[y][x].d = background[y][x].d;
        }
    }
}

void setSelector(const unsigned char *selector, union pixel **screen, const size_t offsetY, const size_t offsetX) {
    size_t i = 0;
    for (size_t y = offsetY; y < (offsetY + 45); y++) {
        for (size_t x = offsetX; x < (offsetX + 45); x++) {
            if (selector[i + 3] == 255) {
                screen[y][x].r = selector[i] >> 3;
                screen[y][x].g = selector[i + 1] >> 2;
                screen[y][x].b = selector[i + 2] >> 3;
            }
            i += 4;
        }
    }
}

void loadScreen(union pixel **screen, unsigned char *parlcd_reg_base) {
    for (size_t y = 0; y < HEIGHT; y++) {
        for (size_t x = 0; x < WIDTH; x++) {
            parlcd_write_data(parlcd_reg_base, screen[y][x].d);
        }
    }
}
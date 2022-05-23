// Load an image and save it in PNG and JPG format using stb_image libraries
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>

#include "mzapo_parlcd.h"
#include "mzapo_phys.h"
#include "mzapo_regs.h"
#include "serialize_lock.h"
#include "font_types.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

union led {
  struct {
    uint8_t r, g, b;
  };
  uint32_t data;
};

union pixel {
  struct {
    unsigned b : 5;
    unsigned g : 6;
    unsigned r : 5;
  };
  uint16_t d;
};

union pixel buffer[480][320];

int main(void) {
    int width, height, channels;
    unsigned char *img = stbi_load("sky.jpg", &width, &height, &channels, 0);
    if(img == NULL) {
        printf("Error in loading the image\n");
        exit(1);
    }
    printf("Loaded image with a width of %dpx, a height of %dpx and %d channels\n", width, height, channels);

    stbi_write_png("sky.png", width, height, channels, img, width * channels);
    stbi_write_jpg("sky2.jpg", width, height, channels, img, 100);

    stbi_image_free(img);
}
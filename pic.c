#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>


#define STB_IMAGE_IMPLEMENTATION
#include "stb_library/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_library/stb_image_write.h"

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
    unsigned char *img = stbi_load("SnakeMenu.jpg", &width, &height, &channels, 0);
    if(img == NULL) {
        printf("Error in loading the image\n");
        exit(1);
    }
    printf("Loaded image with a width of %dpx, a height of %dpx and %d channels\n", width, height, channels);
    stbi_write_jpg("SnakeMenu_new.jpg", width, height, channels, img, 100);

    stbi_image_free(img);
}

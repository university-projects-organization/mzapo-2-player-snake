/*******************************************************************
  Project main function template for MicroZed based MZ_APO board
  designed by Petr Porazil at PiKRON

  change_me.c      - main file

  include your name there and license for distribution.

  Remove next text: This line should not appear in submitted
  work and project name should be change to match real application.
  If this text is there I want 10 points subtracted from final
  evaluation.

 *******************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>

#define STB_IMAGE_IMPLEMENTATION

#include "stb_library/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "stb_library/stb_image_write.h"

#include "mzapo_parlcd.h"
#include "mzapo_phys.h"
#include "mzapo_regs.h"
#include "serialize_lock.h"
#include "font_types.h"

#define HEIGHT 320
#define WIDTH 480
#define BLUEPRESSED 16777216
#define GREENPRESSED 16777216 * 2
#define REDPRESSED 16777216 * 4
#define BLUE 0
#define GREEN 1
#define RED 2
#define ASCIISHIFT 32

union pixel {
    struct {
        unsigned b: 5;
        unsigned g: 6;
        unsigned r: 5;
    };
    uint16_t d;
};

typedef struct {
    uint16_t** array;
    uint16_t height;
    uint16_t width;
} wordBuffer;

union pixel**  allocateScreen(void) {
    union pixel **screen = (union pixel**) malloc(sizeof(union pixel*) * HEIGHT);
    for (size_t i = 0; i < HEIGHT; ++i) {
        screen[i] = (union pixel*) malloc(sizeof(union pixel) * WIDTH);
    }
    return screen;
}

void freeScreen(union pixel** screen) {
    for (size_t i = 0; i < HEIGHT; i++) {
        free(screen[i]);
    }
    free(screen);
}

wordBuffer* allocateWordBuffer(uint16_t width, uint16_t height) {
    wordBuffer* wordBuffer = malloc(sizeof(wordBuffer));
    wordBuffer->array = (uint16_t**) malloc(sizeof(uint16_t*) * height);
    for (size_t i = 0; i < height; ++i) {
        wordBuffer->array[i] = (uint16_t*) malloc(sizeof(uint16_t) * width);
    }
    wordBuffer->height = height;
    wordBuffer->width = width;
    return wordBuffer;
}

void freeWordBuffer(wordBuffer* wordBuffer) {
    for (size_t i = 0; i < wordBuffer->height; i++) {
        free(wordBuffer->array[i]);
    }
    free(wordBuffer->array);
    free(wordBuffer);
}

uint16_t setSymbol(wordBuffer* wordBuffer, uint16_t bufferX, uint8_t scale, uint16_t letterNumber) {
    uint8_t symbolWidth = font_winFreeSystem14x16.width[letterNumber];
    uint8_t symbolHeight = 16;
    uint8_t offset = 16 - symbolWidth;
    uint16_t symbolArrayPosition = letterNumber * symbolHeight;

    for (size_t y = 0; y < symbolHeight * scale;) {
        uint16_t currentStr = font_winFreeSystem14x16.bits[symbolArrayPosition++] >> offset;
        uint16_t temp = currentStr;

        for (size_t j = 0; j < scale; j++) {
            uint16_t widthCopy = 1 << (symbolWidth - 1);
            for (size_t x = 0; x < symbolWidth * scale;) {
                for (size_t i = 0; i < scale; i++) {
                    wordBuffer->array[y][bufferX + x] = currentStr / widthCopy;
                    x++;
                }
                currentStr %= widthCopy;
                widthCopy /= 2;
            }
            currentStr = temp;
            y++;
        }
    }
    return bufferX + (symbolWidth * scale);
}

wordBuffer* makeWordBuffer(char* word, uint8_t scale) {
    uint16_t wordLen = strlen(word);
    uint8_t wordLetters[wordLen];

    uint16_t width = 0;
    uint16_t height = 16 * scale;

    for (size_t i = 0; i < wordLen; i++) {
        wordLetters[i] = (int)word[i] - ASCIISHIFT;
        width += font_winFreeSystem14x16.width[wordLetters[i]];
    }

    wordBuffer* wordBuffer = allocateWordBuffer(width, height);
    uint16_t bufferX = 0;
    for (size_t i = 0; i < wordLen; i++) {
        bufferX = setSymbol(wordBuffer, bufferX, scale, wordLetters[i]);
    }
    return wordBuffer;
}

void setWordBuffer(wordBuffer* wordBuffer, union pixel **screen, uint16_t screenX, uint16_t screenY) {
    for (size_t y = 0; y < wordBuffer->height; y++) {
        for (size_t x = 0; x < wordBuffer->width; x++) {
            if (wordBuffer->array[y][x] == 1) {
                printf("1");
                screen[screenY + y][screenX + x].d = wordBuffer->array[y][x];
            } else {
                printf(" ");
            }
        }
        printf("\n");
    }

}



void setBackground(const unsigned char *image, union pixel **screen) {
    size_t i = 0;
    for (size_t y = 0; y < HEIGHT; y++) {
        for (size_t x = 0; x < WIDTH; x++) {
            screen[y][x].r = image[i] >> 3;
            screen[y][x].g = image[i + 1] >> 2;
            screen[y][x].b = image[i + 2] >> 3;
            i += 3;
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

void endGame(union pixel **screen, unsigned char *background, unsigned char *apple) {
    freeScreen(screen);
    free(background);
    free(apple);
    exit(0);
}

int8_t knobRotated(uint32_t actualValue, uint32_t *previousValue, int8_t knob, int8_t positions) {
    int32_t coef = 1;
    switch (knob) {
        case BLUE:
            coef = 4;
            break;
        case GREEN:
            coef = 1024;
            break;
        case RED:
            coef = 1;
            break;
    }
    int32_t difference = actualValue - *previousValue;
    int8_t step = (difference / coef) % positions;
    *previousValue = actualValue;
    return step;
}

_Bool knobPressed(uint32_t actualKnob, uint32_t knobDigit, uint32_t knob) {
    return actualKnob - knobDigit == knob;
}

void settingsMenu(union pixel **screen, volatile void *spiled_reg_base, unsigned char *parlcd_reg_base) {
    int width, height, channels;
    unsigned char *background = stbi_load("/tmp/nazar/resources/SettingsMenu/SettingsMenu.jpg", &width, &height,
                                          &channels, 0);
    unsigned char *pear = stbi_load("/tmp/nazar/resources/SettingsMenu/pear.png", &width, &height, &channels, 0);
    if (background == NULL || pear == NULL) {
        printf("Can not load image\n");
        exit(0);
    }

    uint16_t offsetY = 15;
    //uint32_t volicBlue = *(volatile uint32_t *) (spiled_reg_base + SPILED_REG_KNOBS_8BIT_o) % 256;
    uint32_t previousG = *(volatile uint32_t *) (spiled_reg_base + SPILED_REG_KNOBS_8BIT_o) % 65536;
    //uint32_t previousKnobs = *(volatile uint32_t *) (spiled_reg_base + SPILED_REG_KNOBS_8BIT_o);
    int8_t position = 0;

    while (1) {
        setBackground(background, screen);
        struct timespec loop_delay = {.tv_sec = 0, .tv_nsec = 200 * 1000 * 1000};


        uint32_t actualKnobs = *(volatile uint32_t *) (spiled_reg_base + SPILED_REG_KNOBS_8BIT_o);
        //uint32_t actualB = actualKnobs % 256;
        uint32_t actualG = actualKnobs % 65536;
        uint32_t actualR = actualKnobs % BLUEPRESSED;


        int8_t step = knobRotated(actualG, &previousG, GREEN, 5);
        position = (position + step + 5) % 5;

        switch (position) {
            case 0:
                offsetY = 20;
                break;
            case 1:
                offsetY = 65;
                break;
            case 2:
                offsetY = 115;
                break;
            case 3:
                offsetY = 180;
                break;
            case 4:
                offsetY = 250;
                break;

        }

        setSelector(pear, screen, offsetY, 65);
        loadScreen(screen, parlcd_reg_base);

        if (knobPressed(actualKnobs, actualR, REDPRESSED)) {
            printf("Red break\n");
            break;
        }

        if (knobPressed(actualKnobs, actualR, GREENPRESSED)) {
            switch (position) {
                case 0:
                    printf("0 pressed\n");
                    break;
                case 1:
                    printf("1 pressed\n");
                    break;
                case 2:
                    printf("2 pressed\n");
                    break;
                case 3:
                    printf("3 pressed\n");
                    break;
                case 4:
                    printf("4 pressed\n");
                    break;
            }
        }
        clock_nanosleep(CLOCK_MONOTONIC, 0, &loop_delay, NULL);
    }
}

int main(int argc, char *argv[]) {
    volatile void *spiled_reg_base = map_phys_address(SPILED_REG_BASE_PHYS, SPILED_REG_SIZE, 0);

    unsigned char *parlcd_reg_base = map_phys_address(PARLCD_REG_BASE_PHYS, PARLCD_REG_SIZE, 0);
    parlcd_write_cmd(parlcd_reg_base, 0x2c);

    int width, height, channels;
    unsigned char *background = stbi_load("/tmp/nazar/resources/MainMenu/MainMenu.jpg", &width, &height, &channels, 0);
    unsigned char *apple = stbi_load("/tmp/nazar/resources/MainMenu/apple.png", &width, &height, &channels, 0);
    if (background == NULL || apple == NULL) {
        printf("Can not load image\n");
        exit(0);
    }

    union pixel **screen = allocateScreen();

    wordBuffer* wordBuffer = makeWordBuffer("Sheesh!", 2);





    uint16_t offsetY = 15;
    uint32_t previousG = *(volatile uint32_t *) (spiled_reg_base + SPILED_REG_KNOBS_8BIT_o) % 65536;
    int8_t position = 0;
    while (1) {
        struct timespec loop_delay = {.tv_sec = 0, .tv_nsec = 200 * 1000 * 1000};
        setBackground(background, screen);

        uint32_t actualKnobs = *(volatile uint32_t *) (spiled_reg_base + SPILED_REG_KNOBS_8BIT_o);
        uint32_t actualG = actualKnobs % 65536;
        uint32_t actualR = actualKnobs % BLUEPRESSED;

        // if (actualG != previousG) {
        int8_t step = knobRotated(actualG, &previousG, GREEN, 4);
        position = (position + step + 4) % 4;
        switch (position) {
            case 0:
                offsetY = 10;
                break;
            case 1:
                offsetY = 90;
                break;
            case 2:
                offsetY = 180;
                break;
            case 3:
                offsetY = 260;
                break;
        }
        // }

        setSelector(apple, screen, offsetY, 30);
        setWordBuffer(wordBuffer, screen, 10, 60);
        loadScreen(screen, parlcd_reg_base);
        /*
        *(volatile uint32_t *) (spiled_reg_base + SPILED_REG_LED_LINE_o) = rgb_knobs_value;
        *(volatile uint32_t *) (spiled_reg_base + SPILED_REG_LED_RGB1_o) = rgb_knobs_value;
        *(volatile uint32_t *) (spiled_reg_base + SPILED_REG_LED_RGB2_o) = rgb_knobs_value;
        */

        if (knobPressed(actualKnobs, actualR, REDPRESSED)) {
            printf("Red break\n");
            break;
        }

        if (knobPressed(actualKnobs, actualR, GREENPRESSED)) {
            switch (position) {
                case 0:
                    printf("New Game pressed\n");
                    break;
                case 1:
                    printf("Records pressed\n");
                    break;
                case 2:
                    settingsMenu(screen, spiled_reg_base, parlcd_reg_base);
                    previousG = *(volatile uint32_t *) (spiled_reg_base + SPILED_REG_KNOBS_8BIT_o) % 65536;
                    break;
                default:
                    printf("Exit pressed\n");
                    endGame(screen, background, apple);
            }
        }
        clock_nanosleep(CLOCK_MONOTONIC, 0, &loop_delay, NULL);
    }
    endGame(screen, background, apple);
}

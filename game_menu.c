#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include "apo_library/mzapo_regs.h"
#include "screen.h"
#include "settings_menu.h"
#include "knobs_control.h"
#include "stb_library/stb_image.h"
#include "stb_library/stb_image_write.h"

void endGame(union pixel **screen, union pixel **background, unsigned char *selector, uint8_t *settings) {
    freeScreen(screen);
    freeScreen(background);
    free(selector);
    free(settings);
}

_Bool gameMenu(union pixel **screen, uint8_t *settings, volatile void *spiled_reg_base, unsigned char *parlcd_reg_base) {
    _Bool ret = 0;

    int width, height, channels;
    unsigned char *backgroundPicture = stbi_load("/tmp/nazar/resources/MainMenu/MainMenu.jpg", &width, &height,
                                                 &channels, 0);
    unsigned char *selector = stbi_load("/tmp/nazar/resources/MainMenu/apple.png", &width, &height, &channels, 0);
    if (backgroundPicture == NULL || selector == NULL) {
        printf("Can not load image\n");
    }

    union pixel **background = allocateScreen();
    imageToPixelArray(backgroundPicture, background);
    free(backgroundPicture);

    uint16_t selectorY = 22;
    uint32_t previousKnobs = *(volatile uint32_t *) (spiled_reg_base + SPILED_REG_KNOBS_8BIT_o);
    uint32_t previousB = previousKnobs % 256;
    uint32_t previousG = previousKnobs % 65536 - previousB;

    int8_t position = 0;
    while (1) {
        struct timespec loop_delay = {.tv_sec = 0, .tv_nsec = 200 * 1000 * 1000};


        uint32_t actualKnobs = *(volatile uint32_t *) (spiled_reg_base + SPILED_REG_KNOBS_8BIT_o);
        uint32_t actualB = actualKnobs % 256;
        uint32_t actualG = actualKnobs % 65536 - actualB;
        uint32_t actualR = actualKnobs % BLUEPRESSED;

        if ((actualG - previousG) % 1024 == 0 && actualG != previousG) {
            int8_t step = knobRotated(actualG, &previousG, GREEN, 4);
            position = (position + step + 4) % 4;
            switch (position) {
                case 0:
                    selectorY = 22;
                    break;
                case 1:
                    selectorY = 98;
                    break;
                case 2:
                    selectorY = 178;
                    break;
                case 3:
                    selectorY = 255;
                    break;
            }
        }
        /*
        *(volatile uint32_t *) (spiled_reg_base + SPILED_REG_LED_LINE_o) = rgb_knobs_value;
        *(volatile uint32_t *) (spiled_reg_base + SPILED_REG_LED_RGB1_o) = rgb_knobs_value;
        *(volatile uint32_t *) (spiled_reg_base + SPILED_REG_LED_RGB2_o) = rgb_knobs_value;
        */

        if (knobPressed(actualKnobs, actualR, REDPRESSED, spiled_reg_base)) {
            printf("Red button\n");
            break;
        }

        if (knobPressed(actualKnobs, actualR, GREENPRESSED, spiled_reg_base)) {
            switch (position) {
                case 0:
                    printf("New Game pressed\n");
                    ret = 1;
                    freeScreen(background);
                    free(selector);
                    goto end;
                case 1:
                    printf("Records pressed\n");
                    break;
                case 2:
                    settingsMenu(screen, spiled_reg_base, parlcd_reg_base, settings, &previousG);
                    break;
                default:
                    printf("Exit pressed\n");
                    endGame(screen, background, selector, settings);
                    goto end;
            }
        }
        setBackground(background, screen);
        setSelector(selector, screen, selectorY, 50);
        loadScreen(screen, parlcd_reg_base);
        clock_nanosleep(CLOCK_MONOTONIC, 0, &loop_delay, NULL);
    }
    end:
    return ret;
}
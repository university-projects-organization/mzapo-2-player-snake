#include <stdint.h>
#include <time.h>
#include "pixel.h"
#include "settings_allocate.h"
#include "apo_library/mzapo_regs.h"
#include "screen.h"
#include "word_from_font.h"
#include "knobs_control.h"
#include "settings_menu.h"

#define STB_IMAGE_IMPLEMENTATION

#include "stb_library/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "stb_library/stb_image_write.h"

uint16_t colors[] = {0x00, 0xF800, 0x07E0, 0x001F};

void setSettingsMenuBackground(union pixel **background, unsigned char *image) {
    imageToPixelArray(image, background);
    setBackground(background, background);
    wordBuffer *wordSnakesColor = makeWordBuffer("Snakes color:", 2);
    wordBuffer *wordFoodOwner = makeWordBuffer("Food owner:", 2);
    wordBuffer *wordFoodAmount = makeWordBuffer("Food amount:", 2);
    wordBuffer *wordPlayVS = makeWordBuffer("Play VS:", 2);
    wordBuffer *wordSpeed = makeWordBuffer("Speed:", 2);
    wordBuffer *wordBoost = makeWordBuffer("Boost:", 2);
    setWord(wordSnakesColor, 0, background, 50, 15);
    setWord(wordFoodOwner, 0, background, 50, 65);
    setWord(wordFoodAmount, 0, background, 50, 115);
    setWord(wordPlayVS, 0, background, 50, 165);
    setWord(wordSpeed, 0, background, 50, 215);
    setWord(wordBoost, 0, background, 50, 265);
    freeWordBuffer(wordSnakesColor);
    freeWordBuffer(wordFoodOwner);
    freeWordBuffer(wordFoodAmount);
    freeWordBuffer(wordPlayVS);
    freeWordBuffer(wordSpeed);
    freeWordBuffer(wordBoost);
}

void loadSettingsMenu(union pixel **background, union pixel **screen, settingsParameter** settingsParameters, unsigned char *selector, unsigned char *parlcd_reg_base, uint16_t selectorY) {
    setBackground(background, screen);
    setWord(settingsParameters[0]->string[settingsParameters[0]->position], colors[settingsParameters[0]->position], screen, 250, 15);
    setWord(settingsParameters[1]->string[settingsParameters[1]->position], colors[settingsParameters[1]->position], screen, 360, 15);
    setWord(settingsParameters[2]->string[settingsParameters[2]->position], 0xFFFF, screen, 300, 65);
    setWord(settingsParameters[3]->string[settingsParameters[3]->position], 0xFFFF, screen, 265, 115);
    setWord(settingsParameters[4]->string[settingsParameters[4]->position], 0xFFFF, screen, 225, 165);
    setWord(settingsParameters[5]->string[settingsParameters[5]->position], 0xFFFF, screen, 190, 215);
    setWord(settingsParameters[6]->string[settingsParameters[6]->position], 0xFFFF, screen, 190, 265);
    setSelector(selector, screen, selectorY, 0);
    loadScreen(screen, parlcd_reg_base);
}

void menuPosition(uint32_t actual, uint32_t *previous, int knob, settingsParameter** settingsParameters, int8_t index, int8_t numOptions) {
    int8_t step = knobRotated(actual, previous, knob, numOptions);
    settingsParameters[index]->position = (settingsParameters[index]->position + step + numOptions) % numOptions;
}

void settingsMenu(union pixel **screen, volatile void *spiled_reg_base, unsigned char *parlcd_reg_base, uint8_t *settings, uint32_t* actualG) {
    int width, height, channels;
    unsigned char *backgroundPicture = stbi_load("/tmp/nazar/resources/SettingsMenu/SettingsMenu.jpg", &width, &height,
                                                 &channels, 0);
    unsigned char *selector = stbi_load("/tmp/nazar/resources/SettingsMenu/pear.png", &width, &height, &channels, 0);
    if (backgroundPicture == NULL || selector == NULL) {
        printf("Can not load image\n");
        exit(0);
    }
    union pixel **background = allocateScreen();
    setSettingsMenuBackground(background, backgroundPicture);
    free(backgroundPicture);

    settingsParameter **settingsParameters = allocateSettingsParameter(settings);

    uint16_t selectorY = 0;
    int8_t selectorPosition = 0;
    uint32_t previousKnobs = *(volatile uint32_t *) (spiled_reg_base + SPILED_REG_KNOBS_8BIT_o);
    uint32_t previousB = previousKnobs % 256;
    uint32_t previousG = previousKnobs % 65536 - previousB;
    uint32_t previousR = previousKnobs % BLUEPRESSED - previousG - previousB;

    while (1) {
        struct timespec loop_delay = {.tv_sec = 0, .tv_nsec = 200 * 1000 * 1000};

        uint32_t actualKnobs = *(volatile uint32_t *) (spiled_reg_base + SPILED_REG_KNOBS_8BIT_o);
        uint32_t actualB = actualKnobs % 256;
        *actualG = actualKnobs % 65536 - actualB;
        uint32_t actualR = actualKnobs % BLUEPRESSED - *actualG - actualB;


        if ((*actualG - previousG) % 1024 == 0 && *actualG != previousG) {
            int8_t step = knobRotated(*actualG, &previousG, GREEN, 6);
            selectorPosition = (selectorPosition + step + 6) % 6;

            switch (selectorPosition) {
                case 0:
                    selectorY = 0;
                    break;
                case 1:
                    selectorY = 50;
                    break;
                case 2:
                    selectorY = 100;
                    break;
                case 3:
                    selectorY = 150;
                    break;
                case 4:
                    selectorY = 200;
                    break;
                case 5:
                    selectorY = 250;
                    break;

            }
        }


        if ((actualB - previousB) % 4 == 0 && actualB != previousB) {
            switch (selectorPosition) {
                case 0:
                    menuPosition(actualB, &previousB, BLUE, settingsParameters, 1, 4);
                    break;
                case 1:
                    menuPosition(actualB, &previousB, BLUE, settingsParameters, 2, 2);
                    break;
                case 2:
                    menuPosition(actualB, &previousB, BLUE, settingsParameters, 3, 2);
                    break;
                case 3:
                    menuPosition(actualB, &previousB, BLUE, settingsParameters, 4, 2);
                    break;
                case 4:
                    menuPosition(actualB, &previousB, BLUE, settingsParameters, 5, 3);
                    break;
                case 5:
                    menuPosition(actualB, &previousB, BLUE, settingsParameters, 6, 3);
                    break;
                default:
                    previousB = actualB;
                    break;
            }
        }

        if ((actualR - previousR) % 262144 == 0 && actualR != previousR) {
            switch (selectorPosition) {
                case 0:
                    menuPosition(actualR, &previousR, RED, settingsParameters, 0, 4);
                    break;
                default:
                    previousR = actualR;
                    break;
            }
        }

        if (knobPressed(actualKnobs, actualKnobs % BLUEPRESSED, REDPRESSED, spiled_reg_base)) {
            for(size_t i = 0; i < 7; i++) {
                settings[i] = settingsParameters[i]->position;
            }
            freeSettingsParameter(settingsParameters);
            free(selector);
            freeScreen(background);
            break;
        }
        loadSettingsMenu(background, screen, settingsParameters, selector, parlcd_reg_base, selectorY);
        clock_nanosleep(CLOCK_MONOTONIC, 0, &loop_delay, NULL);
    }
}
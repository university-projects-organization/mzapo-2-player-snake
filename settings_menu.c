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
    uint16_t colors[] = {0x00, 0xF800, 0x07E0, 0x001F};
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

void menuPosition(uint8_t actual, uint8_t *previous, settingsParameter** settingsParameters, int8_t index, int8_t numOptions) {
    int8_t step = knobRotated(actual, previous, numOptions);
    settingsParameters[index]->position = (settingsParameters[index]->position + step + numOptions) % numOptions;
}

void settingsMenu(union pixel **screen, volatile void *spiled_reg_base, unsigned char *parlcd_reg_base, uint8_t *settings, uint8_t* actualG) {
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
    uint8_t previousB = (uint8_t)previousKnobs;
    uint8_t previousG = (uint8_t)(previousKnobs >> 8);
    uint8_t previousR = (uint8_t)(previousKnobs >> 16);

    while (1) {
        struct timespec loop_delay = {.tv_sec = 0, .tv_nsec = 200 * 1000 * 1000};

        uint32_t actualKnobs = *(volatile uint32_t *) (spiled_reg_base + SPILED_REG_KNOBS_8BIT_o);

        uint8_t actualB = (uint8_t)actualKnobs;
        *actualG = (uint8_t)(actualKnobs >> 8); // actualKnobs % 65536 - actualB;
        uint8_t actualR = (uint8_t)(actualKnobs >> 16); // actualKnobs % BLUEPRESSED;

        uint8_t whichKnobPressed = (uint8_t)(actualKnobs >> 24);


        if ((*actualG - previousG) % 4 == 0 && *actualG != previousG) {
            int8_t step = knobRotated(*actualG, &previousG, 6);
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
                    menuPosition(actualB, &previousB, settingsParameters, 1, 4);
                    break;
                case 1:
                    menuPosition(actualB, &previousB, settingsParameters, 2, 2);
                    break;
                case 2:
                    menuPosition(actualB, &previousB, settingsParameters, 3, 2);
                    break;
                case 3:
                    menuPosition(actualB, &previousB, settingsParameters, 4, 2);
                    break;
                case 4:
                    menuPosition(actualB, &previousB, settingsParameters, 5, 3);
                    break;
                case 5:
                    menuPosition(actualB, &previousB, settingsParameters, 6, 3);
                    break;
                default:
                    previousB = actualB;
                    break;
            }
        }

        if ((actualR - previousR) % 4 == 0 && actualR != previousR) {
            switch (selectorPosition) {
                case 0:
                    menuPosition(actualR, &previousR, settingsParameters, 0, 4);
                    break;
                default:
                    previousR = actualR;
                    break;
            }
        }

        if (whichKnobPressed == 4) {
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
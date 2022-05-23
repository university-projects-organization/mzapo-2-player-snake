/*******************************************************************
  Nazar Ponomarev, Nikita Kisel
 *******************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

#define STB_IMAGE_IMPLEMENTATION

#include "stb_library/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "stb_library/stb_image_write.h"

#include "mzapo_parlcd.h"
#include "knobs_control.h"
#include "game_menu.h"
#include "mzapo_phys.h"
#include "mzapo_regs.h"
<<<<<<< HEAD
#include "pixel.h"
#include "screen.h"
#include "stb_library/stb_image.h"
#include "stb_library/stb_image_write.h"

#define TILESIZE 16

typedef struct tile_t {
    uint16_t x;
    uint16_t y;
} tile_t;

typedef struct snake_t {
    uint16_t length;
    int8_t direction;
    int8_t speed;
    uint16_t color;

    tile_t *tiles;
    tile_t lastTile;
} snake_t;

typedef struct food_t {
    uint16_t x;
    uint16_t y;
    char *image;
} food_t;


void setTile(snake_t *snake, uint16_t x, uint16_t y, uint8_t index) {
    snake->tiles[index].x = x;
    snake->tiles[index].y = y;
}

int8_t knobRotated1(uint32_t actualValue, uint32_t *previousValue, int8_t knob) {
    int coef;
    switch (knob) {
        case BLUE:
            coef = 4;
            break;
        case GREEN:
            coef = 1024;
            break;
        default:
            coef = 262144;
            break;
    }
    int16_t difference = (actualValue - *previousValue) / coef;
    difference = difference == 63 ? -1 : difference == -63 ? 1 : difference;
    int8_t step = difference; // % positions;
    *previousValue = actualValue;
    return step;
}


snake_t *allocateSnake() {
    snake_t *snake = (snake_t *) malloc(sizeof(snake_t));
    snake->length = 1;
    snake->direction = 0;
    snake->speed = 1;
    snake->color = 0xFFF;
    snake->tiles = (tile_t *) malloc(sizeof(tile_t) * 10);
    setTile(snake, 240, 160, 0);
    return snake;
}

_Bool checkFood(tile_t tile, food_t *food) {
    _Bool collision = 0;
    if (tile.x == food->x && tile.y == food->y) {
        collision = 1;
    }
    return collision;
}

void generateFood(food_t *food, snake_t *snake) {
    time_t t;
    srand((unsigned) time(&t));
    int x, y;
    while (1) {
        jump:
        x = (rand() % 30) * TILESIZE;
        y = (rand() % 19) * TILESIZE;
        for (size_t i = 0; i < snake->length; i++) {
            if (x == snake->tiles[i].x || y == snake->tiles[i].y) {
                goto jump;
            }
        }
        break;
    }
    food->x = x;
    food->y = y;
    printf("foodX = %d | foodY = %d\n", food->x, food->y);
}

food_t *allocateFood(snake_t *snake) {
    food_t *food = (food_t *) malloc(sizeof(food_t));
    generateFood(food, snake);
    return food;
}


void setFood(union pixel **screen, food_t *food) {
    for (size_t y = food->y; y < TILESIZE + food->y; y++) {
        for (size_t x = food->x; x < TILESIZE + food->x; x++) {
            screen[y][x].d = 0xFF;
        }
    }
}

void moveForward(snake_t *snake) {
    snake->lastTile.x = snake->tiles[snake->length - 1].x;
    snake->lastTile.y = snake->tiles[snake->length - 1].y;
    for (size_t i = snake->length - 1; i > 0; i--) {
        snake->tiles[i].x = snake->tiles[i - 1].x;
        snake->tiles[i].y = snake->tiles[i - 1].y;
    }

    switch (snake->direction) {
        case 0:
            snake->tiles[0].y -= TILESIZE;
            break;
        case 1:
            snake->tiles[0].x += TILESIZE;
            break;
        case 2:
            snake->tiles[0].y += TILESIZE;
            break;
        case 3:
            snake->tiles[0].x -= TILESIZE;
            break;
    }
}

void setSnake(snake_t *snake, union pixel **screen) {
    for (size_t i = 0; i < snake->length; i++) {
        for (size_t y = snake->tiles[i].y; y < TILESIZE + snake->tiles[i].y; y++) {
            for (size_t x = snake->tiles[i].x; x < TILESIZE + snake->tiles[i].x; x++) {
                screen[y][x].d = snake->color;
            }
        }
    }
}

_Bool checkWalls(snake_t *snake) {
    _Bool collision = 0;
    if (snake->tiles[0].y + TILESIZE == 0 || snake->tiles[0].y == 3 || snake->tiles[0].x + TILESIZE == 0 || snake->tiles[0].x == 480) {
        collision = 1;
    }
    return collision;
}


void chooseDirection(snake_t *snake, int8_t step) {
    // 0 - up, 1 - right, 2 - down, 3 - left

    switch (snake->direction) {
        case 0:
            switch (step) {
                case -1:
                    snake->direction = 3;
                    break;
                case 1:
                    snake->direction = 1;
                    break;
                default:
                    break;
            }
            break;
        case 1:
            switch (step) {
                case -1:
                    snake->direction = 0;
                    break;
                case 1:
                    snake->direction = 2;
                    break;
                default:
                    break;
            }
            break;
        case 2:
            switch (step) {
                case -1:
                    snake->direction = 1;
                    break;
                case 1:
                    snake->direction = 3;
                    break;
                default:
                    break;
            }
            break;
        case 3:
            switch (step) {
                case -1:
                    snake->direction = 2;
                    break;
                case 1:
                    snake->direction = 0;
                    break;
                default:
                    break;
            }
            break;

    }
}

int main(int argc, char *argv[]) {
    volatile void *spiled_reg_base = map_phys_address(SPILED_REG_BASE_PHYS, SPILED_REG_SIZE, 0);
    unsigned char *parlcd_reg_base = map_phys_address(PARLCD_REG_BASE_PHYS, PARLCD_REG_SIZE, 0);
    parlcd_write_cmd(parlcd_reg_base, 0x2c);

    uint8_t *settings = (uint8_t *) malloc(sizeof(uint8_t) * 7);
    for (size_t i = 0; i < 7; i++) {
        settings[i] = 0;
    }
    union pixel **screen = allocateScreen();
    if (!gameMenu(screen, settings, spiled_reg_base, parlcd_reg_base)) {
        return 0;
    }


    int width, height, channels;
    unsigned char *backgroundPicture = stbi_load("/tmp/nazar/resources/GameField/GameBack.jpg", &width, &height,
                                                 &channels, 0);
    if (backgroundPicture == NULL) {
=======
#include "serialize_lock.h"
#include "font_types.h"

#define HEIGHT 320
#define WIDTH 480

uint16_t colors[] = {0x00, 0xF800, 0x07E0, 0x001F};

union pixel {
    struct {
        unsigned b: 5;
        unsigned g: 6;
        unsigned r: 5;
    };
    uint16_t d;
};

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

void endGame(union pixel **screen, union pixel **background, unsigned char *selector, uint8_t *settings) {
    freeScreen(screen);
    freeScreen(background);
    free(selector);
    free(settings);
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

int main(int argc, char *argv[]) {
    volatile void *spiled_reg_base = map_phys_address(SPILED_REG_BASE_PHYS, SPILED_REG_SIZE, 0);

    unsigned char *parlcd_reg_base = map_phys_address(PARLCD_REG_BASE_PHYS, PARLCD_REG_SIZE, 0);
    parlcd_write_cmd(parlcd_reg_base, 0x2c);

    int width, height, channels;
    unsigned char *backgroundPicture = stbi_load("/tmp/nazar/resources/MainMenu/MainMenu.jpg", &width, &height,
                                                 &channels, 0);
    unsigned char *selector = stbi_load("/tmp/nazar/resources/MainMenu/apple.png", &width, &height, &channels, 0);
    if (backgroundPicture == NULL || selector == NULL) {
>>>>>>> b878198bbf26b6f78952c51c8218db35dc7948cb
        printf("Can not load image\n");
    }

    union pixel **background = allocateScreen();
    imageToPixelArray(backgroundPicture, background);
    free(backgroundPicture);
<<<<<<< HEAD


    //union pixel **snakeImage = (union pixel **) malloc(sizeof(union pixel *) * TILESIZE);
    //for (size_t i = 0; i < TILESIZE; ++i) {
    //snakeImage[i] = (union pixel *) malloc(sizeof(union pixel) * TILESIZE);
    //}

    snake_t *snake = allocateSnake();
    food_t *apple = allocateFood(snake);


    uint32_t previousKnobs = *(volatile uint32_t *) (spiled_reg_base + SPILED_REG_KNOBS_8BIT_o);
    uint32_t previousB = previousKnobs % 256;
    uint32_t previousG = previousKnobs % 65536 - previousB;
    uint32_t previousR = previousKnobs % BLUEPRESSED - previousG - previousB;

    while (1) {
        struct timespec loop_delay = {.tv_sec = 0, .tv_nsec = 200 * 1000 * 1000};

        uint32_t actualKnobs = *(volatile uint32_t *) (spiled_reg_base + SPILED_REG_KNOBS_8BIT_o);
        uint32_t actualB = actualKnobs % 256;
        uint32_t actualG = actualKnobs % 65536 - actualB;
        uint32_t actualR = actualKnobs % BLUEPRESSED - actualG - actualB;

        if ((actualB - previousB) % 4 == 0 && actualB != previousB) {
            int8_t step = knobRotated1(actualB, &previousB, BLUE);
            if (step < -1 || step > 1) {
                printf("neok\n");
            } else {
                chooseDirection(snake, step);
            }
        }
        if (knobPressed(actualKnobs, actualKnobs % BLUEPRESSED, REDPRESSED, spiled_reg_base)) {
            printf("RED EXIT\n");
            exit(-1);
        }

        moveForward(snake);

        if (checkWalls(snake)) {
            exit(0);
            //insert exit here;
        }

        if (checkFood(snake->tiles[0], apple)) {
            generateFood(apple, snake);
            snake->length++;
            snake->tiles[snake->length - 1].x = snake->lastTile.x;
            snake->tiles[snake->length - 1].y = snake->lastTile.y;

        }

        setBackground(background, screen);
        setFood(screen, apple);
        setSnake(snake, screen);
        loadScreen(screen, parlcd_reg_base);
        clock_nanosleep(CLOCK_MONOTONIC, 0, &loop_delay, NULL);
    }
=======
    union pixel **screen = allocateScreen();

    uint16_t selectorY = 22;
    uint32_t previousKnobs = *(volatile uint32_t *) (spiled_reg_base + SPILED_REG_KNOBS_8BIT_o);
    uint32_t previousB = previousKnobs % 256;
    uint32_t previousG = previousKnobs % 65536 - previousB;

    int8_t position = 0;
    uint8_t *settings = (uint8_t *) malloc(sizeof(uint8_t) * 7);
    for (size_t i = 0; i < 7; i++) {
        settings[i] = 0;
    }
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
                    break;
                case 1:
                    printf("Records pressed\n");
                    break;
                case 2:
                    settingsMenu(screen, spiled_reg_base, parlcd_reg_base, settings, &previousG);
                    break;
                default:
                    printf("Exit pressed\n");
                    goto end;
            }
        }
        setBackground(background, screen);
        setSelector(selector, screen, selectorY, 50);
        loadScreen(screen, parlcd_reg_base);
        clock_nanosleep(CLOCK_MONOTONIC, 0, &loop_delay, NULL);
    }
    end:
    endGame(screen, background, selector, settings);
    return 0;
>>>>>>> b878198bbf26b6f78952c51c8218db35dc7948cb
}

/*******************************************************************
  Nazar Ponomarev, Nikita Kisel
 *******************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

#include "mzapo_parlcd.h"
#include "knobs_control.h"
#include "game_menu.h"
#include "mzapo_phys.h"
#include "mzapo_regs.h"
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
        printf("Can not load image\n");
    }

    union pixel **background = allocateScreen();
    imageToPixelArray(backgroundPicture, background);
    free(backgroundPicture);


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
}

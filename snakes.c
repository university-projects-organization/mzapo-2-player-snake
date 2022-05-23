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

int num = 0;

typedef struct tile_t {
    uint16_t x;
    uint16_t y;
    uint8_t direction;
} tile_t;

typedef struct snake_t {
    uint16_t length;
    int8_t direction;
    int8_t speed;
    unsigned char *imageTail;
    unsigned char *imageBody;
    unsigned char *imageHead;

    tile_t *tiles;
    tile_t lastTile;
} snake_t;

typedef struct food_t {
    uint16_t x;
    uint16_t y;
    uint16_t color;
    unsigned char *image;
} food_t;

void setTile(snake_t *snake, uint16_t x, uint16_t y, uint8_t index, uint8_t direction) {
    snake->tiles[index].x = x;
    snake->tiles[index].y = y;
    snake->tiles[index].direction = direction;
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


snake_t *allocateSnake(unsigned char *imageTail, unsigned char *imageBody, unsigned char *imageHead) {
    snake_t *snake = (snake_t *) malloc(sizeof(snake_t));
    snake->length = 1; // 7;
    snake->direction = 0;
    snake->speed = 1;
    snake->imageTail = imageTail;
    snake->imageBody = imageBody;
    snake->imageHead = imageHead;
    snake->tiles = (tile_t *) malloc(sizeof(tile_t) * 500);
    setTile(snake, 240, 160, 0, 0);

    /*
    setTile(snake, 240, 176, 1);
    setTile(snake, 240, 192, 2);
    setTile(snake, 240, 208, 3);
    setTile(snake, 240, 224, 4);
    setTile(snake, 240, 240, 5);
    setTile(snake, 240, 256, 6);
     */

    return snake;
}

void freeSnake(snake_t *snake) {
    free(snake->imageHead);
    free(snake->imageBody);
    free(snake->imageTail);
    free(snake->tiles);
    free(snake);
}

_Bool checkFood(tile_t tile, food_t *food) {
    _Bool collision = 0;
    if (tile.x == food->x && tile.y == food->y) {
        collision = 1;
    }
    return collision;
}

void generateFood(food_t *food, snake_t *snake1, snake_t *snake2, uint16_t foodX, uint16_t foodY) {
    time_t t;
    srand((unsigned) time(&t));
    int x, y;
    while (1) {
        jump:
        x = (rand() % 30) * TILESIZE;
        y = (rand() % 19) * TILESIZE;
        if (x == foodX && y == foodY) {
            goto jump;
        }
        for (size_t i = 0; i < snake1->length; i++) {
            if (x == snake1->tiles[i].x || y == snake1->tiles[i].y) {
                goto jump;
            }
        }
        for (size_t i = 0; i < snake2->length; i++) {
            if (x == snake2->tiles[i].x || y == snake2->tiles[i].y) {
                goto jump;
            }
        }
        break;
    }
    food->x = x;
    food->y = y;
}

food_t *allocateFood(snake_t *snake1, snake_t *snake2, uint16_t foodX, uint16_t foodY) {
    food_t *food = (food_t *) malloc(sizeof(food_t));
    generateFood(food, snake1, snake2, foodX, foodY);
    return food;
}

void freeFood(food_t *food) {
    free(food);
}

void setFood(union pixel **screen, food_t *food) {
    for (size_t y = food->y; y < TILESIZE + food->y; y++) {
        for (size_t x = food->x; x < TILESIZE + food->x; x++) {
            screen[y][x].d = food->color;
        }
    }
}

void moveForward(snake_t *snake) {
    snake->lastTile.x = snake->tiles[snake->length - 1].x;
    snake->lastTile.y = snake->tiles[snake->length - 1].y;
    snake->lastTile.direction = snake->tiles[snake->length - 1].direction;
    for (size_t i = snake->length - 1; i > 0; i--) {
        snake->tiles[i].x = snake->tiles[i - 1].x;
        snake->tiles[i].y = snake->tiles[i - 1].y;
        snake->tiles[i].direction = snake->tiles[i - 1].direction;
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
    snake->tiles[0].direction = snake->direction;
}


void up(tile_t tile, unsigned char *image, union pixel **screen) {
    int j = 0;

    for (int32_t y = tile.y + TILESIZE - 1; y >= tile.y; y--) {
        for (int32_t x = tile.x + TILESIZE - 1; x >= tile.x; x--) {
            printf("y - %d, x - %d\n", y, x);
            if ( y < 0) {
                printf("tile y - %d, tile x - %d\n", tile.y, tile.x);
                exit(-2);
            }
            if (image[j + 3] == 255) {
                screen[y][x].r = image[j] >> 3;
                screen[y][x].g = image[j + 1] >> 2;
                screen[y][x].b = image[j + 2] >> 3;
            }
            j += 4;
        }
    }
}

void down(tile_t tile, unsigned char *image, union pixel **screen) {
    int j = 0;
    for (size_t y = tile.y; y < TILESIZE + tile.y; y++) {
        for (size_t x = tile.x; x < TILESIZE + tile.x; x++) {
            if (image[j + 3] == 255) {
                screen[y][x].r = image[j] >> 3;
                screen[y][x].g = image[j + 1] >> 2;
                screen[y][x].b = image[j + 2] >> 3;
            }
            j += 4;
        }
    }
}

void left(tile_t tile, unsigned char *image, union pixel **screen) {
    int j = 0;
    for (size_t x = TILESIZE + tile.x; x > tile.x; x--) {
        for (size_t y = tile.y; y < TILESIZE + tile.y; y++) {
            if (image[j + 3] == 255) {
                screen[y][x].r = image[j] >> 3;
                screen[y][x].g = image[j + 1] >> 2;
                screen[y][x].b = image[j + 2] >> 3;
            }
            j += 4;
        }
    }
}

void right(tile_t tile, unsigned char *image, union pixel **screen) {
    int j = 0;
    for (size_t x = tile.x; x < TILESIZE + tile.x; x++) {
        for (size_t y = tile.y; y < TILESIZE + tile.y; y++) {
            if (image[j + 3] == 255) {
                screen[y][x].r = image[j] >> 3;
                screen[y][x].g = image[j + 1] >> 2;
                screen[y][x].b = image[j + 2] >> 3;
            }
            j += 4;
        }
    }
}

void switchDirection(tile_t tile, unsigned char *image, union pixel **screen) {
    switch (tile.direction) {
        case 0:
            up(tile, image, screen);
            break;
        case 1:
            right(tile, image, screen);
            break;
        case 2:
            down(tile, image, screen);
            break;
        case 3:
            left(tile, image, screen);
            break;
    }
}

void setHead(snake_t *snake, union pixel **screen) {
    switchDirection(snake->tiles[0], snake->imageHead, screen);
}

void setBody(snake_t *snake, union pixel **screen) {
    for (size_t i = 1; i < snake->length - 1; i++) {
        switchDirection(snake->tiles[i], snake->imageBody, screen);
    }
}

void setTail(snake_t *snake, union pixel **screen) {
    if (snake->length >= 2) {
        switchDirection(snake->tiles[snake->length - 1], snake->imageTail, screen);
    }
}

void setSnake(snake_t *snake, union pixel **screen) {
    setHead(snake, screen);
    setBody(snake, screen);
    setTail(snake, screen);
}

_Bool checkWalls(snake_t *snake) {
    _Bool collision = 0;

    if (snake->tiles[0].y < 0 || snake->tiles[0].y >= 320 || snake->tiles[0].x < 0 ||
        snake->tiles[0].x >= 480) {
        collision = 1;
    }
    return collision;
}

_Bool checkSnakeCollision(snake_t *snake1, snake_t *snake2, _Bool sameSnake) {
    size_t i;
    if (sameSnake) {
        i = 1;
    } else {
        i = 0;
    }

    for (; i < snake2->length; i++) {
        if (snake1->tiles[0].y >= snake2->tiles[i].y && snake1->tiles[0].y < snake2->tiles[i].y + TILESIZE &&
            snake1->tiles[0].x >= snake2->tiles[i].x && snake1->tiles[0].x < snake2->tiles[i].x + TILESIZE) {
            return 1;
        }
    }
    return 0;
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

snake_t *chooseSnakeColor(uint16_t color) {
    int width, height, channels;
    snake_t *snake;
    unsigned char *tail;
    unsigned char *body;
    unsigned char *head;

    switch (color) {
        case 0x00:
            tail = stbi_load("/tmp/nazar/resources/snakes/black_tail.png", &width, &height, &channels, 0);
            body = stbi_load("/tmp/nazar/resources/snakes/black_body.png", &width, &height, &channels, 0);
            head = stbi_load("/tmp/nazar/resources/snakes/black_head.png", &width, &height, &channels, 0);
            break;
        case 0xF800:
            tail = stbi_load("/tmp/nazar/resources/snakes/red_tail.png", &width, &height, &channels, 0);
            body = stbi_load("/tmp/nazar/resources/snakes/red_body.png", &width, &height, &channels, 0);
            head = stbi_load("/tmp/nazar/resources/snakes/red_head.png", &width, &height, &channels, 0);
            break;
        case 0x07E0:
            tail = stbi_load("/tmp/nazar/resources/snakes/green_tail.png", &width, &height, &channels, 0);
            body = stbi_load("/tmp/nazar/resources/snakes/green_body.png", &width, &height, &channels, 0);
            head = stbi_load("/tmp/nazar/resources/snakes/green_head.png", &width, &height, &channels, 0);
            break;
        case 0x001F:
            tail = stbi_load("/tmp/nazar/resources/snakes/blue_tail.png", &width, &height, &channels, 0);
            body = stbi_load("/tmp/nazar/resources/snakes/blue_body.png", &width, &height, &channels, 0);
            head = stbi_load("/tmp/nazar/resources/snakes/blue_head.png", &width, &height, &channels, 0);
            break;
    }
    if (tail == NULL || body == NULL || head == NULL) {
        printf("Can not load image\n");
    }

    snake = allocateSnake(tail, body, head);
    return snake;
}

void setUpGame(uint8_t *settings, uint16_t *color1, uint16_t *color2, _Bool *foodOwner, uint8_t *foodAmount, _Bool *vs,
               uint8_t *speed, uint8_t *boost) {
    uint16_t colors[] = {0x00, 0xF800, 0x07E0, 0x001F};
    *color1 = colors[settings[0]];
    *color2 = colors[settings[1]];
    *foodOwner = settings[2];
    switch (settings[3]) {
        case 0:
            *foodAmount = 5;/////////////////////////////////////////////////////////////////
        case 1:
            *foodAmount = 10;///////////////////////////////////////////////////////////////

    }
    *vs = settings[4];
    switch (settings[3]) {
        case 0:
            *speed = 5;/////////////////////////////////////////////////////////////////
        case 1:
            *speed = 10;///////////////////////////////////////////////////////////////

    }
    switch (settings[3]) {
        case 0:
            *boost = 5;/////////////////////////////////////////////////////////////////
        case 1:
            *boost = 10;///////////////////////////////////////////////////////////////

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

    int counterSpeed = 5;
    int boost1 = 5;
    uint16_t color1;
    uint16_t color2;
    _Bool foodOwner;
    uint8_t foodAmount;
    _Bool vs;
    uint8_t speed;
    uint8_t boost;
    setUpGame(settings, &color1, &color2, &foodOwner, &foodAmount, &vs, &speed, &boost);
    snake_t *snake1 = chooseSnakeColor(color1);
    snake_t *snake2 = chooseSnakeColor(color2);
    food_t *apple1 = allocateFood(snake1, snake2, 500, 500);
    food_t *apple2 = allocateFood(snake1, snake2, apple1->x, apple1->y);
    apple1->color = color1;
    apple2->color = color2;

    uint32_t previousKnobs = *(volatile uint32_t *) (spiled_reg_base + SPILED_REG_KNOBS_8BIT_o);
    uint32_t previousB = previousKnobs % 256;
    uint32_t previousG = previousKnobs % 65536 - previousB;
    uint32_t previousR = previousKnobs % BLUEPRESSED - previousG - previousB;

    while (1) {
        struct timespec loop_delay = {.tv_sec = 0, .tv_nsec = 100 * 1000 * 1000};

        uint32_t actualKnobs = *(volatile uint32_t *) (spiled_reg_base + SPILED_REG_KNOBS_8BIT_o);
        uint32_t actualB = actualKnobs % 256;
        uint32_t actualG = actualKnobs % 65536 - actualB;
        uint32_t actualR = actualKnobs % BLUEPRESSED - actualG - actualB;

        if ((actualB - previousB) % 4 == 0 && actualB != previousB) {
            int8_t step = knobRotated1(actualB, &previousB, BLUE);
            if (step < -1 || step > 1) {
                printf("neok\n");
            } else {
                chooseDirection(snake2, step);
            }
        }

        if ((actualR - previousR) % 262144 == 0 && actualR != previousR) {
            int8_t step = knobRotated1(actualR, &previousR, RED);
            if (step < -1 || step > 1) {
                printf("neok\n");
            } else {
                chooseDirection(snake1, step);
            }
        }

        if (knobPressed(actualKnobs, actualKnobs % BLUEPRESSED, REDPRESSED, spiled_reg_base)) {
            printf("RED EXIT\n");
            exit(-1);
        }


        if (counterSpeed == boost1) {
            moveForward(snake1);
            moveForward(snake2);
            counterSpeed = 0;
        }
        counterSpeed++;

        if (checkWalls(snake1)) {
            exit(0);
            //insert exit here;
        }
        if (checkWalls(snake2)) {
            exit(0);
            //insert exit here;
        }

        if (checkSnakeCollision(snake1, snake1, 1)) {
            printf("Coll1 sam\n");
        }

        if (checkSnakeCollision(snake2, snake2, 1)) {
            printf("Coll2 sam\n");
        }

        if (checkSnakeCollision(snake1, snake2, 0)) {
            printf("Coll 12 sam\n");
        }

        if (checkSnakeCollision(snake2, snake1, 0)) {
            printf("Coll 21 sam\n");
        }

        if (checkFood(snake1->tiles[0], apple1)) {
            generateFood(apple1, snake1, snake2, apple2->x, apple2->y);
            snake1->length++;
            snake1->tiles[snake1->length - 1].x = snake1->lastTile.x;
            snake1->tiles[snake1->length - 1].y = snake1->lastTile.y;
            snake1->tiles[snake1->length - 1].direction = snake1->lastTile.direction;
            boost1--;
            boost1 = boost1 <= 0 ? 5 : boost1;

        }
        if (checkFood(snake2->tiles[0], apple2)) {
            generateFood(apple2, snake1, snake2, apple1->x, apple1->y);
            snake2->length++;
            snake2->tiles[snake2->length - 1].x = snake2->lastTile.x;
            snake2->tiles[snake2->length - 1].y = snake2->lastTile.y;
            snake1->tiles[snake2->length - 1].direction = snake2->lastTile.direction;
            boost1--;
            boost1 = boost1 <= 0 ? 5 : boost1;
        }

        setBackground(background, screen);
        setFood(screen, apple1);
        setFood(screen, apple2);
        setSnake(snake1, screen);
        setSnake(snake2, screen);
        loadScreen(screen, parlcd_reg_base);
        clock_nanosleep(CLOCK_MONOTONIC, 0, &loop_delay, NULL);
    }
}
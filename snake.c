#include <stdlib.h>
#include <stdio.h>
#include "snake.h"
#include "food.h"
#include "snake_structure.h"
#include "stb_library/stb_image.h"
#include "stb_library/stb_image_write.h"

#define TILESIZE 16

void setTile(snake_t *snake, uint16_t x, uint16_t y, uint8_t index, uint8_t direction) {
    snake->tiles[index].x = x;
    snake->tiles[index].y = y;
    snake->tiles[index].direction = direction;
}

snake_t *
allocateSnake(unsigned char *imageTail, unsigned char *imageBody, unsigned char *imageHead, unsigned char *color,
              uint16_t x, uint16_t y, int8_t direction) {
    snake_t *snake = (snake_t *) malloc(sizeof(snake_t));
    snake->length = 1; // 7;
    snake->direction = direction;
    snake->speed = 1;
    snake->imageTail = imageTail;
    snake->imageBody = imageBody;
    snake->imageHead = imageHead;
    snake->color = color;
    snake->tiles = (tile_t *) malloc(sizeof(tile_t) * 500);
    setTile(snake, x, y, 0, 0);

    return snake;
}

void freeSnake(snake_t *snake) {
    free(snake->color);
    free(snake->imageHead);
    free(snake->imageBody);
    free(snake->imageTail);
    free(snake->tiles);
    free(snake);
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
            if (y < 0) {
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

snake_t *chooseColor(uint16_t color, food_t *food, uint16_t x, uint16_t y, int8_t direction) {
    int width, height, channels;
    snake_t *snake;
    unsigned char *apple = NULL;
    unsigned char *pic = NULL;
    unsigned char *tail = NULL;
    unsigned char *body = NULL;
    unsigned char *head = NULL;

    switch (color) {
        case 0x00:
            apple = stbi_load("/tmp/nazar/resources/apples/black_apple.png", &width, &height, &channels, 0);
            pic = stbi_load("/tmp/nazar/resources/snakes/black.png", &width, &height, &channels, 0);
            tail = stbi_load("/tmp/nazar/resources/snakes/black_tail.png", &width, &height, &channels, 0);
            body = stbi_load("/tmp/nazar/resources/snakes/black_body.png", &width, &height, &channels, 0);
            head = stbi_load("/tmp/nazar/resources/snakes/black_head.png", &width, &height, &channels, 0);
            break;
        case 0xF800:
            apple = stbi_load("/tmp/nazar/resources/apples/red_apple.png", &width, &height, &channels, 0);
            pic = stbi_load("/tmp/nazar/resources/snakes/red.png", &width, &height, &channels, 0);
            tail = stbi_load("/tmp/nazar/resources/snakes/red_tail.png", &width, &height, &channels, 0);
            body = stbi_load("/tmp/nazar/resources/snakes/red_body.png", &width, &height, &channels, 0);
            head = stbi_load("/tmp/nazar/resources/snakes/red_head.png", &width, &height, &channels, 0);
            break;
        case 0x07E0:
            apple = stbi_load("/tmp/nazar/resources/apples/green_apple.png", &width, &height, &channels, 0);
            pic = stbi_load("/tmp/nazar/resources/snakes/green.png", &width, &height, &channels, 0);
            tail = stbi_load("/tmp/nazar/resources/snakes/green_tail.png", &width, &height, &channels, 0);
            body = stbi_load("/tmp/nazar/resources/snakes/green_body.png", &width, &height, &channels, 0);
            head = stbi_load("/tmp/nazar/resources/snakes/green_head.png", &width, &height, &channels, 0);
            break;
        case 0x001F:
            apple = stbi_load("/tmp/nazar/resources/apples/blue_apple.png", &width, &height, &channels, 0);
            pic = stbi_load("/tmp/nazar/resources/snakes/blue.png", &width, &height, &channels, 0);
            tail = stbi_load("/tmp/nazar/resources/snakes/blue_tail.png", &width, &height, &channels, 0);
            body = stbi_load("/tmp/nazar/resources/snakes/blue_body.png", &width, &height, &channels, 0);
            head = stbi_load("/tmp/nazar/resources/snakes/blue_head.png", &width, &height, &channels, 0);
            break;
    }

    if (apple == NULL) {
        printf("Can not load apple\n");
    }
    if (tail == NULL) {
        printf("Can not load tail\n");
    }
    if (body == NULL) {
        printf("Can not load body\n");
    }
    if (pic == NULL) {
        printf("Can not load pic\n");
    }
    if (head == NULL) {
        printf("Can not load head\n");
    }

    setFoodImage(food, apple);
    snake = allocateSnake(tail, body, head, pic, x, y, direction);
    return snake;
}
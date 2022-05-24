#include <stdlib.h>
#include <stdio.h>
#include "collisions.h"

#define TILESIZE 16

_Bool checkWalls(snake_t *snake) {
    _Bool collision = 0;

    if (snake->tiles[0].y < 0 || snake->tiles[0].y >= 320 || snake->tiles[0].x < 0 || snake->tiles[0].x >= 480) {
        collision = 1;
    }
    return collision;
}

_Bool checkApple(tile_t tile, food_t *food) {
    for (size_t i = 0; i < 20; i++) {
        if (food->array[i].x == 0) continue;
        if (tile.x == food->array[i].x && tile.y == food->array[i].y) {
            food->array[i].x = 0;
            return 1;
        }
    }
    return 0;
}


_Bool checkSnakeCollision(snake_t *snake, snake_t *target, _Bool sameSnake) {
    size_t i;
    if (sameSnake) {
        i = 1;
    } else {
        i = 0;
        if (snake->length == 1 && target->length == 1) {
            if (snake->direction == 1 && target->direction == 3 && snake->tiles[0].x == target->tiles[0].x + 16 &&
                snake->tiles[0].y == target->tiles[0].y && snake->lastTile.direction == 1 &&
                target->lastTile.direction == 3) {
                return 1;
            }
            if (snake->direction == 2 && target->direction == 0 && snake->tiles[0].y == target->tiles[0].y + 16 &&
                snake->tiles[0].x == target->tiles[0].x && snake->lastTile.direction == 2 &&
                target->lastTile.direction == 0) {
                return 1;
            }
        }
    }

    for (; i < target->length; i++) {
        if (snake->tiles[0].y >= target->tiles[i].y && snake->tiles[0].y < target->tiles[i].y + TILESIZE &&
            snake->tiles[0].x >= target->tiles[i].x && snake->tiles[0].x < target->tiles[i].x + TILESIZE) {
            return 1;
        }
    }
    return 0;
}

void checkFood(snake_t *snake1, snake_t *snake2, food_t *food1, food_t *food2, _Bool foodOwner) {
    if (checkApple(snake1->tiles[0], food1)) {
        snake1->length++;
        snake1->tiles[snake1->length - 1].x = snake1->lastTile.x;
        snake1->tiles[snake1->length - 1].y = snake1->lastTile.y;
        snake1->tiles[snake1->length - 1].direction = snake1->lastTile.direction;
        food1->length--;
    }
    if (checkApple(snake2->tiles[0], food2)) {
        snake2->length++;
        snake2->tiles[snake2->length - 1].x = snake2->lastTile.x;
        snake2->tiles[snake2->length - 1].y = snake2->lastTile.y;
        snake2->tiles[snake2->length - 1].direction = snake2->lastTile.direction;
        food2->length--;
    }
    if (!foodOwner) {
        if (checkApple(snake1->tiles[0], food2)) {
            snake1->length++;
            snake1->tiles[snake1->length - 1].x = snake1->lastTile.x;
            snake1->tiles[snake1->length - 1].y = snake1->lastTile.y;
            snake1->tiles[snake1->length - 1].direction = snake1->lastTile.direction;
            food2->length--;
        }
        if (checkApple(snake2->tiles[0], food1)) {
            snake2->length++;
            snake2->tiles[snake2->length - 1].x = snake2->lastTile.x;
            snake2->tiles[snake2->length - 1].y = snake2->lastTile.y;
            snake2->tiles[snake2->length - 1].direction = snake2->lastTile.direction;
            food1->length--;
        }
    }
}

#include <stdlib.h>
#include "food.h"
#include <time.h>
//#include "snakes.c"

#define TILESIZE 16

food_t *allocateFood() {
    food_t *food = (food_t *) malloc(sizeof(food_t));
    food->length = 0;
    food->image = NULL;
    food->array = (apple_t *) malloc(sizeof(apple_t) * 20);
    for (size_t i = 0; i < 20; i++) {
        food->array[i].x = 0;
    }
    return food;
}

void freeFood (food_t *food) {
    free(food->image);
    free(food->array);
    free(food);
}

void setFoodImage(food_t *food, unsigned char *image) {
    food->image = image;
}

void generateApple(snake_t *snake1, snake_t *snake2, food_t *food1, food_t *food2) {
    size_t index;
    for (index = 0; index < 20; index++) {
        if (food1->array[index].x == 0) break;
    }

    time_t t;
    srand((unsigned) time(&t));
    int x, y;
    while (1) {
        jump:
        x = (rand() % 30) * TILESIZE;
        y = (rand() % 19) * TILESIZE;
        for (size_t j = 0; j < 20; j++) {
            if (food1->array[j].x == 0 || j == index) continue;
            if (x == food1->array[j].x && y == food1->array[j].y) {
                goto jump;
            }
        }

        for (size_t j = 0; j < 20; j++) {
            if (food2->array[j].x == 0) continue;
            if (x == food2->array[j].x && y == food2->array[j].y) {
                goto jump;
            }
        }

        for (size_t i = 0; i < snake1->length; i++) {
            if (x == snake1->tiles[i].x && y == snake1->tiles[i].y) {
                goto jump;
            }
        }
        for (size_t i = 0; i < snake2->length; i++) {
            if (x == snake2->tiles[i].x && y == snake2->tiles[i].y) {
                goto jump;
            }
        }
        if ((x <= 64 && y <= 32) || (x >= 416 && y <= 32)) {
            goto jump;
        }
        food1->array[index].x = x;
        food1->array[index].y = y;
        break;
    }
}

void setFood(union pixel **screen, food_t *food) {
    for (size_t i = 0; i < 20; i++) {
        if (food->array[i].x == 0) continue;
        setPngImage(screen, food->array[i].x, food->array[i].y, food->image, TILESIZE);
    }
}
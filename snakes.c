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
#include "word_from_font.h"
#include "food.h"
#include "snake.h"
#include "stb_library/stb_image.h"
#include "stb_library/stb_image_write.h"
#include "records.h"

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
    }

    for (; i < target->length; i++) {
        if (snake->tiles[0].y >= target->tiles[i].y && snake->tiles[0].y < target->tiles[i].y + TILESIZE &&
            snake->tiles[0].x >= target->tiles[i].x && snake->tiles[0].x < target->tiles[i].x + TILESIZE) {
            return 1;
        }
    }
    return 0;
}

void setUpGame(uint8_t *settings, uint16_t *color1, uint16_t *color2, _Bool *foodOwner, uint8_t *foodAmount, _Bool *vs,
               uint8_t *speed, uint8_t *boost) {
    uint16_t colors[] = {0x00, 0xF800, 0x07E0, 0x001F};
    *color1 = colors[settings[0]];
    *color2 = colors[settings[1]];
    *foodOwner = settings[2];
    printf("settings[3] %d\n", settings[3]);
    switch (settings[3]) {
        case 0:
            *foodAmount = 20;
            break;
        case 1:
            *foodAmount = 60;
            break;
    }
    *vs = settings[4];
    switch (settings[5]) {
        case 0: // medium
            *speed = 20;
            break;
        case 1: // fast
            *speed = 10;
            break;
        case 2: // slow
            *speed = 30;
            break;

    }
    switch (settings[6]) {
        case 0:
            *boost = 80;
            break;
        case 1:
            *boost = 40;
            break;
        case 2:
            *boost = 120;
            break;

    }
}

void
freeAllocatedMemory(union pixel **screen, union pixel **background, snake_t *snake1, snake_t *snake2, food_t *food1,
                    food_t *food2) {
    freeScreen(screen);
    freeScreen(background);
    freeSnake(snake1);
    freeSnake(snake2);
    freeFood(food1);
    freeFood(food2);
}

void gameOver(snake_t *snake, uint16_t color, union pixel **screen, volatile void *spiled_reg_base,
              unsigned char *parlcd_reg_base, int8_t winner, uint32_t* colorLed1, uint32_t* colorLed2) {
    *(volatile uint32_t *) (spiled_reg_base + SPILED_REG_LED_LINE_o) = (snake->length - 1) << 8;
    wordBuffer *word = makeWordBuffer("GAME OVER", 5);
    setWord(word, (uint16_t) 0x000, screen, 50, HEIGHT / 2 - 16 * 5);
    freeWordBuffer(word);
    switch (winner) {
        case 0:
            word = makeWordBuffer("LEFT PLAYER WINS", 2);
            setWord(word, (uint16_t) 0x000, screen, 120, 160);
            *colorLed2 = 0;
            break;
        case 1:
            word = makeWordBuffer("RIGHT PLAYER WINS", 2);
            setWord(word, (uint16_t) 0x000, screen, 110, 160);
            *colorLed1 = 0;
            break;
        default:
            word = makeWordBuffer("TIE", 3);
            setWord(word, (uint16_t) 0x000, screen, 220, 165);
    }
    freeWordBuffer(word);
    loadScreen(screen, parlcd_reg_base);
    record_t **records = readRecords();
    saveRecords(snake, color, records);

    int8_t frequency = 30;
    while (1) {
        uint32_t actualKnobs = *(volatile uint32_t *) (spiled_reg_base + SPILED_REG_KNOBS_8BIT_o);
        uint8_t whichKnobPressed = (uint8_t) (actualKnobs >> 24);
        if (whichKnobPressed == 4) {
            knobUnpressed(spiled_reg_base);
            exit(0);
        }

        if (frequency >= 30) {
            *(volatile uint32_t *) (spiled_reg_base + SPILED_REG_LED_RGB1_o) = *colorLed1;
            *(volatile uint32_t *) (spiled_reg_base + SPILED_REG_LED_RGB2_o) = *colorLed2;
            frequency = 0;
        } else {
            *(volatile uint32_t *) (spiled_reg_base + SPILED_REG_LED_RGB1_o) = 0;
            *(volatile uint32_t *) (spiled_reg_base + SPILED_REG_LED_RGB2_o) = 0;
            frequency++;
        }
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
    uint32_t colorLed1 = (255 << 16) + (255 << 8) + 150;
    uint32_t colorLed2 = (255 << 16) + (255 << 8) + 150;

    union pixel **screen = allocateScreen();
    if (!gameMenu(screen, settings, spiled_reg_base, parlcd_reg_base, &colorLed1, &colorLed2)) {
        return 0;
    }

    int width, height, channels;
    unsigned char *backgroundPicture = stbi_load("/tmp/nazar/resources/GameField/GameBack.png", &width, &height,
                                                 &channels, 0);
    if (backgroundPicture == NULL) {
        printf("Can not load image\n");
    }

    union pixel **background = allocateScreen();
    pngImageToPixelArray(backgroundPicture, background);
    free(backgroundPicture);

    uint16_t color1;
    uint16_t color2;
    _Bool foodOwner;
    uint8_t foodAmount;
    _Bool collisions;
    uint8_t speed;
    uint8_t boost;

    setUpGame(settings, &color1, &color2, &foodOwner, &foodAmount, &collisions, &speed, &boost);
    free(settings);
    food_t *food1 = allocateFood();
    food_t *food2 = allocateFood();
    snake_t *snake1 = chooseColor(color1, food1, (uint16_t) 0, (uint16_t) 160, (int8_t) 1);
    snake_t *snake2 = chooseColor(color2, food2, (uint16_t) 464, (uint16_t) 160,
                                  (int8_t) 3);//////////////////////////////////////////////////////////////
    int frequency = foodAmount;
    int counterSpeed = speed;
    int k = speed;
    int l = 0;

    uint32_t previousKnobs = *(volatile uint32_t *) (spiled_reg_base + SPILED_REG_KNOBS_8BIT_o);
    uint8_t previousB = (uint8_t) previousKnobs;
    uint8_t previousR = (uint8_t) (previousKnobs >> 16);

    uint8_t winner = 0;
    while (1) {
        struct timespec loop_delay = {.tv_sec = 0, .tv_nsec = 2000 * 1000 * 1000};

        uint32_t actualKnobs = *(volatile uint32_t *) (spiled_reg_base + SPILED_REG_KNOBS_8BIT_o);
        uint8_t actualB = (uint8_t) actualKnobs;
        uint8_t actualR = (uint8_t) (actualKnobs >> 16);

        uint8_t whichKnobPressed = (uint8_t) (actualKnobs >> 24);

        if ((actualB - previousB) % 4 == 0 && actualB != previousB) {
            int8_t step = knobRotated(actualB, &previousB, 0);
            if (step < -1 || step > 1) {
                printf("neok\n");
            } else {
                chooseDirection(snake2, step);
            }
        }

        if ((actualR - previousR) % 4 == 0 && actualR != previousR) {
            int8_t step = knobRotated(actualR, &previousR, 0);
            if (step < -1 || step > 1) {
                printf("neok\n");
            } else {
                chooseDirection(snake1, step);
            }
        }


        if (whichKnobPressed == 4) {
            printf("RED EXIT\n");
            exit(-1);
        }

        if (l >= boost) {
            counterSpeed = counterSpeed > 1 ? counterSpeed - 1 : 1;
            l = 0;
        } else {
            l++;
        }


        if (k >= counterSpeed) {
            moveForward(snake1);
            moveForward(snake2);
            k = 0;
        } else {
            k++;
        }

        if (checkWalls(snake1)) {
            winner += 1;
            //gameOver(snake2, color2, screen, spiled_reg_base, parlcd_reg_base, 1);
        }
        if (checkWalls(snake2)) {
            winner += 2;
            //gameOver(snake1, color1, screen, spiled_reg_base, parlcd_reg_base, 0);
        }

        if (collisions) {
            if (checkSnakeCollision(snake1, snake1, 1)) {
                winner += 1;
                //gameOver(snake2, color2, screen, spiled_reg_base, parlcd_reg_base, 1);
            }

            if (checkSnakeCollision(snake2, snake2, 1)) {
                winner += 2;
                //gameOver(snake1, color1, screen, spiled_reg_base, parlcd_reg_base, 0);
            }

            if (checkSnakeCollision(snake1, snake2, 0)) {
                winner += 1;
                //gameOver(snake2, color2, screen, spiled_reg_base, parlcd_reg_base, 1);
            }

            if (checkSnakeCollision(snake2, snake1, 0)) {
                winner += 2;
                //gameOver(snake1, color1, screen, spiled_reg_base, parlcd_reg_base, 0);
            }
        }

        if (winner == 1) {
            gameOver(snake2, color2, screen, spiled_reg_base, parlcd_reg_base, 1, &colorLed1, &colorLed2);
            goto end;
        } else if (winner == 2) {
            gameOver(snake1, color1, screen, spiled_reg_base, parlcd_reg_base, 0,  &colorLed1, &colorLed2);
            goto end;
        } else if (winner == 3) {
            if (snake2->length > snake1->length) {
                gameOver(snake1, color1, screen, spiled_reg_base, parlcd_reg_base, 0,  &colorLed1, &colorLed2);
            } else if (snake2->length < snake1->length) {
                gameOver(snake2, color2, screen, spiled_reg_base, parlcd_reg_base, 1,  &colorLed1, &colorLed2);
            } else {
                gameOver(snake2, color2, screen, spiled_reg_base, parlcd_reg_base, 2,  &colorLed1, &colorLed2);
            }
            goto end;
        }

        if (frequency == foodAmount) {
            if (food1->length < 20) {
                generateApple(snake1, snake2, food1, food2);
                food1->length++;
            }
            if (food2->length < 20) {
                generateApple(snake1, snake2, food2, food1);
                food2->length++;
            }
            frequency = 0;
        } else {
            frequency++;
        }

        if (checkApple(snake1->tiles[0], food1)) {
            snake1->length++;
            snake1->tiles[snake1->length - 1].x = snake1->lastTile.x;
            snake1->tiles[snake1->length - 1].y = snake1->lastTile.y;
            snake1->tiles[snake1->length - 1].direction = snake1->lastTile.direction;
            counterSpeed = counterSpeed == 1 ? 1 : counterSpeed - 1;
            food1->length--;
        }
        if (checkApple(snake2->tiles[0], food2)) {
            snake2->length++;
            snake2->tiles[snake2->length - 1].x = snake2->lastTile.x;
            snake2->tiles[snake2->length - 1].y = snake2->lastTile.y;
            snake2->tiles[snake2->length - 1].direction = snake2->lastTile.direction;
            counterSpeed = counterSpeed == 1 ? 1 : counterSpeed - 1;
            food2->length--;
        }
        if (!foodOwner) {
            if (checkApple(snake1->tiles[0], food2)) {
                snake1->length++;
                snake1->tiles[snake1->length - 1].x = snake1->lastTile.x;
                snake1->tiles[snake1->length - 1].y = snake1->lastTile.y;
                snake1->tiles[snake1->length - 1].direction = snake1->lastTile.direction;
                counterSpeed = counterSpeed == 1 ? 1 : counterSpeed - 1;
                food2->length--;
            }
            if (checkApple(snake2->tiles[0], food1)) {
                snake2->length++;
                snake2->tiles[snake2->length - 1].x = snake2->lastTile.x;
                snake2->tiles[snake2->length - 1].y = snake2->lastTile.y;
                snake2->tiles[snake2->length - 1].direction = snake2->lastTile.direction;
                counterSpeed = counterSpeed == 1 ? 1 : counterSpeed - 1;
                food1->length--;
            }
        }

        *(volatile uint32_t *) (spiled_reg_base + SPILED_REG_LED_LINE_o) =
                ((snake1->length - 1) << 16) + (snake2->length - 1);
        *(volatile uint32_t *) (spiled_reg_base + SPILED_REG_LED_RGB1_o) = colorLed1;
        *(volatile uint32_t *) (spiled_reg_base + SPILED_REG_LED_RGB2_o) = colorLed2;

        setBackground(background, screen);
        setFood(screen, food1);
        setFood(screen, food2);
        setSnake(snake1, screen);
        setSnake(snake2, screen);

        setPngImage(screen, 0, 0, snake1->color, 32);
        char string[6];
        sprintf(string, "%d", snake1->length - 1);
        wordBuffer *word = makeWordBuffer(string, 2);
        setWord(word, color1, screen, 33, 0);
        freeWordBuffer(word);

        setPngImage(screen, 447, 0, snake2->color, 32);
        sprintf(string, "%d", snake2->length - 1);
        word = makeWordBuffer(string, 2);
        uint16_t screenX = 432;
        if (snake2->length - 1 >= 10 && snake2->length - 1 < 100) {
            screenX -= 14;
        } else if (snake2->length - 1 >= 100) {
            screenX -= 30;
        }
        setWord(word, color2, screen, screenX, 0);
        freeWordBuffer(word);

        loadScreen(screen, parlcd_reg_base);
        clock_nanosleep(CLOCK_MONOTONIC, 0, &loop_delay, NULL);
    }
    end:
    freeAllocatedMemory(screen, background, snake1, snake2, food1, food2);
}
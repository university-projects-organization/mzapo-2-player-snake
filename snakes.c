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

#define TILESIZE 16

typedef struct {
    char *stringColor;
    int16_t eatenApples;
    int16_t year;
    int16_t month;
    int16_t day;
    int16_t hour;
    int16_t minute;
} record_t;

record_t **allocateRecords() {
    record_t **records = (record_t **)malloc(sizeof(record_t *) * 5);
    for (size_t i = 0; i < 5; i++) {
        records[i] = (record_t *) malloc(sizeof(record_t));
        records[i]->stringColor = (char *) malloc(sizeof(char) * 6);
    }
    return records;
}


int8_t knobRotated1(uint8_t actualValue, uint8_t *previousValue, int8_t knob) {
    int coef;
    switch (knob) {
        case BLUE:
            coef = 4;
            break;
        case RED:
            coef = 4;
            break;
    }
    int16_t difference = (actualValue - *previousValue) / coef;
    difference = difference == 63 ? -1 : difference == -63 ? 1 : difference;
    int8_t step = difference;
    *previousValue = actualValue;
    return step;
}

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
    switch (settings[3]) {
        case 0:
            *speed = 5;
            break;
        case 1:
            *speed = 10;
            break;

    }
    switch (settings[3]) {
        case 0:
            *boost = 5;
            break;
        case 1:
            *boost = 10;
            break;

    }
}

void selectColor(uint16_t color, char *stringColor) {
    switch (color) {
        case 0x00:
            stringColor[0] = 'B';
            stringColor[1] = 'L';
            stringColor[2] = 'A';
            stringColor[3] = 'C';
            stringColor[4] = 'K';
            stringColor[5] = '\0';
            break;
        case 0xF800:
            stringColor[0] = 'R';
            stringColor[1] = 'E';
            stringColor[2] = 'D';
            stringColor[3] = '\0';
            break;
        case 0x07E0:
            stringColor[0] = 'G';
            stringColor[1] = 'R';
            stringColor[2] = 'E';
            stringColor[3] = 'E';
            stringColor[4] = 'N';
            stringColor[5] = '\0';
            break;
        case 0x001F:
            stringColor[0] = 'B';
            stringColor[1] = 'L';
            stringColor[2] = 'U';
            stringColor[2] = 'E';
            stringColor[3] = '\0';
            break;
    }
}

void writeRecord(record_t *record, uint16_t color, FILE *fp) {
    fprintf(fp, "%s %d %d-%02d-%02d %02d:%02d\n", record->stringColor, record->eatenApples, record->year, record->month,
            record->month, record->day, record->hour);
    fclose(fp);
}

void readLine(FILE *fp, record_t *record) {
    size_t i = 0;
    while (1) {
        char str[2];
        fgets(str, 1, fp);
        if (str[0] == ' ') {
            record->stringColor[i] = '\0';
            break;
        } else {
            record->stringColor[i] = str[0];
            i++;
        }
    }


    char number[5];
    i = 0;
    while (1) {
        char str[2];
        fgets(str, 1, fp);
        if (str[0] == ' ') {
            number[i] = '\0';
            break;
        } else {
            number[i] = str[0];
            i++;
        }
    }
    record->eatenApples = atoi(number);

    i = 0;
    while (1) {
        char str[2];
        fgets(number, 1, fp);
        if (number[0] == '-') {
            number[i] = '\0';
            break;
        } else {
            number[i] = str[0];
            i++;
        }
    }
    record->year = atoi(number);

    i = 0;
    while (1) {
        char str[2];
        fgets(number, 1, fp);
        if (number[0] == '-') {
            number[i] = '\0';
            break;
        } else {
            number[i] = str[0];
            i++;
        }
    }
    record->month = atoi(number);

    i = 0;
    while (1) {
        char str[2];
        fgets(number, 1, fp);
        if (number[0] == ' ') {
            number[i] = '\0';
            break;
        } else {
            number[i] = str[0];
            i++;
        }
    }
    record->day = atoi(number);

    i = 0;
    while (1) {
        char str[2];
        fgets(number, 1, fp);
        if (number[0] == ':') {
            number[i] = '\0';
            break;
        } else {
            number[i] = str[0];
            i++;
        }
    }
    record->hour = atoi(number);

    i = 0;
    while (1) {
        char str[2];
        fgets(number, 1, fp);
        if (number[0] == '\n') {
            number[i] = '\0';
            break;
        } else {
            number[i] = str[0];
            i++;
        }
    }
    record->minute = atoi(number);
}

record_t **readRecords() {
    record_t **records = allocateRecords();
    FILE *fp = fopen("\\resources\\records\\records.txt", "r");

    size_t i = 0;
    while (feof(fp)) {
        readLine(fp, records[i]);
        i++;
    }
    return records;
}

record_t *setNewRecord(uint16_t color, uint16_t eatenApples) {
    record_t *record = malloc(sizeof(record));
    record->stringColor = (char *) malloc(sizeof(char) * 6);
    selectColor(color, record->stringColor);
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    record->eatenApples = eatenApples;
    record->year = tm.tm_year + 1900;
    record->month = tm.tm_mon + 1;
    record->day = tm.tm_mday;
    record->hour = tm.tm_hour;
    record->minute = tm.tm_min;
    return record;
}

void saveRecords(snake_t *snake, uint16_t color, record_t **records) {
    FILE *fp = fopen("\\resources\\records\\records.txt", "w");
    if (fp == NULL) {
        printf("Can't open file\n");
    }

    record_t *newRecord = setNewRecord(color, snake->length + 1);

    for (size_t i = 0; i < 5; i++) {
        if (newRecord->eatenApples > records[i]->eatenApples) {
            writeRecord(newRecord, color, fp);
        } else {
            writeRecord(records[i], color, fp);
        }
        i++;
    }
}


void gameOver(snake_t *snake, uint16_t color, union pixel **screen, volatile void *spiled_reg_base,
              unsigned char *parlcd_reg_base) {
    wordBuffer *word = makeWordBuffer("GAME OVER", 5);
    setWord(word, (uint16_t) 0x000, screen, 50, HEIGHT / 2 - 16 * 2);
    loadScreen(screen, parlcd_reg_base);
    record_t **records = readRecords();
    saveRecords(snake, color, records);
    exit(0);
}

void displayOneRecord(record_t *record, union pixel **screen, uint16_t x, uint16_t y) {
    wordBuffer *word = makeWordBuffer(record->stringColor, 2);

    freeWordBuffer(word);

    char string[6];
    sprintf(string, "%d", record->eatenApples);
    word = makeWordBuffer(string, 1);
    setWord(word, (uint16_t) 0x00, screen, x + 1000, y + 1000);

    sprintf(string, "%d", record->year);
    word = makeWordBuffer(string, 1);
    setWord(word, (uint16_t) 0x00, screen, x + 1000, y + 1000);

    sprintf(string, "%d", record->month);
    word = makeWordBuffer(string, 1);
    setWord(word, (uint16_t) 0x00, screen, x + 1000, y + 1000);

    sprintf(string, "%d", record->day);
    word = makeWordBuffer(string, 1);
    setWord(word, (uint16_t) 0x00, screen, x + 1000, y + 1000);

    sprintf(string, "%d", record->hour);
    word = makeWordBuffer(string, 1);
    setWord(word, (uint16_t) 0x00, screen, x + 1000, y + 1000);

    sprintf(string, "%d", record->minute);
    word = makeWordBuffer(string, 1);
    setWord(word, (uint16_t) 0x00, screen, x + 1000, y + 1000);
}

void recordsMenu(union pixel **screen, unsigned char *parlcd_reg_base) {
    record_t **records = readRecords();
    uint16_t x = 0;
    uint16_t y = 0;
    for (size_t i = 0; i < 5; i++) {
        displayOneRecord(records[i], screen,  x,  y);
        y += 20;
    }
    loadScreen(screen, parlcd_reg_base);

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
    unsigned char *backgroundPicture = stbi_load("/tmp/nazar/resources/GameField/GameBack.png", &width, &height,
                                                 &channels, 0);
    if (backgroundPicture == NULL) {
        printf("Can not load image\n");
    }

    union pixel **background = allocateScreen();
    pngImageToPixelArray(backgroundPicture, background);
    free(backgroundPicture);


    int counterSpeed = 30;
    int boost1 = 5;
    uint16_t color1;
    uint16_t color2;
    _Bool foodOwner;
    uint8_t foodAmount;
    _Bool collisions;
    uint8_t speed;
    uint8_t boost;

    setUpGame(settings, &color1, &color2, &foodOwner, &foodAmount, &collisions, &speed, &boost);
    food_t *food1 = allocateFood();
    food_t *food2 = allocateFood();
    snake_t *snake1 = chooseColor(color1, food1, (uint16_t) 0, (uint16_t) 160, (int8_t) 1);
    snake_t *snake2 = chooseColor(color2, food2, (uint16_t) 464, (uint16_t) 160, (int8_t) 3);
    int frequency = foodAmount;

    int k = 30;

    uint32_t previousKnobs = *(volatile uint32_t *) (spiled_reg_base + SPILED_REG_KNOBS_8BIT_o);
    uint8_t previousB = (uint8_t)previousKnobs;
    uint8_t previousG = (uint8_t)(previousKnobs >> 8);
    uint8_t previousR = (uint8_t)(previousKnobs >> 16);

    while (1) {
        struct timespec loop_delay = {.tv_sec = 0, .tv_nsec = 2000 * 1000 * 1000};

        uint32_t actualKnobs = *(volatile uint32_t *) (spiled_reg_base + SPILED_REG_KNOBS_8BIT_o);
        uint8_t actualB = (uint8_t)actualKnobs;
        uint8_t actualG = (uint8_t)(actualKnobs >> 8); // actualKnobs % 65536 - actualB;
        uint8_t actualR = (uint8_t)(actualKnobs >> 16); // actualKnobs % BLUEPRESSED;

        uint8_t whichKnobPressed = (uint8_t)(actualKnobs >> 24);

        if ((actualB - previousB) % 4 == 0 && actualB != previousB) {
            int8_t step = knobRotated1(actualB, &previousB, BLUE);
            if (step < -1 || step > 1) {
                printf("neok\n");
            } else {
                chooseDirection(snake2, step);
            }
        }

        if ((actualR - previousR) % 4 == 0 && actualR != previousR) {
            int8_t step = knobRotated1(actualR, &previousR, RED);
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


        if (k == counterSpeed) {
            moveForward(snake1);
            moveForward(snake2);
            k = 0;
        } else {
            k++;
        }


        if (checkWalls(snake1)) {
            gameOver(snake2, color2, screen, spiled_reg_base, parlcd_reg_base);
        }
        if (checkWalls(snake2)) {
            gameOver(snake1, color1, screen, spiled_reg_base, parlcd_reg_base);
        }

        if (collisions) {
            if (checkSnakeCollision(snake1, snake1, 1)) {
                gameOver(snake2, color2, screen, spiled_reg_base, parlcd_reg_base);
            }

            if (checkSnakeCollision(snake2, snake2, 1)) {
                gameOver(snake1, color1, screen, spiled_reg_base, parlcd_reg_base);
            }

            if (checkSnakeCollision(snake1, snake2, 0)) {
                gameOver(snake2, color2, screen, spiled_reg_base, parlcd_reg_base);
            }

            if (checkSnakeCollision(snake2, snake1, 0)) {
                gameOver(snake1, color1, screen, spiled_reg_base, parlcd_reg_base);
            }
        }

        if (frequency == foodAmount) {
            if (food1->length < 20) {
                generateApple(snake1, snake2, food1, food2);
            }
            if (food2->length < 20) {
                generateApple(snake1, snake2, food2, food1);
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

        }
        if (checkApple(snake2->tiles[0], food2)) {
            snake2->length++;
            snake2->tiles[snake2->length - 1].x = snake2->lastTile.x;
            snake2->tiles[snake2->length - 1].y = snake2->lastTile.y;
            snake1->tiles[snake2->length - 1].direction = snake2->lastTile.direction;
            counterSpeed = counterSpeed == 1 ? 1 : counterSpeed - 1;
        }
        if (!foodOwner) {
            if (checkApple(snake1->tiles[0], food2)) {
                snake1->length++;
                snake1->tiles[snake1->length - 1].x = snake1->lastTile.x;
                snake1->tiles[snake1->length - 1].y = snake1->lastTile.y;
                snake1->tiles[snake1->length - 1].direction = snake1->lastTile.direction;
                counterSpeed = counterSpeed == 1 ? 1 : counterSpeed - 1;
            }
            if (checkApple(snake2->tiles[0], food1)) {
                snake2->length++;
                snake2->tiles[snake2->length - 1].x = snake2->lastTile.x;
                snake2->tiles[snake2->length - 1].y = snake2->lastTile.y;
                snake1->tiles[snake2->length - 1].direction = snake2->lastTile.direction;
                counterSpeed = counterSpeed == 1 ? 1 : counterSpeed - 1;
            }
        }

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
        free(word);

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
        free(word);

        loadScreen(screen, parlcd_reg_base);
        clock_nanosleep(CLOCK_MONOTONIC, 0, &loop_delay, NULL);
    }
}
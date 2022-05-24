#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "records.h"
#include "wordbuffer_structure.h"
#include "word_from_font.h"
#include "screen.h"
#include "knobs_control.h"
#include "apo_library/mzapo_regs.h"
#include "stb_library/stb_image.h"
#include "stb_library/stb_image_write.h"

record_t **allocateRecords() {
    record_t **records = (record_t **) malloc(sizeof(record_t *) * 5);
    for (size_t i = 0; i < 5; i++) {
        records[i] = (record_t *) malloc(sizeof(record_t));
        records[i]->stringColor = (char *) malloc(sizeof(char) * 6);
        records[i]->stringScore = (char *) malloc(sizeof(char) * 22);
    }
    return records;
}

void freeOneRecord(record_t *record) {
    free(record->stringColor);
    free(record->stringScore);
    free(record);
}

void freeRecords(record_t **records) {
    for (size_t i = 0; i < 5; i++) {
        freeOneRecord(records[i]);
    }
    free(records);
}

void selectColor(uint16_t color, char *stringColor) {
    switch (color) {
        case 0x00:
            stringColor[0] = 'B';
            stringColor[1] = 'l';
            stringColor[2] = 'a';
            stringColor[3] = 'c';
            stringColor[4] = 'k';
            stringColor[5] = '\0';
            break;
        case 0xF800:
            stringColor[0] = 'R';
            stringColor[1] = 'e';
            stringColor[2] = 'd';
            stringColor[3] = '\0';
            break;
        case 0x07E0:
            stringColor[0] = 'G';
            stringColor[1] = 'r';
            stringColor[2] = 'e';
            stringColor[3] = 'e';
            stringColor[4] = 'n';
            stringColor[5] = '\0';
            break;
        case 0x001F:
            stringColor[0] = 'B';
            stringColor[1] = 'l';
            stringColor[2] = 'u';
            stringColor[3] = 'e';
            stringColor[4] = '\0';
            break;
    }
}

void readLine(FILE *fp, record_t *record) {
    size_t i = 0;
    char str;
    while (1) {
        str = getc(fp);
        if (str == ' ') {
            record->stringColor[i] = '\0';
            break;
        } else {
            record->stringColor[i] = str;
            i++;
        }
    }

    i = 0;
    char number[5];
    while (1) {
        str = getc(fp);
        if (str == ' ') {
            number[i] = '\0';
            break;
        } else {
            number[i] = str;
            i++;
        }
    }
    record->eatenApples = atoi(number);
    fseek(fp, -(i + 1), SEEK_CUR);

    i = 0;
    while (1) {
        str = getc(fp);
        if (str == '\n') {
            record->stringScore[i] = '\0';
            break;
        } else {
            record->stringScore[i] = str;
            i++;
        }
    }
}

record_t **readRecords() {
    record_t **records = allocateRecords();
    FILE *fp = fopen("/tmp/nazar/resources/records/records.txt", "r");
    if (fp == NULL) {
        printf("Can not open file\n");
    }
    for (size_t i = 0; i < 5; i++) {
        readLine(fp, records[i]);
    }
    fclose(fp);
    return records;
}

void displayOneRecord(record_t *record, union pixel **screen, uint16_t x, uint16_t y) {
    wordBuffer *word = makeWordBuffer(record->stringColor, 3);
    setWord(word, (uint16_t) 0x00, screen, x, y);
    x += 130;
    y += 13;
    freeWordBuffer(word);
    word = makeWordBuffer(record->stringScore, 2);
    setWord(word, (uint16_t) 0x00, screen, x, y);
    freeWordBuffer(word);
}

void recordsMenu(union pixel **screen, volatile void *spiled_reg_base, unsigned char *parlcd_reg_base, uint8_t *actualG) {
    int width, height, channels;
    unsigned char *backgroundPicture = stbi_load("/tmp/nazar/resources/GameField/GameBack.png", &width, &height,
                                                 &channels, 0);
    if (backgroundPicture == NULL) {
        printf("Can not load image\n");
    }
    union pixel **background = allocateScreen();
    pngImageToPixelArray(backgroundPicture, background);
    free(backgroundPicture);
    setBackground(background, screen);
    freeScreen(background);

    record_t **records = readRecords();
    uint16_t x = 20;
    uint16_t y = 20;
    for (size_t i = 0; i < 5; i++) {
        displayOneRecord(records[i], screen, x, y);
        y += 55;
    }
    loadScreen(screen, parlcd_reg_base);
    freeRecords(records);

    while (1) {
        uint32_t actualKnobs = *(volatile uint32_t *) (spiled_reg_base + SPILED_REG_KNOBS_8BIT_o);
        *actualG = (uint8_t) (actualKnobs >> 8);
        uint8_t whichKnobPressed = (uint8_t) (actualKnobs >> 24);
        if (whichKnobPressed == 4) {
            knobUnpressed(spiled_reg_base);
            printf("Red button\n");
            break;
        }
    }
}

record_t *setNewRecord(uint16_t color, uint16_t eatenApples) {
    record_t *record = malloc(sizeof(record_t));
    record->stringColor = (char *) malloc(sizeof(char) * 6);
    selectColor(color, record->stringColor);
    record->eatenApples = eatenApples;
    return record;
}

void saveRecords(snake_t *snake, uint16_t color, record_t **records) {
    FILE *fp = fopen("/tmp/nazar/resources/records/records.txt", "w");
    if (fp == NULL) {
        printf("Can't open file\n");
    }

    record_t *newRecord = setNewRecord(color, snake->length - 1);
    _Bool tmp = 1;
    for (size_t i = 0, j = 0; i < 5; i++) {
        if (newRecord->eatenApples > records[j]->eatenApples && tmp) {
            writeRecord(newRecord, color, fp, 1);
            tmp = 0;
        } else {
            writeRecord(records[j], color, fp, 0);
            j++;
        }
    }
    fclose(fp);
    freeRecords(records);
    free(newRecord->stringColor);
    free(newRecord);
}

void writeRecord(record_t *record, uint16_t color, FILE *fp, _Bool new) {
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    if (new) {
        fprintf(fp, "%s %d %d-%02d-%02d %02d:%02d\n", record->stringColor, record->eatenApples, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min);
    } else {
        fprintf(fp, "%s %s\n", record->stringColor, record->stringScore);
    }
}
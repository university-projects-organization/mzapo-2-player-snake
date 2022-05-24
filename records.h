#ifndef RECORDS_H
#define RECORDS_H

#include <stdio.h>
#include "snake.h"
#include "records_structure.h"

record_t **allocateRecords();

void freeOneRecord(record_t *record);

void freeRecords(record_t **records);

void selectColor(uint16_t color, char *stringColor);

void readLine(FILE *fp, record_t *record);

record_t **readRecords();

void displayOneRecord(record_t *record, union pixel **screen, uint16_t x, uint16_t y);

void recordsMenu(union pixel **screen, void *spiled_reg_base, unsigned char *parlcd_reg_base, uint8_t *actualG);

record_t *setNewRecord(uint16_t color, uint16_t eatenApples);

void writeRecord(record_t *record, uint16_t color, FILE *fp, _Bool);

void saveRecords(snake_t *snake, uint16_t color, record_t **records);

#endif
#ifndef WORD_FROM_FONT_H
#define WORD_FROM_FONT_H

#include <stdint.h>
#include "wordbuffer_structure.h"
#include "pixel_structure.h"

wordBuffer *allocateWordBuffer(uint16_t width, uint16_t height);

void freeWordBuffer(wordBuffer *wordBuffer);

uint16_t setSymbol(wordBuffer *wordBuffer, uint16_t bufferX, uint8_t scale, uint16_t symbolNumber);

wordBuffer *makeWordBuffer(char *word, uint8_t scale);

void setWord(wordBuffer *word, uint16_t wordColor, union pixel **screen, uint16_t screenX, uint16_t screenY);


#endif
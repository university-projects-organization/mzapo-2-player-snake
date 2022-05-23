#define ASCIISHIFT 32

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "apo_library/font_types.h"
#include "word_buffer.h"
#include "pixel.h"

wordBuffer *allocateWordBuffer(uint16_t width, uint16_t height) {
    wordBuffer *font = malloc(sizeof(wordBuffer));
    font->array = (uint16_t **) malloc(sizeof(uint16_t *) * height);
    for (size_t i = 0; i < height; ++i) {
        font->array[i] = (uint16_t *) malloc(sizeof(uint16_t) * width);
    }
    font->height = height;
    font->width = width;
    return font;
}

void freeWordBuffer(wordBuffer *wordBuffer) {
    for (size_t i = 0; i < wordBuffer->height; i++) {
        free(wordBuffer->array[i]);
    }
    free(wordBuffer->array);
    free(wordBuffer);
}


uint16_t setSymbol(wordBuffer *wordBuffer, uint16_t bufferX, uint8_t scale, uint16_t symbolNumber) {
    uint8_t symbolWidth = font_winFreeSystem14x16.width[symbolNumber];
    uint8_t symbolHeight = 16;
    uint8_t offset = 16 - symbolWidth;
    uint16_t symbolArrayPosition = symbolNumber * symbolHeight;

    for (size_t y = 0; y < symbolHeight * scale;) {
        uint16_t currentStr = font_winFreeSystem14x16.bits[symbolArrayPosition++] >> offset;
        uint16_t temp = currentStr;

        for (size_t j = 0; j < scale; j++) {
            uint16_t widthCopy = 1 << (symbolWidth - 1);
            for (size_t x = 0; x < symbolWidth * scale;) {
                for (size_t i = 0; i < scale; i++) {
                    wordBuffer->array[y][bufferX + x] = currentStr / widthCopy;
                    x++;
                }
                currentStr %= widthCopy;
                widthCopy /= 2;
            }
            currentStr = temp;
            y++;
        }
    }
    return bufferX + (symbolWidth * scale);
}

wordBuffer *makeWordBuffer(char *word, uint8_t scale) {
    uint16_t wordLen = strlen(word);
    uint8_t wordLetters[wordLen];

    uint16_t width = 0;
    uint16_t height = 16 * scale;

    for (size_t i = 0; i < wordLen; i++) {
        wordLetters[i] = (int) word[i] - ASCIISHIFT;
        width += font_winFreeSystem14x16.width[wordLetters[i]];
    }
    width *= scale;

    wordBuffer *wordBuffer = allocateWordBuffer(width, height);
    uint16_t bufferX = 0;
    for (size_t i = 0; i < wordLen; i++) {
        bufferX = setSymbol(wordBuffer, bufferX, scale, wordLetters[i]);
    }
    return wordBuffer;
}

void setWord(wordBuffer *word, uint16_t wordColor, union pixel **screen, uint16_t screenX, uint16_t screenY) {
    for (size_t y = 0; y < word->height; y++) {
        for (size_t x = 0; x < word->width; x++) {
            if (word->array[y][x] == 1) {
                screen[screenY + y][screenX + x].d = wordColor;
            }
        }
    }
}
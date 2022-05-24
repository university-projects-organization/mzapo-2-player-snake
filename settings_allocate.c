#ifndef KNOBS_CONTROL_H
#define KNOBS_CONTROL_H

#include <stdlib.h>
#include <stdint.h>
#include "settings_structure.h"
#include "word_from_font.h"

settingsParameter **allocateSettingsParameter(uint8_t *settings) {
    settingsParameter **settingsParameters = malloc(sizeof(settingsParameter *) * 7);
    for (size_t i = 0; i < 7; ++i) {
        settingsParameters[i] = (settingsParameter *) malloc(sizeof(settingsParameter));
    }

    // 1 snake color
    settingsParameters[0]->string = malloc(sizeof(wordBuffer *) * 4);
    settingsParameters[0]->position = settings[0];
    settingsParameters[0]->string[0] = makeWordBuffer("BLACK", 2);
    settingsParameters[0]->string[1] = makeWordBuffer("RED", 2);
    settingsParameters[0]->string[2] = makeWordBuffer("GREEN", 2);
    settingsParameters[0]->string[3] = makeWordBuffer("BLUE", 2);

    // 2 snake color
    settingsParameters[1]->string = malloc(sizeof(wordBuffer *) * 4);
    settingsParameters[1]->position = settings[1];
    settingsParameters[1]->string[0] = makeWordBuffer("BLACK", 2);
    settingsParameters[1]->string[1] = makeWordBuffer("RED", 2);
    settingsParameters[1]->string[2] = makeWordBuffer("GREEN", 2);
    settingsParameters[1]->string[3] = makeWordBuffer("BLUE", 2);

    // food owner
    settingsParameters[2]->string = malloc(sizeof(wordBuffer *) * 2);
    settingsParameters[2]->position = settings[2];
    settingsParameters[2]->string[0] = makeWordBuffer("MUTUAL", 2);
    settingsParameters[2]->string[1] = makeWordBuffer("OWN", 2);

    // food amount
    settingsParameters[3]->string = malloc(sizeof(wordBuffer *) * 2);
    settingsParameters[3]->position = settings[3];
    settingsParameters[3]->string[0] = makeWordBuffer("LOT", 2);
    settingsParameters[3]->string[1] = makeWordBuffer("LACK", 2);

    // Collisions
    settingsParameters[4]->string = malloc(sizeof(wordBuffer *) * 2);
    settingsParameters[4]->position = settings[4];
    settingsParameters[4]->string[0] = makeWordBuffer("OFF", 2);
    settingsParameters[4]->string[1] = makeWordBuffer("ON", 2);

    // Speed
    settingsParameters[5]->string = malloc(sizeof(wordBuffer *) * 3);
    settingsParameters[5]->position = settings[5];
    settingsParameters[5]->string[0] = makeWordBuffer("MEDIUM", 2);
    settingsParameters[5]->string[1] = makeWordBuffer("FAST", 2);
    settingsParameters[5]->string[2] = makeWordBuffer("SLOW", 2);

    // Boost
    settingsParameters[6]->string = malloc(sizeof(wordBuffer *) * 3);
    settingsParameters[6]->position = settings[6];
    settingsParameters[6]->string[0] = makeWordBuffer("MEDIUM", 2);
    settingsParameters[6]->string[1] = makeWordBuffer("FAST", 2);
    settingsParameters[6]->string[2] = makeWordBuffer("SLOW", 2);

    return settingsParameters;
}

void freeSettingsParameter(settingsParameter **settingsParameters) {
    freeWordBuffer(settingsParameters[0]->string[0]);
    freeWordBuffer(settingsParameters[0]->string[1]);
    freeWordBuffer(settingsParameters[0]->string[2]);
    freeWordBuffer(settingsParameters[0]->string[3]);
    free(settingsParameters[0]->string);
    free(settingsParameters[0]);

    freeWordBuffer(settingsParameters[1]->string[0]);
    freeWordBuffer(settingsParameters[1]->string[1]);
    freeWordBuffer(settingsParameters[1]->string[2]);
    freeWordBuffer(settingsParameters[1]->string[3]);
    free(settingsParameters[1]->string);
    free(settingsParameters[1]);

    freeWordBuffer(settingsParameters[2]->string[0]);
    freeWordBuffer(settingsParameters[2]->string[1]);
    free(settingsParameters[2]->string);
    free(settingsParameters[2]);

    freeWordBuffer(settingsParameters[3]->string[0]);
    freeWordBuffer(settingsParameters[3]->string[1]);
    free(settingsParameters[3]->string);
    free(settingsParameters[3]);

    freeWordBuffer(settingsParameters[4]->string[0]);
    freeWordBuffer(settingsParameters[4]->string[1]);
    free(settingsParameters[4]->string);
    free(settingsParameters[4]);

    freeWordBuffer(settingsParameters[5]->string[0]);
    freeWordBuffer(settingsParameters[5]->string[1]);
    freeWordBuffer(settingsParameters[5]->string[2]);
    free(settingsParameters[5]->string);
    free(settingsParameters[5]);

    freeWordBuffer(settingsParameters[6]->string[0]);
    freeWordBuffer(settingsParameters[6]->string[1]);
    freeWordBuffer(settingsParameters[6]->string[2]);
    free(settingsParameters[6]->string);
    free(settingsParameters[6]);

    free(settingsParameters);
}

#endif
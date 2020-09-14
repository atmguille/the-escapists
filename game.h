
#ifndef GAME_H
#define GAME_H

#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <string.h>
#include "util.h"
#include "image.h"
#include "player.h"
#include "entity.h"
#include "strprint.h"
#include "map.h"
#include "minigame.h"
#include "sound.h"

typedef struct {
    struct termios backup;
    Player *player;
    Entity objects[MAX_OBJECTS];
    unsigned short nObjects;
    Entity characters[MAX_CHARACTERS];
    unsigned short nCharacters;
    Map *map;
    Minigame minigames[MAX_MINIGAMES];
    unsigned short nMinigames;
    char lastMapName[MAP_NAME_LEN];
    int pid; /* So we can stop a long sound */
} Game;


Status game_start();
void game_stop(Game *game);
void game_get_input(Game *game);

#endif

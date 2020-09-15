#ifndef MINIGAME_H
#define MINIGAME_H

#include "util.h"
#include "player.h"
#include "image.h"
#include "strprint.h"
#include "map.h"
#include "sound.h"

#define MAX_MINIGAMES 2

typedef struct {
    char pitcherName[ENTITY_NAME_LEN];
    /* Initial position of the pitcher */
    unsigned short pitcherX;
    unsigned short pitcherY;
    /* Y range of the pitcher */
    unsigned short pitcherMin;
    unsigned short pitcherMax;
    char objectName[ENTITY_NAME_LEN];
    char map_name[MAP_NAME_LEN];
    char nextMap[MAP_NAME_LEN];
    unsigned short nThreads;
    long objectSpeed;
    /* If the pitcher is dead, we won't load the minigame */
    bool isPitcherDead;
} Minigame;

void minigame_launch(Image *background, Player *player, Minigame *minigame);
void minigame_destroy();
bool minigame_isPlayerDead();

#endif
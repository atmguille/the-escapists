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
    char pitcher_name[ENTITY_NAME_LEN];
    /* Initial position of the pitcher */
    unsigned short pitcher_x;
    unsigned short pitcher_y;
    /* Y range of the pitcher */
    unsigned short pitcher_min_y;
    unsigned short pitcher_max_y;
    char object_name[ENTITY_NAME_LEN];
    char map_name[MAP_NAME_LEN];
    char next_map[MAP_NAME_LEN];
    unsigned short num_threads;
    long object_speed;
    /* If the pitcher is dead, we won't load the minigame */
    bool is_pitcher_dead;
} Minigame;

void minigame_launch(Image *background, Player *player, Minigame *minigame);
void minigame_destroy();
bool minigame_is_player_dead();

#endif

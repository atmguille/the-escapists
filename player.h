#ifndef PLAYER_H
#define PLAYER_H

#include "image.h"

typedef struct {
    Image *right;
    Image *left;
    Image *backwards;
    Image *current;       /* We don't allocate memory for this one */
    unsigned short posX;
    unsigned short posY;
} Player;

Player *player_ini();
void player_turn_left(Player *player);
void player_turn_right(Player *player);
void player_turn_back(Player *player);
void player_turn_front(Player *player);
void player_enable_police_mode(Player *player);
void player_free(Player *player);

#endif

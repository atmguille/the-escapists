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
void player_turnLeft(Player *player);
void player_turnRight(Player *player);
void player_turnBack(Player *player);
void player_turnFront(Player *player);
void player_enablePoliceMode(Player *player);
void player_free(Player *player);

#endif

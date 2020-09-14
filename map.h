#ifndef MAP_H
#define MAP_H

#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include "util.h"
#include "image.h"
#include "player.h"
#include "entity.h"
#include "strprint.h"

/* Structure to know to which maps we can go from the current map */
typedef struct {
    char name[MAP_NAME_LEN];
    RGB colorCode;
    /* Future position of the player in the new map */
    unsigned short newX;
    unsigned short newY;
    /* If the pin is 0, there's no pin */
    int pin;
    /* If we need an object (or to kill someone), we'll print a sentence right at the door, and if we do that, we won't be able to change maps */
} ContiguousMap;

typedef struct {
    char string[MSG_LEN];
    char entityName[ENTITY_NAME_LEN];
    RGB colorCode;
} Sentence;

typedef struct {
    Image *background;
    Image *limits;                 /* B&W map so the player cannot go through walls
                                    * exit and places where something is said have a color code */
    char name[MAP_NAME_LEN];
    Entity *objects;               /* Reference to the array of objects (saved in game) */
    unsigned short nObjects;
    Entity *characters;            /* Reference to the array of characters (saved in game) */
    unsigned short nCharacters;                
    ContiguousMap *contiguousMaps; /* Maps accessibles from here */
    unsigned short nContigousMaps;
    char initialSentence[MSG_LEN];
    Sentence *sentences;
    unsigned short nSentences;
} Map;

/* Name of the map: knowing that we can load the background...
 * Player: we need to update the position
 * Objects and characters: we need to print them */
Map *map_ini(const char *name, Player *player, Entity *objects, unsigned short nObjects, Entity *characters, unsigned short nCharacters, short newX, short newY);
void map_free(Map *map);
void map_print(Map *map);
void print_map_objects(Map *map);
void print_map_characters(Map *map);
Bool player_has(Map *map, char *entityName);

/* Function to see if we can move to one of the contiguous maps, returning its name if so, or NULL if not */
ContiguousMap *map_getContiguousMap(int x, int y, Map *map);
/* Function that returns the sentence of that position or NULL if there is none */
char *map_getSentence(int x, int y, Map *map);

#endif
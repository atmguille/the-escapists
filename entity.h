#ifndef ENTITY_H
#define ENTITY_H

#define MAX_OBJECTS 20
#define MAX_CHARACTERS 10

#include "image.h"
#include "util.h"

/* Objects and Characters are Entities. With inheritance this would be nicer :( */
typedef struct {
    Image *image;
    int posX;
    int posY;
    char mapName[MAP_NAME_LEN];
    char name[ENTITY_NAME_LEN];
    char description[MSG_LEN]; /* Description of the object or greeting of the character */
    Bool mBool;                /* Check if the user have an object, or if a character is dead 
                                * This is done like this because of the constrints when we want to change maps
                                * If it is TRUE, (we have an object or a character is dead), we can change maps */
} Entity;

#endif
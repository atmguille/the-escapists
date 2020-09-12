#include "map.h"

Map* map_ini(const char* name, Player* player, Entity* objects, unsigned short nObjects, Entity* characters, unsigned short nCharacters, short newX, short newY) {
    FILE* f;
    Map* map;
    int i;
    int j = 0;
    unsigned short dummy;
    char dummychar;
    char path[MAP_NAME_LEN + 21];

    if (name == NULL || player == NULL)
        return NULL;

    map = (Map *)malloc(sizeof(Map));
    if (map == NULL)
        return NULL;

    sprintf(path, "Maps/%s/background.bmp", name);
    map->background = image_ini(path);
    if (map->background == NULL) {
        free(map);
        free(map);
        return NULL;
    }
    assert(map->background->heigth == BACKGROUND_HEIGTH);
    assert(map->background->width == BACKGROUND_WIDTH);
    
    sprintf(path, "Maps/%s/limits.bmp", name);
    map->limits = image_ini(path);
    if (map->limits == NULL) {
        image_free(map->background);
        free(map);
        return NULL;
    }
    assert(map->limits->heigth == BACKGROUND_HEIGTH);
    assert(map->limits->width == BACKGROUND_WIDTH);

    sprintf(path, "Maps/%s/info.txt", name);
    f = fopen(path, "r");
    if (f == NULL) {
        image_free(map->background);
        image_free(map->limits);
        free(map);
        return NULL;
    }
    /* Update player's position */
    /* If we just started the game, we use the default position of the map */
    if (newX == DEFAULT_INITIAL_POS && newY == DEFAULT_INITIAL_POS)
        fscanf(f, "%hd %hd", &player->posX, &player->posY);
    /* If we come from another map, we use the provided values */
    else {
        fscanf(f, "%hd %hd", &dummy, &dummy);
        player->posX = newX;
        player->posY = newY;
    }

    /* Read the maps accessibles from this one */
    fscanf(f, "%hd", &map->nContigousMaps);

    map->contiguousMaps = (ContiguousMap *)malloc(map->nContigousMaps * sizeof(ContiguousMap));
    if (map->contiguousMaps == NULL) {
        fclose(f);
        image_free(map->background);
        image_free(map->limits);
        free(map);
        return NULL;
    }

    /* As we are going to read more from the file, we have to keep track of the number of fields read */
    i = 0;
    while (i < map->nContigousMaps && fscanf(f, "%s %hd %hd %hd %hd %hd %d", map->contiguousMaps[i].name, 
      &(map->contiguousMaps[i].colorCode.red), &(map->contiguousMaps[i].colorCode.green), &(map->contiguousMaps[i].colorCode.blue),
      &(map->contiguousMaps[i].newX), &(map->contiguousMaps[i].newY), 
      &(map->contiguousMaps[i].pin)) > 0) {
        i++;
    }

    /* Scan initial sentence. The dummychar reads the \n, needed to do the following:
     * It reads until a " is found. We need that to be able to read the whole sentence, including spaces
     * TODO: parece un poco cutre, pero funciona... */
    fscanf(f, "%c%[^\"]%*c", &dummychar, map->initialSentence);

    /* Scan sentences to print */
    fscanf(f, "%hd", &map->nSentences);

    if (map->nSentences > 0) {
        map->sentences = (Sentence *)malloc(map->nSentences * sizeof(Sentence));
        if (map->sentences == NULL) {
            fclose(f);
            image_free(map->background);
            image_free(map->limits);
            free(map->contiguousMaps);
            free(map);
            return NULL;
        }

        /* Since we are not going to read more from the file it is no neccessary to keep track of the
        * number of fields read. We check it anyway just in case we want to read more in a future */
        i = 0;
        while (i < map->nSentences && fscanf(f, "%c %[^\"]%*c %hd %hd %hd %s", &dummychar, map->sentences[i].string, 
            &(map->sentences[i].colorCode.red), &(map->sentences[i].colorCode.green), &(map->sentences[i].colorCode.blue), 
            map->sentences[i].entityName) > 0) {
                i++;
        }
    }
    
    fclose(f);

    /* We need the name of the map for the objects */
    strcpy(map->name, name);

    map->objects = objects;
    map->nObjects = nObjects;

    map->characters = characters;
    map->nCharacters = nCharacters;

    map_print(map);
    print_map_objects(map);
    print_map_characters(map);
    /* Prints player */
    image_print_transparent(player->current, map->background, player->posX, player->posY);
    strprint_time(map->initialSentence, PRINT_TIME);

    return map;
}

ContiguousMap* map_getContiguousMap(int x, int y, Map* map) {
    /* We are going to return a copy, so we can destroy the old game securely */
    char* name;
    int i;
    unsigned short r, g, b;
    ContiguousMap* mapcpy;    

    r = map->limits->rgb[y][x].red;
    g = map->limits->rgb[y][x].green;
    b = map->limits->rgb[y][x].blue;

    for (i = 0; i < map->nContigousMaps; i++) {
        if (map->contiguousMaps[i].colorCode.red == r && map->contiguousMaps[i].colorCode.green == g && map->contiguousMaps[i].colorCode.blue == b) {
            return &(map->contiguousMaps[i]);
        }
    }

    return NULL;
}

/* Returns True if the player has the object of that name */
Bool player_has(Map* map, char* entityName) {
    int i;

    for (i = 0; i < map->nObjects; i++) {
        if (strcmp(map->objects[i].name, entityName) == 0)
            return map->objects[i].mBool;
    }

    for (i = 0; i < map->nCharacters; i++) {
        if (strcmp(map->characters[i].name, entityName) == 0)
            return map->characters[i].mBool;
    }

    return FALSE;
}

char* map_getSentence(int x, int y, Map* map) {
    int i;
    unsigned short r, g, b;

    r = map->limits->rgb[y][x].red;
    g = map->limits->rgb[y][x].green;
    b = map->limits->rgb[y][x].blue;

    for (i = 0; i < map->nSentences; i++) {
        if (map->sentences[i].colorCode.red == r && map->sentences[i].colorCode.green == g && map->sentences[i].colorCode.blue == b)
            if (!(player_has(map, map->sentences[i].entityName)))
                return map->sentences[i].string;
    }

    return NULL;
}

void map_free(Map* map) {
    if (map != NULL) {
        image_free(map->background);
        image_free(map->limits);
        free(map->contiguousMaps);
        if (map->nSentences > 0)
            free(map->sentences);
        free(map);
    }
}

void map_print(Map* map) {
    image_print(map->background, 0, 0);
}

void print_map_objects(Map* map) {
    int i;

    for (i = 0; i < map->nObjects; i++) {
        if (map->objects[i].mBool == FALSE && strcmp(map->objects[i].mapName, map->name) == 0) {
            image_print_transparent(map->objects[i].image, map->background, map->objects[i].posX, map->objects[i].posY);
        }
    }
}

void print_map_characters(Map* map) {
    int i;

    for (i = 0; i < map->nCharacters; i++) {
        if (map->characters[i].mBool == FALSE && strcmp(map->characters[i].mapName, map->name) == 0) {
            image_print_transparent(map->characters[i].image, map->background, map->characters[i].posX, map->characters[i].posY);
        }
    }
}

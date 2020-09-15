#include "map.h"

Map *map_ini(const char *name, Player *player, Entity *objects, unsigned short num_objects, Entity *characters, unsigned short num_characters, short new_x, short new_y) {
    FILE *f;
    Map *map;
    int i;
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
    assert(map->background->heigth == BACKGROUND_HEIGHT);
    assert(map->background->width == BACKGROUND_WIDTH);
    
    sprintf(path, "Maps/%s/limits.bmp", name);
    map->limits = image_ini(path);
    if (map->limits == NULL) {
        image_free(map->background);
        free(map);
        return NULL;
    }
    assert(map->limits->heigth == BACKGROUND_HEIGHT);
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
    if (new_x == DEFAULT_INITIAL_POS && new_y == DEFAULT_INITIAL_POS)
        fscanf(f, "%hd %hd", &player->posX, &player->posY);
    /* If we come from another map, we use the provided values */
    else {
        fscanf(f, "%hd %hd", &dummy, &dummy);
        player->posX = new_x;
        player->posY = new_y;
    }

    /* Read the maps accessibles from this one */
    fscanf(f, "%hd", &map->num_contigous_maps);

    map->contiguous_maps = (ContiguousMap *)malloc(map->num_contigous_maps * sizeof(ContiguousMap));
    if (map->contiguous_maps == NULL) {
        fclose(f);
        image_free(map->background);
        image_free(map->limits);
        free(map);
        return NULL;
    }

    /* As we are going to read more from the file, we have to keep track of the number of fields read */
    i = 0;
    while (i < map->num_contigous_maps && fscanf(f, "%s %hd %hd %hd %hd %hd %d", map->contiguous_maps[i].name, 
      &(map->contiguous_maps[i].color_code.red), &(map->contiguous_maps[i].color_code.green), &(map->contiguous_maps[i].color_code.blue),
      &(map->contiguous_maps[i].new_x), &(map->contiguous_maps[i].new_y), 
      &(map->contiguous_maps[i].pin)) > 0) {
        i++;
    }

    /* Scan initial sentence. The dummychar reads the \n, needed to do the following:
     * It reads until a " is found. We need that to be able to read the whole sentence, including spaces
     * TODO: parece un poco cutre, pero funciona... */
    fscanf(f, "%c%[^\"]%*c", &dummychar, map->initial_sentence);

    /* Scan sentences to print */
    fscanf(f, "%hd", &map->num_sentences);

    if (map->num_sentences > 0) {
        map->sentences = (Sentence *)malloc(map->num_sentences * sizeof(Sentence));
        if (map->sentences == NULL) {
            fclose(f);
            image_free(map->background);
            image_free(map->limits);
            free(map->contiguous_maps);
            free(map);
            return NULL;
        }

        /* Since we are not going to read more from the file it is no neccessary to keep track of the
        * number of fields read. We check it anyway just in case we want to read more in a future */
        i = 0;
        while (i < map->num_sentences && fscanf(f, "%c %[^\"]%*c %hd %hd %hd %s", &dummychar, map->sentences[i].string, 
            &(map->sentences[i].color_code.red), &(map->sentences[i].color_code.green), &(map->sentences[i].color_code.blue), 
            map->sentences[i].entity_name) > 0) {
                i++;
        }
    }
    
    fclose(f);

    /* We need the name of the map for the objects */
    strcpy(map->name, name);

    map->objects = objects;
    map->num_objects = num_objects;

    map->characters = characters;
    map->num_characters = num_characters;

    map_print(map);
    print_map_objects(map);
    print_map_characters(map);
    /* Prints player */
    image_print_transparent(player->current, map->background, player->posX, player->posY);
    strprint_time(map->initial_sentence, PRINT_TIME);

    return map;
}

ContiguousMap *map_get_contiguous_map(int x, int y, Map *map) {
    /* We are going to return a copy, so we can destroy the old game securely */
    int i;
    unsigned short r, g, b;

    r = map->limits->rgb[y][x].red;
    g = map->limits->rgb[y][x].green;
    b = map->limits->rgb[y][x].blue;

    for (i = 0; i < map->num_contigous_maps; i++) {
        if (map->contiguous_maps[i].color_code.red == r && map->contiguous_maps[i].color_code.green == g && map->contiguous_maps[i].color_code.blue == b) {
            return &(map->contiguous_maps[i]);
        }
    }

    return NULL;
}

/* Returns True if the player has the object of that name */
bool player_has(Map *map, char *entity_name) {
    int i;

    for (i = 0; i < map->num_objects; i++) {
        if (strcmp(map->objects[i].name, entity_name) == 0)
            return map->objects[i].mbool;
    }

    for (i = 0; i < map->num_characters; i++) {
        if (strcmp(map->characters[i].name, entity_name) == 0)
            return map->characters[i].mbool;
    }

    return false;
}

char *map_get_sentence(int x, int y, Map *map) {
    int i;
    unsigned short r, g, b;

    r = map->limits->rgb[y][x].red;
    g = map->limits->rgb[y][x].green;
    b = map->limits->rgb[y][x].blue;

    for (i = 0; i < map->num_sentences; i++) {
        if (map->sentences[i].color_code.red == r && map->sentences[i].color_code.green == g && map->sentences[i].color_code.blue == b)
            if (!(player_has(map, map->sentences[i].entity_name)))
                return map->sentences[i].string;
    }

    return NULL;
}

void map_free(Map *map) {
    if (map != NULL) {
        image_free(map->background);
        image_free(map->limits);
        free(map->contiguous_maps);
        if (map->num_sentences > 0)
            free(map->sentences);
        free(map);
    }
}

void map_print(Map *map) {
    image_print(map->background, 0, 0);
}

void print_map_objects(Map *map) {
    int i;

    for (i = 0; i < map->num_objects; i++) {
        if (map->objects[i].mbool == false && strcmp(map->objects[i].map_name, map->name) == 0) {
            image_print_transparent(map->objects[i].image, map->background, map->objects[i].posX, map->objects[i].posY);
        }
    }
}

void print_map_characters(Map *map) {
    int i;

    for (i = 0; i < map->num_characters; i++) {
        if (map->characters[i].mbool == false && strcmp(map->characters[i].map_name, map->name) == 0) {
            image_print_transparent(map->characters[i].image, map->background, map->characters[i].posX, map->characters[i].posY);
        }
    }
}

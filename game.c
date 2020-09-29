#include "game.h"

#define DEFAULT_MAP_FILE "Maps/default.txt"
#define OBJECTS_FILE "Objects/objects.txt"
#define CHARACTERS_FILE "Characters/characters.txt"
#define PLAYERFILE "Player/right.bmp"
#define MINIGAME_FILE "Minigame/minigame.txt"
#define COVER_PATH "Miscellaneous/Cover.bmp"
#define PIN_PATH "Miscellaneous/PIN.bmp"

typedef enum {UP, DOWN, RIGHT, LEFT} MOVEMENTS;

/* Prints the initial cover. The bool beginning indicates which string to print: beggining or end
 * The return bool indicates the calling function whether we want to exit or not */
Bool _print_cover(Game* game, Bool beginning) {
    char c;
    Image* cover = image_ini(COVER_PATH);

    if (cover == NULL) {
        /* If there's no memory for the cover, the game will exit */
        return TRUE;
    }
    image_print(cover, 0, 0);
    image_free(cover);

    if (beginning == TRUE) {
        strprint("Welcome!\nPlease, press any key to start playing");
        game->pid = play_sound(BEGINNING_PATH);
    }
    else
        strprint("Thanks for playing!\nWe hope you enjoyed. Press any key to exit");
    c = getchar();
    /* We kill the sound */
    stop_sound(game->pid);
    /* See game_get_input */
    /* We only care if the user hits the Q if it's the beginning */
    if ((c == 'q' || c == 'Q' || c == 3) && (beginning == TRUE)) {
        return TRUE;
    }

    return FALSE;
}

void restore_terminal(Game* game) {
    /* Reset color */
	printf(ANSI_COLOR_RESET);
    /* Show cursor */
    printf(SHOW_CURSOR);
    /* Clear screen */
    printf(CLEAR_SCREEN);
    /* Restore terminal */
    tcsetattr(fileno(stdin), TCSANOW, &(game->backup));
}

Status game_start() {
    Game* game;
    struct termios new;
    FILE* f;
    int i;
    char imagePath[ENTITY_NAME_LEN + 16]; /* 16 is the maximum length of the path */
    char defaultMapName[MAP_NAME_LEN];
    
    /* Initialize semaphore */
    pthread_mutex_init(&semaphore, NULL);

    game = (Game *)malloc(sizeof(Game));
    if (game == NULL)
        return ERROR;

    game->pid = -1;

    /* Backups current terminal coniguration */
    tcgetattr(fileno(stdin), &(game->backup));
    new = game->backup;
    /* Local modes */
    new.c_lflag &= ~ICANON;  /* Line editing mode */
    new.c_lflag &= ~ECHO;    /* Don't print stdin */
    new.c_lflag &= ~ISIG;    /* Ctr + C disabled  */
    /* Control chars */
    new.c_cc[VMIN] = 1;
   	new.c_cc[VTIME] = 0;
    tcsetattr(fileno(stdin), TCSANOW, &(new));

    /* Clear screen */
    printf(CLEAR_SCREEN);
    /* Hide cursor */
    printf(HIDE_CURSOR);

    /* Prints the cover of the game (and exit if the user wants so) */
    if (_print_cover(game, TRUE) == TRUE) {
        restore_terminal(game);
        free(game);
        return OK;
    }

    game->player = player_ini();
    if (game->player == NULL) {
        restore_terminal(game);
        free(game);
        return ERROR;
    }

    f = fopen(OBJECTS_FILE, "r");
    if (f == NULL) {
        player_free(game->player);
        restore_terminal(game);
        free(game);
        return ERROR;
    }

    fscanf(f, "%hd", &game->nObjects);
    assert(game->nObjects <= MAX_OBJECTS);

    for (i = 0; i < game->nObjects; i++) {
        fscanf(f, "%s %s %d %d %d %[^\"]%*c", game->objects[i].name, game->objects[i].mapName, &game->objects[i].posX, &game->objects[i].posY, &game->objects[i].mBool, game->objects[i].description);
        sprintf(imagePath, "Objects/%s.bmp", game->objects[i].name);
        game->objects[i].image = image_ini(imagePath);
        if (game->objects[i].image == NULL) {
            for (i = i - 1; i >= 0; i--) {
                image_free(game->objects[i].image);
            }
            player_free(game->player);
            fclose(f);
            restore_terminal(game);
            free(game);
            return ERROR;
        }
    }
    fclose(f);

    f = fopen(CHARACTERS_FILE, "r");
    if (f == NULL) {
        player_free(game->player);
        for (i = 0; i < game->nObjects; i++) {
            image_free(game->objects[i].image);
        }
        restore_terminal(game);
        free(game);
        return ERROR;
    }

    fscanf(f, "%hd", &game->nCharacters);
    assert(game->nCharacters <= MAX_CHARACTERS);

    for (i = 0; i < game->nCharacters; i++) {
        fscanf(f, "%s %s %d %d %d %[^\"]%*c", game->characters[i].name, game->characters[i].mapName, &game->characters[i].posX, &game->characters[i].posY, &game->characters[i].mBool, game->characters[i].description);
        sprintf(imagePath, "Characters/%s.bmp", game->characters[i].name);
        game->characters[i].image = image_ini(imagePath);
        if (game->characters[i].image == NULL) {
            for (i = i - 1; i >= 0; i--) {
                image_free(game->characters[i].image);
            }
            for (i = 0; i < game->nObjects; i++) {
                image_free(game->objects[i].image);
            }
            player_free(game->player);
            fclose(f);
            restore_terminal(game);
            free(game);
            return ERROR;
        }
    }
    fclose(f);

    /* Read information about minigames */
    f = fopen(MINIGAME_FILE, "r");
    if (f == NULL) {
        player_free(game->player);
        for (i = 0; i < game->nObjects; i++) {
            image_free(game->objects[i].image);
        }
        for (i = 0; i < game->nCharacters; i++) {
            image_free(game->characters[i].image);
        }
        restore_terminal(game);
        free(game);
        return ERROR;
    }
    fscanf(f, "%hd", &game->nMinigames);
    assert(game->nMinigames <= MAX_MINIGAMES);
    for (i = 0; i < game->nMinigames; i++) {
        fscanf(f, "%s %hd %hd %hd %hd %s %s %s %hd %ld %d", game->minigames[i].pitcherName, &game->minigames[i].pitcherX, &game->minigames[i].pitcherY, &game->minigames[i].pitcherMax, &game->minigames[i].pitcherMin, game->minigames[i].objectName, game->minigames[i].mapName, game->minigames[i].nextMap, &game->minigames[i].nThreads, &game->minigames[i].objectSpeed, &game->minigames[i].isPitcherDead);
    }
    fclose(f);

    f = fopen(DEFAULT_MAP_FILE, "r");
    if (f == NULL) {
        player_free(game->player);
        for (i = 0; i < game->nObjects; i++) {
            image_free(game->objects[i].image);
        }
        for (i = 0; i < game->nCharacters; i++) {
            image_free(game->characters[i].image);
        }
        restore_terminal(game);
        free(game);
        return ERROR;
    }
    fscanf(f, "%s", defaultMapName);
    fscanf(f, "%s", game->lastMapName);
    fclose(f);
    /* map_ini prints the map (that's why we have to change the terminal's settings first) */
    if (strcmp(game->lastMapName, defaultMapName) == 0)
        game->pid = play_sound(LASTSONG_PATH);
    game->map = map_ini(defaultMapName, game->player, game->objects, game->nObjects, game->characters, game->nCharacters, DEFAULT_INITIAL_POS, DEFAULT_INITIAL_POS);
    if (game->map == NULL) {
        player_free(game->player);
        for (i = 0; i < game->nObjects; i++) {
            image_free(game->objects[i].image);
        }
        for (i = 0; i < game->nCharacters; i++) {
            image_free(game->characters[i].image);
        }
        restore_terminal(game);                
        free(game);
        return ERROR;
    }

    for (i = 0; i < game->nMinigames; i++) {
        if (strcmp(game->map->name, game->minigames[i].mapName) == 0) {
            minigame_launch(game->map->background, game->player, &(game->minigames[i]));
        }
    }      

    /* Start reading keystrokes */
    game_get_input(game);

    /* Free resouces */
    game_stop(game);

    return OK;
}

void game_stop(Game* game) {
    int i;
    if (game != NULL) {
        /* Free allocated resources */
        for (i = 0; i < game->nMinigames; i++) {
            if (strcmp(game->map->name, game->minigames[i].mapName) == 0) {
                minigame_destroy();
                break;
            }
        }
        /* The user evidently wants to exit, we don't check the result of the function */
        _print_cover(game, FALSE);
        map_free(game->map);
        player_free(game->player);
        for (i = 0; i < game->nObjects; i++) {
            image_free(game->objects[i].image);
        }
        for (i = 0; i < game->nCharacters; i++) {
            image_free(game->characters[i].image);
        }
        stop_sound(game->pid);
        restore_terminal(game);
        free(game);
    }
}

void _print_player(Game* game) {    
    image_print_transparent(game->player->current, game->map->background, game->player->posX, game->player->posY);
}


/* Returns True if player is close enough to catch an object */
Bool _is_close(Player* player, Entity* entity, int d) {
    /* Player's top left corner */
    int x1 = player->posX;
    int y1 = player->posY;
    /* Player's bottom right corner */
    int x1b = player->posX + player->right->width;
    int y1b = player->posY + player->right->heigth;

    /* Object's top left corner */
    int x2 = entity->posX;
    int y2 = entity->posY;
    /* Object's bottom right corner */    
    int x2b = entity->posX + entity->image->width;
    int y2b = entity->posY + entity->image->heigth;                                       
    
    Bool left = x2b < x1;
    Bool right = x1b < x2;
    Bool bottom = y2b < y1;
    Bool top = y1b < y2;

    if (top && left) {
        return distance(x1, y1b, x2b, y2) < (d * d);
    } else if (left && bottom) {
        return distance(x1, y1, x2b, y2b) < (d * d);
    } else if (bottom && right) {
        return distance(x1b, y1, x2, y2b) < (d * d);
    } else if (right && top) {
        return distance(x1b, y1b, x2, y2) < (d * d);
    } else if (left) {
        return (x1 - x2b) < d;
    } else if (right) {
        return (x2 - x1b) < d;
    } else if (bottom) {
        return (y1 - y2b) < d;
    } else if (top) {
        return (y2 - y1b) < d;
    } else {          /* rectangles intersect */
        return TRUE;                
    }
}

void _obj_catch(Game* game) {
    char msg[MSG_LEN];
    int i, j, pos;
    
    for (pos = 0; pos < game->nObjects; pos++) {
        if (game->objects[pos].mBool == FALSE && strcmp(game->map->name, game->objects[pos].mapName) == 0 && _is_close(game->player, &(game->objects[pos]), 1)) {
            play_sound(OBJECTCATCH_PATH);
            sprintf(msg, "You caught a %s\n%s", game->objects[pos].name, game->objects[pos].description);
            strprint_time(msg, PRINT_TIME);
            game->objects[pos].mBool = TRUE;
            pthread_mutex_lock(&semaphore);
            for (i = 0; i < game->objects[pos].image->heigth; i++) {
                moveCursorTo(game->objects[pos].posX, game->objects[pos].posY + i);
                for (j = 0; j < game->objects[pos].image->width; j++)
                    printf(COLOR_SPACE(game->map->background->rgb[game->objects[pos].posY + i][game->objects[pos].posX + j].red, game->map->background->rgb[game->objects[pos].posY + i][game->objects[pos].posX + j].green, game->map->background->rgb[game->objects[pos].posY + i][game->objects[pos].posX + j].blue));
            }
            pthread_mutex_unlock(&semaphore);
        }
    }
}

void _character_greeting(Game* game) {
    char msg[MSG_LEN];
    int pos;
    
    for (pos = 0; pos < game->nCharacters; pos++) {
        if (game->characters[pos].mBool == FALSE && strcmp(game->map->name, game->characters[pos].mapName) == 0 && _is_close(game->player, &(game->characters[pos]), 7)) {
            sprintf(msg, "%s: %s", game->characters[pos].name, game->characters[pos].description);
            strprint_time(msg, PRINT_TIME);
        }
    }
}

/* The Bool return is used to allow or not the player to go it that direction */
Bool _character_kill(Game* game) {
    char msg[MSG_LEN];
    int i, j, pos;
    
    for (pos = 0; pos < game->nCharacters; pos++) {
        if (game->characters[pos].mBool == FALSE && strcmp(game->map->name, game->characters[pos].mapName) == 0 && _is_close(game->player, &(game->characters[pos]), 1)) {
            if (strcmp(game->characters[pos].name, "Policeman") == 0 && player_has(game->map, "Knife")) {                                                        
                play_sound(KILL_PATH);
                sprintf(msg, "Great, you killed the %s", game->characters[pos].name);
                strprint_time(msg, PRINT_TIME);
                game->characters[pos].mBool = TRUE;
                pthread_mutex_lock(&semaphore);
                for (i = 0; i < game->characters[pos].image->heigth; i++) {
                    moveCursorTo(game->characters[pos].posX, game->characters[pos].posY + i);
                    for (j = 0; j < game->characters[pos].image->width; j++)
                        printf(COLOR_SPACE(game->map->background->rgb[game->characters[pos].posY + i][game->characters[pos].posX + j].red, game->map->background->rgb[game->characters[pos].posY + i][game->characters[pos].posX + j].green, game->map->background->rgb[game->characters[pos].posY + i][game->characters[pos].posX + j].blue));
                }
                pthread_mutex_unlock(&semaphore);
                player_enablePoliceMode(game->player);
                _print_player(game);
                return TRUE;
            } else {
                return FALSE;
            }
        }
    }
    return TRUE;
}


Bool _check_limits(int x, int y, Image* limits) {
    int red, blue, green;

	red = limits->rgb[y][x].red;
    green = limits->rgb[y][x].green;
    blue = limits->rgb[y][x].blue;

    if (red == 0 && green == 0 && blue == 0)
    	return TRUE;
    return FALSE;
}

/* We need the reference to the game to print the current map again if the pin is not correct */
Bool _checkPin(Game* game, int pin) {
    const unsigned int inputSize = 5;
    char input[inputSize];
    int i;
    Image* image = image_ini(PIN_PATH);
    if (image == NULL) {
        return FALSE;
    }

    cleanText();
    image_print(image, 0, 0);
    image_free(image);
    for (i = 0; i < inputSize - 1; i++) {
        input[i] = getchar();
        /* See get_input */
        if (input[i] == 'q' || input[i] == 'Q' || input[i] == 3) {
            /* We are going to exit from this screen, not from the game. The user will have to press q twice in that case */
            cleanText();
            map_print(game->map);
            print_map_objects(game->map);
            print_map_characters(game->map);
            _print_player(game);
            return FALSE;
        } else if (input[i] == 127) {
            /* 127 is the ASCII code for DEL key */
            i -= 2;
            if (i < -1)
                i = -1;
            play_sound(BACKSPACE_PATH);
        } else if (input[i] < '0' || input[i] > '9') {
            i--;
            continue;
        } else {
            /* Valid entry */
            play_sound(BEEP_PATH);
        }
        input[i + 1] = '\0';
        strprint_PIN(input);
    }

    if (atoi(input) == pin) {
        play_sound(ACCESSGRANTED_PATH);
        return TRUE;
    } else {
        play_sound(BUZZ_PATH);
        strprint_time("Wrong PIN", PRINT_TIME);
        map_print(game->map);
        print_map_objects(game->map);
        print_map_characters(game->map);
        _print_player(game);
        return FALSE;
    }

            
    return FALSE;
}

void _changeMap(Game* game, ContiguousMap* contiguousMap) {
    ContiguousMap* mapcpy;
    int i;

    if (contiguousMap->pin != 0) {
        if (_checkPin(game, contiguousMap->pin) == FALSE) {
            return;
        }
    }

    mapcpy = (ContiguousMap *)malloc(sizeof(ContiguousMap));
    if (mapcpy == NULL) {
        game_stop(game);
        /* Exit the game completely */
        exit(1);
    }

    strcpy(mapcpy->name, contiguousMap->name);
    mapcpy->newX = contiguousMap->newX;
    mapcpy->newY = contiguousMap->newY;
    
    for (i = 0; i < game->nMinigames; i++) {
        if (strcmp(game->map->name, game->minigames[i].mapName) == 0 && minigame_isPlayerDead() == FALSE) {
            minigame_destroy();
            /* There's only one minigame at the same time */
            break;
        }
    }

    map_free(game->map);
    if (strcmp(mapcpy->name, game->lastMapName) == 0)
        game->pid = play_sound(LASTSONG_PATH);
    game->map = map_ini(mapcpy->name, game->player, game->objects, game->nObjects, game->characters, game->nCharacters, mapcpy->newX, mapcpy->newY);
    free(mapcpy);
    if (game->map == NULL) {
        /* We can call game_stop because game->map is NULL */
        game_stop(game);
        /* Exit the game completely */
        exit(1);
    }
    for (i = 0; i < game->nMinigames; i++) {
        if (strcmp(game->map->name, game->minigames[i].mapName) == 0) {
            minigame_launch(game->map->background, game->player, &(game->minigames[i]));
        }
    }

    /* We have to print the player so when we go back to get_input the player gets printed */
    _print_player(game);
}

void _move(Game* game, MOVEMENTS mov) {
    const char movX[] = {0 , 0, 1, -1};
    const char movY[] = {-1, 1, 0,  0};
    int x1, y1, x2, y2;
    int newX1, newY1, newX2, newY2;
    int i;
    ContiguousMap* contiguousMap;
    Bool permission;

    if (game == NULL)         
        return;

    /* Left top corner */
    x1 = game->player->posX;
    y1 = game->player->posY;
    /* Right bottom corner */
    x2 = game->player->posX + game->player->right->width - 1;
    y2 = game->player->posY + game->player->right->heigth - 1;

    /* New left top corner */
    newX1 = game->player->posX + movX[mov];
    newY1 = game->player->posY + movY[mov];
    /* New right bottom corner */
    newX2 = newX1 + game->player->right->width - 1;
    newY2 = newY1 + game->player->right->heigth - 1;

    /* If a message is shown, we won't be able to change Maps or even move, that is, we cannot print sentences 
     * in arbitrary places, because we won't be able to move there */
    permission = (strprint(map_getSentence(newX1, newY1, game->map)) == ERROR &&
                  strprint(map_getSentence(newX2, newY1, game->map)) == ERROR &&
                  strprint(map_getSentence(newX1, newY2, game->map)) == ERROR &&
                  strprint(map_getSentence(newX2, newY2, game->map)) == ERROR);

    if (permission == FALSE)
        return;                        

    /* Check if we can move there */
    if (newX1 < 0 || newX2 < 0 || newY1 < 0 || newY2 < 0 || 
        newX1 >= game->map->background->width || newX2 >= game->map->background->width || newY1 >= game->map->background->heigth || newY2 >= game->map->background->heigth || 
        _check_limits(newX2, newY2, game->map->limits) || _check_limits(newX1, newY2, game->map->limits))
            return;

    /* Check if we have to move to one of the contiguous maps, checking first if we have permission */
    if ((contiguousMap = map_getContiguousMap(newX1, newY1, game->map)) != NULL) {
        _changeMap(game, contiguousMap);
        return;
    }  
    if ((contiguousMap = map_getContiguousMap(newX2, newY1, game->map)) != NULL) {
        _changeMap(game, contiguousMap);
        return;
    }  
    if ((contiguousMap = map_getContiguousMap(newX1, newY2, game->map)) != NULL) {
        _changeMap(game, contiguousMap);
        return;
    }  
    if ((contiguousMap = map_getContiguousMap(newX2, newY2, game->map)) != NULL) {
        _changeMap(game, contiguousMap);
        return;
    }
    /* Update players' position. If we don't finally move, we'll restore them, but this way it is easier to pass parameters
     * to the function character_kill */
    game->player->posX = newX1;
    game->player->posY = newY1;                
    /* Check if there are objects nearby */
    _obj_catch(game);
    /* If in the new coordinates there is a character, don't allow the player to move there */
    if (_character_kill(game) == FALSE) {
        game->player->posX = x1;
        game->player->posY = y1;
        return;
    }
    _character_greeting(game);
    
    /* Clear last position */
    pthread_mutex_lock(&semaphore);
    switch(mov) {
        case UP:
            player_turnBack(game->player);
            moveCursorTo(x1, y2);
            for (i = 0; i < game->player->right->width; i++) {
                printf(COLOR_SPACE(game->map->background->rgb[y2][x1 + i].red, game->map->background->rgb[y2][x1 + i].green, game->map->background->rgb[y2][x1 + i].blue));
            }
            break;
        case DOWN:
            player_turnFront(game->player);
            moveCursorTo(x1, y1);
            for (i = 0; i < game->player->right->width; i++) {
                printf(COLOR_SPACE(game->map->background->rgb[y1][x1 + i].red, game->map->background->rgb[y1][x1 + i].green, game->map->background->rgb[y1][x1 + i].blue));
            }
            break;
        case RIGHT:
            player_turnRight(game->player);
            for (i = 0; i < game->player->right->heigth; i++) {
                moveCursorTo(x1, y1 + i);
                printf(COLOR_SPACE(game->map->background->rgb[y1 + i][x1].red, game->map->background->rgb[y1 + i][x1].green, game->map->background->rgb[y1 + i][x1].blue));
            }
            break;
        case LEFT:
            player_turnLeft(game->player);
            for (i = 0; i < game->player->right->heigth; i++) {
                moveCursorTo(x2, y1 + i);
                printf(COLOR_SPACE(game->map->background->rgb[y1 + i][x2].red, game->map->background->rgb[y1 + i][x2].green, game->map->background->rgb[y1 + i][x2].blue));
            }
            break;
        default:
            /* We shouldn't be here. If we are, we'll exit, if we don't the semaphore will remain on forever */
            assert(0 == 1);
            return;
    }
    pthread_mutex_unlock(&semaphore);
    /* Print current position */
    _print_player(game);
}

void game_get_input(Game* game) {
    char c;
    int i;

    do {
        c = getchar();
        /* After hours trying to figure out how to know asynchronously when the player died because of and object thrown at him (minigame)
         * we gave up and went with this synchronous approach, where we ask the player to press a key. 
         * We check if c is 'q' so we don't load a map if we are going to destroy it afterwards */
        if (minigame_isPlayerDead() == TRUE && c != 'q' && c != 'Q' && c != 3) {
            for (i = 0; i < game->nMinigames; i++) {
                /* If we are in a minigame map, we change map */
                if (strcmp(game->map->name, game->minigames[i].mapName) == 0) {
                    ContiguousMap nextMap;
                    strcpy(nextMap.name, game->minigames[i].nextMap);
                    nextMap.newX = DEFAULT_INITIAL_POS;
                    nextMap.newY = DEFAULT_INITIAL_POS;
                    nextMap.pin  = 0; /* If we don't set the pin to 0, _changeMap will request one */
                    minigame_destroy();
                    _changeMap(game, &nextMap);
                    break;
                }
            }                
        }

        // Arrow keys use three ASCII CODES
        if (c == 27 && (c = getchar()) == 91) {
            c = getchar();
            if (c == 65)
                _move(game, UP);
            else if (c == 66)
                _move(game, DOWN);
            else if (c == 67)
                _move(game, RIGHT);
            else if (c == 68)
                _move(game, LEFT);
        }
        // Regular keys
        else if (c == 'W' || c == 'w')
            _move(game, UP);
        else if (c == 'S' || c == 's')
            _move(game, DOWN);
        else if (c == 'D' || c == 'd')
            _move(game, RIGHT);
        else if (c == 'A' || c == 'a')
            _move(game, LEFT);

        // Ctr + C is 3 in ASCII
    } while(c != 'q' && c != 'Q' && c != 3);
}

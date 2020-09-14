#include "minigame.h"

/* Structures for passing arguments to the threads */
typedef struct {
    Player *player;
    Image *background;
    /* They're not the same. is_running could be false and is_stopped won't be true until everything has finished 
     * Basicly, is_running is used to stop the threads, and is_stopped is used to know if every thread is dead */
    bool is_running;
    bool is_stopped;
    bool isPlayerDead;
    Minigame *minigame;
} MinigameArgs;

typedef struct{
    Player *player;
    Image *object;
    Image *background;
    unsigned short x;
    unsigned short y;
    bool is_running;
} ThrowArgs;

/* minigameArgs and t need to be persistent */
MinigameArgs minigameArgs;
pthread_t t;

/* We use this function to detect collisions between the player and an object */
bool _intersect(Player *player, Image *image, int x, int y) {
    /* Player's top left corner */
    int x1 = player->posX;
    int y1 = player->posY;
    /* Player's bottom right corner */
    int x1b = player->posX + player->right->width;
    int y1b = player->posY + player->right->heigth;

     /* Object's top left corner */
    int x2 = x;
    int y2 = y;
    /* Object's bottom right corner */    
    int x2b = x + image->width;
    int y2b = y + image->heigth;

    return (x1 < x2b && x2 < x1b && y1 < y2b && y2 < y1b);
}

void *_throw(void *args) {
    /* Dereference the args, so the code is easier to read */
    Player *player = ((ThrowArgs *)args)->player;
    Image *object = ((ThrowArgs *)args)->object;
    Image *background = ((ThrowArgs *)args)->background;
    unsigned short x = ((ThrowArgs *)args)->x;
    unsigned short y = ((ThrowArgs *)args)->y;
    int i, j;

    /* We'll stop whenever the caller wants or if the object reaches the end of the screen */
    while (x > 0 && ((ThrowArgs *)args)->is_running == true) {
        /* If the user hit an object, exit (not only this game but the minigame) */
        if (_intersect(player, object, x, y)) {
            play_sound(DEATH_PATH);
            minigameArgs.is_running = false;
            minigameArgs.isPlayerDead = true;
            break;
        }
        /* Clean the right part of the object (the object is moving left) */
        pthread_mutex_lock(&semaphore);
        for (i = 0; i < object->heigth; i++) {
            moveCursorTo(x + object->width - 1, y + i);
            printf(COLOR_SPACE(background->rgb[y + i][x + object->width - 1].red, background->rgb[y + i][x + object->width - 1].green, background->rgb[y + i][x + object->width - 1].blue));
        }
        pthread_mutex_unlock(&semaphore);
        image_print_transparent(object, background, --x, y);
        nanosleep((const struct timespec[]){{0, minigameArgs.minigame->objectSpeed}}, NULL);    
    }
    /* Before destroying the thread, we clean the object from the screen */
    pthread_mutex_lock(&semaphore);
    for (i = 0; i < object->heigth; i++) {
        moveCursorTo(x, y + i);
        for (j = 0; j < object->width; j++)
            printf(COLOR_SPACE(background->rgb[y + i][x + j].red, background->rgb[y + i][x + j].green, background->rgb[y + i][x + j].blue));
    }
    fflush(stdout);
    pthread_mutex_unlock(&semaphore);
    
    return NULL;
}

void *_minigame_launch(void *args) {
    /* Dereference the args, so the code is easier to read */
    Image *background = minigameArgs.background;
    Player *player = minigameArgs.player;
    /* Thread stuff */
    const unsigned short nThreads = minigameArgs.minigame->nThreads;
    pthread_t throwThreads[nThreads];
    ThrowArgs throwArgs[nThreads];
    unsigned short count = 0;
    /* Other stuff */
    Image *pitcher; /* Character who throws objects */
    Image *object;
    unsigned short pitcherX = minigameArgs.minigame->pitcherX;
    unsigned short pitcherY = minigameArgs.minigame->pitcherY; 
    short direction = 1;             /* This variable will be either 1 or -1 */
    unsigned short i, j;
    char PITCHER_PATH[ENTITY_NAME_LEN + 11];
    char OBJECT_PATH[ENTITY_NAME_LEN + 11];

    sprintf(PITCHER_PATH, "Minigame/%s.bmp", minigameArgs.minigame->pitcherName);
    pitcher = image_ini(PITCHER_PATH);
    if (pitcher == NULL)
        return NULL;
    /* Although only the _throw function will use it, it's better to read it only once */
    sprintf(OBJECT_PATH, "Minigame/%s.bmp", minigameArgs.minigame->objectName);
    object  = image_ini(OBJECT_PATH);
    if (object == NULL) {
        image_free(pitcher);
        return NULL;
    }

    /* We'll stop whenever the caller wants or if the object reaches the end of the screen */
    while (minigameArgs.is_running == true) {
        /* If the user touches the pitcher, kill him */
        if (_intersect(player, pitcher, pitcherX, pitcherY)) {
            minigameArgs.is_running = false;
            minigameArgs.minigame->isPitcherDead = true;
            break;
        }
        if (pitcherY >= minigameArgs.minigame->pitcherMax || pitcherY <= minigameArgs.minigame->pitcherMin) { 
            /* If the pitcher has reached the limits, change the direction */
            direction *= - 1;
        }
        /* Every now and then we throw a object */
        if (pitcherY % 10 == 0) {
                /* If the count is greater than nThreads, we'll have to reuse one 
                 * This is the reasons que don't do count = (count + 1) % nThreads at the end of this block */
                if (count >= nThreads) {
                    throwArgs[count % nThreads].is_running = false;
                    pthread_join(throwThreads[count % nThreads], NULL);
                }
                throwArgs[count % nThreads].object = object;
                throwArgs[count % nThreads].background = background;
                throwArgs[count % nThreads].player = player;
                /* We throw the objects from the left of the pitcher at medium height */
                throwArgs[count % nThreads].x = pitcherX - object->width;
                throwArgs[count % nThreads].y = pitcherY + pitcher->heigth / 2;
                throwArgs[count % nThreads].is_running = true;
                pthread_create(&(throwThreads[count % nThreads]), NULL, _throw, &(throwArgs[count % nThreads]));
                count++;
        }
        pthread_mutex_lock(&semaphore);
        if (direction == - 1) { //va para arriba
            moveCursorTo(pitcherX, pitcherY + pitcher->heigth -1);
                for (i = 0; i < pitcher->width; i++) {
                    printf(COLOR_SPACE(background->rgb[pitcherY + pitcher->heigth -1][pitcherX + i].red, background->rgb[pitcherY + pitcher->heigth - 1][pitcherX + i].green, background->rgb[pitcherY + pitcher->heigth -1][pitcherX + i].blue));
                }
        } else { //va para abajo
            moveCursorTo(pitcherX, pitcherY);
            for (i = 0; i < pitcher->width; i++) {
                printf(COLOR_SPACE(background->rgb[pitcherY][pitcherX + i].red, background->rgb[pitcherY][pitcherX + i].green, background->rgb[pitcherY][pitcherX + i].blue));
            }
        }
        pthread_mutex_unlock(&semaphore);
        image_print_transparent(pitcher, background, pitcherX, (pitcherY += direction));
        nanosleep((const struct timespec[]){{0, 100000000L}}, NULL);
    }
    /* We have to tell all threads to stop (they might be uninitialized ones) */
    for (i = 0; i < nThreads && i < count; i++) {
        throwArgs[i].is_running = false;
        pthread_join(throwThreads[i], NULL);
    }
    /* We have to clean the pitcher from screen */
    pthread_mutex_lock(&semaphore);
    for (i = 0; i < pitcher->heigth; i++) {
        moveCursorTo(pitcherX, pitcherY + i);
        for (j = 0; j < pitcher->width; j++)
            printf(COLOR_SPACE(background->rgb[pitcherY + i][pitcherX + j].red, background->rgb[pitcherY + i][pitcherX + j].green, background->rgb[pitcherY + i][pitcherX + j].blue));
    }
    fflush(stdout);
    pthread_mutex_unlock(&semaphore);


    image_free(pitcher);
    image_free(object);

    /* If the user has been killed with an object */
    if (minigameArgs.isPlayerDead == true) {
       strprint("You are dead, press any key to revive");
    }

    minigameArgs.is_stopped = true;
    return NULL;
}

void minigame_launch(Image *background, Player *player, Minigame *minigame) {
    /* If the pitcher is dead, we won't load the minigame */
    if (minigame->isPitcherDead == true)
        return;
    minigameArgs.background = background;
    minigameArgs.player = player;
    minigameArgs.minigame = minigame;
    minigameArgs.is_running = true;
    minigameArgs.isPlayerDead = false;
    minigameArgs.is_stopped = false;
    pthread_create(&t, NULL, _minigame_launch, NULL);
}

void minigame_destroy() {
    if (minigameArgs.is_stopped == false) {
        minigameArgs.is_running = false;
        pthread_join(t, NULL);
    }
}

bool minigame_isPlayerDead() {
    return minigameArgs.isPlayerDead;
}

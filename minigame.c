#include "minigame.h"

/* Structures for passing arguments to the threads */
typedef struct {
    Player *player;
    Image *background;
    /* They're not the same. is_running could be false and is_stopped won't be true until everything has finished 
     * Basicly, is_running is used to stop the threads, and is_stopped is used to know if every thread is dead */
    bool is_running;
    bool is_stopped;
    bool is_player_dead;
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

/* minigame_args and t need to be persistent */
MinigameArgs minigame_args;
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
            minigame_args.is_running = false;
            minigame_args.is_player_dead = true;
            break;
        }
        /* Clean the right part of the object (the object is moving left) */
        pthread_mutex_lock(&semaphore);
        for (i = 0; i < object->heigth; i++) {
            move_cursor_to(x + object->width - 1, y + i);
            printf(COLOR_SPACE(background->rgb[y + i][x + object->width - 1].red, background->rgb[y + i][x + object->width - 1].green, background->rgb[y + i][x + object->width - 1].blue));
        }
        pthread_mutex_unlock(&semaphore);
        image_print_transparent(object, background, --x, y);
        nanosleep((const struct timespec[]){{0, minigame_args.minigame->object_speed}}, NULL);    
    }
    /* Before destroying the thread, we clean the object from the screen */
    pthread_mutex_lock(&semaphore);
    for (i = 0; i < object->heigth; i++) {
        move_cursor_to(x, y + i);
        for (j = 0; j < object->width; j++)
            printf(COLOR_SPACE(background->rgb[y + i][x + j].red, background->rgb[y + i][x + j].green, background->rgb[y + i][x + j].blue));
    }
    fflush(stdout);
    pthread_mutex_unlock(&semaphore);
    
    return NULL;
}

void *_minigame_launch(void *args) {
    /* Dereference the args, so the code is easier to read */
    Image *background = minigame_args.background;
    Player *player = minigame_args.player;
    /* Thread stuff */
    const unsigned short num_threads = minigame_args.minigame->num_threads;
    pthread_t throw_threads[num_threads];
    ThrowArgs throw_args[num_threads];
    unsigned short count = 0;
    /* Other stuff */
    Image *pitcher; /* Character who throws objects */
    Image *object;
    unsigned short pitcher_x = minigame_args.minigame->pitcher_x;
    unsigned short pitcher_y = minigame_args.minigame->pitcher_y; 
    short direction = 1;             /* This variable will be either 1 or -1 */
    unsigned short i, j;
    char PITCHER_PATH[ENTITY_NAME_LEN + 11];
    char OBJECT_PATH[ENTITY_NAME_LEN + 11];

    sprintf(PITCHER_PATH, "Minigame/%s.bmp", minigame_args.minigame->pitcher_name);
    pitcher = image_ini(PITCHER_PATH);
    if (pitcher == NULL)
        return NULL;
    /* Although only the _throw function will use it, it's better to read it only once */
    sprintf(OBJECT_PATH, "Minigame/%s.bmp", minigame_args.minigame->object_name);
    object  = image_ini(OBJECT_PATH);
    if (object == NULL) {
        image_free(pitcher);
        return NULL;
    }

    /* We'll stop whenever the caller wants or if the object reaches the end of the screen */
    while (minigame_args.is_running == true) {
        /* If the user touches the pitcher, kill him */
        if (_intersect(player, pitcher, pitcher_x, pitcher_y)) {
            minigame_args.is_running = false;
            minigame_args.minigame->is_pitcher_dead = true;
            break;
        }
        if (pitcher_y >= minigame_args.minigame->pitcher_max_y || pitcher_y <= minigame_args.minigame->pitcher_min_y) { 
            /* If the pitcher has reached the limits, change the direction */
            direction *= - 1;
        }
        /* Every now and then we throw a object */
        if (pitcher_y % 10 == 0) {
                /* If the count is greater than num_threads, we'll have to reuse one 
                 * This is the reasons que don't do count = (count + 1) % num_threads at the end of this block */
                if (count >= num_threads) {
                    throw_args[count % num_threads].is_running = false;
                    pthread_join(throw_threads[count % num_threads], NULL);
                }
                throw_args[count % num_threads].object = object;
                throw_args[count % num_threads].background = background;
                throw_args[count % num_threads].player = player;
                /* We throw the objects from the left of the pitcher at medium height */
                throw_args[count % num_threads].x = pitcher_x - object->width;
                throw_args[count % num_threads].y = pitcher_y + pitcher->heigth / 2;
                throw_args[count % num_threads].is_running = true;
                pthread_create(&(throw_threads[count % num_threads]), NULL, _throw, &(throw_args[count % num_threads]));
                count++;
        }
        pthread_mutex_lock(&semaphore);
        if (direction == - 1) { //va para arriba
            move_cursor_to(pitcher_x, pitcher_y + pitcher->heigth -1);
                for (i = 0; i < pitcher->width; i++) {
                    printf(COLOR_SPACE(background->rgb[pitcher_y + pitcher->heigth -1][pitcher_x + i].red, background->rgb[pitcher_y + pitcher->heigth - 1][pitcher_x + i].green, background->rgb[pitcher_y + pitcher->heigth -1][pitcher_x + i].blue));
                }
        } else { //va para abajo
            move_cursor_to(pitcher_x, pitcher_y);
            for (i = 0; i < pitcher->width; i++) {
                printf(COLOR_SPACE(background->rgb[pitcher_y][pitcher_x + i].red, background->rgb[pitcher_y][pitcher_x + i].green, background->rgb[pitcher_y][pitcher_x + i].blue));
            }
        }
        pthread_mutex_unlock(&semaphore);
        image_print_transparent(pitcher, background, pitcher_x, (pitcher_y += direction));
        nanosleep((const struct timespec[]){{0, 100000000L}}, NULL);
    }
    /* We have to tell all threads to stop (they might be uninitialized ones) */
    for (i = 0; i < num_threads && i < count; i++) {
        throw_args[i].is_running = false;
        pthread_join(throw_threads[i], NULL);
    }
    /* We have to clean the pitcher from screen */
    pthread_mutex_lock(&semaphore);
    for (i = 0; i < pitcher->heigth; i++) {
        move_cursor_to(pitcher_x, pitcher_y + i);
        for (j = 0; j < pitcher->width; j++)
            printf(COLOR_SPACE(background->rgb[pitcher_y + i][pitcher_x + j].red, background->rgb[pitcher_y + i][pitcher_x + j].green, background->rgb[pitcher_y + i][pitcher_x + j].blue));
    }
    fflush(stdout);
    pthread_mutex_unlock(&semaphore);


    image_free(pitcher);
    image_free(object);

    /* If the user has been killed with an object */
    if (minigame_args.is_player_dead == true) {
       strprint("You are dead, press any key to revive");
    }

    minigame_args.is_stopped = true;
    return NULL;
}

void minigame_launch(Image *background, Player *player, Minigame *minigame) {
    /* If the pitcher is dead, we won't load the minigame */
    if (minigame->is_pitcher_dead == true)
        return;
    minigame_args.background = background;
    minigame_args.player = player;
    minigame_args.minigame = minigame;
    minigame_args.is_running = true;
    minigame_args.is_player_dead = false;
    minigame_args.is_stopped = false;
    pthread_create(&t, NULL, _minigame_launch, NULL);
}

void minigame_destroy() {
    if (minigame_args.is_stopped == false) {
        minigame_args.is_running = false;
        pthread_join(t, NULL);
    }
}

bool minigame_is_player_dead() {
    return minigame_args.is_player_dead;
}

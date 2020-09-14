#ifndef UTIL_H
#define UTIL_H

#define BACKGROUND_HEIGTH 150
#define BACKGROUND_WIDTH  240
#define LETTER_HEIGTH 6
#define MAP_NAME_LEN 32
#define ENTITY_NAME_LEN 32
#define DEFAULT_INITIAL_POS -1
#define MSG_LEN 128
#define PRINT_TIME 8
#define LOG_FILE "log.txt"

#define CLEAR_SCREEN "\e[1;1H\e[2J"
#define HIDE_CURSOR "\e[?25l"
#define SHOW_CURSOR "\e[?25h"

#include <stdio.h>
#include <assert.h>
#include <pthread.h>

typedef enum {FALSE, TRUE} Bool;
typedef enum {ERROR, OK}   Status;

typedef struct {
    unsigned short red;
    unsigned short green;
    unsigned short blue;
} RGB;

pthread_mutex_t semaphore;

void moveCursorTo(int x, int y);
int distance(int x1, int y1, int x2, int y2);
void log_str(char *str);
void log_int(int n);

#endif
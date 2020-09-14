#include "strprint.h"

#define MAX_FILENAME 16
#define PADDING 3
#define LINE_SPACING 2
#define MAX_ROWS 2
#define BUBBLE_HEIGHT MAX_ROWS*LETTER_HEIGHT + 2*PADDING + (MAX_ROWS - 1)*LINE_SPACING

int globalSecs;
char printedSentece[MSG_LEN] = "";
pthread_t sleepTimer;

void cleanText() {
    int i = 0, j;

    pthread_mutex_lock(&semaphore);
    moveCursorTo(0, BACKGROUND_HEIGHT);
    for (; i < BUBBLE_HEIGHT; i++) {
        for (j = 0; j < BACKGROUND_WIDTH; j++)
            printf(COLOR_SPACE(255, 255, 255));
        printf(ANSI_COLOR_RESET "\n");
    }
    pthread_mutex_unlock(&semaphore);
}

void strprint_PIN(char *str) {
    /* We assume the input is correct (_checkPin will handle that), that is, only up to 4 numbers */
    const int digits = 4;
    /* We tried to do posX[digits] but it only compiles in Mac, not in Linux */
    const unsigned short posX[] = {99, 110, 121, 132};
    const unsigned short posY = 81;
    unsigned short i = 0;
    unsigned short x, y;
    char filename[MAX_FILENAME];
    Image *image;

    /* Print the string passed */
    while (*str != '\0') {
        sprintf(filename, "Fonts/%c.bmp", *str);
        image = image_ini(filename);
        if (image == NULL) {
            continue;
        }
        image_print(image, posX[i], posY);
        image_free(image);
        str++;
        i++;
    }

    /* Clear the last positions (because we handle the DEL key) */
    pthread_mutex_lock(&semaphore);
    for(; i < digits; i++) {
        for (y = 0; y < 7; y++) {
            moveCursorTo(posX[i], posY + y);
            for (x = 0; x < 4; x++) {
                printf(COLOR_SPACE(255, 255, 255));
            }
        }
    }
    pthread_mutex_unlock(&semaphore);
}

Status _strprint(char *str) {
    char filename[MAX_FILENAME];
    Image *image;
    int x = PADDING;
    int y = BACKGROUND_HEIGHT + PADDING;
    int row = 0;
    char c;
    
    if (str == NULL)
        return ERROR;
    
    /* If we are trying to print the same sentece that it is printed, we won't */
    if (strcmp(str, printedSentece) == 0)
        return OK;
    strcpy(printedSentece, str);
    cleanText();
    while (*str != '\0') {
        if ((*str >= 'A' && *str <= 'Z') || (*str >= 'a' && *str <= 'z') || *str == '!' || *str == '?' || *str == ',' || *str == '.' || *str == ':' || *str == '\'' || (*str >= '0' && *str <= '9')) {
            /* Print lowercase uppercase */
            if (*str >= 'a' && *str <= 'z')
                c = *str - 32;
            else
                c = *str;
            
            /* As we couldn't create files named '..bmp' or ':.bmp' */
            if (c == '.') {
                sprintf(filename, "Fonts/dot.bmp");
            } else if (c == ':') {
                sprintf(filename, "Fonts/colon.bmp");
            } else {
                sprintf(filename, "Fonts/%c.bmp", c);
            }
            image = image_ini(filename);
            /* Check if the letter fits in, if it doesn't continue on the next line */
            if (x + image->width + PADDING > BACKGROUND_WIDTH) {
                x = PADDING;
                y += LETTER_HEIGHT + LINE_SPACING;
                row++;
                image_free(image);
                /* Exit if no more rows can fit in (it is an ERROR, but we are not going to check it)
                 * This way the code in game.c for allowing map changes is nicer */
                if (row == MAX_ROWS)
                    return OK;
                continue;
            }
            if (image != NULL) {
                image_print(image, x, y);
                x += image->width + 1;
                image_free(image);
            }
        } else if (*str == ' ') {
            x += 4;
        } else if (*str == '\n') {
            x = PADDING;
            y += LETTER_HEIGHT + LINE_SPACING;
            row++;
            /* Exit if no more rows can fit in (it is an ERROR, but we are not going to check it)
             * This way the code in game.c for allowing map changes is nicer */
            if (row == MAX_ROWS)
                return OK;
        }
        str++;
    }
    fflush(stdout);
    return OK;
}

Status strprint(char *str) {
    /* Cancel the previous sleepTimer, if there's one */
    pthread_cancel(sleepTimer);

    return _strprint(str);
}

void *_wait(void *secs) {
    sleep(*((int *)secs));
    cleanText();
    printedSentece[0] = '\0';
    return NULL;
}


Status strprint_time(char *str, int secs) {
    /* If we don't copy secs to a global variable, it will get destroyed by the time we want to use it */
    globalSecs = secs;

    /* If we want to print the same sentence, we won't, we'll just reset the sleep timer (strprint won't print anything) */
    if (_strprint(str) == ERROR)
        return ERROR;
    /* Cancel the previous sleepTimer, if there's one */
    pthread_cancel(sleepTimer);
    /* Spawn a new thread */
    pthread_create(&sleepTimer, NULL, _wait, &globalSecs);
    
    return OK;
}

#ifndef IMAGE_H
#define IMAGE_H

#define COLOR_SPACE(R, G, B) "\x1b[48;2;%d;%d;%dm  ", R, G, B
#define ANSI_COLOR_RESET   "\x1b[0m"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"

typedef struct {
    unsigned short width;
    unsigned short heigth;
    RGB **rgb;
} Image;

Image *image_ini(const char *filename);
void image_print(Image *image, int x, int y);
void image_free(Image *image);
void image_print_transparent(Image *image, Image *background, int x, int y);

#endif

#ifndef STRING_H
#define STRING_H

#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include "image.h"
#include "util.h"

Status strprint(char *str);
Status strprint_time(char *str, int secs);
void strprint_PIN(char *str);
void clean_text();

#endif

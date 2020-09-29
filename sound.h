#ifndef SOUND_H
#define SOUND_H

#include <stdlib.h>
#include <stdio.h>
#include "util.h"
/* For safety reasons */
#include <string.h>
#include <assert.h>

#define BEEP_PATH "Sounds/beep.wav"
#define BACKSPACE_PATH "Sounds/backspace.wav"
#define ACCESSGRANTED_PATH "Sounds/accessgranted.wav"
#define BUZZ_PATH "Sounds/buzz.wav"
#define DEATH_PATH "Sounds/death.wav"
#define OBJECTCATCH_PATH "Sounds/objectcatch.wav"
#define LASTSONG_PATH "Sounds/libre.wav"
#define BEGINNING_PATH "Sounds/beginning.wav"
#define KILL_PATH "Sounds/kill.wav"
#define THROW_PATH "Sounds/throw.wav"

int play_sound(const char* path);
void stop_sound(int pid);

#endif


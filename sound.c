#include "sound.h"

#define COMMAND_LEN 128
#define OUTPUT_LEN 128

#ifdef __APPLE__
const char player[] = "afplay";
#elif linux
const char player[] = "aplay";
#else
const char player[] = "";
#endif


/* We return the PID of the background process. The sound should be short if the PID is ignored */
int play_sound(const char *path) {
    char output[OUTPUT_LEN];
    char command[COMMAND_LEN];
    int pid;
    FILE *p;

    if (player[0] == '\0') /* The computer doesn't have afplay nor aplay */
        return - 1;

    assert(28 + strlen(path) + strlen(player) <= COMMAND_LEN);
    /* We don't want anything printed on the screen, thus the >/dev/null 2>&1*/
    sprintf(command, "%s %s >/dev/null 2>&1 & echo $!", player, path);
    /* Run the command */
    p = popen(command, "r");
    /* Get the output */
    fgets(output, OUTPUT_LEN - 1, p);
    /* Close the pipe */
    pclose(p);
    pid = atoi(output);
    /* If we assume that the pid is not going to be 0, we can check if it's cero (atoi found an error) and return -1 */
    if (pid == 0)
        return -1;

    return pid;
}

void stop_sound(int pid) {
    char command[COMMAND_LEN];

    /* We don't want anything printed on the screen, thus the >/dev/null 2>&1*/
    sprintf(command, "kill %d >/dev/null 2>&1", pid);
    system(command);
}
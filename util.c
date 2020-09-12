#include "util.h"

void moveCursorTo(int x, int y) {
    /* Cursor starts at 1,1 and there are two spaces per pixel */
    printf("%c[%d;%dH", 27, y + 1, x * 2 + 1);
}

int distance(int x1, int y1, int x2, int y2) {
    return (x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1);
}

void log_str(char* str) {
    FILE* f = fopen(LOG_FILE, "a");
    if (f != NULL) {
        fprintf(f, "%s\n", str);
        fclose(f);
    }
}

void log_int(int n) {
    FILE* f = fopen(LOG_FILE, "a");
    if (f != NULL) {
        fprintf(f, "%d\n", n);
        fclose(f);
    }
}
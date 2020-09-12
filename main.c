#include "game.h"

int main() {
    Status status = game_start(NULL);
    if (status == ERROR)
        return 1;
    return 0;
}

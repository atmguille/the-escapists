#include "player.h"

#define PLAYER_IMAGE_RIGHT             "Player/right.bmp"
#define PLAYER_IMAGE_LEFT              "Player/left.bmp"
#define PLAYER_IMAGE_BACKWARDS         "Player/backwards.bmp"

#define PLAYER_IMAGE_RIGHT_POLICE      "Player/Police/right.bmp"
#define PLAYER_IMAGE_LEFT_POLICE       "Player/Police/left.bmp"
#define PLAYER_IMAGE_BACKWARDS_POLICE  "Player/Police/backwards.bmp"

Player *player_ini() {
    Player *player;

    player = (Player *)malloc(sizeof(Player));
    if (player == NULL)
        return NULL;

    player->right = image_ini(PLAYER_IMAGE_RIGHT);
    if (player->right == NULL) {
        free(player);
        return NULL;
    }

    player->left = image_ini(PLAYER_IMAGE_LEFT);
    if (player->left == NULL) {
        image_free(player->right);
        free(player);
        return NULL;
    }

    player->backwards = image_ini(PLAYER_IMAGE_BACKWARDS);
    if (player->backwards == NULL) {
        image_free(player->right);
        image_free(player->left);
        free(player);
        return NULL;
    }

    /* Default position */
    player_turn_right(player);

    return player;
}

void player_turn_left(Player *player) {
    player->current = player->left;
}

void player_turn_right(Player *player) {
    player->current = player->right;
}

void player_turn_back(Player *player) {
    player->current = player->backwards;
}

void player_turn_front(Player *player) {
    /* If the user is facing back, right is default. If the user is facing either right or left, we'll leave it as it is */
    if (player->current == player->backwards)
        player->current = player->right;
}

void player_enable_police_mode(Player *player) {
    /* We are going to assume that the image allocation won't fail (we free two images, we allocate another two of the same size) */
    image_free(player->right);
    image_free(player->left);
    image_free(player->backwards);

    player->right     = image_ini(PLAYER_IMAGE_RIGHT_POLICE);
    player->left      = image_ini(PLAYER_IMAGE_LEFT_POLICE);
    player->backwards = image_ini(PLAYER_IMAGE_BACKWARDS_POLICE);

    player_turn_right(player);
}

void player_free(Player *player) {
    if (player != NULL) {
        image_free(player->right);
        image_free(player->left);
        image_free(player->backwards);
        free(player);
    }
}

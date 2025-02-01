// A really basic implementation of Tazar and an AI player, so I have someone to play with.
// https://kelleherbros.itch.io/tazar
// Been running this alongside and inputting all the moves into the real game, that's why it
// asks for the results of Volley rolls. Not really meant to be "played" standalone.

#include "tazar_game.c"
#include "tazar_parse.c"
#include "tazar_cli.c"

#define STEVE_IMPLEMENTATION
#include "steve.h"

int main(void) {
    return cli_main();
}
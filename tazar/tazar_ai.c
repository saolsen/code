#include "steve.h"
#include "tazar.h"

#include <stdlib.h>

int rand_in_range(int min, int max) {
    return min + rand() / (RAND_MAX / (max - min + 1) + 1);
}


Command ai_select_command_random(Game *game, CommandSlice commands) {
    // @note: This is a placeholder AI that just selects the first command.
    int selected = rand_in_range(0, (int) (commands.len - 1));
    return commands.e[selected];
}
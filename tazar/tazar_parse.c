#include "steve.h"
#include "tazar.h"

#include <stdio.h>

ParseAction action_parse(Arena *a, String action_str) {
    Arena *scratch = scratch_acquire();
    StringSlice parts = str_split(scratch, action_str, ' ');

    ParseAction action = {0};
    if (parts.len < 3) {
        action.error = str_format(a, "Not enough arguments, expected '[UNIT] [ACTION] [TARGET PATH..]'");
        goto end;
    }
    String unit_str = parts.e[0];
    if (unit_str.len != 2) {
        action.error = str_format(a, "Invalid unit '%.*s', expected unit type and id", unit_str.len,
                                  unit_str.e);
        goto end;
    }
    if (unit_str.e[0] != 'c' && unit_str.e[0] != 'p' && unit_str.e[0] != 'h' && unit_str.e[0] != 'b') {
        action.error = str_format(a, "Invalid unit type '%.*s', expected 'p', 'b', 'h', or 'c'", unit_str.len,
                                  unit_str.e);
        goto end;
    }
    switch (unit_str.e[0]) {
        case 'c':
            action.action.piece = PIECE_CROWN;
            break;
        case 'p':
            action.action.piece = PIECE_PIKE;
            break;
        case 'h':
            action.action.piece = PIECE_HORSE;
            break;
        case 'b':
            action.action.piece = PIECE_BOW;
            break;
    }
    if (unit_str.e[1] < '1' || unit_str.e[1] > '9') {
        action.error = str_format(a, "Invalid unit id '%.*s', expected a number", unit_str.len - 1, unit_str.e + 1);
        goto end;
    }
    action.action.piece_id = unit_str.e[1] - '0';

    String action_kind_str = parts.e[1];
    if (action_kind_str.len == 4 && strncmp((const char *) action_kind_str.e, "move", 4) == 0) {
        action.action.kind = ACTION_MOVE;
    } else if (action_kind_str.len == 6 && strncmp((const char *) action_kind_str.e, "volley", 6) == 0) {
        action.action.kind = ACTION_VOLLEY;
    } else if (action_kind_str.len == 6 && strncmp((const char *) action_kind_str.e, "charge", 6) == 0) {
        action.action.kind = ACTION_CHARGE;
    } else if (action_kind_str.len == 3 && strncmp((const char *) action_kind_str.e, "end", 3) == 0) {
        action.action.kind = ACTION_END;
    } else {
        action.error = str_format(a, "Invalid action '%.*s', expected 'move', 'volley', 'charge', or 'end'",
                                  action_kind_str.len, action_kind_str.e);
        goto end;
    }

    CPos target = {0};
    for (uint64_t i = 2; i < parts.len; i++) {
        if (parts.e[i].len == 1) {
            if (parts.e[i].e[0] == 'r') {
                target = cpos_add(target, CPOS_RIGHT);
            } else if (parts.e[i].e[0] == 'l') {
                target = cpos_add(target, CPOS_LEFT);
            } else {
                action.error = str_format(a, "Invalid direction '%.*s'", parts.e[i].len, parts.e[i].e);
                goto end;
            }
        } else if (parts.e[i].len == 2) {
            if (parts.e[i].e[0] == 'r' && parts.e[i].e[1] == 'u') {
                target = cpos_add(target, CPOS_RIGHT_UP);
            } else if (parts.e[i].e[0] == 'r' && parts.e[i].e[1] == 'd') {
                target = cpos_add(target, CPOS_RIGHT_DOWN);
            } else if (parts.e[i].e[0] == 'l' && parts.e[i].e[1] == 'u') {
                target = cpos_add(target, CPOS_LEFT_UP);
            } else if (parts.e[i].e[0] == 'l' && parts.e[i].e[1] == 'd') {
                target = cpos_add(target, CPOS_LEFT_DOWN);
            } else {
                action.error = str_format(a, "Invalid direction '%.*s'", parts.e[i].len, parts.e[i].e);
                goto end;
            }

        } else {
            action.error = str_format(a, "Invalid direction '%.*s'", parts.e[i].len, parts.e[i].e);
            goto end;
        }
    }
    action.action.target = target;

    end:
    scratch_release();
    return action;
}

// @ts-check

/**
 * @typedef {{
 *     x: number,
 *     y: number,
 * }} Vec2
 */

/**
 * @typedef {{
 *     q: number,
 *     r: number,
 *     s: number,
 * }} CPos
 */

const CPOS_RIGHT_UP = { q: 1, r: -1, s: 0 };
const CPOS_RIGHT = { q: 1, r: 0, s: -1 };
const CPOS_RIGHT_DOWN = { q: 0, r: 1, s: -1 };
const CPOS_LEFT_DOWN = { q: -1, r: 1, s: 0 };
const CPOS_LEFT = { q: -1, r: 0, s: 1 };
const CPOS_LEFT_UP = { q: 0, r: -1, s: 1 };

/**
 * @typedef {number} PieceKind
 */

/**
 * @enum {PieceKind}
 */
var PIECE_KINDS = {
    PIECE_NONE: 0,
    PIECE_EMPTY: 1,
    PIECE_CROWN: 2,
    PIECE_PIKE: 3,
    PIECE_HORSE: 4,
    PIECE_BOW: 5,
};

/**
 * @typedef {number} Player
 */

/**
 * @enum {Player}
 */
var PLAYERS = {
    PLAYER_NONE: 0,
    PLAYER_RED: 1,
    PLAYER_BLUE: 2,
};

/**
 * @typedef {{
 *     kind: PieceKind,
 *     player: Player,
 *     id: number,
 * }} Piece
 */

/**
 * @typedef {number} OrderKind
 */

/**
 * @enum {OrderKind}
 */
var ORDER_KINDS = {
    ORDER_NONE: 0,
    ORDER_MOVE: 1,
    ORDER_VOLLEY: 2,
    ORDER_MUSTER: 3,
};

/**
 * @typedef {{
 *     kind: OrderKind,
 *     target: CPos,
 * }} Order
 */

/**
 * @typedef {{
 *     piece_id: number,
 *     orders: Order[],
 *     order_i: number,
 * }} Activation
 */

/**
 * @typedef {{
 *     player: Player,
 *     activations: Activation[],
 *     activation_i: number,
 * }} Turn
 */

/** @typedef {number} Status */

/**
 * @enum {Status}
 */
var STATUSES = {
    STATUS_NONE: 0,
    STATUS_IN_PROGRESS: 1,
    STATUS_OVER: 2,
};

/**
 * @typedef {{
 *     board: Piece[],
 *     board_width: number,
 *     board_height: number,
 *     status: Status,
 *     winner: Player,
 *     gold: number[],
 *     turn: Turn,
 * }} Game
 */

/**
 * @typedef {number} CommandKind
 */

/**
 * @enum {CommandKind}
 */
var COMMAND_KINDS = {
    COMMAND_NONE: 0,
    COMMAND_MOVE: 1,
    COMMAND_VOLLEY: 2,
    COMMAND_MUSTER: 3,
    COMMAND_END_TURN: 4,
};

/**
 * @typedef {{
 *     kind: CommandKind,
 *     piece_pos: CPos,
 *     target_pos: CPos,
 * }} Command
 */

/** @typedef {number} VolleyResult  */

/**
 * @enum {VolleyResult}
 */
var VOLLEY_RESULTS = {
    VOLLEY_ROLL: 0,
    VOLLEY_HIT: 1,
    VOLLEY_MISS: 2,
};

/**
 * Check if two coordinates are equal.
 * @param {CPos} a - The first coordinate.
 * @param {CPos} b - The second coordinate.
 * @returns {boolean} True if the coordinates are equal, false otherwise.
 */
function cpos_eq(a, b) {
    return a.q === b.q && a.r === b.r && a.s === b.s;
}

/**
 * Add two coordinates.
 * @param {CPos} a - The first coordinate.
 * @param {CPos} b - The second coordinate.
 * @returns {CPos} The sum of the two coordinates.
 */
function cpos_add(a, b) {
    return { q: a.q + b.q, r: a.r + b.r, s: a.s + b.s };
}

/**
 * Convert a hex position to double coordinates.
 * @param {CPos} cpos - The hex position.
 * @returns {Vec2} The double coordinates of the hex position.
 */
function vec2_from_cpos(cpos) {
    return { x: cpos.q, y: cpos.r };
}

/**
 * Convert double coordinates to a hex position.
 * @param {Vec2} dpos - The double coordinates.
 * @returns {CPos} The hex position.
 */
function cpos_from_dpos(dpos) {
    const q = (dpos.x - dpos.y) / 2;
    const r = dpos.y;
    const s = -q - r;
    return { q, r, s };
}

// I do actually want to store the board as a 3d array because
// that's a good representation to feed to the neural network.
// dimension is the square size of the board.
// offset is the offset from 0,0,0 that the board represents.
// maybe I'll also just always store it as the largest map.

// I think sanguine ravine is the largest map, so my board
// needs to be able to store that.

/**
 * Get the piece at a given hex position.
 * @param {Game} game - The game.
 * @param {CPos} cpos - The hex position.
 * @returns {Piece} The piece at the given position.
 */
function game_board_at(game, cpos) {
    if (
        cpos.q + cpos.r + cpos.s !== 0 ||
        cpos.q < -4 || cpos.q > 4 ||
        cpos.r < -4 || cpos.r > 4 ||
        cpos.s < -4 || cpos.s > 4
    ) {
        return {
            kind: PIECE_KINDS.PIECE_NONE,
            player: PLAYERS.PLAYER_NONE,
            id: 0,
        };
    }
    const index = (cpos.r + 4) * 9 + (cpos.q + 4);
    return game.board[index];
}

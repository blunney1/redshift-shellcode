#include <stdint.h>

#ifdef DEBUG
#define abort() __builtin_trap();
//#define abort() throw "derp";
#define private static
#else
#define private static __attribute__((always_inline)) inline
#define abort() __builtin_unreachable();
#endif

// bb is 'bitboard'
typedef struct gamestate {
  uint64_t rooks_bb;
  uint64_t knights_bb;
  uint64_t bishops_bb;
  uint64_t queens_bb;
  uint64_t kings_bb;
  uint64_t pawns_bb;
  union {
    uint64_t current_player_bb;
    uint64_t current_piece_bb; // For iterators
  };
  int en_passant_sq;
} gamestate;

struct move {
  int from;
  int to;
};

// TODO: Add promotions to the iterator
typedef gamestate iterator;

const int PIECE_EMPTY  = 0;
const int PIECE_ROOK   = 1;
const int PIECE_KNIGHT = 2;
const int PIECE_BISHOP = 3;
const int PIECE_QUEEN  = 4;
const int PIECE_KING   = 5;
const int PIECE_PAWN   = 6;


const int DIRECTION_EAST  = 0;
const int DIRECTION_WEST  = 1;
const int DIRECTION_NORTH = 2;
const int DIRECTION_SOUTH = 3;
const int DIRECTION_NORTHEAST = 4;
const int DIRECTION_NORTHWEST = 5;
const int DIRECTION_SOUTHEAST = 6;
const int DIRECTION_SOUTHWEST = 7;

const int NUM_BOARD_SQUARES = 8*8;

const int RANK = 8;

const int POSITION_INVALID = 255;

const uint64_t RAY_A1A8 = 0x0101010101010101;
const uint64_t RAY_H1H8 = RAY_A1A8 << (RANK-1);

// Public functions

uint64_t best_move(uint64_t rooks_bb, uint64_t knights_bb, uint64_t bishops_bb, uint64_t queens_bb, uint64_t kings_bb, uint64_t pawns_bb, uint64_t colors_bb)
{
  return 0;
}

// Private functions

private uint64_t bit(uint64_t idx) {
  if (idx >= 64 || idx < 0) {
    return 0;
  }
  return ((uint64_t)(1) << idx);
}
private uint64_t clear_bit(uint64_t x, uint64_t idx) { return x & ~bit(idx); }
private bool is_bit_set(uint64_t x, uint64_t idx) {
  if (idx >= 64) { return false; }
  return x & bit(idx);
}
private uint64_t set_bit(uint64_t x, uint64_t idx) { return x | bit(idx); }
private uint64_t lsb_first_set(uint64_t x) { return __builtin_ctzll(x); }
private uint64_t msb_first_set(uint64_t x) { return (63 - __builtin_clzll(x)); }

private int rank(int x) { return x / 8; }
private int file(int x) { return x % 8; }
private uint64_t offset(uint64_t x, int os)
{
#ifdef DEBUG
  if (os >= 64 || os <= -64) {
    throw "???";
  }
#endif
  if (os < 0) {
    return (x >> -os);
  } else {
    return (x << os);
  }
}

private gamestate zerostate()
{
  gamestate x;
  x.current_piece_bb = 0;
  x.rooks_bb = 0;
  x.knights_bb = 0;
  x.bishops_bb = 0;
  x.queens_bb = 0;
  x.kings_bb = 0;
  x.pawns_bb = 0;
  x.en_passant_sq = POSITION_INVALID;
  return x;
}


private uint64_t mkPosition(int file, int rank)
{
  return rank * RANK + file;
}

const uint64_t RAY_A1H8 =
  bit(mkPosition(0,0)) |
  bit(mkPosition(1,1)) |
  bit(mkPosition(2,2)) |
  bit(mkPosition(3,3)) |
  bit(mkPosition(4,4)) |
  bit(mkPosition(5,5)) |
  bit(mkPosition(6,6)) |
  bit(mkPosition(7,7))
  ;

#ifdef DEBUG
#include <stdio.h>
private void print_bitboard(uint64_t bb)
{
  for (int r = 8; r --> 0;) {
    for (int f = 0; f < 8; f++) {
      char c = is_bit_set(bb, mkPosition(f, r)) ? '1' : '0';
      putchar(c);
    }
    putchar('\n');
  }
}
#endif

// https://chessprogramming.wikispaces.com/On+an+empty+Board#RayAttacks
private uint64_t diagonal_mask(int center)
{
   const uint64_t maindia = 0x8040201008040201;
   int diag =8*(center & 7) - (center & 56);
   int nort = -diag & ( diag >> 31);
   int sout =  diag & (-diag >> 31);
   return (maindia >> sout) << nort;
}

private uint64_t antidiagonal_mask(int center)
{
  const uint64_t maindia = 0x0102040810204080;
  int diag =56- 8*(center&7) - (center&56);
  int nort = -diag & ( diag >> 31);
  int sout =  diag & (-diag >> 31);
  return (maindia >> sout) << nort;
}

// https://chessprogramming.wikispaces.com/Flipping+Mirroring+and+Rotating
// See 'flipDiagA1H8'
private uint64_t flipDiagA1H8(uint64_t x)
{
  uint64_t t;
  const uint64_t k1 = 0x5500550055005500;
  const uint64_t k2 = 0x3333000033330000;
  const uint64_t k4 = 0x0f0f0f0f00000000;
  t  = k4 & (x ^ (x << 28));
  x ^=       t ^ (t >> 28) ;
  t  = k2 & (x ^ (x << 14));
  x ^=       t ^ (t >> 14) ;
  t  = k1 & (x ^ (x <<  7));
  x ^=       t ^ (t >>  7) ;
  return x;
}

private uint64_t flip_vertical(uint64_t x)
{
  return
    ( (x << 56)                        ) |
    ( (x << 40) & (0x00ff000000000000) ) |
    ( (x << 24) & (0x0000ff0000000000) ) |
    ( (x <<  8) & (0x000000ff00000000) ) |
    ( (x >>  8) & (0x00000000ff000000) ) |
    ( (x >> 24) & (0x0000000000ff0000) ) |
    ( (x >> 40) & (0x000000000000ff00) ) |
    ( (x >> 56)                        );
}

private uint64_t rotate_bb(uint64_t x)
{
  return flip_vertical(flipDiagA1H8(x));
}

const uint64_t RAY_A8H1 = rotate_bb(RAY_A1H8);

private uint64_t get_piece_bb(gamestate x, int piece)
{
  switch (piece) {
  case PIECE_ROOK:
    return x.rooks_bb;
  case PIECE_KNIGHT:
    return x.knights_bb;
  case PIECE_BISHOP:
    return x.bishops_bb;
  case PIECE_QUEEN:
    return x.queens_bb;
  case PIECE_KING:
    return x.kings_bb;
  case PIECE_PAWN:
    return x.pawns_bb;
  }
  abort();
}

private gamestate set_piece_bb(gamestate x, int piece, uint64_t bb)
{
  switch (piece) {
  case PIECE_ROOK:
    x.rooks_bb = bb;
    return x;
  case PIECE_KNIGHT:
    x.knights_bb = bb;
    return x;
  case PIECE_BISHOP:
    x.bishops_bb = bb;
    return x;
  case PIECE_QUEEN:
    x.queens_bb = bb;
    return x;
  case PIECE_KING:
    x.kings_bb = bb;
    return x;
  case PIECE_PAWN:
    x.pawns_bb = bb;
    return x;
  default:
    abort();
  }
}

private int next_iterator_piece(iterator x)
{
  if (x.rooks_bb)
    return PIECE_ROOK;
  if (x.knights_bb)
    return PIECE_KNIGHT;
  if (x.bishops_bb)
    return PIECE_BISHOP;
  if (x.queens_bb)
    return PIECE_QUEEN;
  if (x.kings_bb)
    return PIECE_KING;
  if (x.pawns_bb)
    return PIECE_PAWN;
  return PIECE_EMPTY;
}

private int iterator_position(iterator x)
{
  int piece = next_iterator_piece(x);
  if (piece == PIECE_EMPTY) {
    return POSITION_INVALID;
  } 
  uint64_t piece_bb = get_piece_bb(x, piece);
  int idx = lsb_first_set(piece_bb);
  return idx;
}

private move dereference_iterator(iterator x)
{
  move y;
  y.from = iterator_position(x);
  y.to = lsb_first_set(x.current_piece_bb);
  return y;
}

private uint64_t advance_bb_iterator(uint64_t x)
{
  return x & (x-1);
}

private bool is_iterator_finished(iterator x)
{
  return
    ! x.rooks_bb &&
    ! x.knights_bb &&
    ! x.bishops_bb &&
    ! x.queens_bb &&
    ! x.kings_bb &&
    ! x.pawns_bb &&
    ! x.current_piece_bb
    ;
}

// When a piece is moving west, it may wrap around to be east.
// So we mask out the eastmost column, since it's impossible anyways.
private uint64_t guard_west(uint64_t x) {
  return x & ~RAY_H1H8;
}

private uint64_t guard_east(uint64_t x) {
  return x & ~RAY_A1A8;
}

private int move_direction(int idx, int direction)
{
  if (idx < 0 || idx >= 64) {
    return POSITION_INVALID;
  }
  switch (direction) {
  case DIRECTION_EAST:
    if (idx % 8 == 7)
      return POSITION_INVALID;
    return idx + 1;
  case DIRECTION_WEST:
    if (idx % 8 == 0)
      return POSITION_INVALID;
    return idx - 1;
  case DIRECTION_NORTH:
    return idx + RANK;
  case DIRECTION_SOUTH:
    return idx - RANK;
  case DIRECTION_NORTHEAST:
    if (file(idx) == 7) {
      return POSITION_INVALID;
    }
    return (idx + RANK + 1);
  case DIRECTION_NORTHWEST:
    if (file(idx) == 0) {
      return POSITION_INVALID;
    }
    return (idx + RANK - 1);
  case DIRECTION_SOUTHEAST:
    if (file(idx) == 7) {
      return POSITION_INVALID;
    }
    return (idx - RANK + 1);
  case DIRECTION_SOUTHWEST:
    if (file(idx) == 0) {
      return POSITION_INVALID;
    }
    return (idx - RANK - 1);
  default:
    abort();
  }
}

private uint64_t all_pieces(gamestate x)
{
  return
    x.rooks_bb |
    x.knights_bb |
    x.bishops_bb |
    x.queens_bb |
    x.kings_bb |
    x.pawns_bb
    ;
}

private uint64_t enemy_pieces(gamestate x)
{
  return all_pieces(x) ^ x.current_player_bb;
}

private uint64_t valid_pawn_moves(gamestate x, int center)
{
  uint64_t moves_bb = 0;
  // Non-captures
  if (! is_bit_set(all_pieces(x), center + RANK)) {
    uint64_t noncaptures_bb = bit(center + RANK);
    uint64_t pcs = all_pieces(x);
    if (rank(center) == 1 && ! is_bit_set(pcs, center + RANK) && ! is_bit_set(pcs, center + RANK*2)) {
      noncaptures_bb |= bit(center + RANK + RANK);
    }
    moves_bb |= noncaptures_bb;
  }
  // Captures
  {
    uint64_t captures_bb = 0;
    if (file(center) > 0) {
      captures_bb |= bit(move_direction(center + RANK, DIRECTION_WEST));
    }
    if (file(center) < 7) {
      captures_bb |= bit(move_direction(center + RANK, DIRECTION_EAST));
    }
    captures_bb &= all_pieces(x) ^ x.current_player_bb;
    if ((file(center) < 7 && x.en_passant_sq == center + RANK + 1) ||
        (file(center) > 0 && x.en_passant_sq == center + RANK - 1)) {
      captures_bb |= bit(x.en_passant_sq);
    }
    moves_bb |= captures_bb;
  }
  return moves_bb;
}

private uint64_t valid_knight_moves(gamestate x, int idx)
{
  uint64_t moves_bb = 0;
  moves_bb |= guard_west(bit(move_direction(idx + 2 * RANK, DIRECTION_WEST)));
  moves_bb |= guard_east(bit(move_direction(idx + 2 * RANK, DIRECTION_EAST)));
  
  moves_bb |= guard_west(bit(move_direction(idx - 2 * RANK, DIRECTION_WEST)));
  moves_bb |= guard_east(bit(move_direction(idx - 2 * RANK, DIRECTION_EAST)));
  
  moves_bb |= bit(move_direction(move_direction(move_direction(idx, DIRECTION_EAST), DIRECTION_EAST), DIRECTION_NORTH));
  moves_bb |= bit(move_direction(move_direction(move_direction(idx, DIRECTION_EAST), DIRECTION_EAST), DIRECTION_SOUTH));

  moves_bb |= bit(move_direction(move_direction(move_direction(idx, DIRECTION_WEST), DIRECTION_WEST), DIRECTION_NORTH));
  moves_bb |= bit(move_direction(move_direction(move_direction(idx, DIRECTION_WEST), DIRECTION_WEST), DIRECTION_SOUTH));

  moves_bb &= ~x.current_player_bb;
  return moves_bb;
}

private uint64_t bitrange(int start, int end)
{
  uint64_t end_bb = bit(end+1)-1;
  uint64_t start_bb = bit(start)-1;
  return end_bb ^ start_bb;
}

private uint64_t mkRay(int center, int direction)
{
  int rOs, fOs;
  switch (direction) {
  case DIRECTION_NORTHWEST: rOs =  1; fOs = -1; break;
  case DIRECTION_NORTH:     rOs =  1; fOs =  0; break;
  case DIRECTION_NORTHEAST: rOs =  1; fOs =  1; break;
  case DIRECTION_EAST:      rOs =  0; fOs =  1; break;
  case DIRECTION_SOUTHEAST: rOs = -1; fOs =  1; break;
  case DIRECTION_SOUTH:     rOs = -1; fOs =  0; break;
  case DIRECTION_SOUTHWEST: rOs = -1; fOs = -1; break;
  case DIRECTION_WEST:      rOs =  0; fOs = -1; break;
  default: abort();
  }
  uint64_t ray;
  uint64_t next = bit(center);
  do {
    ray = next;
    next = offset(next, RANK * rOs);
    next = offset(next, fOs);
    switch (fOs) {
    case 1:  next &= ~RAY_A1A8; break;
    case -1: next &= ~RAY_H1H8; break;
    default: break;
    }
    next |= ray;
  } while (ray != next);
  ray = clear_bit(ray, center);
  return ray;
}

private int closest_blocker(uint64_t blockers_ray, int direction)
{
  if (blockers_ray == 0)
    return POSITION_INVALID;
  switch (direction) {
  case DIRECTION_NORTHWEST:
  case DIRECTION_NORTH:
  case DIRECTION_NORTHEAST:
  case DIRECTION_EAST:
    return lsb_first_set(blockers_ray);
  case DIRECTION_SOUTHEAST:
  case DIRECTION_SOUTH:
  case DIRECTION_SOUTHWEST:
  case DIRECTION_WEST:
    return msb_first_set(blockers_ray);
  default:
    abort();
  }  
}

private uint64_t shoot_ray_until_blocker(gamestate state, int idx, int direction)
{
  uint64_t pieces = all_pieces(state);
  uint64_t base_ray = mkRay(idx, direction);
  uint64_t blockers = base_ray & pieces;
  int blocker = closest_blocker(blockers, direction);
  if (blocker == POSITION_INVALID) {
    return base_ray;
  } else {
    uint64_t blocker_ray = mkRay(blocker, direction);
    uint64_t movable_squares_without_capture = base_ray ^ blocker_ray ^ bit(blocker);
    bool allow_capture = ! is_bit_set(state.current_player_bb, blocker);
    if (allow_capture)
      return movable_squares_without_capture | bit(blocker);
    else
      return movable_squares_without_capture;
  }
}

private uint64_t valid_bishop_moves(gamestate x, int idx)
{
  return
    shoot_ray_until_blocker(x, idx, DIRECTION_NORTHEAST) |
    shoot_ray_until_blocker(x, idx, DIRECTION_NORTHWEST) |
    shoot_ray_until_blocker(x, idx, DIRECTION_SOUTHEAST) |
    shoot_ray_until_blocker(x, idx, DIRECTION_SOUTHWEST)
    ;
}

private uint64_t valid_rook_moves(gamestate x, int idx)
{
  return
    shoot_ray_until_blocker(x, idx, DIRECTION_NORTH) |
    shoot_ray_until_blocker(x, idx, DIRECTION_WEST) |
    shoot_ray_until_blocker(x, idx, DIRECTION_EAST) |
    shoot_ray_until_blocker(x, idx, DIRECTION_SOUTH)
    ;
}

private uint64_t valid_king_moves(gamestate x, int idx)
{
  uint64_t ret =
    bit(move_direction(idx, DIRECTION_EAST)) |
    bit(move_direction(idx, DIRECTION_WEST)) |
    bit(move_direction(idx, DIRECTION_NORTH)) |
    bit(move_direction(idx, DIRECTION_SOUTH)) |
    bit(move_direction(idx, DIRECTION_NORTHEAST)) |
    bit(move_direction(idx, DIRECTION_SOUTHEAST)) |
    bit(move_direction(idx, DIRECTION_NORTHWEST)) |
    bit(move_direction(idx, DIRECTION_SOUTHWEST))
    ;
  ret &= ~ x.current_piece_bb;
  return ret;
}

private uint64_t valid_queen_moves(gamestate x, int idx)
{
  return
    valid_bishop_moves(x, idx) |
    valid_rook_moves(x, idx)
    ;
}

private int get_piece(gamestate x, int idx)
{
  if (is_bit_set(x.rooks_bb, idx))
    return PIECE_ROOK;
  if (is_bit_set(x.knights_bb, idx))
    return PIECE_KNIGHT;
  if (is_bit_set(x.bishops_bb, idx))
    return PIECE_BISHOP;
  if (is_bit_set(x.queens_bb, idx))
    return PIECE_QUEEN;
  if (is_bit_set(x.kings_bb, idx))
    return PIECE_KING;
  if (is_bit_set(x.pawns_bb, idx))
    return PIECE_PAWN;
  return PIECE_EMPTY;
}

private uint64_t valid_piece_moves(gamestate x, int idx)
{
  int piece = get_piece(x, idx);
  switch (piece) {
  case PIECE_ROOK:
    return valid_rook_moves(x, idx);
  case PIECE_KNIGHT:
    return valid_knight_moves(x, idx);
  case PIECE_BISHOP:
    return valid_bishop_moves(x, idx);
  case PIECE_QUEEN:
    return valid_queen_moves(x, idx);
  case PIECE_KING:
    return valid_king_moves(x, idx);
  case PIECE_PAWN:
    return valid_pawn_moves(x, idx);
  default:
    abort();
  }
}

private iterator reset_iterator_moves(gamestate g, iterator x)
{
  if (is_iterator_finished(x)) {
    return zerostate();
  } else {
    int idx = iterator_position(x);
    uint64_t moves = valid_piece_moves(g, idx);
    x.current_piece_bb = moves;
    return x;
  }
}


private iterator advance_iterator(gamestate g, iterator x)
{
  if (x.current_piece_bb) {
    x.current_piece_bb = advance_bb_iterator(x.current_piece_bb);
  }

  while (! is_iterator_finished(x) && ! x.current_piece_bb) {
    if (x.rooks_bb) {
      x.rooks_bb = advance_bb_iterator(x.rooks_bb);
    } else if (x.knights_bb) {
      x.knights_bb = advance_bb_iterator(x.knights_bb);
    } else if (x.bishops_bb) {
      x.bishops_bb = advance_bb_iterator(x.bishops_bb);
    } else if (x.queens_bb) {
      x.queens_bb = advance_bb_iterator(x.queens_bb);
    } else if (x.kings_bb) {
      x.kings_bb = advance_bb_iterator(x.kings_bb);
    } else if (x.pawns_bb) {
      x.pawns_bb = advance_bb_iterator(x.pawns_bb);
    }
    x = reset_iterator_moves(g, x);
  }
  if (is_iterator_finished(x) && x.current_piece_bb != 0) {
    abort();
  }
  
  return x;
}

private gamestate switch_sides(gamestate x)
{
  uint64_t enemy_pieces_bb = enemy_pieces(x);
  x.current_player_bb = enemy_pieces_bb;

  x.rooks_bb   = __builtin_bswap64(x.rooks_bb);
  x.knights_bb = __builtin_bswap64(x.knights_bb);
  x.bishops_bb = __builtin_bswap64(x.bishops_bb);
  x.queens_bb  = __builtin_bswap64(x.queens_bb);
  x.kings_bb   = __builtin_bswap64(x.kings_bb);
  x.pawns_bb   = __builtin_bswap64(x.pawns_bb);
  return x;
}
                 
private iterator mkIterator(gamestate g)
{
  iterator x = g;
  x.rooks_bb   &= x.current_player_bb;
  x.knights_bb &= x.current_player_bb;
  x.bishops_bb &= x.current_player_bb;
  x.queens_bb  &= x.current_player_bb;
  x.kings_bb   &= x.current_player_bb;
  x.pawns_bb   &= x.current_player_bb;

  x = reset_iterator_moves(g, x);
  if (! x.current_piece_bb) {
    x = advance_iterator(g, x);
  }
  
  return x;
}

private uint64_t all_rotations(uint64_t bb)
{
  bb |= rotate_bb(bb);
  bb |= rotate_bb(bb);
  bb |= rotate_bb(bb);
  return bb;
}

private gamestate new_game()
{
  gamestate x;
  x.rooks_bb =
    bit(mkPosition(0,0)) |
    bit(mkPosition(7,0)) |
    bit(mkPosition(0,7)) |
    bit(mkPosition(7,7));
  x.knights_bb =
    bit(mkPosition(1,0)) |
    bit(mkPosition(6,0)) |
    bit(mkPosition(1,7)) |
    bit(mkPosition(6,7));
  x.bishops_bb =
    bit(mkPosition(2,0)) |
    bit(mkPosition(5,0)) |
    bit(mkPosition(2,7)) |
    bit(mkPosition(5,7));
  x.queens_bb =
    bit(mkPosition(3,0)) |
    bit(mkPosition(4,7));
  x.kings_bb =
    bit(mkPosition(4,0)) |
    bit(mkPosition(3,7));
  x.pawns_bb =
    ((uint64_t)0xFF << RANK) |
    ((uint64_t)0xFF << (6*RANK));
  x.current_player_bb = 0xFFFF;
  x.en_passant_sq = POSITION_INVALID;
  return x;
}

// Flips the board so white is black and black is white.
// In our model, it is always white's turn to move.
private gamestate swap_board(gamestate g)
{
  g.current_player_bb ^= all_pieces(g);
  g.rooks_bb = __builtin_bswap64(g.rooks_bb);
  g.knights_bb = __builtin_bswap64(g.knights_bb);
  g.bishops_bb = __builtin_bswap64(g.bishops_bb);
  g.queens_bb = __builtin_bswap64(g.queens_bb);
  g.kings_bb = __builtin_bswap64(g.kings_bb);
  g.pawns_bb = __builtin_bswap64(g.pawns_bb);
  g.current_player_bb = __builtin_bswap64(g.current_player_bb);
  if (g.en_passant_sq != POSITION_INVALID) {
    g.en_passant_sq = mkPosition(file(g.en_passant_sq), 7 - rank(g.en_passant_sq));
  }
  return g;
}

private gamestate apply_move(gamestate x, move m)
{
  int from_piece = get_piece(x, m.from);
  int to_piece = get_piece(x, m.to);
  if (to_piece != PIECE_EMPTY) {
    uint64_t to_bb = get_piece_bb(x, to_piece);
    x = set_piece_bb(x, to_piece, clear_bit(to_bb, m.to));
  }
  // Capture the pawn properly during En Passant capture
  if (from_piece == PIECE_PAWN && m.to == x.en_passant_sq) {
    x.pawns_bb = clear_bit(x.pawns_bb, x.en_passant_sq - RANK);
  }
  // Set En Passant target square on double-jump
  if (from_piece == PIECE_PAWN && rank(m.to) - rank(m.from) == 2) {
    x.en_passant_sq = m.from + RANK;
  } else {
    x.en_passant_sq = POSITION_INVALID;
  }
  uint64_t from_bb = get_piece_bb(x, from_piece);
  x = set_piece_bb(x, from_piece, clear_bit(from_bb, m.from) | bit(m.to));

  // Set colors
  x.current_player_bb = clear_bit(x.current_player_bb, m.from) | bit(m.to);
  
  return x;
}

private uint64_t mkRank(int idx) { return ((uint64_t)0xFF << (idx*RANK)); }
private uint64_t mkFile(int idx) {
  return
    bit(mkPosition(idx,0)) |
    bit(mkPosition(idx,1)) |
    bit(mkPosition(idx,2)) |
    bit(mkPosition(idx,3)) |
    bit(mkPosition(idx,4)) |
    bit(mkPosition(idx,5)) |
    bit(mkPosition(idx,6)) |
    bit(mkPosition(idx,7));
}

private bool iter_lt(iterator x, iterator y)
{
  return
    (x.rooks_bb < y.rooks_bb ||
     (x.rooks_bb == y.rooks_bb &&
      (x.knights_bb < y.knights_bb ||
       (x.knights_bb == y.knights_bb &&
        (x.bishops_bb < y.bishops_bb ||
         (x.bishops_bb == y.bishops_bb &&
          (x.queens_bb < y.queens_bb ||
           (x.queens_bb == y.queens_bb &&
            (x.kings_bb < y.kings_bb ||
             (x.kings_bb == y.kings_bb &&
              (x.pawns_bb < y.pawns_bb ||
               (x.pawns_bb == y.pawns_bb &&
                (x.current_piece_bb < y.current_piece_bb)))))))))))));
}

uint64_t movepoints(gamestate g)
{
  uint64_t movepoints = 0;
  iterator i = mkIterator(g);
  while (! is_iterator_finished(i)) {
    movepoints |= i.current_piece_bb;
    i.current_piece_bb = 0;
    i = advance_iterator(g, i);
  }
  return movepoints;
}

uint64_t vulnerables(gamestate g)
{
  g = swap_board(g);
  uint64_t ret = movepoints(g) & enemy_pieces(g);
  ret = __builtin_bswap64(ret);
  return ret;
}

bool is_in_check(gamestate g)
{
  uint64_t white_kings = g.kings_bb & g.current_player_bb;
  return ((white_kings & vulnerables(g)) != 0);
}

// Like advance_iterator, but skip moves that leave the king in check
private iterator advance_iterator_legal(gamestate g, iterator i)
{
  while (1) {
    i = advance_iterator(g, i);
    if (is_iterator_finished(i)) {
      break;
    }
    move m = dereference_iterator(i);
    gamestate g2 = apply_move(g, m);
    if (! is_in_check(g2)) {
      break;
    }
  }
  return i;
}

private iterator mkLegalIterator(gamestate g)
{
  iterator i = mkIterator(g);
  move m = dereference_iterator(i);
  gamestate g2 = apply_move(g, m);
  if (is_in_check(g2)) {
    i = advance_iterator_legal(g, i);
  }
  return i;
}

private uint64_t legal_movepoints(gamestate g)
{
  uint64_t movepoints = 0;
  iterator i = mkLegalIterator(g);
  while (! is_iterator_finished(i)) {
    move m = dereference_iterator(i);
    movepoints |= bit(m.to);
    i = advance_iterator_legal(g, i);
  }
  return movepoints;
}

private int num_legal_moves(gamestate g)
{
  iterator i = mkLegalIterator(g);
  int count = 0;
  while (! is_iterator_finished(i)) {
    i = advance_iterator_legal(g, i);
    count++;
  }
  return count;
}

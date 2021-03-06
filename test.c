#define DEBUG 1
#define PRINT 1
#include "chess.c"
#include <stdio.h>

typedef void (*ft_action)(gamestate, move, int);

void print_iterator(iterator i)
{
  printf("ITERATOR\n");
  printf("== Promotion piece: %lu\n", i.promotion_piece);
  printf("== Current pieces\n");
  print_bitboard(i.current_piece_bb);
  printf("== Rooks\n");
  print_bitboard(i.rooks_bb);
  printf("== Knights\n");
  print_bitboard(i.knights_bb);
  printf("== Bishops\n");
  print_bitboard(i.bishops_bb);
  printf("== Queens\n");
  print_bitboard(i.queens_bb);
  printf("== Kings\n");
  print_bitboard(i.kings_bb);
  printf("== Pawns\n");
  print_bitboard(i.pawns_bb);
}

void print_move_stdout(move m)
{
  char b[7];
  print_move(m, b);
  printf("%s", b);
}

void print_gamestate(gamestate g)
{
  for (int r = 8; r --> 0;) {
    for (int f = 0; f < 8; f++) {
      int pos = mkPosition(f, r);
      int piece = get_piece(g, pos);
      bool is_white = is_bit_set(g.current_player_bb, pos);
      char c = piece_char(piece, is_white);
      putchar(c);
    }
    putchar('\n');
  }
  printf("Flags: ");
  if (! g.castle_flags) { putchar('-'); }
  else {
    if (g.castle_flags & CASTLE_WHITE_KINGSIDE) { putchar('K'); }
    if (g.castle_flags & CASTLE_WHITE_QUEENSIDE) { putchar('Q'); }
    if (g.castle_flags & CASTLE_BLACK_KINGSIDE) { putchar('k'); }
    if (g.castle_flags & CASTLE_BLACK_QUEENSIDE) { putchar('q'); }
  }
  putchar('\n');
}

uint64_t n;

void tick_counter(gamestate x, move m, int d) {
  if (d == 1) 
    n++;
}

void walk_game_tree(gamestate init, int depth, ft_action f)
{
  if (depth <= 0) {
    return;
  } else {
    iterator i = mkLegalIterator(init);
    while (! is_iterator_finished(i)) {
      move m = dereference_iterator(i);
      gamestate g = apply_move(init, m);

      f(init, m, depth);
      walk_game_tree(g, depth-1, f);
      iterator i2 = advance_iterator_legal(init, i);
      if (! iter_lt(i2, i)) {
        printf("%lu\n", n);
        print_iterator(i);
        print_iterator(i2);
        throw "derp";
      }
      i = i2;
    }
  }
}

void print_gamestate_action(gamestate g, move m, int d) { print_move_stdout(m); putchar('\n');}

void print_moves(gamestate g)
{
  walk_game_tree(g, 1, print_gamestate_action);
}

void assert_equal_string(const char* message, const char* x, const char *y)
{
  const char* orig_x = x, *orig_y = y;
  while (*x) {
    if (*x != *y)
      goto error;
    if (*x == 0)
      break;
    x++;
    y++;
  }
  return;
 error:
  printf("ASSERTION FAILURE: %s\n", message);
  printf("== expected: %s\n", orig_x);
  printf("== actual: %s\n", orig_y);
  throw "derp";
}

void assert_equal_bb(const char* message, uint64_t x, uint64_t y)
{
  if (x != y) {
    printf("ASSERTION FAILURE: %s\n", message);
    printf("== expected\n");
    print_bitboard(x);
    printf("== actual\n");
    print_bitboard(y);
    throw "derp";
  }
}
void assert_equal_int(const char* message, int x, int y)
{
  if (x != y) {
    printf("ASSERTION_FAILURE: %s\n", message);
    printf("%d != %d", x,y);
    throw "derp";
  }
}

void assert_equal_u64(const char* message, uint64_t x, uint64_t y)
{
  if (x != y) {
    printf("ASSERTION_FAILURE: %s\n", message);
    printf("%lu != %lu", x,y);
    throw "derp";
  }
}

void assert(const char* message, bool x)
{
  if (! x) {
    printf("ASSERTION_FAILURE: %s\n", message);
    throw "derp";
  }
}

void test_ray()
{
  // Base rays
  {
    uint64_t center = mkPosition(3,3);
    uint64_t nw_ray =
      bit(mkPosition(0,6)) |
      bit(mkPosition(1,5)) |
      bit(mkPosition(2,4));
    uint64_t n_ray =
      bit(mkPosition(3,4)) |
      bit(mkPosition(3,5)) |
      bit(mkPosition(3,6)) |
      bit(mkPosition(3,7));
    uint64_t ne_ray =
      bit(mkPosition(4,4)) |
      bit(mkPosition(5,5)) |
      bit(mkPosition(6,6)) |
      bit(mkPosition(7,7));
    uint64_t e_ray =
      bit(mkPosition(4,3)) |
      bit(mkPosition(5,3)) |
      bit(mkPosition(6,3)) |
      bit(mkPosition(7,3));
    uint64_t se_ray =
      bit(mkPosition(4,2)) |
      bit(mkPosition(5,1)) |
      bit(mkPosition(6,0));
    uint64_t s_ray =
      bit(mkPosition(3,2)) |
      bit(mkPosition(3,1)) |
      bit(mkPosition(3,0));
    uint64_t sw_ray =
      bit(mkPosition(2,2)) |
      bit(mkPosition(1,1)) |
      bit(mkPosition(0,0));
    uint64_t w_ray =
      bit(mkPosition(2,3)) |
      bit(mkPosition(1,3))|
      bit(mkPosition(0,3));

    assert_equal_bb("nw", nw_ray, mkRay(center, DIRECTION_NORTHWEST));
    assert_equal_bb("n",  n_ray,  mkRay(center, DIRECTION_NORTH));
    assert_equal_bb("ne", ne_ray, mkRay(center, DIRECTION_NORTHEAST));
    assert_equal_bb("e",  e_ray,  mkRay(center, DIRECTION_EAST));
    assert_equal_bb("se", se_ray, mkRay(center, DIRECTION_SOUTHEAST));
    assert_equal_bb("s",  s_ray,  mkRay(center, DIRECTION_SOUTH));
    assert_equal_bb("sw", sw_ray, mkRay(center, DIRECTION_SOUTHWEST));
    assert_equal_bb("w",  w_ray,  mkRay(center, DIRECTION_WEST));
  }
  // Rays with blockers
  {
    uint64_t center = mkPosition(3,3);
    // W
    {
      gamestate g = zerostate();
      g.pawns_bb =
        bit(mkPosition(1,3)) |
        bit(mkPosition(0,3));
      uint64_t expected =
        bit(mkPosition(2,3)) |
        bit(mkPosition(1,3));
      /* printf("== expected:\n"); print_bitboard(expected); printf("\n"); */
      uint64_t actual = shoot_ray_until_blocker(g, center, DIRECTION_WEST);
      /* printf("== actual:\n"); print_bitboard(actual); printf("\n"); */
      /* uint64_t base_ray = mkRay(center, DIRECTION_WEST); */
      /* printf("== base_ray:\n"); print_bitboard(base_ray); printf("\n"); */
      /* uint64_t blockers = base_ray & all_pieces(g); */
      /* printf("== blockers:\n"); print_bitboard(blockers); printf("\n"); */
      /* int blocker = closest_blocker(blockers, DIRECTION_WEST); */
      /* printf("== blocker: "); print_pos(blocker); printf("\n"); */
      /* uint64_t blocker_ray = mkRay(blocker, DIRECTION_WEST); */
      /* printf("== blocker_ray:\n"); print_bitboard(blocker_ray); */
      /* uint64_t movable_squares_without_capture = base_ray ^ blocker_ray ^ bit(blocker); */
      /* printf("== movable_squares_without_capture\n"); print_bitboard(movable_squares_without_capture); printf("\n"); */

      assert_equal_int("w_blocker", mkPosition(1,3), closest_blocker(g.pawns_bb, DIRECTION_WEST));
      assert_equal_bb("w", expected, actual);
    }
    // NW
    {
      gamestate g = zerostate();
      g.pawns_bb =
        bit(mkPosition(1,5)) |
        bit(mkPosition(0,6));
      uint64_t expected =
        bit(mkPosition(2,4)) |
        bit(mkPosition(1,5));
      uint64_t actual = shoot_ray_until_blocker(g, center, DIRECTION_NORTHWEST);
      assert_equal_int("nw_blocker", mkPosition(1,5), closest_blocker(g.pawns_bb, DIRECTION_NORTHWEST));
      assert_equal_bb("nw", expected, actual);
    }
    // N
    {
      gamestate g = zerostate();
      g.pawns_bb =
        bit(mkPosition(3,6)) |
        bit(mkPosition(3,7));
      uint64_t expected =
        bit(mkPosition(3,4)) |
        bit(mkPosition(3,5)) |
        bit(mkPosition(3,6));
      uint64_t actual = shoot_ray_until_blocker(g, center, DIRECTION_NORTH);
      assert_equal_int("n_blocker", mkPosition(3,6), closest_blocker(g.pawns_bb, DIRECTION_NORTH));
      assert_equal_bb("n", expected, actual);
    }
    // NE
    {
      gamestate g = zerostate();
      g.pawns_bb =
        bit(mkPosition(6,6)) |
        bit(mkPosition(7,7));
      uint64_t expected =
        bit(mkPosition(4,4)) |
        bit(mkPosition(5,5)) |
        bit(mkPosition(6,6));
      uint64_t actual = shoot_ray_until_blocker(g, center, DIRECTION_NORTHEAST);
      assert_equal_int("ne_blocker", mkPosition(6,6), closest_blocker(g.pawns_bb, DIRECTION_NORTHEAST));
      assert_equal_bb("ne", expected, actual);
    }
    // NE 2
    {
      uint64_t center = mkPosition(0,2);
      gamestate g = zerostate();
      g.pawns_bb =
        bit(mkPosition(4,6)) |
        bit(mkPosition(5,7));
      uint64_t expected =
        bit(mkPosition(1,3)) |
        bit(mkPosition(2,4)) |
        bit(mkPosition(3,5)) |
        bit(mkPosition(4,6));
      uint64_t actual = shoot_ray_until_blocker(g, center, DIRECTION_NORTHEAST);
      assert_equal_bb("ne_2", expected, actual);
    }
    // NE 3
    {
      uint64_t bb = bit(mkPosition(4,6)) | bit(mkPosition(5,7));
      int i = closest_blocker(bb, DIRECTION_NORTHEAST);
      assert_equal_int("ne_3", mkPosition(4,6), i);
    }
    // E
    {
      gamestate g = zerostate();
      g.pawns_bb =
        bit(mkPosition(6,3)) |
        bit(mkPosition(7,3));
      uint64_t expected =
        bit(mkPosition(4,3)) |
        bit(mkPosition(5,3)) |
        bit(mkPosition(6,3)); 
      uint64_t actual = shoot_ray_until_blocker(g, center, DIRECTION_EAST);
      assert_equal_int("e_blocker", mkPosition(6,3), closest_blocker(g.pawns_bb, DIRECTION_EAST));
      assert_equal_bb("e", expected, actual);
    }
    // SE
    {
      gamestate g = zerostate();
      g.pawns_bb =
        bit(mkPosition(5,1)) |
        bit(mkPosition(6,0));
      uint64_t expected =
        bit(mkPosition(4,2)) |
        bit(mkPosition(5,1));
      uint64_t actual = shoot_ray_until_blocker(g, center, DIRECTION_SOUTHEAST);
      assert_equal_int("se_blocker", mkPosition(5,1), closest_blocker(g.pawns_bb, DIRECTION_SOUTHEAST));
      assert_equal_bb("se", expected, actual);
    }
    // S
    {
      gamestate g = zerostate();
      g.pawns_bb =
        bit(mkPosition(3,1)) |
        bit(mkPosition(3,0));
      uint64_t expected =
        bit(mkPosition(3,2)) |
        bit(mkPosition(3,1));
      uint64_t actual = shoot_ray_until_blocker(g, center, DIRECTION_SOUTH);
      assert_equal_int("s_blocker", mkPosition(3,1), closest_blocker(g.pawns_bb, DIRECTION_SOUTH));
      assert_equal_bb("s", expected, actual);
    }
    // SW
    {
      gamestate g = zerostate();
      g.pawns_bb =
        bit(mkPosition(1,1)) |
        bit(mkPosition(0,0));
      uint64_t expected =
        bit(mkPosition(2,2)) |
        bit(mkPosition(1,1));
      uint64_t actual = shoot_ray_until_blocker(g, center, DIRECTION_SOUTHWEST);
      assert_equal_int("sw_blocker", mkPosition(1,1), closest_blocker(g.pawns_bb, DIRECTION_SOUTHWEST));
      assert_equal_bb("sw", expected, actual);
    }
  }
}

void test_rook()
{
  // Rook in middle
  {
    uint64_t center = mkPosition(3,3);
    gamestate g = zerostate();
    g.rooks_bb = bit(center);
    g.current_piece_bb = g.rooks_bb;
    
    uint64_t expected = clear_bit(mkRank(3) | mkFile(3), center);
    uint64_t actual = valid_rook_moves(g, center);
    assert_equal_bb("test_rook_middle", expected, actual);
  }
  // Rook in corner
  {
    uint64_t center = mkPosition(0,0);
    gamestate g = zerostate();
    g.rooks_bb = bit(center);
    g.current_piece_bb = g.rooks_bb;
    
    uint64_t expected = clear_bit(mkRank(0) | mkFile(0), center);
    uint64_t actual = valid_rook_moves(g, center);
    assert_equal_bb("test_rook_corner", expected, actual);
  }
  // Rook blocked in left corner
  {
    uint64_t center = mkPosition(0,0);
    gamestate g = zerostate();
    g.rooks_bb = bit(center) | bit(mkPosition(0,1)) | bit(mkPosition(1,0));
    g.current_piece_bb = g.rooks_bb;

    assert_equal_bb("test_rook_blocked_1", 0, valid_rook_moves(g, center));
  }
  // Rook blocked in right corner
  {
    uint64_t center = mkPosition(7,0);
    gamestate g = zerostate();
    g.rooks_bb = bit(center) | bit(mkPosition(7,1)) | bit(mkPosition(6,0));
    g.current_piece_bb = g.rooks_bb;

    assert_equal_bb("test_rook_blocked_2", 0, valid_rook_moves(g, center));
  }
}

void test_pawn()
{
  // Left edge
  {
    uint64_t center = mkPosition(0,1);
    gamestate g = zerostate();
    g.pawns_bb = bit(center);
    g.current_piece_bb = g.pawns_bb;

    uint64_t expected = bit(mkPosition(0,2)) | bit(mkPosition(0,3));
    uint64_t actual = valid_pawn_moves(g, center);
    assert_equal_bb("test_pawn_1", expected, actual);

  }
  // Right edge
  {
    uint64_t center = mkPosition(7,1);
    gamestate g = zerostate();
    g.pawns_bb = bit(center);
    g.current_piece_bb = g.pawns_bb;

    uint64_t expected = bit(mkPosition(7,2)) | bit(mkPosition(7,3));
    uint64_t actual = valid_pawn_moves(g, center);
    assert_equal_bb("test_pawn_2", expected, actual);
  }
  // Middle
  {
    uint64_t center = mkPosition(3,1);
    gamestate g = zerostate();
    g.pawns_bb = bit(center);
    g.current_piece_bb = g.pawns_bb;

    uint64_t expected = bit(mkPosition(3,2)) | bit(mkPosition(3,3));
    uint64_t actual = valid_pawn_moves(g, center);
    assert_equal_bb("test_pawn_3", expected, actual);
  }
  // Capture
  {
    uint64_t center = mkPosition(7,1);
    gamestate g = zerostate();
    g.pawns_bb = bit(center);
    g.current_piece_bb = g.pawns_bb;
    g.pawns_bb |= bit(move_direction(center, DIRECTION_NORTHWEST));
    
    uint64_t expected = bit(mkPosition(7,2)) | bit(mkPosition(7,3)) | bit(mkPosition(6,2));
    uint64_t actual = valid_pawn_moves(g, center);
    assert_equal_bb("test_pawn_4", expected, actual);
  }
  // Blocked by another piece
  {
    uint64_t center = mkPosition(0,1);
    gamestate g = zerostate();
    g.pawns_bb = bit(center) | bit(center + RANK);
    g.current_piece_bb = g.pawns_bb;
    
    uint64_t expected = 0;
    uint64_t actual = valid_pawn_moves(g, center);
    assert_equal_bb("test_pawn_5", expected, actual);
  }
  // Blocked by another piece when double-jumping
  {
    uint64_t center = mkPosition(0,1);
    gamestate g = zerostate();
    g.pawns_bb = bit(center);
    g.current_piece_bb = g.pawns_bb;
    g.pawns_bb |=  bit(mkPosition(0,3));
    
    uint64_t expected = bit(mkPosition(0,2));
    uint64_t actual = valid_pawn_moves(g, center);
    assert_equal_bb("test_pawn_6", expected, actual);
  }
  // En Passant
  {
    gamestate g = zerostate();
    g.pawns_bb =
      bit(mkPosition(0,1)) |
      bit(mkPosition(1,3));
    g.current_piece_bb = bit(mkPosition(0,1));

    iterator i = mkIterator(g);
    {
      uint64_t expected =
        bit(mkPosition(0,2)) |
        bit(mkPosition(0,3));
      assert_equal_bb("test_pawn_en_passant_1", expected, i.current_piece_bb);
    }
    // Double jump allows en passant
    move m; m.from = mkPosition(0,1); m.to = mkPosition(0,3);
    g = apply_move(g, m);
    i = mkIterator(g);
    {
      uint64_t expected =
        bit(mkPosition(0,5)) |
        bit(mkPosition(1,5));
      assert_equal_int("test_pawn_en_passant_target", mkPosition(0,5), g.en_passant_sq);
      assert_equal_bb("test_pawn_en_passant_2", expected, i.current_piece_bb);
    }
    // The actual capture
    move m2; m2.from = mkPosition(1,3); m2.to = mkPosition(0,2);
    g = apply_move(g, swap_move(m2));
    {
      assert_equal_int("test_pawn_en_passant_target_2", POSITION_INVALID, g.en_passant_sq);
      assert_equal_bb("test_pawn_en_passant_3", bit(mkPosition(0,2)), g.pawns_bb);
    }
  }
}

void test_iterator()
{
  // Piece has valid moves
  {
    uint64_t center = mkPosition(0,0);
    gamestate g = zerostate();
    g.rooks_bb = bit(center);
    g.current_piece_bb = g.rooks_bb;

    uint64_t expected =
      mkRay(center, DIRECTION_NORTH) |
      mkRay(center, DIRECTION_EAST);
    iterator i = mkIterator(g);
    uint64_t actual = i.current_piece_bb;
    assert_equal_bb("test_iterator_1", expected, actual);
  }
  // Piece has no valid moves; skip to next
  {
    uint64_t center = mkPosition(0,0);
    gamestate g = zerostate();
    g.pawns_bb = bit(mkPosition(0,1));
    g.rooks_bb = bit(center);
    g.knights_bb = bit(mkPosition(1,0));
    g.current_piece_bb = all_pieces(g);

    iterator i = mkIterator(g);    
    assert_equal_bb("test_iterator_2_rooks", 0, i.rooks_bb);
    assert_equal_bb("test_iterator_2_knights", bit(mkPosition(1,0)), i.knights_bb);
    uint64_t expected =
      bit(mkPosition(0,2)) |
      bit(mkPosition(2,2)) |
      bit(mkPosition(3,1));
    assert_equal_bb("test_iterator_2_knight_moves", expected, i.current_piece_bb);
  }
  // Two pieces have no valid moves; skip to next
  {
    gamestate g = zerostate();
    g.pawns_bb =
      bit(mkPosition(0,1)) |
      bit(mkPosition(7,1));
    g.rooks_bb =
      bit(mkPosition(0,0)) |
      bit(mkPosition(7,0));
    g.knights_bb =
      bit(mkPosition(1,0)) |
      bit(mkPosition(6,0));
    g.current_piece_bb = all_pieces(g);
    iterator i = mkIterator(g);
    assert_equal_bb("test_iterator_3_rooks", 0, i.rooks_bb);
    assert_equal_bb("test_iterator_3_knights", bit(mkPosition(1,0)) | bit(mkPosition(6,0)), i.knights_bb);
    uint64_t expected = valid_knight_moves(g, mkPosition(1,0));
    assert_equal_bb("test_iterator_3_knight_moves", expected, i.current_piece_bb);
      
  }
  // Empty iterator is finished
  {
    gamestate g = zerostate();
    iterator i = zerostate();
    assert("test_iterator_empty_1", is_iterator_finished(i));
    iterator j = advance_iterator(g, i);
    assert("test_iterator_empty_2", is_iterator_finished(j));
  }
}
    
void test_knight()
{
  // Knight in left bottom
  {
    uint64_t center = mkPosition(1,0);
    gamestate g = zerostate();
    g.knights_bb = bit(center);
    g.current_piece_bb = g.knights_bb;

    uint64_t expected =
      bit(mkPosition(0,2)) |
      bit(mkPosition(2,2)) |
      bit(mkPosition(3,1));
    uint64_t actual = valid_knight_moves(g, center);
    assert_equal_bb("test_knight_1", expected, actual);
  }
  // Knight in right bottom
  {
    uint64_t center = mkPosition(6,0);
    gamestate g = zerostate();
    g.knights_bb = bit(center);
    g.current_piece_bb = g.knights_bb;

    uint64_t expected =
      bit(mkPosition(7,2)) |
      bit(mkPosition(5,2)) |
      bit(mkPosition(4,1));
    uint64_t actual = valid_knight_moves(g, center);
    assert_equal_bb("test_knight_2", expected, actual);
  }
}

void test_king()
{
  // King in middle
  {
    uint64_t center = mkPosition(3,3);
    gamestate g = zerostate();
    g.kings_bb = bit(center);
    g.current_piece_bb = g.kings_bb;

    uint64_t expected =
      bit(mkPosition(4,3)) |
      bit(mkPosition(4,4)) |
      bit(mkPosition(3,4)) |
      bit(mkPosition(2,4)) |
      bit(mkPosition(2,3)) |
      bit(mkPosition(2,2)) |
      bit(mkPosition(3,2)) |
      bit(mkPosition(4,2));
    uint64_t actual = valid_king_moves(g, center);
    assert_equal_bb("test_king_1", expected, actual);
  }
  // King blocked on 3 sides in left corner
  {
    uint64_t center = mkPosition(0,0);
    gamestate g = zerostate();
    g.kings_bb = bit(center);
    g.pawns_bb =
      bit(mkPosition(1,0)) |
      bit(mkPosition(1,1)) |
      bit(mkPosition(0,1));
    g.current_piece_bb = all_pieces(g);

    uint64_t expected = 0;
    uint64_t actual = valid_king_moves(g, center);
    assert_equal_bb("test_king_2", expected, actual);
  }
}

void test_bishop()
{
  // Bishop southwest blocker
  {
    gamestate g = zerostate();
    g.pawns_bb = bit(mkPosition(3,1));
    g.kings_bb = bit(mkPosition(2,0));
    g.current_piece_bb = bit(mkPosition(7,5));
    g.bishops_bb = bit(mkPosition(7,5));

    {
      uint64_t expected =
        bit(mkPosition(6,6)) |
        bit(mkPosition(5,7)) |
        bit(mkPosition(6,4)) |
        bit(mkPosition(5,3)) |
        bit(mkPosition(4,2)) |
        bit(mkPosition(3,1));
      uint64_t actual = valid_bishop_moves(g, mkPosition(7,5));
      assert_equal_bb("test_bishop_1", expected, actual);
    }
    // King not in check from bishop
    g.current_piece_bb ^= all_pieces(g);
    assert("test_bishop_1_check", ! is_in_check(g));
  }
  // Bishop northeast blocker
  {
    gamestate g = zerostate();
    g.pawns_bb = bit(mkPosition(3,1));
    g.kings_bb = bit(mkPosition(2,0));
    g.current_piece_bb = all_pieces(g);
    g.bishops_bb = bit(mkPosition(7,5));
    {
      uint64_t expected =
        bit(mkPosition(6,6)) |
        bit(mkPosition(5,7)) |
        bit(mkPosition(6,4)) |
        bit(mkPosition(5,3)) |
        bit(mkPosition(4,2));
      uint64_t actual = valid_bishop_moves(g, mkPosition(7,5));
      assert_equal_bb("test_bishop_2", expected, actual);
    }
    // King not in check from bishop
    assert("test_bishop_2_check", ! is_in_check(g));
  } 
}

void test_apply_move()
{
  // Knight in middle
  {
    uint64_t center = mkPosition(3,3);
    gamestate g = zerostate();
    g.knights_bb = bit(center);
    g.current_piece_bb = g.knights_bb;

    iterator i = mkIterator(g);
    move m = dereference_iterator(i);
    assert_equal_int("test_apply_move_from", center, m.from);
    assert_equal_int("test_apply_move_to", mkPosition(2,1), m.to);
    gamestate g2 = swap_board(apply_move(g, m));

    assert_equal_bb("test_apply_move", bit(mkPosition(2,1)), g2.knights_bb);
  }
  // Doesn't affect other pieces
  {
    uint64_t center = mkPosition(3,3);
    gamestate g = zerostate();
    g.knights_bb = bit(center);
    g.current_piece_bb = g.knights_bb;
    g.knights_bb |=
      bit(mkPosition(1,0)) |
      bit(mkPosition(6,0));

    iterator i = mkIterator(g);
    move m = dereference_iterator(i);
    assert_equal_int("test_apply_move_from", center, m.from);
    assert_equal_int("test_apply_move_to", mkPosition(2,1), m.to);
    gamestate g2 = swap_board(apply_move(g, m));

    uint64_t expected = bit(m.to) | bit(mkPosition(1,0)) | bit(mkPosition(6,0));
    assert_equal_bb("test_apply_move_2", expected, g2.knights_bb);
  }
}

void test_check()
{
  // King in check 1
  {
    uint64_t center = mkPosition(3,3);
    gamestate g = zerostate();
    g.kings_bb = bit(center);
    g.rooks_bb = bit(center+2);
    g.current_player_bb = g.kings_bb;

    assert("test_check_1", is_in_check(g));
  }
  // King in check 2
  {
    uint64_t center = mkPosition(3,3);
    gamestate g = zerostate();
    g.kings_bb = bit(center);
    g.rooks_bb = bit(center-2*RANK);
    g.current_player_bb = g.kings_bb;

    assert("test_check_2", is_in_check(g));
  }
  // King blocked by rooks
  {
    uint64_t center = mkPosition(3,3);
    gamestate g = zerostate();
    g.kings_bb = bit(center);
    g.rooks_bb =
      bit(mkPosition(0,4)) |
      bit(mkPosition(0,2)) |
      bit(mkPosition(2,0));
    //bit(mkPosition(0,2));
    g.current_player_bb = g.kings_bb;
    assert_equal_int("test_check_3", 1, num_legal_moves(g));
  }
  // King can safely move to a square protected by own unit.
  {
    gamestate g = zerostate();
    g.pawns_bb =
      bit(mkPosition(3,1)) |
      bit(mkPosition(4,0)) |
      bit(mkPosition(4,1)) |
      bit(mkPosition(2,1));
    g.kings_bb = bit(mkPosition(3,0));
    g.bishops_bb =
      bit(mkPosition(0,2)) |
      bit(mkPosition(0,5));
    g.knights_bb =
      bit(mkPosition(1,0)) |
      bit(mkPosition(6,0));
    g.current_piece_bb = all_pieces(g);
    g.bishops_bb |= bit(mkPosition(7,5));
    {
      uint64_t expected =
        bit(mkPosition(6,6)) |
        bit(mkPosition(5,7)) |
        bit(mkPosition(6,4)) |
        bit(mkPosition(5,3)) |
        bit(mkPosition(4,2));
      uint64_t actual = valid_bishop_moves(g, mkPosition(7,5));
      assert_equal_bb("test_check_4_bishop", expected, actual);
    }
    // King not in check from bishop
    assert("test_check_4_check", ! is_in_check(g));
    {
      int from = mkPosition(3,0);
      int target = mkPosition(2,0);
      uint64_t expected = bit(target);
      uint64_t actual = piece_legal_movepoints(g, from);
      assert_equal_bb("test_check_4_king_valid", bit(target), valid_king_moves(g, from));
      iterator i = zerostate();
      int piece = get_piece(g, from);
      i = set_piece_bb(i, piece, bit(from));
      i = reset_iterator_moves(g,i);
      i = legalize(g,i);

      assert_equal_bb("test_check_4_next", bit(target), i.current_piece_bb);
      assert_equal_bb("test_check_4_moves", expected, actual);
    }
  }
}

void test_castling()
{
  // Kingside castle 1
  {
    gamestate g = zerostate();
    g.kings_bb = bit(mkPosition(4,0));
    g.rooks_bb = bit(mkPosition(7,0));
    g.castle_flags = CASTLE_WHITE_KINGSIDE;
    g.current_player_bb = all_pieces(g);

    uint64_t expected =
      bit(mkPosition(3,0)) |
      bit(mkPosition(3,1)) |
      bit(mkPosition(4,1)) |
      bit(mkPosition(5,1)) |
      bit(mkPosition(5,0)) |
      bit(mkPosition(6,0));
    uint64_t actual = valid_king_moves(g, mkPosition(4,0));
    assert_equal_bb("test_castling_1_moves", expected, actual);
    move m; m.from = mkPosition(4,0); m.to = CASTLE_KINGSIDE_KPOS;
    gamestate g2 = swap_board(apply_move(g, m));
    assert_equal_bb("test_castling_1_king", bit(CASTLE_KINGSIDE_KPOS), g2.kings_bb);
    assert_equal_bb("test_castling_1_rook", bit(CASTLE_KINGSIDE_RPOS), g2.rooks_bb);
    assert_equal_u64("test_castling_1_flags", 0, g2.castle_flags);
  }
  // Queenside castle 1
  {
    gamestate g = zerostate();
    g.kings_bb = bit(mkPosition(4,0));
    g.rooks_bb = bit(mkPosition(0,0));
    g.castle_flags = CASTLE_WHITE_QUEENSIDE;
    g.current_player_bb = all_pieces(g);

    uint64_t expected =
      bit(mkPosition(3,0)) |
      bit(mkPosition(3,1)) |
      bit(mkPosition(4,1)) |
      bit(mkPosition(5,1)) |
      bit(mkPosition(5,0)) |
      bit(mkPosition(2,0));
    uint64_t actual = valid_king_moves(g, mkPosition(4,0));
    assert_equal_bb("test_castling_2_moves", expected, actual);
    move m; m.from = mkPosition(4,0); m.to = mkPosition(2,0);
    gamestate g2 = swap_board(apply_move(g, m));
    assert_equal_bb("test_castling_2_king", bit(mkPosition(2,0)), g2.kings_bb);
    assert_equal_bb("test_castling_2_rook", bit(mkPosition(3,0)), g2.rooks_bb);
    assert_equal_u64("test_castling_2_flags", 0, g2.castle_flags);
  }
  // Kingside castle blocked by check 1
  {
    gamestate g = zerostate();
    g.kings_bb = bit(mkPosition(4,0));
    g.rooks_bb = bit(mkPosition(7,0));
    g.castle_flags = CASTLE_WHITE_KINGSIDE;
    g.current_player_bb = all_pieces(g);
    g.rooks_bb |= bit(mkPosition(5,7));

    uint64_t expected =
      bit(mkPosition(3,0)) |
      bit(mkPosition(3,1)) |
      bit(mkPosition(4,1));
    uint64_t actual = piece_legal_movepoints(g, mkPosition(4,0));
    move not_castle; not_castle.from = mkPosition(4,0); not_castle.to = mkPosition(4,1);
    move castle_kingside; castle_kingside.from = mkPosition(4,0); castle_kingside.to = mkPosition(6,0);
    assert("test_castling_3_not_kingside_1", ! is_kingside_castle(g, not_castle));
    assert("test_castling_3_not_kingside_2", ! is_queenside_castle(g, not_castle));
    assert("test_castling_3_not_kingside_3", ! results_in_check(g, not_castle));
    assert("test_castling_3_not_kingside_4", is_legal(g, not_castle));
    assert("test_castling_3_illegal", ! is_legal(g, castle_kingside));
    assert_equal_bb("test_castling_3_moves", expected, actual);
  }
  // Queenside castle blocked by check 1
  {
    gamestate g = zerostate();
    g.kings_bb = bit(mkPosition(4,0));
    g.rooks_bb = bit(mkPosition(0,0));
    g.castle_flags = CASTLE_WHITE_QUEENSIDE;
    g.current_player_bb = all_pieces(g);
    g.rooks_bb |= bit(mkPosition(3,7));

    uint64_t expected =
      bit(mkPosition(4,1)) |
      bit(mkPosition(5,1)) |
      bit(mkPosition(5,0));
    uint64_t actual = piece_legal_movepoints(g, mkPosition(4,0));
    assert_equal_bb("test_castling_4_moves", expected, actual);
  }
  // Kingside castle blocked by check 2
  {
    gamestate g = zerostate();
    g.kings_bb = bit(mkPosition(4,0));
    g.rooks_bb = bit(mkPosition(7,0));
    g.castle_flags = CASTLE_WHITE_KINGSIDE;
    g.current_player_bb = all_pieces(g);
    g.rooks_bb |= bit(mkPosition(4,7));

    uint64_t expected =
      bit(mkPosition(3,0)) |
      bit(mkPosition(3,1)) |
      bit(mkPosition(5,1)) |
      bit(mkPosition(5,0));
    uint64_t actual = piece_legal_movepoints(g, mkPosition(4,0));
    assert_equal_bb("test_castling_5_moves", expected, actual);
  }
  // Queenside castle blocked by check 2
  {
    gamestate g = zerostate();
    g.kings_bb = bit(mkPosition(4,0));
    g.rooks_bb = bit(mkPosition(0,0));
    g.castle_flags = CASTLE_WHITE_QUEENSIDE;
    g.current_player_bb = all_pieces(g);
    g.rooks_bb |= bit(mkPosition(4,7));

    uint64_t expected =
      bit(mkPosition(3,0)) |
      bit(mkPosition(3,1)) |
      bit(mkPosition(5,1)) |
      bit(mkPosition(5,0));
    uint64_t actual = piece_legal_movepoints(g, mkPosition(4,0));
    assert_equal_bb("test_castling_6_moves", expected, actual);
  }
  // Kingside castle blocked by unit
  {
    gamestate g = zerostate();
    g.kings_bb = bit(mkPosition(4,0));
    g.rooks_bb = bit(mkPosition(7,0));
    g.bishops_bb |= bit(mkPosition(5,0));
    g.castle_flags = CASTLE_WHITE_KINGSIDE;
    g.current_player_bb = all_pieces(g);

    uint64_t expected =
      bit(mkPosition(3,0)) |
      bit(mkPosition(3,1)) |
      bit(mkPosition(4,1)) |
      bit(mkPosition(5,1));
    uint64_t actual = valid_king_moves(g, mkPosition(4,0));
    assert_equal_bb("test_castling_7_moves", expected, actual);
  }
  // Queenside castle blocked by unit
  {
    gamestate g = zerostate();
    g.kings_bb = bit(mkPosition(4,0));
    g.rooks_bb = bit(mkPosition(0,0));
    g.bishops_bb = bit(mkPosition(3,0));
    g.castle_flags = CASTLE_WHITE_QUEENSIDE;
    g.current_player_bb = all_pieces(g);

    uint64_t expected =
      bit(mkPosition(3,1)) |
      bit(mkPosition(4,1)) |
      bit(mkPosition(5,1)) |
      bit(mkPosition(5,0));
    uint64_t actual = valid_king_moves(g, mkPosition(4,0));
    assert_equal_bb("test_castling_8_moves", expected, actual);
  }
  // Castle flags swap
  {
    {
      gamestate g = zerostate();
      g.castle_flags = CASTLE_WHITE_KINGSIDE;
      g = swap_board(g);
      assert_equal_u64("test_castling_9_flag_wk", CASTLE_BLACK_KINGSIDE, g.castle_flags);
    }
    {
      gamestate g = zerostate();
      g.castle_flags = CASTLE_WHITE_QUEENSIDE;
      g = swap_board(g);
      assert_equal_u64("test_castling_9_flag_wk", CASTLE_BLACK_QUEENSIDE, g.castle_flags);
    }
    {
      gamestate g = zerostate();
      g.castle_flags = CASTLE_BLACK_KINGSIDE;
      g = swap_board(g);
      assert_equal_u64("test_castling_9_flag_wk", CASTLE_WHITE_KINGSIDE, g.castle_flags);
    }
    {
      gamestate g = zerostate();
      g.castle_flags = CASTLE_BLACK_QUEENSIDE;
      g = swap_board(g);
      assert_equal_u64("test_castling_9_flag_wk", CASTLE_WHITE_QUEENSIDE, g.castle_flags);
    }
  }
  // Moving kingside rook deletes flag
  {
    gamestate g = zerostate();
    g.rooks_bb = bit(mkPosition(7,0));
    g.kings_bb = bit(mkPosition(4,0));
    g.castle_flags = CASTLE_WHITE_KINGSIDE;
    move m; m.from = mkPosition(7,0); m.to = mkPosition(6,0);
    gamestate g2 = apply_move(g, m);
    assert_equal_u64("test_castling_10_flag_kingrook_delete", 0, g2.castle_flags);
  }
  // Moving queenside rook deletes flag
  {
    gamestate g = zerostate();
    g.rooks_bb = bit(mkPosition(0,0));
    g.kings_bb = bit(mkPosition(4,0));
    g.castle_flags = CASTLE_WHITE_QUEENSIDE;
    move m; m.from = mkPosition(0,0); m.to = mkPosition(2,0);
    gamestate g2 = apply_move(g, m);
    assert_equal_u64("test_castling_11_flag_queenrook_delete", 0, g2.castle_flags);
  }
  // Moving king deletes both flags
  {
    gamestate g = zerostate();
    g.kings_bb = bit(mkPosition(4,0));
    g.rooks_bb =
      bit(mkPosition(7,0)) |
      bit(mkPosition(0,0));
    g.castle_flags = CASTLE_WHITE_KINGSIDE | CASTLE_WHITE_QUEENSIDE;
    move m; m.from = mkPosition(4,0); m.to = mkPosition(5,0);
    gamestate g2 = apply_move(g, m);
    assert_equal_u64("test_castling_12_flag_king_delete", 0, g2.castle_flags);
  }
  // Can't castle with unit blocking rook queenside
  {
    gamestate g = zerostate();
    g.kings_bb = bit(mkPosition(4,0));
    g.rooks_bb = bit(mkPosition(0,0));
    g.castle_flags = CASTLE_WHITE_QUEENSIDE;
    g.knights_bb = bit(mkPosition(1,0));
    g.current_piece_bb = all_pieces(g);

    uint64_t expected =
      bit(mkPosition(3,0)) |
      bit(mkPosition(3,1)) |
      bit(mkPosition(4,1)) |
      bit(mkPosition(5,1)) |
      bit(mkPosition(5,0));
    uint64_t actual = piece_legal_movepoints(g, mkPosition(4,0));
    assert_equal_bb("test_castling_13_knight_blocks_rook_queenside", expected, actual);
  }
  // Eating a rook cancels its flag
  {
    gamestate g = zerostate();
    g.rooks_bb =
      bit(mkPosition(7,0)) |
      bit(mkPosition(0,0));
    g.current_piece_bb = g.rooks_bb;
    g.rooks_bb |=
      bit(mkPosition(7,7)) |
      bit(mkPosition(0,7));
    g.castle_flags =
      CASTLE_BLACK_QUEENSIDE |
      CASTLE_BLACK_KINGSIDE;
    // West Rook
    {
      move m; m.from = mkPosition(7,0); m.to = mkPosition(7,7);
      gamestate g2 = swap_board(apply_move(g, m));
      assert_equal_u64("test_castling_13_remove_flag_on_eat_rook_1", CASTLE_BLACK_QUEENSIDE, g2.castle_flags);
    }
    // East Rook
    {
      move m; m.from = mkPosition(0,0); m.to = mkPosition(0,7);
      gamestate g2 = swap_board(apply_move(g, m));
      assert_equal_u64("test_castling_13_remove_flag_on_eat_rook_2", CASTLE_BLACK_KINGSIDE, g2.castle_flags);
    }
  }
}

void test_promotions()
{
  gamestate g = zerostate();
  g.pawns_bb = bit(mkPosition(0,6));
  g.current_player_bb = all_pieces(g);
  // Promotion to Rook
  {
    move m = mkPromotion(mkPosition(0,6), PIECE_ROOK);
    gamestate g2 = swap_board(apply_move(g, m));
    assert_equal_bb("test_promotions_1_pawns", 0, g2.pawns_bb);
    assert_equal_bb("test_promotions_1_rooks", bit(mkPosition(0,7)), g2.rooks_bb);
  }
  // Promotion to Knight
  {
    move m = mkPromotion(mkPosition(0,6), PIECE_KNIGHT);
    gamestate g2 = swap_board(apply_move(g, m));
    assert_equal_bb("test_promotions_2_pawns", 0, g2.pawns_bb);
    assert_equal_bb("test_promotions_2_knights", bit(mkPosition(0,7)), g2.knights_bb);

  }
  // Promotion to Bishop
  {
    move m = mkPromotion(mkPosition(0,6), PIECE_BISHOP);
    gamestate g2 = swap_board(apply_move(g, m));
    assert_equal_bb("test_promotions_3_pawns", 0, g2.pawns_bb);
    assert_equal_bb("test_promotions_3_bishops", bit(mkPosition(0,7)), g2.bishops_bb);

  }
  // Promotion to Queen
  {
    move m = mkPromotion(mkPosition(0,6), PIECE_QUEEN);
    gamestate g2 = swap_board(apply_move(g, m));
    assert_equal_bb("test_promotions_4_pawns", 0, g2.pawns_bb);
    assert_equal_bb("test_promotions_4_queens", bit(mkPosition(0,7)), g2.queens_bb);
  }
  // Iterator generates promotions
  {
    assert_equal_int("test_promotions_5_num_moves", 4, num_legal_moves(g));
    iterator i = mkLegalIterator(g);
    move m1 = dereference_iterator(i); i = advance_iterator_legal(g,i);
    move m2 = dereference_iterator(i); i = advance_iterator_legal(g,i);
    move m3 = dereference_iterator(i); i = advance_iterator_legal(g,i);
    move m4 = dereference_iterator(i); i = advance_iterator_legal(g,i);
    assert_equal_int("test_promotions_5_m1_pc", PIECE_QUEEN, promotion_piece(m1));
    assert_equal_int("test_promotions_5_m2_pc", PIECE_BISHOP, promotion_piece(m2));
    assert_equal_int("test_promotions_5_m3_pc", PIECE_KNIGHT, promotion_piece(m3));
    assert_equal_int("test_promotions_5_m4_pc", PIECE_ROOK, promotion_piece(m4));
  }
  // Iterator generates promotions 2
  {
    gamestate g = zerostate();
    g.pawns_bb = bit(mkPosition(1,6));
    g.current_player_bb = all_pieces(g);
    g.bishops_bb =
      bit(mkPosition(0,7)) |
      bit(mkPosition(2,7));

    iterator i = mkLegalIterator(g);
    move m1 = dereference_iterator(i); i = advance_iterator_legal(g,i);
    move m2 = dereference_iterator(i); i = advance_iterator_legal(g,i);
    move m3 = dereference_iterator(i); i = advance_iterator_legal(g,i);
    move m4 = dereference_iterator(i); i = advance_iterator_legal(g,i);
    assert_equal_int("test_promotions_6_num_moves", 12, num_legal_moves(g));
  }
}

uint64_t perft_from(gamestate g, int depth)
{
  n = 0;
  walk_game_tree(g, depth, tick_counter);
  return n;
}

/* uint64_t perft(int depth) */
/* { */
/*   return perft_from(new_game(), depth); */
/* } */

int perft_divide_depth;


void perft_divide_helper1(gamestate g, move m, int d)
{
  n = 0;

  gamestate g2 = swap_board(apply_move(g, m));
  walk_game_tree(g2, perft_divide_depth, tick_counter);
  char b[7];
  print_move(m, b);
  printf("%s\n", b);
  printf(" %lu\n", n);
}

int perft_divide(gamestate g, int depth)
{
  perft_divide_depth = depth;
  walk_game_tree(g, 1, perft_divide_helper1);
}

void test_perft()
{
  // Initial position
  {
    gamestate g = new_game();
    /* printf("perft(1): %lu\n", perft(g,1)); */
    /* printf("perft(2): %lu\n", perft(g,2)); */
    /* printf("perft(3): %lu\n", perft(g,3)); */
    /* printf("perft(4): %lu\n", perft(g,4)); */
    /* printf("perft(5): %lu\n", perft(g,5)); */
    /* assert_equal_u64("Perft(2)", 400, perft(g,2)); */
    /* assert_equal_u64("Perft(3)", 8902, perft(g,3)); */
    /* assert_equal_u64("Perft(4)", 197281, perft(g,4)); */
    /* assert_equal_u64("Perft(5)", 4865609, perft(g,5)); */
  }
  // Initial position(read from string)
  {
    gamestate g = parse_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    assert_equal_u64("Perft(1)", 20,      perft_from(g,1));
    assert_equal_u64("Perft(2)", 400,     perft_from(g,2));
    assert_equal_u64("Perft(3)", 8902,    perft_from(g,3));
    assert_equal_u64("Perft(4)", 197281,  perft_from(g,4));
    /* assert_equal_u64("Perft(5)", 4865609, perft_from(g,5)); */
    /* assert_equal_u64("Perft(6)", 119060324, perft_from(g,6)); */
    /* assert_equal_u64("Perft(7)", 3195901860, perft_from(g,7)); */
    
  }
  // Castling position
  {
    gamestate g = parse_fen("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
    // 1: e5f7 - (88799, 88825)
    /* move m; m.from = mkPosition(4,4); m.to = mkPosition(5,6); */
    /* g = swap_board(apply_move(g, m)); */
    /* // 2: a1b1 - (2126,2127) */
    /* move m2; m2.from = mkPosition(0,0); m2.to = mkPosition(1,0); */
    /* g = swap_board(apply_move(g, m2)); */
    /* // 3: f7h8 - (40, 41) */
    /* move m3; m3.from = mkPosition(5,6); m3.to = mkPosition(7,7); */
    /* g = swap_board(apply_move(g, m3)); */
    /* perft_divide(g, 1); */
    /* /\* print_gamestate(g); *\/ */
    /* print_fen(g); */
    assert_equal_u64("castling_perft_1", 48,          perft_from(g,1));
    assert_equal_u64("castling_perft_2", 2039,        perft_from(g,2));
    assert_equal_u64("castling_perft_3", 97862,       perft_from(g,3));
    assert_equal_u64("castling_perft_4", 4085603,     perft_from(g,4));
    /* assert_equal_u64("castling_perft_5", 193690690,   perft_from(g,5)); */
    /* assert_equal_u64("good_perft_6", 8031647685,  perft_from(g,6)); */
  }
  // Promotion position
  {
    gamestate g = parse_fen("n1n5/PPPk4/8/8/8/8/4Kppp/5N1N b - - 0 1");
    assert_equal_u64("promotion_perft_1", 24,          perft_from(g,1));
    assert_equal_u64("promotion_perft_2", 496,         perft_from(g,2));
    assert_equal_u64("promotion_perft_3", 9483,        perft_from(g,3));
    assert_equal_u64("promotion_perft_4", 182838,      perft_from(g,4));
    assert_equal_u64("promotion_perft_5", 3605103,     perft_from(g,5));
    assert_equal_u64("promotion_perft_6", 71179139,     perft_from(g,6));
  }
}

void test_parsers()
{
  // Position
  {
    int idx = mkPosition(3,6);
    char buffer[3];
    print_pos(idx, buffer);
    assert_equal_string("print_pos", "d7", buffer);
    int parsed = parse_pos(buffer);
    assert_equal_int("parse_pos", idx, parsed);
  }
  // Move
  {
    move m = mkPromotion(mkPosition(3,6), PIECE_ROOK);
    char buffer[7];
    print_move(m, buffer);
    assert_equal_string("print_move", "d7d8/R", buffer);
    move parsed = parse_move(buffer);
    assert_equal_int("parse_move_from", m.from, parsed.from);
    assert_equal_int("parse_move_to", m.to, parsed.to);
  }
  // Gamestate
  {
    const char *reference = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    gamestate g = parse_fen(reference);
    char buffer[100];
    print_fen(g, buffer);
    assert_equal_string("print_fen", reference, buffer);
    gamestate g2 = parse_fen(buffer);
    assert_equal_u64("parse_fen_rooks", g.rooks_bb, g2.rooks_bb);
    assert_equal_u64("parse_fen_knights", g.knights_bb, g2.knights_bb);
    assert_equal_u64("parse_fen_bishops", g.bishops_bb, g2.bishops_bb);
    assert_equal_u64("parse_fen_queens", g.queens_bb, g2.queens_bb);
    assert_equal_u64("parse_fen_kings", g.kings_bb, g2.kings_bb);
    assert_equal_u64("parse_fen_pawns", g.pawns_bb, g2.pawns_bb);
    assert_equal_u64("parse_fen_current_player", g.current_player_bb, g2.current_player_bb);
    assert_equal_int("parse_fen_en_passant", g.en_passant_sq, g2.en_passant_sq);
    assert_equal_u64("parse_fen_castle", g.castle_flags, g2.castle_flags);
  }
  // Gamestate (castle flags 1)
  {
    const char *fen = "rnb1kb1r/ppp1p2p/6p1/6pn/P1BqN3/R7/1PP2PPP/4K1NR b Kkq - 0 1";
    gamestate g = parse_fen(fen);
    char actual[200];
    g = apply_move(g, swap_move(parse_move("d4e4")));
    print_fen(g, actual);
    const char *expected = "rnb1kb1r/ppp1p2p/6p1/6pn/P1B1q3/R7/1PP2PPP/4K1NR w Kkq - 0 1";
    assert_equal_string("test_parser_castle_flags", expected, actual);
  }
  // Gamestate (castle flags 2)
  {
    const char *fen = "2b1kb1r/p1p2p2/5n1p/2qp2p1/3rp3/2P3P1/PPQPBP1P/RNB1K1NR w KQk - 0 1";
    gamestate g = parse_fen(fen);
    char actual[200];
    g = apply_move(g, parse_move("e1f1"));
    print_fen(g, actual);
    const char *expected = "2b1kb1r/p1p2p2/5n1p/2qp2p1/3rp3/2P3P1/PPQPBP1P/RNB2KNR b k - 0 1";
    assert_equal_string("test_parser_castle_flags_2", expected, actual);
  }
}

int negamax_simple(gamestate g, int depth, int color) {
  if (depth == 0)
    return color*evaluate(g);
  int max = VALUE_NEGAMAX_START;
  for (iterator i = mkLegalIterator(g); ! is_iterator_finished(i); i = advance_iterator_legal(g, i))  {
    move m = dereference_iterator(i);
    gamestate g2 = apply_move(g, m);
    int score = -negamax_simple(g2, depth - 1, color*-1);
    if (score > max)
      max = score;
  }
  return max;
}

void test_search()
{
  {
    /* gamestate g = new_game(); */
    /* /\* move m = best_move(g); *\/ */
    /* print_move(m); */
    /* int score = negamax_original(g); */
    /* printf("Score: %d\n", score); */
    /* negamax_ret x = negamax(g); */
    /* print_move(x.m); */
    /* printf("\nScore: %d\n", x.score); */
  }
}

/* int main() */
/* { */
/*   gamestate g = new_game(); */
/*   move m = parse_move("e2e4"); */
/*   g = apply_move(g, m); */
  
/*   printf("%lu", result); */
/*   return 0; */
/* } */

void print_move_out(move m)
{
  char c[10];
  print_move(m, c);
  printf("%s|", c);
}

void print_fen_out(gamestate g)
{
  char c[300];
  print_fen(g, c);
  printf("%s\n", c);
}

gamestate computer_move(gamestate g)
{
  move m = best_move(g,3);
  g = apply_move(g, m);
  print_move_out(m);
  print_fen_out(g);
  return g;
}

gamestate player_move(gamestate g, const char *m_str)
{
  move m = parse_move(m_str);
  print_move_out(m);
  if (! g.is_white)
    m = swap_move(m);
  g = apply_move(g, m);
  print_fen_out(g);
  return g;
}

int main() {

  
  /* test_ray(); */
  /* test_rook(); */
  /* test_pawn(); */
  /* test_knight(); */
  /* test_king(); */
  /* test_bishop(); */
  /* test_iterator(); */
  /* test_apply_move(); */
  /* test_check(); */
  /* test_castling(); */
  /* test_promotions(); */
  /* test_perft(); */
  test_parsers();
  // test_search();

  gamestate g = parse_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
  g = player_move(g, "e2e3"); // g = computer_move(g);
  g = player_move(g, "d7d6");
  g = player_move(g, "d1h5"); // g = computer_move(g);
  g = player_move(g, "g8f6");
  g = player_move(g, "h5g5"); // g = computer_move(g);
  g = player_move(g, "h7h6"); 
  g = player_move(g, "g5f4"); // g = computer_move(g);
  g = player_move(g, "g7g5");
  g = player_move(g, "f4d4"); // g = computer_move(g);
  g = player_move(g, "e7e5");
  g = player_move(g, "d4a4"); // g = computer_move(g);
  g = player_move(g, "c8d7");
  g = player_move(g, "a4c4"); // g = computer_move(g);
  g = player_move(g, "d6d5");
  g = player_move(g, "c4b3"); // g = computer_move(g);
  g = player_move(g, "b8c6");
  g = player_move(g, "g1f3"); // g = computer_move(g);
  g = player_move(g, "e5e4");
  g = player_move(g, "f3g1"); // g = computer_move(g);
  g = player_move(g, "c6b4");
  g = player_move(g, "g2g3"); // g = computer_move(g);
  g = player_move(g, "d8e7");
  g = player_move(g, "f1e2"); // g = computer_move(g);
  g = player_move(g, "e7c5");
  g = player_move(g, "c2c3"); // g = computer_move(g);
  g = player_move(g, "b4c6");
  g = player_move(g, "b3b7"); // g = computer_move(g);
  g = player_move(g, "a8b8");
  g = player_move(g, "b7a6"); // g = computer_move(g);
  g = player_move(g, "d7c8");
  g = player_move(g, "a6a4"); // g = computer_move(g);
  g = player_move(g, "b8b4");
  g = player_move(g, "a4c2"); // g = computer_move(g);
  g = player_move(g, "c6d4");
  g = player_move(g, "e3d4"); // g = computer_move(g);
  g = player_move(g, "b4d4");
  g = player_move(g, "e1f1"); // g = computer_move(g);
  g = player_move(g, "c8g4");
  g = player_move(g, "e2a6"); // g = computer_move(g);
  g = player_move(g, "c5c6");
  g = player_move(g, "f1e1"); // g = computer_move(g);
  g = player_move(g, "d4a4");
  g = player_move(g, "a6f1"); // g = computer_move(g);
  g = player_move(g, "d5d4");
  g = player_move(g, "c2b3"); // g = computer_move(g);
  g = player_move(g, "f8b4"); 
  g = player_move(g, "h2h4"); // g = computer_move(g);
  g = player_move(g, "e4e3");
  g = player_move(g, "f2f3"); // g = computer_move(g);
  g = player_move(g, "e3d2");
  g = player_move(g, "e1d2"); // g = computer_move(g);
  g = player_move(g, "d4c3");
  g = player_move(g, "d2c2"); // g = computer_move(g);
  g = player_move(g, "g4f5");
  g = player_move(g, "f1d3"); // g = computer_move(g);
  g = player_move(g, "f5d3");
  g = player_move(g, "c2d3"); // g = computer_move(g);
  g = player_move(g, "c6d5");
  g = player_move(g, "b3d5"); // g = computer_move(g);
  g = player_move(g, "f6d5");
  g = player_move(g, "h1h2"); // g = computer_move(g);
  g = player_move(g, "e8g8");
  g = player_move(g, "g1h3"); // g = computer_move(g);
  g = player_move(g, "f8e8");
  g = player_move(g, "f3f4"); // g = computer_move(g);
  g = player_move(g, "e8e3");
  g = player_move(g, "d3c4"); // g = computer_move(g);
  g = player_move(g, "c3b2");
  g = player_move(g, "c1b2"); // g = computer_move(g);
  g = player_move(g, "b4c3");
  g = player_move(g, "c4c5"); // g = computer_move(g);
  g = player_move(g, "a4a5");
  g = player_move(g, "c5c6"); // g = computer_move(g);
  g = player_move(g, "e3e6");
  g = player_move(g, "c6b7"); // g = computer_move(g);
  g = player_move(g, "a5b5");
  g = player_move(g, "b7c8"); // g = computer_move(g);
  g = player_move(g, "b5b2");
  g = player_move(g, "h2h1"); // g = computer_move(g);
  g = player_move(g, "c3a5");
  g = player_move(g, "f4f5"); // g = computer_move(g);
  g = player_move(g, "e6d6");
  g = player_move(g, "b1a3"); // g = computer_move(g);
  g = player_move(g, "d5e7");
  /* gamestate g = parse_fen("rnb1kb1r/ppp1p2p/6p1/6pn/P1BqN3/8/1PP2PPP/R3K1NR w KQkq - 0 1"); */
  /* g = apply_move(g, parse_move("a1a3")); */
  /* char fen[300]; */
  /* print_fen(g, fen); */
  /* printf("%s\n", fen); */
  /* gamestate g = new_game(); */
  /* char fen[300]; */
  /* g = apply_move(g, parse_move("e2e4")); */
  /* move m = best_move(g, 3); */

  /* print_fen(g, fen); */
  /* printf("%s\n", fen); */
  
  /* g = apply_move(g, swap_move(parse_move("d7d5"))); */
  /* print_fen(g, fen); */
  /* printf("%s\n", fen); */
  
  
  /* gamestate g = new_game(); */
  /* iterator i = mkIterator(g); */
  
  /* print_bitboard(i.current_piece_bb); */

  /* gamestate g = new_game(); */
  /* // 1: c2c4 */
  /* move m1; m1.from = mkPosition(2,1); m1.to = mkPosition(2,3); */
  /* g = swap_board(apply_move(g, m1)); */
  /* // 2: d2d4 */
  /* move m2; m2.from = mkPosition(3,1); m2.to = mkPosition(3,3); */
  /* g = swap_board(apply_move(g, m2)); */
  /* // 3: c4d5 */
  /* move m3; m3.from = mkPosition(2,3); m3.to = mkPosition(3,4); */
  /* g = swap_board(apply_move(g, m3)); */
  /* // 4: c1a3 */
  /* move m4; m4.from = mkPosition(2,0); m4.to = mkPosition(0,2); */
  /* g = swap_board(apply_move(g, m4)); */
  /* // 5: a2a3 */
  /* move m5; m5.from = mkPosition(0,1); m5.to = mkPosition(0,2); */
  /* g = swap_board(apply_move(g, m4)); */

  // NOTE: D1C1 as m6 is in roce, but not the redshift shellcode.
  
  /* g = swap_board(apply_move(g, m2)); */
  /* move m3; m3.from = mkPosition(0,3); m3.to = mkPosition(0,4); */
  /* g = swap_board(apply_move(g, m3)); */
  /* g = apply_move(g, m3); */
  /* perft_divide(g,3); */
  /* print_fen(g); */
  /* // 1: B1A3 */

  /* g = swap_board(apply_move(g, m1)); */
  /* /\* // 2: B1A3 *\/ */
  /* g = swap_board(apply_move(g, m1)); */
  /* print_fen(g); */
  /* /\* print_gamestate(g); *\/ */
  // perft_divide(g, 1);
  /* print_gamestate(g); */
  /* printf("== rooks\n"); print_bitboard(g.rooks_bb); */
  /* printf("== pawns\n"); print_bitboard(g.pawns_bb); */

  /* print_moves(g); */
 
}
/* int main() */
/* { */
/*   test_parsers(); */
/*   /\* packed_move m; *\/ */
/*   /\* m.packed = 68719476737; // b1a3 *\/ */
/*   /\* print_move(m.m); *\/ */
/*   /\* gamestate g = new_game(); *\/ */
/*   /\* printf("rooks: %ld\n", g.rooks_bb); *\/ */
/*   /\* printf("knights: %ld\n", g.knights_bb); *\/ */
/*   /\* printf("bishops: %ld\n", g.bishops_bb); *\/ */
/*   /\* printf("queens: %ld\n", g.queens_bb); *\/ */
/*   /\* printf("kings: %ld\n", g.kings_bb); *\/ */
/*   /\* printf("pawns: %ld\n", g.pawns_bb); *\/ */
/*   /\* printf("player: %ld\n", g.current_player_bb); *\/ */
/*   /\* printf("en_passant_sq: %d\n", g.en_passant_sq); *\/ */
/*   /\* printf("castle_flags: %ld\n", g.castle_flags); *\/ */
/*   /\* printf("perft: %lu\n", custom_main(g)); *\/ */
/* } */

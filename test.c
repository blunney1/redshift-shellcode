#include <stdio.h>
#include "chess.c"

typedef void (*ft_action)(gamestate x);

void print_iterator(iterator i)
{
  printf("ITERATOR\n");
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

uint64_t n;

void tick_counter(gamestate x) {
  n++;
}

void walk_game_tree(gamestate init, int depth, ft_action f)
{
  if (depth <= 0) {
    f(init);
    return;
  } else {
    iterator i = mkIterator(init);
    while (! is_iterator_finished(i)) {
      move m = dereference_iterator(i);
      gamestate g = apply_move(init, m);
      walk_game_tree(g, depth-1, f);
      iterator i2 = advance_iterator(i);
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

void test_ray()
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
  /* printf("== RAY_A1A8\n"); */
  /* print_bitboard(RAY_A1A8); */
  /* printf("== RAY_H1H8\n"); */
  /* print_bitboard(RAY_H1H8); */
  /* printf("== offset(RAY, 1)\n"); */
  /* print_bitboard(offset(RAY_A8H1, 1)); */

  /* printf("== offset(RAY, RANK)\n"); */
  /* print_bitboard(offset(RAY_A8H1, RANK)); */

  /*   printf("== offset(RAY, -RANK)\n"); */
  /* print_bitboard(offset(RAY_A8H1, -RANK)); */

  assert_equal_bb("nw", nw_ray, mkRay(center, DIRECTION_NORTHWEST));
  assert_equal_bb("n",  n_ray,  mkRay(center, DIRECTION_NORTH));
  assert_equal_bb("ne", ne_ray, mkRay(center, DIRECTION_NORTHEAST));
  assert_equal_bb("e",  e_ray,  mkRay(center, DIRECTION_EAST));
  assert_equal_bb("se", se_ray, mkRay(center, DIRECTION_SOUTHEAST));
  assert_equal_bb("s",  s_ray,  mkRay(center, DIRECTION_SOUTH));
  assert_equal_bb("sw", sw_ray, mkRay(center, DIRECTION_SOUTHWEST));
  assert_equal_bb("w",  w_ray,  mkRay(center, DIRECTION_WEST));
}

void test_rook()
{
  uint64_t center = mkPosition(3,3);
  gamestate g = zerostate();
  g.rooks_bb = bit(center);
  g.current_piece_bb = g.rooks_bb;

  uint64_t expected = clear_bit(mkRank(3) | mkFile(3), center);
  uint64_t actual = valid_rook_moves(g, center);
  assert_equal_bb("test_rook", expected, actual);
}

int perft(int depth)
{
  n = 0;
  gamestate init = new_game();
  walk_game_tree(init, depth, tick_counter);
  return n;
}


int main() {
  test_ray();
  test_rook();
  
  gamestate g = new_game();
  iterator i = mkIterator(g);
  
  print_bitboard(i.current_piece_bb);
  
  printf("Perft(0): %d\n", perft(0));
  printf("Perft(1): %d\n", perft(1));
  /* printf("Perft(2): %d\n", perft(2)); */
  /* printf("Perft(3): %d\n", perft(3)); */
}

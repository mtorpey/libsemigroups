//
// Semigroups++ - C/C++ library for computing with semigroups and monoids
// Copyright (C) 2016 James D. Mitchell
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#include "catch.hpp"

#include "../blocks.h"
#include "../elements.h"

TEST_CASE("Blocks: empty blocks", "") {
  Blocks b1;
  Blocks b2 = Blocks(new std::vector<u_int32_t>({0, 1, 2, 1, 0, 2}),
                     new std::vector<bool>({true, false, true}));
  REQUIRE(b1 == b1);
  REQUIRE(!(b1 == b2));
  REQUIRE(b1 < b2);
  REQUIRE(!(b2 < b1));
  REQUIRE(b1.degree() == 0);
  REQUIRE(b1.lookup() == nullptr);
  REQUIRE(b1.nr_blocks() == 0);
  REQUIRE(b1.rank() == 0);
  REQUIRE(b1.hash_value() == 0);
}

TEST_CASE("Blocks: non-empty blocks", "") {
  Blocks b = Blocks(new std::vector<u_int32_t>({0, 1, 2, 1, 0, 2}),
                    new std::vector<bool>({true, false, true}));
  REQUIRE(b == b);
  REQUIRE(!(b < b));
  REQUIRE(b.degree() == 6);
  REQUIRE(*b.lookup() == std::vector<bool>({true, false, true}));
  REQUIRE(b.nr_blocks() == 3);
  REQUIRE(b.rank() == 2);
  REQUIRE(b.hash_value() == 381493);
  REQUIRE(b.is_transverse_block(0));
  REQUIRE(!b.is_transverse_block(1));
  REQUIRE(b.is_transverse_block(2));
  REQUIRE(b.block(0) == 0);
  REQUIRE(b.block(1) == 1);
  REQUIRE(b.block(2) == 2);
  REQUIRE(b.block(3) == 1);
  REQUIRE(b.block(4) == 0);
  REQUIRE(b.block(5) == 2);
  size_t i = 0;
  for (auto it = b.cbegin(); it < b.cend(); it++) {
    REQUIRE(*it == b.block(i++));
  }
}

TEST_CASE("Blocks: left blocks of bipartition", "") {
  Bipartition x =
      Bipartition({0, 1, 2, 1, 0, 2, 1, 0, 2, 2, 0, 0, 2, 0, 3, 4, 4, 1, 3, 0});
  Blocks* b = x.left_blocks();
  REQUIRE(b == b);
  REQUIRE(!(b < b));
  REQUIRE(b->degree() == 10);
  REQUIRE(*b->lookup() == std::vector<bool>({true, true, true}));
  REQUIRE(b->nr_blocks() == 3);
  REQUIRE(b->rank() == 3);
  REQUIRE(b->hash_value() == 121021022111);
  REQUIRE(b->is_transverse_block(0));
  REQUIRE(b->is_transverse_block(1));
  REQUIRE(b->is_transverse_block(2));
  REQUIRE(b->block(0) == 0);
  REQUIRE(b->block(1) == 1);
  REQUIRE(b->block(2) == 2);
  REQUIRE(b->block(3) == 1);
  REQUIRE(b->block(4) == 0);
  REQUIRE(b->block(5) == 2);
  delete b;
}

TEST_CASE("Blocks: right blocks of bipartition", "") {
  Bipartition x = Bipartition(
      {0, 1, 1, 1, 1, 2, 3, 2, 4, 4, 5, 2, 4, 2, 1, 1, 1, 2, 3, 2});
  Blocks* b = x.right_blocks();
  REQUIRE(b == b);
  REQUIRE(!(b < b));
  REQUIRE(b->degree() == 10);
  REQUIRE(*b->lookup() == std::vector<bool>({false, true, true, true, true}));
  REQUIRE(b->nr_blocks() == 5);
  REQUIRE(b->rank() == 4);
  REQUIRE(b->hash_value() == 12133314101111);
  REQUIRE(!b->is_transverse_block(0));
  REQUIRE(b->is_transverse_block(1));
  REQUIRE(b->is_transverse_block(2));
  REQUIRE(b->is_transverse_block(3));
  REQUIRE(b->is_transverse_block(4));
  REQUIRE(b->block(0) == 0);
  REQUIRE(b->block(1) == 1);
  REQUIRE(b->block(2) == 2);
  REQUIRE(b->block(3) == 1);
  REQUIRE(b->block(4) == 3);
  REQUIRE(b->block(5) == 3);
  REQUIRE(b->block(6) == 3);
  REQUIRE(b->block(7) == 1);
  REQUIRE(b->block(8) == 4);
  REQUIRE(b->block(9) == 1);
  delete b;
}

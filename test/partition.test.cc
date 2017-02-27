//
// libsemigroups - C++ library for semigroups and monoids
// Copyright (C) 2017 Michael Torpey
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

#include <utility>

#include "../src/partition.h"
#include "../src/semigroups.h"
#include "catch.hpp"

using namespace libsemigroups;

TEST_CASE("Partition 01: 0-argument constructor", "[partition][quick]") {
  Partition<size_t>* part = new Partition<size_t>();
  REQUIRE(part->size() == 0);
  delete part;
}

TEST_CASE("Partition 02: 1-argument constructor", "[partition][quick]") {
  std::vector<std::vector<word_t*>*>* vector
      = new std::vector<std::vector<word_t*>*>();

  for (size_t i = 0; i < 3; i++) {
    vector->push_back(new std::vector<word_t*>());
  }

  vector->at(0)->push_back(new word_t({1}));
  vector->at(0)->push_back(new word_t({0, 1}));
  vector->at(1)->push_back(new word_t({0, 0, 1}));
  vector->at(1)->push_back(new word_t({1, 3, 2, 2}));
  vector->at(2)->push_back(new word_t({3}));

  Partition<word_t>* part = new Partition<word_t>(vector);

  REQUIRE(part->size() == 3);
  REQUIRE(part->at(0)->size() == 2);
  REQUIRE((*part)[1]->size() == 2);
  REQUIRE((*part)[2]->size() == 1);
  REQUIRE(*part->at(1, 1) == word_t({1, 3, 2, 2}));

  delete part;

TEST_CASE("Partition 03: really_delete",
          "[partition][quick]") {
  Partition<word_t*>* part = new Partition<word_t*>(3);
  part->add_element(0, new word_t({0, 1, 1, 2, 0, 1}));
  part->add_element(2, new word_t({0, 1, 1}));
  part->add_element(1, new word_t({1, 2, 3, 1, 1}));
  part->add_element(0, new word_t({0}));
  word_t* w = new word_t({3, 2, 1, 3, 2, 1});
  part->add_element(2, w);
  REQUIRE(w->size() == 6);
  part->really_delete();
  // REQUIRE(w->size() == 6); // this would fail since w has been deleted
  delete part;
}

TEST_CASE("Partition 03: really_delete",
          "[partition][quick]") {
  Partition<word_t*> part(4);
  part.add_element(0, new word_t({0, 1, 1, 2, 0, 1}));
  part.add_element(2, new word_t({0, 1, 1}));
  part.add_element(1, new word_t({1, 2, 3, 1, 1}));
  part.add_element(1, new word_t({0}));
  part.add_element(2, new word_t({1, 3, 1}));
  part.add_element(1, new word_t({2, 0}));
  word_t* w = new word_t({3, 2, 1, 3, 2, 1});
  part.add_element(3, w);

  REQUIRE(part.nr_classes() == 4);
  REQUIRE(part.size_of_class(0) == 1);
  REQUIRE(part.size_of_class(1) == 3);
  REQUIRE(part.size_of_class(2) == 2);
  REQUIRE(part.size_of_class(4) == 1);

  part.remove_singletons();
  REQUIRE(part.nr_classes() == 2);
  REQUIRE(part.size_of_class(0) == 3);
  REQUIRE(part.size_of_class(1) == 2);
  
  part.really_delete();
}

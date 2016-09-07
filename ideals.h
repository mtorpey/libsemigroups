//
// Semigroups++ - C/C++ library for computing with semigroups and monoids
// Copyright (C) 2016 James D. Mitchell and Wilf A. Wilson
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

#include <vector>

#include "semigroups.h"

class RightIdeal {
  typedef size_t       pos_t;

 public:
  RightIdeal(Semigroup* semigroup, std::vector<Element*>* gens)
      : _elements(),
        _gens(),
        _lookup(semigroup->size(), false),
        _semigroup(semigroup),
        _size(0) {
          _gens.reserve(gens->size());
          std::vector<pos_t> positions;
          for (Element* x: *gens) {
            _gens.push_back(x->really_copy());
            pos_t pos = semigroup->position(x);
            assert(pos != Semigroup::UNDEFINED);
            positions.push_back(pos);
            if (!_lookup[pos]) {
              _size++;
              _lookup[pos] = true;
            }
          }

          cayley_graph_t* graph = semigroup->right_cayley_graph();

          for (size_t i = 0; i != positions.size(); ++i) {
            std::vector<pos_t>::iterator end = graph->row_end(positions[i]);
            for (std::vector<pos_t>::iterator it = graph->row_begin(positions[i]);
                 it < end;
                 it++) {
              if (!_lookup[*it]) {
                _lookup[*it] = true;
                _elements.push_back(semigroup->at(*it)->really_copy());
                _size++;
                positions.push_back(*it);
              }
            }
          }
        }

  bool test_membership(Element* x) {
    pos_t pos = _semigroup->position(x);
    if (pos == Semigroup::UNDEFINED) {
      return false;
    }
    return _lookup[pos];
  }

  size_t size() {
    return _size;
  }

 private:
  std::vector<Element*> _elements;
  std::vector<Element*> _gens;
  std::vector<bool>     _lookup;
  Semigroup*            _semigroup;
  size_t                _size;
};

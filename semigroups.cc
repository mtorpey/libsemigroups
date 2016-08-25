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

#include "semigroups.h"

Semigroup::Semigroup(std::vector<Element*>* gens, size_t degree)
    : _batch_size(8192),
      _degree(degree),
      _duplicate_gens(),
      _elements(new std::vector<Element*>()),
      _final(),
      _first(),
      _found_one(false),
      _gens(new std::vector<Element*>()),
      _genslookup(),
      _index(),
      _left(new cayley_graph_t(gens->size())),
      _length(),
      _lenindex(),
      _map(),
      _multiplied(),
      _nr(0),
      _nrgens(gens->size()),
      _nr_idempotents(0),
      _nrrules(0),
      _pos(0),
      _pos_one(0),
      _prefix(),
      _reduced(gens->size()),
      _relation_gen(0),
      _relation_pos(-1),
      _right(new cayley_graph_t(gens->size())),
      _sorted(nullptr),
      _pos_sorted(nullptr),
      _suffix(),
      _wordlen(0),  // (length of the current word) - 1
      _nr_threads(1),
      _reporter(*this) {
  assert(_nrgens != 0);

  for (Element* x : *gens) {
    _gens->push_back(x->really_copy());
  }

  _tmp_product = _gens->at(0)->identity();
  _lenindex.push_back(0);
  _id = _gens->at(0)->identity();

  // inverse of genslookup for keeping track of duplicate gens
  // maps from positions in _elements to positions in _gens
  std::vector<letter_t> inv_genslookup;

  // add the generators
  for (size_t i = 0; i < _nrgens; i++) {
    auto it = _map.find(_gens->at(i));
    if (it != _map.end()) {  // duplicate generator
      _genslookup.push_back(it->second);
      _nrrules++;
      _duplicate_gens.push_back(std::make_pair(i, inv_genslookup[it->second]));
      // i.e. _gens[i] = _gens[inv_genslookup[it->second]]
    } else {
      inv_genslookup.push_back(_genslookup.size());
      is_one(_gens->at(i), _nr);
      _elements->push_back(_gens->at(i)->really_copy());
      _first.push_back(i);
      _final.push_back(i);
      _genslookup.push_back(_nr);
      _length.push_back(1);
      _map.insert(std::make_pair(_elements->back(), _nr));
      _prefix.push_back(-1);
      _suffix.push_back(-1);
      _index.push_back(_nr);
      _nr++;
    }
  }
  expand(_nr);
  _lenindex.push_back(_index.size());
}

Semigroup::Semigroup(const Semigroup& copy)
    : _batch_size(copy._batch_size),
      _degree(copy._degree),
      _duplicate_gens(copy._duplicate_gens),
      _elements(new std::vector<Element*>()),
      _final(copy._final),
      _first(copy._first),
      _found_one(copy._found_one),
      _gens(),
      _genslookup(copy._genslookup),
      _id(copy._id->really_copy()),
      _index(copy._index),
      _left(new cayley_graph_t(*copy._left)),
      _length(copy._length),
      _lenindex(copy._lenindex),
      _multiplied(copy._multiplied),
      _nr(copy._nr),
      _nrgens(copy._nrgens),
      _nr_idempotents(copy._nr_idempotents),
      _nrrules(copy._nrrules),
      _pos(copy._pos),
      _pos_one(copy._pos_one),
      _prefix(copy._prefix),
      _reduced(copy._reduced),
      _relation_gen(copy._relation_gen),
      _relation_pos(copy._relation_pos),
      _right(new cayley_graph_t(*copy._right)),
      _sorted(nullptr),
      _pos_sorted(nullptr),
      _suffix(copy._suffix),
      _wordlen(copy._wordlen) {
  _elements->reserve(_nr);
  _map.reserve(_nr);
  _tmp_product = copy._id->really_copy();

  for (Element const* x : *_gens) {
    _gens->push_back(x->really_copy());
  }

  for (size_t i = 0; i < copy._elements->size(); i++) {
    _elements->push_back(copy._elements->at(i)->really_copy());
    _map.insert(std::make_pair(_elements->back(), i));
  }
}

Semigroup::Semigroup(const Semigroup&       copy,
                     std::vector<Element*>* coll,
                     bool                   report)
    : _batch_size(copy._batch_size),
      _degree(copy._degree),  // copy for comparison in add_generators
      _duplicate_gens(copy._duplicate_gens),
      _elements(new std::vector<Element*>()),
      _found_one(copy._found_one),  // copy in case degree doesn't change in
                                    // add_generators
      _gens(new std::vector<Element*>()),
      _genslookup(copy._genslookup),
      _left(new cayley_graph_t(*copy._left)),
      _multiplied(copy._multiplied),
      _nr(copy._nr),
      _nrgens(copy._nrgens),
      _nr_idempotents(0),
      _nrrules(0),
      _pos(copy._pos),
      _pos_one(copy._pos_one),  // copy in case degree doesn't change in
                                // add_generators
      _reduced(copy._reduced),
      _relation_gen(0),
      _relation_pos(-1),
      _right(new cayley_graph_t(*copy._right)),
      _sorted(nullptr),
      _pos_sorted(nullptr),
      _wordlen(0) {
  assert(!coll->empty());

  _elements->reserve(copy._nr);
  _map.reserve(copy._nr);

  // the following are required for assignment to specific positions in
  // add_generators
  _final.resize(copy._nr, 0);
  _first.resize(copy._nr, 0);
  _length.resize(copy._nr, 0);
  _prefix.resize(copy._nr, 0);
  _suffix.resize(copy._nr, 0);

  std::unordered_set<Element*> new_gens;

  // remove duplicate generators
  for (Element* x : *coll) {
    assert(x->degree() == coll->at(0)->degree());
    new_gens.insert(x->really_copy());
    // copy here so that after add_generators, the semigroup is responsible
    // for the destruction of gens.
  }

  assert((*new_gens.begin())->degree() >= copy.degree());

  size_t deg_plus = (*new_gens.begin())->degree() - copy.degree();

  if (deg_plus != 0) {
    _degree += deg_plus;
    _found_one = false;
    _pos_one   = 0;
  }

  _lenindex.push_back(0);
  _lenindex.push_back(copy._lenindex.at(1));
  _index.reserve(copy._nr);

  // add the distinct old generators to new _index
  for (size_t i = 0; i < copy._lenindex.at(1); i++) {
    _index.push_back(copy._index.at(i));
    _final.at(_index.at(i))  = i;
    _first.at(_index.at(i))  = i;
    _prefix.at(_index.at(i)) = -1;
    _suffix.at(_index.at(i)) = -1;
    _length.at(_index.at(i)) = 1;
  }

  for (size_t i = 0; i < copy.nrgens(); i++) {
    _gens->push_back(copy._gens->at(i)->really_copy(deg_plus));
  }

  _id          = copy._id->really_copy(deg_plus);
  _tmp_product = copy._id->really_copy(deg_plus);

  for (size_t i = 0; i < copy._elements->size(); i++) {
    _elements->push_back(copy._elements->at(i)->really_copy(deg_plus));
    _map.insert(std::make_pair(_elements->back(), i));
    is_one(_elements->back(), i);
  }

  add_generators(new_gens, report);
}

Semigroup::~Semigroup() {

  _tmp_product->really_delete();
  delete _tmp_product;

  delete _left;
  delete _right;
  delete _sorted;
  delete _pos_sorted;

  for (auto x : _duplicate_gens) {
    _gens->at(x.first)->really_delete();
  }
  delete _gens;

  for (Element* x : *_elements) {
    x->really_delete();
    delete x;
  }
  delete _elements;

  _id->really_delete();
  delete _id;
}

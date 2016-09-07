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

// Static data members

Semigroup::pos_t Semigroup::UNDEFINED = -1;

// default
// @gens   the generators of the semigroup

Semigroup::Semigroup(std::vector<Element*>* gens)
    : _batch_size(8192),
      _degree(UNDEFINED),
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
      _reporter(*this) {
  assert(_nrgens != 0);

  _degree = (*gens)[0]->degree();

  for (Element* x : *gens) {
    assert(x->degree() == _degree);
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

// Copy constructor

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

// Copy <copy> and add the generators in <coll>

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

// Destructor

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

// Product by tracing in the left or right Cayley graph

Semigroup::pos_t Semigroup::product_by_reduction(pos_t i, pos_t j) const {
  assert(i < _nr && j < _nr);
  if (length_const(i) <= length_const(j)) {
    while (i != (size_t) -1) {
      j = _left->get(j, _final.at(i));
      i = _prefix.at(i);
    }
    return j;
  } else {
    while (j != (size_t) -1) {
      i = _right->get(i, _first.at(j));
      j = _suffix.at(j);
    }
    return i;
  }
}

// Product by multiplying or by tracing in the Cayley graph whichever is faster

Semigroup::pos_t Semigroup::fast_product(pos_t i, pos_t j) const {
  assert(i < _nr && j < _nr);
  if (length_const(i) < 2 * _tmp_product->complexity()
      || length_const(j) < 2 * _tmp_product->complexity()) {
    return product_by_reduction(i, j);
  } else {
    _tmp_product->redefine(_elements->at(i), _elements->at(j));
    return _map.find(_tmp_product)->second;
  }
}

// Count the number of idempotents

size_t Semigroup::nr_idempotents(bool report, size_t nr_threads) {
  if (_nr_idempotents == 0) {
    enumerate(-1, report);

    _reporter.report(report);
    _reporter.start_timer();
    _reporter(__func__);
    if (nr_threads == 1 || size() < 65537) {
      size_t sum_word_lengths = 0;
      for (size_t i = 1; i < _lenindex.size(); i++) {
        sum_word_lengths += i * (_lenindex[i] - _lenindex[i - 1]);
      }

      if (_nr * _tmp_product->complexity() < sum_word_lengths) {
        for (size_t i = 0; i < _nr; i++) {
          _tmp_product->redefine(_elements->at(i), _elements->at(i));
          if (*_tmp_product == *_elements->at(i)) {
            _nr_idempotents++;
          }
        }
      } else {
        for (size_t i = 0; i < _nr; i++) {
          if (product_by_reduction(i, i) == i) {
            _nr_idempotents++;
          }
        }
      }
    } else {
      pos_t                    begin = 0;
      pos_t                    end   = size() / nr_threads;
      std::vector<size_t>      nr(nr_threads, 0);
      std::vector<std::thread> threads;
      // FIXME distribute better here!
      for (size_t i = 0; i < nr_threads; i++) {
        threads.push_back(std::thread(&Semigroup::nr_idempotents_thread,
                                      this,
                                      i,
                                      std::ref(nr[i]),
                                      begin,
                                      end,
                                      report));
        begin = end;
        if (i == nr_threads - 1) {
          end = size();
        } else {
          end += size() / nr_threads;
        }
      }
      for (size_t i = 0; i < nr_threads; i++) {
        threads[i].join();
        _nr_idempotents += nr[i];
      }
    }
  }
  _reporter.stop_timer();
  return _nr_idempotents;
}

// Get the position of an element in the semigroup

Semigroup::pos_t Semigroup::position(Element* x, bool report) {
  if (x->degree() != _degree) {
    return UNDEFINED;
  }

  while (true) {
    auto it = _map.find(x);
    if (it != _map.end()) {
      return it->second;
    }
    if (is_done()) {
      return UNDEFINED;
    }
    enumerate(_nr + 1, report);
    // _nr + 1 means we enumerate _batch_size more elements
  }
}

size_t Semigroup::position_sorted(Element* x, bool report) {
  pos_t pos = position(x, report);

  if (pos == UNDEFINED) {
    return UNDEFINED;
  } else if (_pos_sorted == nullptr) {
    sort_elements(report);
    _pos_sorted = new std::vector<size_t>();
    _pos_sorted->resize(_sorted->size());
    for (size_t i = 0; i < _sorted->size(); i++) {
      (*_pos_sorted)[(*_sorted)[i].second] = i;
    }
  }
  return (*_pos_sorted)[pos];
}

Element* Semigroup::at(pos_t pos, bool report) {
  enumerate(pos + 1, report);

  if (pos < _elements->size()) {
    return (*_elements)[pos];
  } else {
    return nullptr;
  }
}

Element* Semigroup::sorted_at(pos_t pos, bool report) {
  sort_elements(report);
  if (pos < _sorted->size()) {
    return (*_sorted)[pos].first;
  } else {
    return nullptr;
  }
}

void Semigroup::factorisation(word_t& word, pos_t pos, bool report) {
  if (pos > _nr && !is_done()) {
    enumerate(pos, report);
  }

  if (pos < _nr) {
    word.clear();
    while (pos != UNDEFINED) {
      word.push_back(_first.at(pos));
      pos = _suffix.at(pos);
    }
  }
}

void Semigroup::next_relation(std::vector<size_t>& relation, bool report) {
  if (!is_done()) {
    enumerate(-1, report);
  }

  relation.clear();  // FIXME use an array instead since this has fixed size

  if (_relation_pos == _nr) {  // no more relations
    return;
  }

  if (_relation_pos != (size_t) -1) {
    while (_relation_pos < _nr) {
      while (_relation_gen < _nrgens) {
        if (!_reduced.get(_index.at(_relation_pos), _relation_gen)
            && (_relation_pos < _lenindex.at(1)
                || _reduced.get(_suffix.at(_index.at(_relation_pos)),
                                _relation_gen))) {
          relation.push_back(_index.at(_relation_pos));
          relation.push_back(_relation_gen);
          relation.push_back(
              _right->get(_index.at(_relation_pos), _relation_gen));
          break;
        }
        _relation_gen++;
      }
      if (relation.empty()) {
        _relation_gen = 0;
        _relation_pos++;
      } else {
        break;
      }
    }
    if (_relation_gen == _nrgens) {
      _relation_gen = 0;
      _relation_pos++;
    } else {
      _relation_gen++;
    }
  } else {
    // duplicate generators
    if (_relation_gen < _duplicate_gens.size()) {
      relation.push_back(_duplicate_gens.at(_relation_gen).first);
      relation.push_back(_duplicate_gens.at(_relation_gen).second);
      _relation_gen++;
    } else {
      _relation_gen = 0;
      _relation_pos++;
      next_relation(relation, report);
    }
  }
}

void Semigroup::enumerate(size_t limit, bool report) {
  if (_pos >= _nr || limit <= _nr) {
    return;
  }
  limit = std::max(limit, _nr + _batch_size);

  _reporter.report(report);
  _reporter.start_timer();
  _reporter(__func__) << "limit = " << limit << std::endl;

  // multiply the generators by every generator
  if (_pos < _lenindex.at(1)) {
    size_t nr_shorter_elements = _nr;
    while (_pos < _lenindex.at(1)) {
      size_t i          = _index.at(_pos);
      _multiplied.at(i) = true;
      for (size_t j = 0; j < _nrgens; j++) {
        _tmp_product->redefine(_elements->at(i), _gens->at(j));
        auto it = _map.find(_tmp_product);

        if (it != _map.end()) {
          _right->set(i, j, it->second);
          _nrrules++;
        } else {
          is_one(_tmp_product, _nr);
          _elements->push_back(_tmp_product->really_copy());
          _first.push_back(_first.at(i));
          _final.push_back(j);
          _index.push_back(_nr);
          _length.push_back(2);
          _map.insert(std::make_pair(_elements->back(), _nr));
          _prefix.push_back(i);
          _reduced.set(i, j, true);
          _right->set(i, j, _nr);
          _suffix.push_back(_genslookup.at(j));
          _nr++;
        }
      }
      _pos++;
    }
    for (size_t i = 0; i < _pos; i++) {
      size_t b = _final.at(_index.at(i));
      for (size_t j = 0; j < _nrgens; j++) {
        _left->set(_index.at(i), j, _right->get(_genslookup.at(j), b));
      }
    }
    _wordlen++;
    expand(_nr - nr_shorter_elements);
    _lenindex.push_back(_index.size());
  }

  // multiply the words of length > 1 by every generator
  bool stop = (_nr >= limit);

  while (_pos < _nr && !stop) {
    size_t nr_shorter_elements = _nr;
    while (_pos < _lenindex.at(_wordlen + 1) && !stop) {
      size_t i          = _index.at(_pos);
      size_t b          = _first.at(i);
      size_t s          = _suffix.at(i);
      _multiplied.at(i) = true;
      for (size_t j = 0; j < _nrgens; j++) {
        if (!_reduced.get(s, j)) {
          size_t r = _right->get(s, j);
          if (_found_one && r == _pos_one) {
            _right->set(i, j, _genslookup.at(b));
          } else if (_prefix.at(r) != (size_t) -1) {  // r is not a generator
            _right->set(
                i, j, _right->get(_left->get(_prefix.at(r), b), _final.at(r)));
          } else {
            _right->set(i, j, _right->get(_genslookup.at(b), _final.at(r)));
          }
        } else {
          _tmp_product->redefine(_elements->at(i), _gens->at(j));
          auto it = _map.find(_tmp_product);

          if (it != _map.end()) {
            _right->set(i, j, it->second);
            _nrrules++;
          } else {
            is_one(_tmp_product, _nr);
            _elements->push_back(_tmp_product->really_copy());
            _first.push_back(b);
            _final.push_back(j);
            _length.push_back(_wordlen + 2);
            _map.insert(std::make_pair(_elements->back(), _nr));
            _prefix.push_back(i);
            _reduced.set(i, j, true);
            _right->set(i, j, _nr);
            _suffix.push_back(_right->get(s, j));
            _index.push_back(_nr);
            _nr++;
            stop = (_nr >= limit);
          }
        }
      }  // finished applying gens to <_elements->at(_pos)>
      _pos++;
    }  // finished words of length <wordlen> + 1
    expand(_nr - nr_shorter_elements);

    if (_pos > _nr || _pos == _lenindex.at(_wordlen + 1)) {
      for (size_t i = _lenindex.at(_wordlen); i < _pos; i++) {
        size_t p = _prefix.at(_index.at(i));
        size_t b = _final.at(_index.at(i));
        for (size_t j = 0; j < _nrgens; j++) {
          _left->set(_index.at(i), j, _right->get(_left->get(p, j), b));
        }
      }
      _wordlen++;
      _lenindex.push_back(_index.size());
    }

    _reporter(__func__) << "found " << _nr << " elements, " << _nrrules
                        << " rules, max word length "
                        << current_max_word_length();

    if (!is_done()) {
      _reporter << ", so far" << std::endl;
    } else {
      _reporter << ", finished!" << std::endl;
      _reporter.stop_timer();
    }
  }
  _reporter.stop_timer();
}

void Semigroup::add_generators(const std::unordered_set<Element*>& coll,
                               bool                                report) {
  if (coll.empty()) {
    return;
  }

  assert(degree() == (*coll.begin())->degree());

  _reporter.report(report);
  _reporter.start_timer();

  // get some parameters from the old semigroup
  size_t old_nrgens  = _nrgens;
  size_t old_nr      = _nr;
  size_t nr_old_left = _pos;

  bool              there_are_new_gens = false;
  std::vector<bool> old_new;  // have we seen _elements->at(i) yet in new?

  // add the new generators to new _gens, _elements, and _index
  for (Element* x : coll) {
    if (_map.find(x) == _map.end()) {
      if (!there_are_new_gens) {
        // erase the old index
        _index.erase(_index.begin() + _lenindex.at(1), _index.end());

        // set up old_new
        old_new.resize(old_nr, false);
        for (size_t i = 0; i < _genslookup.size(); i++) {
          old_new.at(_genslookup.at(i)) = true;
        }
        there_are_new_gens = true;
      }

      _first.push_back(_gens->size());
      _final.push_back(_gens->size());

      _gens->push_back(x);
      _elements->push_back(x);
      _genslookup.push_back(_nr);
      _index.push_back(_nr);

      is_one(x, _nr);
      _map.insert(std::make_pair(x, _nr));
      _multiplied.push_back(false);
      _prefix.push_back(-1);
      _suffix.push_back(-1);
      _length.push_back(1);

      _nr++;
    }
  }

  if (!there_are_new_gens) {  // everything in coll was already in the
    // semigroup
    _reporter.stop_timer();
    return;
  }

  // reset the data structure
  _nr_idempotents = 0;
  _nrrules        = _duplicate_gens.size();
  _pos            = 0;
  _wordlen        = 0;
  _nrgens         = _gens->size();
  _lenindex.clear();
  _lenindex.push_back(0);
  _lenindex.push_back(_nrgens - _duplicate_gens.size());

  // add columns for new generators
  _reduced = flags_t(_nrgens, _reduced.nr_rows() + _nrgens - old_nrgens);
  _left->add_cols(_nrgens - _left->nr_cols());
  _right->add_cols(_nrgens - _right->nr_cols());

  // add rows in for newly added generators
  _left->add_rows(_nrgens - old_nrgens);
  _right->add_rows(_nrgens - old_nrgens);

  size_t nr_shorter_elements;

  // repeat until we have multiplied all of the elements of <old> up to the
  // old value of _pos by all of the (new and old) generators

  while (nr_old_left > 0) {
    nr_shorter_elements = _nr;
    while (_pos < _lenindex.at(_wordlen + 1) && nr_old_left > 0) {
      size_t i = _index.at(_pos);  // position in _elements
      size_t b = _first.at(i);
      size_t s = _suffix.at(i);
      if (_multiplied.at(i)) {
        nr_old_left--;
        // _elements.at(i) is in old semigroup, and its descendants are
        // known
        for (size_t j = 0; j < old_nrgens; j++) {
          size_t k = _right->get(i, j);
          if (!old_new.at(k)) {  // it's new!
            is_one(_elements->at(k), k);
            _first.at(k)  = _first.at(i);
            _final.at(k)  = j;
            _length.at(k) = _wordlen + 2;
            _prefix.at(k) = i;
            _reduced.set(i, j, true);
            if (_wordlen == 0) {
              _suffix.at(k) = _genslookup.at(j);
            } else {
              _suffix.at(k) = _right->get(s, j);
            }
            _index.push_back(k);
            old_new.at(k) = true;
          } else if (s == (size_t) -1 || _reduced.get(s, j)) {
            // this clause could be removed if _nrrules wasn't necessary
            _nrrules++;
          }
        }
        for (size_t j = old_nrgens; j < _nrgens; j++) {
          closure_update(i, j, b, s, old_new, old_nr);
        }

      } else {
        // _elements.at(i) is not in old
        _multiplied.at(i) = true;
        for (size_t j = 0; j < _nrgens; j++) {
          closure_update(i, j, b, s, old_new, old_nr);
        }
      }
      _pos++;
    }  // finished words of length <wordlen> + 1

    expand(_nr - nr_shorter_elements);
    if (_pos > _nr || _pos == _lenindex.at(_wordlen + 1)) {
      if (_wordlen == 0) {
        for (size_t i = 0; i < _pos; i++) {
          size_t b = _final.at(_index.at(i));
          for (size_t j = 0; j < _nrgens; j++) {
            // TODO(JDM) reuse old info here!
            _left->set(_index.at(i), j, _right->get(_genslookup.at(j), b));
          }
        }
      } else {
        for (size_t i = _lenindex.at(_wordlen); i < _pos; i++) {
          size_t p = _prefix.at(_index.at(i));
          size_t b = _final.at(_index.at(i));
          for (size_t j = 0; j < _nrgens; j++) {
            // TODO(JDM) reuse old info here!
            _left->set(_index.at(i), j, _right->get(_left->get(p, j), b));
          }
        }
      }
      _lenindex.push_back(_index.size());
      _wordlen++;
    }
    _reporter(__func__) << "found " << _nr << " elements, " << _nrrules
                        << " rules, max word length "
                        << current_max_word_length();

    if (!is_done()) {
      _reporter << ", so far" << std::endl;
    } else {
      _reporter << ", finished!" << std::endl;
      _reporter.stop_timer();
    }
  }
  _reporter.stop_timer();
}

// Private methods

void Semigroup::sort_elements(bool report) {
  if (_sorted != nullptr) {
    return;
  }
  enumerate(-1, report);
  _sorted = new std::vector<std::pair<Element*, size_t>>();
  _sorted->reserve(_elements->size());
  for (size_t i = 0; i < _elements->size(); i++) {
    _sorted->push_back(std::make_pair((*_elements)[i], i));
  }
  std::sort(_sorted->begin(), _sorted->end(), Less(*this));
}

void Semigroup::nr_idempotents_thread(size_t  thread_id,
                                      size_t& nr,
                                      pos_t   begin,
                                      pos_t   end,
                                      bool    report) {
  Reporter reporter(*this);
  reporter.report(report);
  reporter.start_timer();
  reporter(__func__, thread_id);

  for (pos_t i = begin; i < end; i++) {
    if (product_by_reduction(i, i) == i) {
      nr++;
    }
  }
  reporter.stop_timer();
}

void inline Semigroup::closure_update(pos_t              i,
                                      letter_t           j,
                                      letter_t           b,
                                      letter_t           s,
                                      std::vector<bool>& old_new,
                                      pos_t              old_nr) {
  if (_wordlen != 0 && !_reduced.get(s, j)) {
    size_t r = _right->get(s, j);
    if (_found_one && r == _pos_one) {
      _right->set(i, j, _genslookup.at(b));
    } else if (_prefix.at(r) != (size_t) -1) {
      _right->set(
          i, j, _right->get(_left->get(_prefix.at(r), b), _final.at(r)));
    } else {
      _right->set(i, j, _right->get(_genslookup.at(b), _final.at(r)));
    }
  } else {
    _tmp_product->redefine(_elements->at(i), _gens->at(j));
    auto it = _map.find(_tmp_product);
    if (it == _map.end()) {  // it's new!
      is_one(_tmp_product, _nr);
      _elements->push_back(_tmp_product->really_copy());
      _first.push_back(b);
      _final.push_back(j);
      _length.push_back(_wordlen + 2);
      _map.insert(std::make_pair(_elements->back(), _nr));
      _prefix.push_back(i);
      _reduced.set(i, j, true);
      _right->set(i, j, _nr);
      if (_wordlen == 0) {
        _suffix.push_back(_genslookup.at(j));
      } else {
        _suffix.push_back(_right->get(s, j));
      }
      _index.push_back(_nr);
      _nr++;
    } else if (it->second < old_nr && !old_new.at(it->second)) {
      // we didn't process it yet!
      is_one(_tmp_product, it->second);
      _first.at(it->second)  = b;
      _final.at(it->second)  = j;
      _length.at(it->second) = _wordlen + 2;
      _prefix.at(it->second) = i;
      _reduced.set(i, j, true);
      _right->set(i, j, it->second);
      if (_wordlen == 0) {
        _suffix.at(it->second) = _genslookup.at(j);
      } else {
        _suffix.at(it->second) = _right->get(s, j);
      }
      _index.push_back(it->second);
      old_new.at(it->second) = true;
    } else {  // it->second >= old->_nr || old_new.at(it->second)
      // it's old
      _right->set(i, j, it->second);
      _nrrules++;
    }
  }
}

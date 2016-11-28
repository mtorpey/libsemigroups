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

// This file contains implementations of the non-template derived classes of
// the Elements abstract base class.

#include "elements.h"

#include <algorithm>

// BooleanMat

BooleanMat::BooleanMat(std::vector<std::vector<bool>> const& matrix)
    : ElementWithVectorData<bool, BooleanMat>() {
  assert(matrix.size() != 0);
  assert(all_of(matrix.begin(), matrix.end(), [matrix](std::vector<bool> row) {
    return row.size() == matrix.size();
  }));
  this->_vector->reserve(matrix.size() * matrix.size());
  for (auto const& row : matrix) {
    for (auto entry : row) {
      this->_vector->push_back(entry);
    }
  }
}

size_t BooleanMat::complexity() const {
  return pow(this->degree(), 3);
}

size_t BooleanMat::degree() const {
  return sqrt(_vector->size());
}

size_t BooleanMat::hash_value() const {
  size_t seed = 0;
  for (size_t i = 0; i < _vector->size(); i++) {
    seed = ((seed << 1) + _vector->at(i));
  }
  return seed;
}

Element* BooleanMat::identity() const {
  std::vector<bool>* matrix(new std::vector<bool>());
  matrix->resize(_vector->size(), false);
  for (size_t i = 0; i < this->degree(); i++) {
    matrix->at(i * this->degree() + i) = true;
  }
  return new BooleanMat(matrix);
}

// multiply x and y into this
void BooleanMat::redefine(Element const* x, Element const* y) {
  assert(x->degree() == y->degree());
  assert(x->degree() == this->degree());
  assert(x != this && y != this);

  size_t             k;
  size_t             dim = this->degree();
  std::vector<bool>* xx(static_cast<BooleanMat const*>(x)->_vector);
  std::vector<bool>* yy(static_cast<BooleanMat const*>(y)->_vector);

  for (size_t i = 0; i < dim; i++) {
    for (size_t j = 0; j < dim; j++) {
      for (k = 0; k < dim; k++) {
        if (xx->at(i * dim + k) && yy->at(k * dim + j)) {
          break;
        }
      }
      _vector->at(i * dim + j) = (k < dim);
    }
  }
}

// Bipartition

std::vector<u_int32_t> Bipartition::_fuse     = std::vector<u_int32_t>();
std::vector<u_int32_t> Bipartition::_lookup   = std::vector<u_int32_t>();
u_int32_t              Bipartition::UNDEFINED = -1;

u_int32_t Bipartition::block(size_t pos) const {
  assert(pos < 2 * degree());
  return (*_vector)[pos];
}

size_t Bipartition::complexity() const {
  return (_vector->empty() ? 0 : pow(_vector->size(), 2));
}

size_t Bipartition::degree() const {
  return (_vector->empty() ? 0 : _vector->size() / 2);
}

size_t Bipartition::hash_value() const {
  size_t seed = 0;
  size_t deg  = this->_vector->size();
  for (auto const& val : *(this->_vector)) {
    seed *= deg;
    seed += val;
  }
  return seed;
}

// the identity of this
Element* Bipartition::identity() const {
  std::vector<u_int32_t>* blocks(new std::vector<u_int32_t>());
  blocks->reserve(this->_vector->size());
  for (size_t j = 0; j < 2; j++) {
    for (u_int32_t i = 0; i < this->degree(); i++) {
      blocks->push_back(i);
    }
  }
  return new Bipartition(blocks);
}

// multiply x and y into this
void Bipartition::redefine(Element const* x, Element const* y) {
  assert(x->degree() == y->degree());
  assert(x->degree() == this->degree());
  assert(x != this && y != this);
  u_int32_t n = this->degree();

  Bipartition const* xx = static_cast<Bipartition const*>(x);
  Bipartition const* yy = static_cast<Bipartition const*>(y);

  std::vector<u_int32_t>* xblocks(xx->_vector);
  std::vector<u_int32_t>* yblocks(yy->_vector);

  u_int32_t nrx(xx->const_nr_blocks());
  u_int32_t nry(yy->const_nr_blocks());

  _fuse.clear();
  _fuse.reserve(nrx + nry);
  _lookup.clear();
  _lookup.reserve(nrx + nry);

  for (size_t i = 0; i < nrx + nry; i++) {
    _fuse.push_back(i);
    _lookup.push_back(-1);
  }

  for (size_t i = 0; i < n; i++) {
    u_int32_t j = fuseit(xblocks->at(i + n));
    u_int32_t k = fuseit(yblocks->at(i) + nrx);
    if (j != k) {
      if (j < k) {
        _fuse.at(k) = j;
      } else {
        _fuse.at(j) = k;
      }
    }
  }

  u_int32_t next = 0;

  for (size_t i = 0; i < n; i++) {
    u_int32_t j = fuseit((*xblocks)[i]);
    if (_lookup[j] == (u_int32_t) -1) {
      _lookup[j] = next;
      next++;
    }
    (*this->_vector)[i] = _lookup[j];
  }

  for (size_t i = n; i < 2 * n; i++) {
    u_int32_t j = fuseit((*yblocks)[i] + nrx);
    if (_lookup[j] == (u_int32_t) -1) {
      _lookup[j] = next;
      next++;
    }
    (*this->_vector)[i] = _lookup[j];
  }
}

inline u_int32_t Bipartition::fuseit(u_int32_t pos) {
  while (_fuse.at(pos) < pos) {
    pos = _fuse.at(pos);
  }
  return pos;
}

// nr blocks

u_int32_t Bipartition::const_nr_blocks() const {
  if (_nr_blocks != Bipartition::UNDEFINED) {
    return _nr_blocks;
  } else if (degree() == 0) {
    return 0;
  }

  return *std::max_element(_vector->begin(), _vector->end()) + 1;
}

u_int32_t Bipartition::nr_blocks() {
  if (_nr_blocks == Bipartition::UNDEFINED) {
    _nr_blocks = this->const_nr_blocks();
  }
  return _nr_blocks;
}

u_int32_t Bipartition::nr_left_blocks() {
  if (_nr_left_blocks == Bipartition::UNDEFINED) {
    if (degree() == 0) {
      _nr_left_blocks = 0;
    } else {
      _nr_left_blocks =
          *std::max_element(_vector->begin(),
                            _vector->begin() + (_vector->size() / 2))
          + 1;
    }
  }
  return _nr_left_blocks;
}

u_int32_t Bipartition::nr_right_blocks() {
  return nr_blocks() - nr_left_blocks() + rank();
}

// Transverse blocks lookup table.
// Returns a vector of bools <out> for the left blocks so that <out[i]> is
// <true> if and only the <i>th block of <this> is a transverse block.
//
// @return a const std::vector<bool>*.

bool Bipartition::is_transverse_block(size_t index) {
  if (index < nr_left_blocks()) {
    init_trans_blocks_lookup();
    return _trans_blocks_lookup[index];
  }
  return false;
}

void Bipartition::init_trans_blocks_lookup() {
  if (_trans_blocks_lookup.empty() && degree() > 0) {
    _trans_blocks_lookup.resize(this->nr_left_blocks());
    for (auto it = _vector->begin() + degree(); it < _vector->end(); it++) {
      if ((*it) < this->nr_left_blocks()) {
        _trans_blocks_lookup[(*it)] = true;
      }
    }
  }
}

size_t Bipartition::rank() {
  if (_rank == this->UNDEFINED) {
    init_trans_blocks_lookup();
    _rank = std::count(
        _trans_blocks_lookup.begin(), _trans_blocks_lookup.end(), true);
  }
  return _rank;
}

Blocks* Bipartition::left_blocks() {
  if (degree() == 0) {
    return new Blocks();
  }
  init_trans_blocks_lookup();
  return new Blocks(
      new std::vector<u_int32_t>(_vector->begin(),
                                 _vector->begin() + (_vector->size() / 2)),
      new std::vector<bool>(_trans_blocks_lookup));
}

Blocks* Bipartition::right_blocks() {
  if (degree() == 0) {
    return new Blocks();
  }

  std::vector<u_int32_t>* blocks        = new std::vector<u_int32_t>();
  std::vector<bool>*      blocks_lookup = new std::vector<bool>();

  // must reindex the blocks
  _lookup.clear();
  _lookup.resize(this->nr_blocks(), Bipartition::UNDEFINED);
  u_int32_t nr_blocks = 0;

  for (auto it = _vector->begin() + (_vector->size() / 2); it < _vector->end();
       it++) {
    if (_lookup[*it] == Bipartition::UNDEFINED) {
      _lookup[*it] = nr_blocks;
      blocks_lookup->push_back(this->is_transverse_block(*it));
      nr_blocks++;
    }
    blocks->push_back(_lookup[*it]);
  }

  return new Blocks(blocks, blocks_lookup, nr_blocks);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// Matrices over semirings
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

MatrixOverSemiring::MatrixOverSemiring(
    std::vector<std::vector<int64_t>> const& matrix,
    Semiring*                                semiring)
    : ElementWithVectorData<int64_t, MatrixOverSemiring>(),
      _semiring(semiring) {
  assert(matrix.size() != 0);
  assert(
      all_of(matrix.begin(), matrix.end(), [matrix](std::vector<int64_t> row) {
        return row.size() == matrix.size();
      }));

  this->_vector->reserve(matrix.size() * matrix.size());
  for (auto const& row : matrix) {
    for (auto const& entry : row) {
      this->_vector->push_back(entry);
    }
  }
}

Semiring* MatrixOverSemiring::semiring() const {
  return _semiring;
}

size_t MatrixOverSemiring::complexity() const {
  return pow(this->degree(), 3);
}

size_t MatrixOverSemiring::degree() const {
  return sqrt(_vector->size());
}

size_t MatrixOverSemiring::hash_value() const {
  int64_t seed = 0;
  for (int64_t x : *_vector) {
    seed += ((seed << 4) + x);
  }
  return seed;
}

// the identity
Element* MatrixOverSemiring::identity() const {
  std::vector<int64_t>* matrix(new std::vector<int64_t>());
  matrix->resize(_vector->size(), _semiring->zero());

  size_t n = this->degree();
  for (size_t i = 0; i < n; i++) {
    matrix->at(i * n + i) = _semiring->one();
  }
  return new MatrixOverSemiring(matrix, _semiring);
}

Element* MatrixOverSemiring::really_copy(size_t increase_degree_by) const {
  MatrixOverSemiring* copy(static_cast<MatrixOverSemiring*>(
      ElementWithVectorData::really_copy(increase_degree_by)));
  // FIXME the previous line should be something like (increase_degree_by +
  // degree()) ^ 2 - degree(), and then something more complicated should be
  // done to actually cpy the matrix entries correctly.
  assert(copy->_semiring == nullptr);
  copy->_semiring = _semiring;
  return copy;
}

void MatrixOverSemiring::redefine(Element const* x, Element const* y) {
  MatrixOverSemiring const* xx(static_cast<MatrixOverSemiring const*>(x));
  MatrixOverSemiring const* yy(static_cast<MatrixOverSemiring const*>(y));

  assert(xx->degree() == yy->degree());
  assert(xx->degree() == this->degree());
  assert(xx != this && yy != this);
  assert(xx->semiring() == yy->semiring()
         && xx->semiring() == this->semiring());
  size_t deg = this->degree();

  for (size_t i = 0; i < deg; i++) {
    for (size_t j = 0; j < deg; j++) {
      int64_t v = _semiring->zero();
      for (size_t k = 0; k < deg; k++) {
        v = _semiring->plus(
            v, _semiring->prod(xx->at(i * deg + k), yy->at(k * deg + j)));
      }
      _vector->at(i * deg + j) = v;
    }
  }
  after();  // post process this
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// Projective max-plus matrices
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

size_t ProjectiveMaxPlusMatrix::hash_value() const {
  size_t seed = 0;
  for (size_t i = 0; i < _vector->size(); i++) {
    seed = ((seed << 4) + _vector->at(i));
  }
  return seed;
}

void ProjectiveMaxPlusMatrix::after() {
  int64_t norm = *std::max_element(_vector->begin(), _vector->end());
  size_t  deg  = pow(this->degree(), 2);

  for (size_t i = 0; i < deg; i++) {
    if (_vector->at(i) != LONG_MIN) {
      _vector->at(i) -= norm;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// Partitioned binary relations (PBRs)
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

std::vector<bool> PBR::x_seen;
std::vector<bool> PBR::y_seen;
RecVec<bool>      PBR::out;
RecVec<bool>      PBR::tmp;

size_t PBR::complexity() const {
  return pow((2 * this->degree()), 3);
}

size_t PBR::degree() const {
  return _vector->size() / 2;
}

size_t PBR::hash_value() const {
  size_t       seed = 0;
  size_t const pow  = 101;
  for (auto const& row : *this->_vector) {
    for (auto const& val : row) {
      seed *= pow;
      seed += val;
    }
  }
  return seed;
}

Element* PBR::identity() const {
  std::vector<std::vector<u_int32_t>>* adj(
      new std::vector<std::vector<u_int32_t>>());
  size_t n = this->degree();
  adj->reserve(2 * n);
  for (u_int32_t i = 0; i < 2 * n; i++) {
    adj->push_back(std::vector<u_int32_t>());
  }
  for (u_int32_t i = 0; i < n; i++) {
    adj->at(i).push_back(i + n);
    adj->at(i + n).push_back(i);
  }
  return new PBR(adj);
}

void PBR::redefine(Element const* xx, Element const* yy) {
  assert(xx->degree() == yy->degree());
  assert(xx->degree() == this->degree());
  assert(xx != this && yy != this);

  PBR const* x(static_cast<PBR const*>(xx));
  PBR const* y(static_cast<PBR const*>(yy));

  u_int32_t const n = this->degree();

  if (x_seen.size() != 2 * n) {
    x_seen.clear();
    x_seen.resize(2 * n, false);
    y_seen.clear();
    y_seen.resize(2 * n, false);
    out.clear();
    out.add_cols(2 * n);
    out.add_rows(2 * n);
    tmp.clear();
    tmp.add_cols(2 * n + 1);
  } else {
    std::fill(x_seen.begin(), x_seen.end(), false);
    std::fill(y_seen.begin(), y_seen.end(), false);
    std::fill(out.begin(), out.end(), false);
    std::fill(tmp.begin(), tmp.end(), false);
  }

  for (size_t i = 0; i < n; i++) {
    for (auto const& j : (*x)[i]) {
      if (j < n) {
        out.set(i, j, true);
      } else if (j < tmp.nr_rows() && tmp.get(j, 0)) {
        unite_rows(i, j);
      } else {
        if (j >= tmp.nr_rows()) {
          tmp.add_rows(j - tmp.nr_rows() + 1);
        }
        tmp.set(j, 0, true);
        x_seen[i] = true;
        y_dfs(n, j - n, x, y, j);
        unite_rows(i, j);
        std::fill(x_seen.begin(), x_seen.end(), false);
        std::fill(y_seen.begin(), y_seen.end(), false);
      }
      if (out.all_of(i, [](bool val) { return val; })) {
        break;
      }
    }
  }

  for (size_t i = n; i < 2 * n; i++) {
    for (auto const& j : (*y)[i]) {
      if (j >= n) {
        out.set(i, j, true);
      } else if (j < tmp.nr_rows() && tmp.get(j, 0)) {
        unite_rows(i, j);
      } else {
        if (j >= tmp.nr_rows()) {
          tmp.add_rows(j - tmp.nr_rows() + 1);
        }
        tmp.set(j, 0, true);
        y_seen[i] = true;
        x_dfs(n, j + n, x, y, j);
        unite_rows(i, j);
        std::fill(x_seen.begin(), x_seen.end(), false);
        std::fill(y_seen.begin(), y_seen.end(), false);
      }
      if (out.all_of(i, [](bool val) { return val; })) {
        break;
      }
    }
  }

  for (size_t i = 0; i < 2 * n; i++) {
    (*_vector)[i].clear();
    for (size_t j = 0; j < 2 * n; j++) {
      if (out.get(i, j)) {
        (*_vector)[i].push_back(j);
      }
    }
  }
}

inline void PBR::unite_rows(size_t const& i, size_t const& j) {
  for (size_t k = 0; k < out.nr_cols(); k++) {
    out.set(i, k, (out.get(i, k) || tmp.get(j, k + 1)));
  }
}

void PBR::x_dfs(u_int32_t const& n,
                u_int32_t const& i,
                PBR const* const x,
                PBR const* const y,
                size_t const&    adj) {
  if (!x_seen[i]) {
    x_seen[i] = true;
    for (auto const& j : (*x)[i]) {
      if (j < n) {
        tmp.set(adj, j + 1, true);
      } else {
        y_dfs(n, j - n, x, y, adj);
      }
    }
  }
}

void PBR::y_dfs(u_int32_t const& n,
                u_int32_t const& i,
                PBR const* const x,
                PBR const* const y,
                size_t const&    adj) {
  if (!y_seen[i]) {
    y_seen[i] = true;
    for (auto const& j : (*y)[i]) {
      if (j >= n) {
        tmp.set(adj, j + 1, true);
      } else {
        x_dfs(n, j + n, x, y, adj);
      }
    }
  }
}

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

#ifndef TC_H_
#define TC_H_

#include <atomic>
#include <forward_list>
#include <mutex>
#include <stack>
#include <string>
#include <utility>
#include <vector>

#include "semigroups++/elements.h"
#include "semigroups++/report.h"
#include "semigroups++/semigroups.h"

class Congruence {

  enum cong_t { LEFT = 0, RIGHT = 1, TWOSIDED = 2 };

  typedef size_t  coset_t;
  typedef int64_t signed_coset_t;

 public:
  Congruence(std::string                    type,
             size_t                         nrgens,
             std::vector<relation_t> const& relations,
             std::vector<relation_t> const& extra,
             size_t                         thread_id = 0);

  Congruence(std::string                    type,
             Semigroup*                     semigroup,
             std::vector<relation_t> const& extra,
             bool                           prefill,
             size_t                         thread_id = 0);

  Congruence(std::string                    type,
             size_t                         nrgens,
             std::vector<relation_t> const& relations,
             std::vector<relation_t> const& extra,
             RecVec<coset_t>&               prefill,
             size_t                         thread_id = 0);

  ~Congruence() {}

  void todd_coxeter(size_t limit = INFTY);

  size_t nr_active_cosets() {
    return _active;
  }

  coset_t word_to_coset(word_t);

  void terminate();
  bool is_tc_done();
  void set_report(bool val);
  void compress();

  size_t nr_classes() {
    if (!is_tc_done()) {
      todd_coxeter();
    }
    return _active - 1;
  }

 private:
  Congruence(cong_t                         type,
             size_t                         nrgens,
             std::vector<relation_t> const& relations,
             std::vector<relation_t> const& extra,
             size_t                         thread_id = 0);

  Congruence(cong_t                         type,
             Semigroup*                     semigroup,
             std::vector<relation_t> const& extra,
             bool                           prefill,
             size_t                         thread_id = 0);

  Congruence(cong_t                         type,
             size_t                         nrgens,
             std::vector<relation_t> const& relations,
             std::vector<relation_t> const& extra,
             RecVec<coset_t>&               prefill,
             size_t                         thread_id = 0);

  void init_after_prefill();

  void        new_coset(coset_t const&, letter_t const&);
  void        identify_cosets(coset_t, coset_t);
  inline void trace(coset_t const&, relation_t const&, bool add = true);
  void   check_forwd();
  cong_t type_from_string(std::string);

  cong_t _type;

  bool _tc_done; // Has todd_coxeter already been run?
  bool _is_compressed;

  coset_t                 _id_coset; // TODO: Remove?
  size_t                  _nrgens;
  std::vector<relation_t> _relations;
  std::vector<relation_t> _extra;

  size_t _active; // Number of active cosets

  size_t _pack; // Nr of active cosets allowed before a
                // packing phase starts

  std::atomic<bool> _stop;

  //
  // COSET LISTS:
  //
  // We use these two arrays to simulate a doubly-linked list of active cosets
  // (the "active list") with deleted cosets attached to the end (the "free
  // list").  If <c> is an active coset:
  //   _forwd[c] is the coset that comes after <c> in the list.
  //   _bckwd[c] is the coset that comes before <c> in the list.
  // If <c> is a free coset (has been deleted) the backward reference is not
  // needed, and so instead, _bckwd[c] is set to the coset <c> was identified
  // with.  To indicate this alternative use of the list, the entry is negated
  // (_backwd[c] == -3 indicates that <c> was identified with coset 3).
  //
  std::vector<coset_t>        _forwd;
  std::vector<signed_coset_t> _bckwd;
  //
  // We also store some special locations in the list:
  //   _current is the coset to which we are currently applying relations.
  //   _current_no_add is used instead of _current if we are in a packing phase.
  //   _last points to the final active coset in the list.
  //   _next points to the first free coset in the list.
  // Hence usually _next == _last + 1.
  //
  coset_t _current;
  coset_t _current_no_add;
  coset_t _last;
  coset_t _next;

  //
  // COSET TABLES:
  //
  // We use these three tables to store all a coset's images and preimages.
  //   _table[c][i] is coset c's image under generator i.
  //   _preim_init[c][i] is ONE of coset c's preimages under generator i.
  //   _preim_next[c][i] is a coset that has THE SAME IMAGE as coset c (under i)
  //
  // Hence to find all the preimages of c under i:
  //   - Let u = _preim_init[c][i] ONCE.
  //   - Let u = _preim_next[u][i] REPEATEDLY until it becomes UNDEFINED.
  // Each u is one preimage.
  //
  // To add v, a new preimage of c under i:
  //   - Set _preim_next[v][i] to point to the current _preim_init[c][i].
  //   - Then change _preim_init[c][i] to point to v.
  // Now the new preimage and all the old preimages are stored.
  //
  RecVec<coset_t> _table;
  RecVec<coset_t> _preim_init;
  RecVec<coset_t> _preim_next;

  // Stacks for identifying cosets
  std::stack<coset_t> _lhs_stack;
  std::stack<coset_t> _rhs_stack;

  // Statistics etc.
  bool   _report;
  size_t _defined;
  size_t _killed;
  size_t _stop_packing; // TODO: make this a bool?
  size_t _next_report;

  size_t _thread_id;

  static size_t   INFTY;
  static size_t   UNDEFINED;
  static Reporter _reporter;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

Congruence* parallel_todd_coxeter(Congruence* cong_t,
                                  Congruence* cong_f,
                                  bool        report = false);

Congruence* cong_pairs_enumerate(std::string,
                                 Semigroup*,
                                 std::vector<relation_t> const&,
                                 bool report = false);
#endif // TC_H_

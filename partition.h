//
// Semigroups++ - C/C++ library for computing with semigroups and monoids
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

#ifndef LIBSEMIGROUPS_PARTITION_H_
#define LIBSEMIGROUPS_PARTITION_H_

#include <assert.h>

#include <algorithm>
#include <vector>

namespace libsemigroups {

  // Non-abstract
  // Class for representing a partition of a semigroup, i.e. an equivalence
  // relation on some subset of the semigroup.  T is the type of element stored,
  // and may be a pointer type, e.g. word_t*
  //
  template <typename T> class Partition {
   public:
    // 1 parameter
    //
    // This constructor returns a partition with the specified number of empty
    // classes.
    Partition(size_t nr_classes) // remove
      : _classes(new std::vector<std::vector<T>>(nr_classes, std::vector<T>())) {}

    // O parameters
    //
    // This constructor returns a partition with no classes
    Partition()
      : Partition(0) {}

    //
    // A default destructor.
    ~Partition() {
      delete _classes;
    }

    // non-const
    //
    // Delete every element in the partition
    // This is only valid if T is a pointer type
    void really_delete() {
      assert(std::is_pointer<T>::value);
      for (std::vector<T> block : *_classes) {
        for (T elm : block) {
          delete elm;
        }
      }
    }

    // const
    //
    // @return the number of classes in the partition
    size_t nr_classes() const {
      return _classes->size();
    }

    // const
    // @class_nr index of a class in the partition
    //
    // @return the number of elements in the specified class
    size_t size_of_class(size_t class_nr) {
      assert(class_nr < nr_classes());
      return (*_classes)[class_nr].size();
    }

    // const
    // @class_nr index of a class in the partition
    // @elm_nr index of an element in the specified class
    //
    // @return the element in the given position in the given class
    T at(size_t class_nr, size_t elm_nr) const {
      assert(class_nr < nr_classes());
      assert(elm_nr < size_of_class(class_nr));
      return (*_classes)[class_nr][elm_nr];
    }

    // non-const
    // @class_nr index of a class in the partition
    // @elm an element to be added to the class
    //
    // Add the given element to the class with the given index
    void add_element(size_t class_nr, T elm) {
      assert(class_nr < nr_classes());
      (*_classes)[class_nr].push_back(elm);
    }

    // non-const
    //
    // Add a single empty class to the partition
    void add_class() {
      _classes->push_back(std::vector<T>());
    }

    // non-const
    // @nr number of classes to be added
    //
    // Add a given number of empty classes to the partition
    void add_classes(size_t nr) {
      for (size_t i = 0; i < nr; i++) {
        add_class();
      }
    }

    // non-const
    //
    // Remove any classes which contain precisely one element, deleting the
    // element contained within.  This will change the index of any class above
    // the lowest-numbered singleton class.
    void remove_singletons() {
      // Get an iterator to all the singleton classes
      auto s_it = std::find_if(_classes->begin(), _classes->end(), is_singleton);
      for (auto it : s_it) {
        delete (*it)[0];
      }
      // Erase them all (we use an iterator so we reallocate only once)
      _classes->erase(s_it.begin(), s_it.end());
    }

   private:
    std::vector<std::vector<T>>* _classes;

    // lambda this
    static bool is_singleton(std::vector<T>& block const) {
      return (block.size == 1);
    }
  };
}  // namespace libsemigroups
#endif  // LIBSEMIGROUPS_PARTITION_H_

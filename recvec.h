/*
 * Semigroups++
 *
 * This file contains a method for a 2-dimensional vector class.
 *
 */

#ifndef RECVEC_H_
#define RECVEC_H_

#include <assert.h>
#include <algorithm>
#include <iostream>
#include <vector>

template <typename T>
class RecVec {

      public:

        RecVec () {
          //std::cout << "RecVec: default constructor!\n";
        };

        /***********************************************************************
         * constructor
        ***********************************************************************/

        explicit RecVec (size_t nr_cols, size_t nr_rows = 0, T default_val = 0)
          : _vec            (),
            _nr_used_cols   (nr_cols),
            _nr_unused_cols (0),
            _nr_rows        (0),
            _default_val    (default_val)
        {
          this->add_rows(nr_rows);
        }

        /***********************************************************************
         * copy constructor
        ***********************************************************************/

        RecVec (const RecVec& copy)
          : _vec            (copy._vec),
            _nr_used_cols   (copy._nr_used_cols),
            _nr_unused_cols (copy._nr_unused_cols),
            _nr_rows        (copy._nr_rows),
            _default_val    (copy._default_val)
        {
          //std::cout << "RecVec: copy constructor!\n";
        }

        /***********************************************************************
         * constructor from pointer
        ***********************************************************************/

        explicit RecVec (RecVec* copy)
          : _vec            (copy->_vec),
            _nr_used_cols   (copy->_nr_used_cols),
            _nr_unused_cols (copy->_nr_unused_cols),
            _nr_rows        (copy->_nr_rows),
            _default_val    (copy->_default_val)
        {
          //std::cout << "RecVec: copy constructor!\n";
          //assert((_nr_used_cols + _nr_unused_cols) * _nr_rows == _vec.size());
        }

        /***********************************************************************
         * copy from <copy> and add new cols constructor
        ***********************************************************************/

        RecVec (const RecVec& copy, size_t nr_cols_to_add)
          : _vec            (),
            _nr_used_cols   (copy._nr_used_cols),
            _nr_unused_cols (copy._nr_unused_cols),
            _nr_rows        (copy.nr_rows()),
            _default_val    (copy._default_val) {

          if (nr_cols_to_add <= _nr_unused_cols) {
            _vec = copy._vec;
            _nr_used_cols += nr_cols_to_add;
            _nr_unused_cols -= nr_cols_to_add;
            return;
          }

          size_t new_nr_cols = std::max(5 * nr_cols() / 4 + 4,
                                        nr_cols_to_add + nr_cols());
          _nr_used_cols += nr_cols_to_add;
          _nr_unused_cols = new_nr_cols - _nr_used_cols;

          _vec.reserve(new_nr_cols * _nr_rows);

          for (size_t i = 0; i < _nr_rows; i++) {
            size_t j;
            for (j = 0; j < copy._nr_used_cols; j++) {
              _vec.push_back(copy.get(i, j));
            }
            for (; j < new_nr_cols; j++) {
              _vec.push_back(_default_val);
            }
          }
        }

        /***********************************************************************
         * destructor
        ***********************************************************************/

        ~RecVec() {}

        /***********************************************************************
         * add_rows: add <nr> new rows defaults to add one row
        ***********************************************************************/

        void inline add_rows (size_t nr = 1) {
          _nr_rows += nr;
          _vec.resize(_vec.size() + (_nr_used_cols + _nr_unused_cols) * nr,
                      _default_val);
        }

        /***********************************************************************
         * add_columns: add new columns
        ***********************************************************************/

        void add_cols (size_t nr) {

          if (nr <= _nr_unused_cols) {
            _nr_used_cols += nr;
            _nr_unused_cols -= nr;
            return;
          }

          size_t old_nr_cols = _nr_used_cols + _nr_unused_cols;
          size_t new_nr_cols = std::max(5 * old_nr_cols / 4 + 4, nr + old_nr_cols);

          _vec.resize(new_nr_cols * _nr_rows, _default_val);

          typename std::vector<T>::iterator old_it(_vec.begin() + (old_nr_cols * _nr_rows) - old_nr_cols);
          typename std::vector<T>::iterator new_it(_vec.begin() + (new_nr_cols * _nr_rows) - new_nr_cols);

          while (old_it != _vec.begin()) {
            std::move(old_it, old_it + _nr_used_cols, new_it);
            old_it -= old_nr_cols;
            new_it -= new_nr_cols;
          }
          _nr_used_cols += nr;
          _nr_unused_cols = new_nr_cols - _nr_used_cols;
        }

        /***********************************************************************
         * set: set the value of vec[i][j] = val
        ***********************************************************************/

        void inline set (size_t i, size_t j, T val) {
          //assert(((_nr_used_cols + _nr_unused_cols) * _nr_rows) == _vec.size());
          assert(i < _nr_rows && j < _nr_used_cols);
          _vec[i * (_nr_used_cols + _nr_unused_cols) + j] = val;
        }

        /***********************************************************************
         * get: get the value in vec[i][j]
        ***********************************************************************/

        T inline get (size_t i, size_t j) const {
          //assert(((_nr_used_cols + _nr_unused_cols) * _nr_rows) == _vec.size());
          assert(i < _nr_rows && j < _nr_used_cols);
          return _vec[i * (_nr_used_cols + _nr_unused_cols) + j];
        }

        /***********************************************************************
         * clear: set all values to _default_val, set number of used cols
         * to 0, does not reset the number of rows!!!
        ***********************************************************************/

        void inline clear () {
          size_t nr_cols = _nr_used_cols + _nr_unused_cols;
          for (size_t i = 0; i < _nr_rows; i++) {
            for (size_t j = 0; j < _nr_used_cols; j++) {
              _vec[i * nr_cols + j] = _default_val;
            }
          }
          _nr_used_cols = 0;
        }

        /***********************************************************************
         * size: the total amount of used space
        ***********************************************************************/

        size_t size () const {
          return _nr_rows * _nr_used_cols;
        }

        /***********************************************************************
         * nr_rows: the number of rows (1st dimension)
        ***********************************************************************/

        size_t nr_rows () const {
          return _nr_rows;
        }

        /***********************************************************************
         * nr_cols: the number of columns (2nd dimension)
        ***********************************************************************/

        size_t nr_cols () const {
          return _nr_used_cols;
        }

        /***********************************************************************
         * cols_capacity: the total number of columns available!
        ***********************************************************************/

        size_t cols_capacity () const {
          return _nr_used_cols + _nr_unused_cols;
        }

        /***********************************************************************
         * adjoins the RecVec copy to the end of this
        ***********************************************************************/

        void adjoin (const RecVec<T>& copy) {
          assert(copy._nr_used_cols == _nr_used_cols);

          size_t old_nr_rows = _nr_rows;
          add_rows(copy._nr_rows);

          if (copy._nr_unused_cols == _nr_unused_cols) {
            std::copy(copy._vec.begin(),
                      copy._vec.end(),
                      _vec.begin() + (_nr_used_cols + _nr_unused_cols) *
                      old_nr_rows);
          } else {
            for (size_t i = old_nr_rows; i < _nr_rows; i++) {
              for (size_t j = 0; j < _nr_used_cols; j++) {
                set(i, j, copy.get(i - old_nr_rows, j));
              }
            }
          }
        }

        inline typename std::vector<T>::iterator begin () {
          return _vec.begin();
        }

        inline typename std::vector<T>::iterator end () {
          return _vec.end();
        }

      private:
        std::vector<T> _vec;
        size_t         _nr_used_cols;
        size_t         _nr_unused_cols;
        size_t         _nr_rows;
        T              _default_val;
};

#endif  // RECVEC_H_

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

// This file contains a class for reporting things during a computation.

#ifndef SEMIGROUPSPLUSPLUS_REPORT_H_
#define SEMIGROUPSPLUSPLUS_REPORT_H_

#include <assert.h>
#include <cxxabi.h>

#include <atomic>
#include <iostream>
#include <mutex>
#include <string>

#include "timer.h"

#define DEFAULT_LEVEL 2

// This is a simple class which can be used to print information to
// the standard output.

class Reporter {
 public:
  template <class T>
  explicit Reporter(T const& obj, size_t thread_id = 0)
      : _class(), _timer(), _thread_id(thread_id), _level(DEFAULT_LEVEL) {
    set_class_name(obj);
  }  // FIXME init other data members

  explicit Reporter(size_t thread_id = 0)
      : _class(), _timer(), _thread_id(thread_id), _level(DEFAULT_LEVEL) {}

  ~Reporter() {}

  template <class T> friend Reporter& operator<<(Reporter& rep, const T& tt) {
    if (!rep._report) {
      return rep;
    }
    if (rep._first_call && rep._level > 1) {
      std::cout << "Thread #" << rep._thread_id << ": ";

      if (rep._class != "") {
        std::cout << rep._class;
        if (rep._func != "") {
          std::cout << "::";
        }
      }
      if (rep._func != "") {
        std::cout << rep._func << ": ";
      }
      rep._first_call = false;
    }
    std::cout << tt;
    return rep;
  }

  Reporter& operator<<(std::ostream& (*function)(std::ostream&) ) {
    if (_report) {
      if (_first_call && _level > 1) {
        std::cout << "Thread #" << _thread_id << ": ";

        if (_class != "") {
          std::cout << _class;
          if (_func != "") {
            std::cout << "::";
          }
        }
        if (_func != "") {
          std::cout << _func << ": ";
        }
        _first_call = false;
      }
      std::cout << function;
    }
    return *this;
  }

  Reporter& operator()(std::string func = "", size_t thread_id = 0) {
    _thread_id  = thread_id;
    _func       = func;
    _first_call = true;
    return *this;
  }

  template <class T> void set_class_name(T const& obj) {
    _class = abi::__cxa_demangle(typeid(obj).name(), 0, 0, 0);
  }

  void lock() {
    if (_report) {
      _mtx.lock();
    }
  }

  void unlock() {
    if (_report) {
      _mtx.unlock();
    }
  }

  void report(bool val) {
    _report = val;
  }

  void start_timer() {
    if (_report) {
      _mtx.lock();
      _timer.start();
      _mtx.unlock();
    }
  }

  void stop_timer(std::string prefix = "elapsed time = ") {
    if (_report && _timer.is_running()) {
      _mtx.lock();
      (*this)(_func, _thread_id) << prefix << _timer.string() << std::endl;
      _timer.stop();
      _mtx.unlock();
    }
  }

  void set_level(size_t level) {
    _level = level;
  }

 private:
  std::string       _func;
  std::string       _class;
  std::mutex        _mtx;
  Timer             _timer;
  size_t            _thread_id;
  bool              _first_call;
  std::atomic<bool> _report;
  size_t            _level;
};

#endif  // SEMIGROUPSPLUSPLUS_REPORT_H_

/*******************************************************************************

 Copyright (C) 2016 James D. Mitchell

 This work is licensed under a Creative Commons Attribution-ShareAlike 4.0
 International License. See
 http://creativecommons.org/licenses/by-sa/4.0/

 For a discussion about this code and latest version:

*******************************************************************************/

#ifndef TIMER_H_
#define TIMER_H_

#include <assert.h>
#include <chrono>
#include <iostream>
#include <string>

//
// This is a simple class to which can be used to send timing information to
// the standard output.

class Timer {

  typedef std::chrono::duration<long long int, std::nano> nano_t;
  typedef std::chrono::steady_clock::time_point time_point_t;

 public:
  // Default constructor
  Timer() : _start(), _running(false) {}

  // Start the timer
  //
  // This starts the timer running if it is not already running. If it is
  // already running, then we **assert(false)**.
  void start() {
    _running = true;
    _start   = std::chrono::steady_clock::now();
  }

  // Stop the timer
  // @str prepend this to the printed statement (defaults to "")
  //
  // Stops the timer regardless of its state, and calls <print> with the
  // argument **str**.
  void stop(std::string str = "") {
    print(str);
    _running = false;
  }

  // Print elapsed time
  // @str prepend this to the printed statement (defaults to "")
  //
  // If the timer is running, then this prints the time elapsed since
  // <start> was called. The format of the returned value is
  // "Elapsed time = " followed by the time in some (hopefully) human
  // readable format.

  void print(std::string prefix = "") {
    if (_running) {
      std::cout << string(prefix);
    }
  }

  std::string string(std::string prefix = "") {
    if (_running) {
      time_point_t end     = std::chrono::steady_clock::now();
      nano_t       elapsed = std::chrono::duration_cast<nano_t>(end - _start);

      if (string_it<std::chrono::hours>(elapsed, prefix, "h ", 0)) {
        string_it<std::chrono::minutes>(elapsed, prefix, "m", 0);
        return prefix;
      } else if (string_it<std::chrono::minutes>(elapsed, prefix, "m ", 0)) {
        string_it<std::chrono::seconds>(elapsed, prefix, "s", 0);
        return prefix;
      } else if (string_it<std::chrono::milliseconds>(
                     elapsed, prefix, "ms ", 9)) {
        return prefix;
      } else if (string_it<std::chrono::microseconds>(
                     elapsed, prefix, "\u03BCs ", 9)) {
        return prefix;
      } else if (string_it<std::chrono::nanoseconds>(
                     elapsed, prefix, "ns ", 0)) {
        return prefix;
      }
    }
    return prefix;
  }

 private:
  std::chrono::steady_clock::time_point _start;
  bool                                  _running;

  template <typename T>
  bool string_it(nano_t&      elapsed,
                 std::string& str,
                 std::string  unit,
                 size_t       threshold) {
    T x = std::chrono::duration_cast<T>(elapsed);
    if (x > T(threshold)) {
      str += std::to_string(x.count()) + unit;
      elapsed -= x;
      return true;
    }
    return false;
  }
};

#endif // TIMER_H_

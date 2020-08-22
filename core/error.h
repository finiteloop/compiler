// Copyright 2020 Bret Taylor
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include <iostream>
#include <sstream>

#include "common.h"
#include "file.h"

namespace compiler {

// All compiler errors are reported through an instance of this abstract class.
// If you are embedding the compiler in another application, you can subclass
// this to redirect errors through the appropriate interface.
//
// If you are running from the command line, use the standard
// TerminalError implemenation below.
class Error {
 public:
  Error();
  virtual ~Error();

  // The error level, where WARNING represents a non-fatal error.
  enum class Level {
    WARNING,
    ERROR,
  };

  class Stream;

  // Reports the given error at the given file location.
  void report(shared_ptr<Location> location, Level level, string message);

  // Returns a stream-style interface for error-reporting:
  //
  //   error.report(ast->location) << "Unrecognized name: " << ast->identifier;
  //
  // The error is reported when the returned object is destructed.
  Stream report(shared_ptr<Location> location, Level level = Level::ERROR);

  // Returns the number of errors at or above the given level.
  inline size_t count(Level min_level = Level::ERROR) const {
    return min_level == Level::WARNING ? num_warnings_ + num_errors_ :
                                         num_errors_;
  }

 protected:
  // Displays the given error to the end user.
  virtual void show(shared_ptr<Location> location, Level level,
                    string message) = 0;

 private:
  size_t num_warnings_;
  size_t num_errors_;
};

// A helper class to support a stream-style interface for error-reporting:
//
//   TerminalError error;
//   error.report(ast->location) << "Unrecognized name: " << ast->identifier;
//
class Error::Stream {
 public:
  Stream(Error& error, shared_ptr<Location> location, Error::Level level);
  ~Stream();

  template <typename T>
  Stream& operator<<(const T& value) {
    message_ << value;
    return *this;
  }

 private:
  Error& error_;
  shared_ptr<Location> location_;
  Error::Level level_;
  std::stringstream message_;
};

// Prints errors to the terminal, using color if supported.
class TerminalError : public Error {
 public:
  TerminalError(Level min_level = Level::WARNING);

 protected:
  virtual void show(shared_ptr<Location> location, Level level,
                    string message) override;

 private:
  std::ostream& out_;
  Level min_level_;
  bool tty_;
};

}

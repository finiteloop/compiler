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

#include "common.h"
#include "location.h"

namespace compiler {

// Displays error messages from the compiler to the end user. We also track
// the number of warnings and errors so clients can track if there were
// errors at different phases of compilation.
class Error {
 public:
  class Terminal;

  enum Level {
    WARNING,
    ERROR,
  };

  virtual ~Error() {
  }

  // Reports an error at the given location in the given file.
  void report(Level level, Location location, const string& message);

  // Reports an error not associated with a file location.
  void report(Level level, const string& message);

  // Returns the number of errors at or above the given level.
  inline size_t count(Level min_level = ERROR) const {
    return min_level == WARNING ? warning_count_ + error_count_ : error_count_;
  }

 protected:
  virtual void display(Level level, Location location,
                       const string& message) = 0;
  virtual void display(Level level, const string& message) = 0;

 private:
  size_t error_count_ = 0;
  size_t warning_count_ = 0;
};

// An implementation of Error that prints errors to stderr, with color if
// stderr is a TTY.
class Error::Terminal : public Error {
 public:
  // Prints errors at or above the given minimum level.
  Terminal(Level min_level = WARNING) : min_level_(min_level) {
  }

 protected:
  void display(Level level, Location location, const string& message) override;
  void display(Level level, const string& message) override;

 private:
  Level min_level_;
};

};

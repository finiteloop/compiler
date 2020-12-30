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

#include "../core/common.h"

namespace compiler::commands {

// Our named colors, which make our command line interface more useful in
// terminals that support color.
class Color {
 public:
  Color(bool tty) : tty(tty) {
  }

  inline string error(const string& value) const {
    if (tty) {
      return "\033[31;1m" + value + "\033[0m";
    } else {
      return value;
    }
  }

  inline string success(const string& value) const {
    if (tty) {
      return "\033[32;1m" + value + "\033[0m";
    } else {
      return value;
    }
  }

  inline string command(const string& value) const {
    if (tty) {
      return "\033[1m" + value + "\033[0m";
    } else {
      return value;
    }
  }

  inline string option(const string& value) const {
    if (tty) {
      return "\033[2m[" + value + "]\033[0m";
    } else {
      return "[" + value + "]";
    }
  }

  inline string arguments(const string& value) const {
    if (tty) {
        return "\033[2m" + value + "\033[0m";
    } else {
      return value;
    }
  }

  bool tty;
};

}

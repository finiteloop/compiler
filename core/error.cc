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

#include "error.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <unistd.h>
#include <utf8.h>

namespace compiler {

namespace {

// Formats colors for TTY-compatible terminals.
class Color {
 public:
  Color(Error::Level level) : level_(level) {
  }

  inline string underline(const string& value) {
    return (level_ == Error::WARNING ? "\033[33;1;4m" : "\033[31;1;4m") +
           value + "\033[0m";
  }

  inline string bold(const string& value) {
    return (level_ == Error::WARNING ? "\033[33;1m" : "\033[31;1m") + value +
           "\033[0m";
  }

  inline string colorful(const string& value) {
    return (level_ == Error::WARNING ? "\033[33m" : "\033[31m") + value +
           "\033[0m";
  }

  inline string light(const string& value) {
    return "\033[2m" + value + "\033[0m";
  }

 private:
  Error::Level level_;
};

}

// Left pads the given line number based on a given uniform prefix size.
static string line_number_prefix(int line_number, string prefix = "",
                                 size_t prefix_size = 0) {
  std::stringstream s;
  s << line_number;
  auto value = s.str();
  string result;
  for (size_t i = value.size(); i < 6 - prefix_size; i++) {
    result += " ";
  }
  return result + prefix + "\033[2m" + value + "\033[0m\t";
}

// Displays the given message in color in addition to printing an excerpt of
// the given file at the given location, highlighting the erroneous segment.
static void display_tty_error(Error::Level level, Location location,
                              const string& message) {
  // Display the error message
  Color color(level);
  std::error_code error;
  std::cerr << color.underline(
                   filesystem::proximate(*location.begin.path, error).string() +
                   ":" + std::to_string(location.begin.line))
            << color.colorful(" · " + message) << std::endl;

  // Don't try to display context for special paths like /dev/stdin.
  if (!filesystem::is_regular_file(*location.begin.path)) {
    return;
  }

  // Attempt to show an excerpt of the file and highlight the error
  static const size_t excerpt_window = 2;
  std::ifstream file(*location.begin.path);
  size_t line_number = 0;
  bool printed_excerpt = false;
  string line;
  while (std::getline(file, line)) {
    line_number++;
    if (line_number == location.begin.line) {
      if (!printed_excerpt) {
        std::cerr << std::endl;
        printed_excerpt = true;
      }
      std::cerr << line_number_prefix(line_number, color.bold("→ "), 2);
      size_t line_length = utf8::distance(line.begin(), line.end());
      if (location.end.line > location.begin.line ||
          line_length >= location.end.column) {
        // Highlight the erroneous segment of the line in color
        auto end_column = location.end.line > location.begin.line ?
                              line_length :
                              location.end.column;
        auto start = line.begin();
        utf8::advance(start, location.begin.column - 1, line.end());
        auto end = line.begin();
        utf8::advance(end, end_column, line.end());
        std::cerr << string(line.begin(), start)
                  << color.colorful(string(start, end))
                  << string(end, line.end()) << std::endl;
      } else {
        std::cerr << line << std::endl;
      }
    } else if (line_number + excerpt_window >= location.begin.line &&
               line_number <= location.begin.line + excerpt_window) {
      if (!printed_excerpt) {
        std::cerr << std::endl;
        printed_excerpt = true;
      }
      std::cerr << line_number_prefix(line_number) << color.light(line)
                << std::endl;
      if (location.begin.line + excerpt_window == line_number) {
        break;
      }
    }
  }
  if (printed_excerpt) {
    std::cerr << std::endl;
  }
}

void Error::report(Error::Level level, Location location,
                   const string& message) {
  if (level == WARNING) {
    warning_count_++;
  } else {
    error_count_++;
  }
  display(level, location, message);
}

void Error::report(Error::Level level, const string& message) {
  if (level == WARNING) {
    warning_count_++;
  } else {
    error_count_++;
  }
  display(level, message);
}

void Error::Terminal::display(Error::Level level, Location location,
                              const string& message) {
  assert(location.begin.path);
  if (min_level_ == ERROR && level == WARNING) {
    return;
  }
  if (isatty(STDERR_FILENO)) {
    display_tty_error(level, location, message);
  } else {
    std::error_code error;
    auto prefix = level == WARNING ? "Warning" : "Error";
    std::cerr << prefix << ": "
              << filesystem::proximate(*location.begin.path, error).string()
              << ":" << location.begin.line << ": " << message << std::endl;
  }
}

void Error::Terminal::display(Error::Level level, const string& message) {
  if (min_level_ == ERROR && level == WARNING) {
    return;
  }
  string prefix = level == WARNING ? "Warning" : "Error";
  if (isatty(STDERR_FILENO)) {
    Color color(level);
    std::error_code error;
    std::cerr << color.underline(prefix + ":") << color.colorful(" " + message)
              << std::endl;
  } else {
    std::cerr << prefix << ": " << message << std::endl;
  }
}

}

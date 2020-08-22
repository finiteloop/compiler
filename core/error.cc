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

#include <unistd.h>

namespace compiler {

// Returns the given path with the current working directory prefix removed.
static string short_path_name(const string& path) {
  char cwd[1024];
  if (getcwd(cwd, sizeof(cwd))) {
    size_t cwd_length = strlen(cwd);
    if (path.size() > cwd_length && path.substr(0, cwd_length) == cwd) {
      return path.substr(cwd_length + 1);
    }
  }
  return path;
}

// Left pads the given line number based on a given uniform prefix size.
static string line_number_prefix(int line_number, string prefix = "",
                                 size_t prefix_size = 0) {
  std::stringstream s;
  s << line_number;
  auto value = s.str();
  string result = "";
  for (size_t i = value.size(); i < 6 - prefix_size; i++) {
    result += " ";
  }
  return result + prefix + "\033[2m" + value + "\033[0m\t";
}

Error::Error() : num_warnings_(0), num_errors_(0) {
}

Error::~Error() {
}

void Error::report(shared_ptr<Location> location, Level level, string message) {
  switch (level) {
    case Level::WARNING:
      num_warnings_++;
      break;
    case Level::ERROR:
      num_errors_++;
      break;
  }
  show(location, level, message);
}

Error::Stream Error::report(shared_ptr<Location> location, Level level) {
  return Error::Stream(*this, location, level);
}

TerminalError::TerminalError(Error::Level min_level)
    : Error(),
      out_(std::cerr),
      min_level_(min_level),
      tty_(isatty(STDERR_FILENO)) {
}

void TerminalError::show(shared_ptr<Location> location, Error::Level level,
                         string message) {
  if (min_level_ == Level::ERROR && level == Level::WARNING) {
    return;
  }

  // Show in plaintext if stderr does not support ANSI colors
  if (!tty_) {
    if (location) {
      out_ << "[" << short_path_name(location->file->path) << ":"
           << location->start_line << "] ";
    }
    switch (level) {
      case Level::WARNING:
        out_ << "Warning: " << message << std::endl;
        break;
      case Level::ERROR:
        out_ << "Error: " << message << std::endl;
        break;
    }
  }

  // Otherwise, highlighting the error location in color
  string color = level == Level::WARNING ? "\033[33m" : "\033[31m";
  string bold = level == Level::WARNING ? "\033[33;1m" : "\033[31;1m";
  string underline = level == Level::WARNING ? "\033[33;1;4m" : "\033[31;1;4m";
  string reset = "\033[0m";

  // Print file location and error message
  std::stringstream formatted;
  if (location) {
    formatted << underline << short_path_name(location->file->path) << ":"
              << location->start_line << reset << color << " · " << reset;
  }
  formatted << color << message << reset << std::endl;

  // Print an excerpt from the file around the error
  if (location) {
    // How many lines around the erroneous line to print
    static const int excerpt_window = 2;
    bool printed_excerpt = false;
    std::stringstream in(location->file->contents);
    int line_number = 0;
    while (in.good()) {
      line_number++;
      std::string line;
      std::getline(in, line);
      if (line_number == location->start_line) {
        string arrow = bold + "→ " + reset;
        formatted << line_number_prefix(line_number, arrow, 2);
        if (line.size() >= location->end_column) {
          // Highlight the erroneous segment of the line in color
          auto end_column = location->end_line == location->start_line ?
                                location->end_column :
                                line.size();
          formatted << line.substr(0, location->start_column - 1) << color
                    << line.substr(location->start_column - 1,
                                   end_column - location->start_column + 1)
                    << reset << line.substr(end_column) << std::endl;
        } else {
          formatted << line << std::endl;
        }
      } else if (line_number + excerpt_window >= location->start_line &&
                 line_number - excerpt_window <= location->start_line) {
        if (!printed_excerpt) {
          formatted << std::endl;
          printed_excerpt = true;
        }
        formatted << line_number_prefix(line_number) << "\033[2m" << line
                  << reset << std::endl;
        if (location->start_line + excerpt_window == line_number) {
          break;
        }
      }
    }
    if (printed_excerpt) {
      formatted << std::endl;
    }
  }

  out_ << formatted.str();
}

Error::Stream::Stream(Error& error, shared_ptr<Location> location,
                      Error::Level level)
    : error_(error), location_(location), level_(level) {
}

Error::Stream::~Stream() {
  error_.report(location_, level_, message_.str());
}

}

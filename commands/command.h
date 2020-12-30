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

#include "../core/common.h"

namespace compiler::commands {

// A command-line option for a compiler commands.
class Option {
 public:
  // A FLAG takes no arguments and represents a Boolean flag. An OPTION
  // requires an argument.
  enum Type {
    FLAG,
    OPTION,
  };

  Option(const string& name, const string& description, Type type = FLAG,
         string default_value = "")
      : name(name),
        description(description),
        type(type),
        default_value(default_value) {
  }

  string name;
  string description;
  Type type;
  string default_value;
};

// One of our compiler commands (e.g., "run" or "build").
class Command {
 public:
  Command(const string& name, const string& description,
          const vector<Option>& options, string argument_placeholder = "")
      : name(name),
        description(description),
        options(options),
        argument_placeholder(argument_placeholder) {
  }

  virtual ~Command() {
  }

  // Runs this command with the given arguments, returning true if the command
  // successfully executes.
  bool run(const filesystem::path& executable, const vector<string>& arguments);

  // Prints the help information for this command to stderr.
  void print_help(const filesystem::path& executable);

  // Prints the help information for this command to the given stream. If `tty`
  // is true, we print using terminal colors.
  void print_help(const filesystem::path& executable, std::ostream& out,
                  bool tty);

  string name;
  string description;
  vector<Option> options;
  string argument_placeholder;

 protected:
  virtual bool execute(const filesystem::path& executable,
                       map<string, bool>& flags, map<string, string>& options,
                       vector<string>& arguments) = 0;
};

}

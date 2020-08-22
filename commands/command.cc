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

#include "command.h"

#include <algorithm>
#include <iostream>
#include <unistd.h>

#include "color.h"

namespace compiler::commands {

Option::Option(const string& name, const string& description, Type type,
               const char* default_value)
    : name(name),
      description(description),
      type(type),
      default_value(default_value) {
}

Command::Command(const string& name, const string& description,
                 const vector<Option>& options,
                 const char* argument_placeholder)
    : name(name),
      description(description),
      options(options),
      argument_placeholder(argument_placeholder) {
}

Command::~Command() {
}

bool Command::run(const string& executable, const vector<string>& arguments) {
  // Initialize with the default value for all options
  map<string, bool> flag_arguments;
  map<string, string> option_arguments;
  map<string, Option> option_map;
  for (auto& option : options) {
    option_map.insert(std::make_pair(option.name, option));
    switch (option.type) {
      case Option::FLAG:
        flag_arguments[option.name] = false;
        break;
      case Option::OPTION:
        option_arguments[option.name] = option.default_value;
        break;
    }
  }

  // Parse the command line flags
  vector<string> tail;
  for (size_t i = 0; i < arguments.size(); i++) {
    auto& argument(arguments[i]);
    if (argument.size() > 0 && argument[0] == '-') {
      size_t i = 1;
      while (i < argument.size() && argument[i] == '-') {
        i++;
      }
      string option_name;
      string option_value;
      string option_string(argument.substr(i));
      auto loc = option_string.find('=');
      if (loc != string::npos) {
        option_name = option_string.substr(0, loc);
        option_value = option_string.substr(loc + 1);
      } else {
        option_name = option_string;
      }
      auto option_loc = option_map.find(option_name);
      if (option_loc == option_map.end()) {
        std::cerr << "Unrecognized option: "
                  << color(Color::ERROR, argument, isatty(STDERR_FILENO))
                  << std::endl;
        print_help(executable);
        return false;
      }
      auto option = option_loc->second;
      if (option.type == Option::OPTION) {
        if (option_value.empty()) {
          std::cerr << "Option "
                    << color(Color::ERROR, "-" + option.name,
                             isatty(STDERR_FILENO))
                    << " requires a value" << std::endl;
          print_help(executable);
          return false;
        }
        option_arguments[option.name] = option_value;
      } else if (loc != string::npos) {
        if (option_value == "true") {
          flag_arguments[option.name] = true;
        } else if (option_value == "false") {
          flag_arguments[option.name] = false;
        } else {
          std::cerr << "Option "
                    << color(Color::ERROR, "-" + option.name,
                             isatty(STDERR_FILENO))
                    << " is a boolean flag" << std::endl;
          print_help(executable);
          return false;
        }
      } else {
        flag_arguments[option.name] = true;
      }
    } else {
      for (size_t j = i; j < arguments.size(); j++) {
        tail.push_back(arguments[j]);
      }
      break;
    }
  }

  return execute(executable, flag_arguments, option_arguments, tail);
}

void Command::print_help(const string& executable) {
  print_help(executable, std::cerr, isatty(STDERR_FILENO));
}

void Command::print_help(const string& executable, std::ostream& out,
                         bool tty) {
  out << "Usage: " << executable << " " << color(Color::COMMAND, name, tty);
  if (options.size() > 0) {
    out << " " << color(Color::OPTIONS, "options", tty);
  }
  if (!argument_placeholder.empty()) {
    out << " " << color(Color::ARGUMENTS, argument_placeholder, tty);
  }
  out << std::endl << std::endl;
  if (options.size() > 0) {
    out << "Options for " << color(Color::COMMAND, name, tty) << ": "
        << std::endl;
    size_t tab_width = 0;
    for (auto& option : options) {
      size_t width = option.name.size() + 4;
      if (option.type != Option::FLAG) {
        width += 2;
      }
      tab_width = std::max(tab_width, width);
    }
    for (auto& option : options) {
      size_t width = option.name.size();
      out << "  -" << option.name;
      if (option.type != Option::FLAG) {
        out << "=…";
        width += 2;
      }
      for (size_t i = width; i < tab_width; i++) {
        out << " ";
      }
      out << color(Color::DESCRIPTION, option.description, tty) << std::endl;
    }
    out << std::endl;
  }
  print_secondary_help(out, tty);
  out << "Use `" << executable << " "
      << color(Color::COMMAND, "help " + name, tty)
      << "` to show this command documentation." << std::endl;
}

void Command::print_secondary_help(std::ostream& out, bool tty) {
}

}

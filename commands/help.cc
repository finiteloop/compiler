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

#include "help.h"

#include <unistd.h>

#include "color.h"

namespace compiler::commands {

Help::Help(const filesystem::path& executable,
           const vector<shared_ptr<Command>>& commands)
    : Command("help", "Print information about the available commands", {},
              "command"),
      executable(executable),
      commands(commands) {
}

bool Help::execute(const filesystem::path& executable, map<string, bool>& flags,
                   map<string, string>& options, vector<string>& arguments) {
  if (arguments.size() > 1) {
    print_help(executable);
    return false;
  } else if (arguments.size() == 1) {
    for (auto& command : commands) {
      if (command->name == arguments[0]) {
        command->print_help(executable, std::cout, isatty(STDOUT_FILENO));
        return true;
      }
    }
    Color color(isatty(STDERR_FILENO));
    std::cerr << "Unrecognized command: " << color.error(arguments[0])
              << std::endl;
    std::cerr << "Try `" << executable.string() << " " << color.command("help")
              << "` to see a list of available commands." << std::endl;
    return false;
  } else {
    print(std::cout, isatty(STDOUT_FILENO));
    return true;
  }
}

void Help::print(std::ostream& out, bool tty) {
  Color color(tty);
  out << "Usage: " << executable.string() << " " << color.command("command")
      << " " << color.option("options") << std::endl;
  out << std::endl;
  out << "Commands: " << std::endl;
  size_t tab_width = 0;
  for (auto& command : commands) {
    tab_width = std::max(tab_width, command->name.size() + 4);
  }
  for (auto& command : commands) {
    out << "  " << color.command(command->name);
    for (size_t i = command->name.size(); i < tab_width; i++) {
      out << " ";
    }
    out << command->description << std::endl;
  }
  out << std::endl;
  out << "Try `" << executable.string() << " help " << color.command("command")
      << "` for help with a command." << std::endl;
}

}

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

#include <iostream>
#include <unistd.h>

#include "core/common.h"
#include "commands/build.h"
#include "commands/check.h"
#include "commands/color.h"
#include "commands/help.h"
#include "commands/ir.h"
#include "commands/parse.h"
#include "commands/run.h"

using namespace compiler::commands;

int main(int argc, const char* argv[]) {
  std::filesystem::path executable(argv[0]);
  std::vector<std::shared_ptr<Command>> commands({
      std::make_shared<Run>(),
      std::make_shared<Build>(),
      std::make_shared<Check>(),
      std::make_shared<Parse>(),
      std::make_shared<IR>(),
  });
  auto help = std::make_shared<Help>(executable, commands);
  commands.push_back(help);

  if (argc < 2) {
    help->print(std::cerr, isatty(STDERR_FILENO));
    return EXIT_FAILURE;
  }

  std::string name = argv[1];
  std::vector<std::string> arguments;
  for (int i = 2; i < argc; i++) {
    arguments.push_back(argv[i]);
  }
  for (auto& command : commands) {
    if (command->name == name) {
      if (command->run(executable, arguments)) {
        return EXIT_SUCCESS;
      } else {
        return EXIT_FAILURE;
      }
    }
  }

  Color color(isatty(STDERR_FILENO));
  std::cerr << "Unrecognized command: " << color.error(name) << std::endl;
  std::cerr << "Try `" << executable.stem().string() << " "
            << color.command("help") << "` for a list of available commands."
            << std::endl;
  return EXIT_FAILURE;
}

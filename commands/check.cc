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

#include "check.h"

#include <stdlib.h>

#include "../checker/check.h"
#include "../parser/parse.h"

namespace compiler::commands {

Check::Check()
    : Command("check", "Check the correctness of a module",
              {Option("strict", "Treat warnings as fatal errors")}, "path…") {
}

bool Check::execute(const filesystem::path& executable,
                    map<string, bool>& flags, map<string, string>& options,
                    vector<string>& arguments) {
  if (arguments.size() < 1) {
    print_help(executable);
    return false;
  }
  auto fail_level = flags["strict"] ? Error::WARNING : Error::ERROR;
  bool success = true;
  for (auto& path : arguments) {
    auto error = make_shared<Error::Terminal>();
    auto module = parser::parse(error, path);
    if (!module) {
      success = false;
      continue;
    }
    auto symbols = checker::check(error, module);
    if (!symbols || error->count(fail_level) > 0) {
      success = false;
    }
  }
  return success;
}

}

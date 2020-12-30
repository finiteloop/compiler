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

#include "parse.h"

#include <fstream>
#include <utf8.h>

#include "grammar.h"
#include "scanner.h"

extern int yyparse(void*);

namespace compiler::parser {

shared_ptr<Module> parse(shared_ptr<Error> error,
                         const filesystem::path& path) {
  std::ifstream input(path);
  if (!input) {
    error->report(Error::ERROR, "Could not open " + path.string());
    return nullptr;
  }

  State state{
      .position{.path = make_shared<filesystem::path>(path)},
      .input = input,
      .error = error,
  };
  yyscan_t scanner;
  yylex_init_extra(&state, &scanner);
  Grammar grammar(scanner);
  int result = -1;
  try {
    result = grammar.parse();
  } catch (const utf8::exception&) {
    error->report(Error::ERROR,
                  path.string() + " contains invalid UTF-8 characters");
  }
  yylex_destroy(scanner);
  return result == 0 ? state.module : nullptr;
}

}

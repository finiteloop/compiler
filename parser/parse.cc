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

#include "grammar.h"
#include "scanner.h"
#include "state.h"

extern int yyparse(void*);

namespace compiler::parser {

shared_ptr<Module> parse(shared_ptr<Error> error, const string& path) {
  auto file = File::read(path);
  if (!file) {
    error->report(nullptr) << "Cannot open " << path;
    return nullptr;
  }
  auto module = make_shared<Module>(file);
  State state(error, module);
  yylex_init(&state.state);
  yyset_extra(&state, state.state);
  int result = yyparse(&state);
  yylex_destroy(state.state);
  if (result == 0) {
    return module;
  }
  return nullptr;
}

}

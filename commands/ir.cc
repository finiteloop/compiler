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

#include "ir.h"

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/FileSystem.h>
#include <stdlib.h>
#include <unistd.h>

#include "../checker/check.h"
#include "../emitter/emit.h"
#include "../emitter/optimize.h"
#include "../parser/parse.h"

namespace compiler::commands {

IR::IR()
    : Command(
          "ir", "Emit LLVM assembly language for a program",
          {Option("output", "Write IR code to the given path", Option::OPTION),
           Option("strict", "Treat warnings as fatal errors"),
           Option("unoptimized", "Do not optimize the program")},
          "path") {
}

bool IR::execute(const filesystem::path& executable, map<string, bool>& flags,
                 map<string, string>& options, vector<string>& arguments) {
  if (arguments.size() < 1) {
    print_help(executable);
    return false;
  }

  // Parse the program
  auto error = make_shared<Error::Terminal>();
  auto fail_level = flags["strict"] ? Error::WARNING : Error::ERROR;
  auto module = parser::parse(error, arguments[0]);
  if (!module) {
    return false;
  }

  // Check for correctness
  auto symbols = checker::check(error, module);
  if (!symbols || error->count(fail_level) > 0) {
    return false;
  }

  // Emit LLVM IR code
  llvm::LLVMContext llvm_context;
  auto llvm_module = new llvm::Module(arguments[0], llvm_context);
  auto llvm_function = emitter::emit(module, llvm_module);
  if (!llvm_function) {
    return false;
  }
  if (!flags["unoptimized"]) {
    emitter::optimize(llvm_module);
  }

  // Write the LLVM IR
  shared_ptr<llvm::raw_fd_ostream> out;
  if (!options["output"].empty()) {
    std::error_code file_error;
    out = make_shared<llvm::raw_fd_ostream>(options["output"], file_error,
                                            llvm::sys::fs::F_None);
    if (file_error) {
      error->report(Error::ERROR, "Could not write " + options["output"] +
                                      ": " + file_error.message());
      return false;
    }
  } else {
    out = make_shared<llvm::raw_fd_ostream>(STDOUT_FILENO, false);
  }
  llvm_module->print(*out, nullptr);
  return true;
}

}

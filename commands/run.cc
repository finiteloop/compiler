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

#include "run.h"

#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include <llvm/ExecutionEngine/Interpreter.h>
#include <llvm/ExecutionEngine/MCJIT.h>
#include <llvm/ExecutionEngine/SectionMemoryManager.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/Support/TargetSelect.h>

#include "../checker/check.h"
#include "../core/error.h"
#include "../emitter/emit.h"
#include "../emitter/optimize.h"
#include "../parser/parse.h"

namespace compiler::commands {

Run::Run()
    : Command("run", "Runs a program",
              {
                  Option("strict", "Treat compiler warnings as fatal errors"),
                  Option("unoptimized",
                         "Do not optimize the program before executing"),
              },
              "path…") {
}

bool Run::execute(const string& executable, map<string, bool>& flags,
                  map<string, string>& options, vector<string>& arguments) {
  if (arguments.size() < 1) {
    print_help(executable);
    return false;
  }

  // Initialize LLVM
  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmPrinter();

  // Parse the program
  auto error = make_shared<TerminalError>();
  Error::Level fail_level =
      flags["strict"] ? Error::Level::WARNING : Error::Level::ERROR;
  vector<shared_ptr<parser::Module>> modules;
  auto module = parser::parse(error, arguments[0]);
  if (!module || error->count(Error::Level::ERROR) > 0) {
    return false;
  }

  // Check for correctness
  if (!checker::check(error, module) || error->count(fail_level) > 0) {
    return false;
  }

  // Emit LLVM IR code
  llvm::LLVMContext llvm_context;
  auto llvm_module = new llvm::Module(arguments[0], llvm_context);
  auto llvm_function = emitter::emit(module, llvm_module);
  if (!llvm_function) {
    return false;
  }

  // Set up the LLVM JIT engine
  llvm::EngineBuilder factory((std::unique_ptr<llvm::Module>(llvm_module)));
  if (!flags["unoptimized"]) {
    llvm::TargetOptions target_options;
    std::unique_ptr<llvm::RTDyldMemoryManager> memory_manager(
        new llvm::SectionMemoryManager());
    factory.setEngineKind(llvm::EngineKind::JIT)
        .setTargetOptions(target_options)
        .setMCJITMemoryManager(std::move(memory_manager));
  }
  std::string llvm_error;
  auto engine = factory.setErrorStr(&llvm_error).create();
  if (!engine) {
    error->report(nullptr, Error::Level::ERROR, llvm_error);
    return false;
  }
  llvm_module->setDataLayout(engine->getDataLayout());

  // Run IR code optimization passes
  if (!flags["unoptimized"]) {
    emitter::optimize(llvm_module);
  }

  // Run the program
  engine->finalizeObject();
  engine->runFunction(llvm_function, {});
  return true;
}

}

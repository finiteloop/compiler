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

#include "build.h"

#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/TargetRegistry.h>
#include <llvm/Support/TargetSelect.h>
#include <stdlib.h>
#include <unistd.h>

#include "../checker/check.h"
#include "../emitter/emit.h"
#include "../emitter/optimize.h"
#include "../parser/parse.h"

namespace compiler::commands {

Build::Build()
    : Command("build", "Build an executable binary for a program",
              {Option("strict", "Treat warnings as fatal errors"),
               Option("unoptimized", "Do not optimize the program"),
               Option("output", "Output binary name", Option::OPTION),
               Option("target", "Target architecture", Option::OPTION),
               Option("linker", "Linker command", Option::OPTION, "cc"),
               Option("object", "Generate an unlinked object file")},
              "path") {
}

bool Build::execute(const filesystem::path& executable,
                    map<string, bool>& flags, map<string, string>& options,
                    vector<string>& arguments) {
  if (arguments.size() < 1) {
    print_help(executable);
    return false;
  }

  // Initialize LLVM
  llvm::InitializeAllTargetInfos();
  llvm::InitializeAllTargets();
  llvm::InitializeAllTargetMCs();
  llvm::InitializeAllAsmPrinters();

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
  string target = options["target"];
  if (target.empty()) {
    target = llvm::sys::getDefaultTargetTriple();
  }
  string llvm_error;
  auto llvm_target =
      llvm::TargetRegistry::lookupTarget(target.c_str(), llvm_error);
  if (!llvm_target) {
    error->report(Error::ERROR, llvm_error);
    return false;
  }
  auto llvm_machine = llvm_target->createTargetMachine(
      target.c_str(), "generic", "", llvm::TargetOptions(),
      llvm::Reloc::Model::PIC_);
  auto llvm_module = new llvm::Module(arguments[0], llvm_context);
  llvm_module->setDataLayout(llvm_machine->createDataLayout());
  auto llvm_function = emitter::emit(module, llvm_module);
  if (!llvm_function) {
    return false;
  }
  if (!flags["unoptimized"]) {
    emitter::optimize(llvm_module);
  }

  // Write the object file
  char object_path[PATH_MAX];
  auto pattern = filesystem::temp_directory_path() / "XXXXXXXXXX.o";
  strcpy(object_path, pattern.c_str());
  auto object_fd = mkstemps(object_path, 2);
  if (object_fd == -1) {
    error->report(Error::ERROR,
                  "Could not create temporary file: " + string(object_path));
    return false;
  }
  close(object_fd);
  std::error_code file_error;
  llvm::raw_fd_ostream out(object_path, file_error, llvm::sys::fs::F_None);
  if (file_error) {
    error->report(Error::ERROR,
                  "Could not write file: " + file_error.message());
    return false;
  }
  llvm::legacy::PassManager pass;
  if (llvm_machine->addPassesToEmitFile(pass, out, nullptr,
                                        llvm::CGFT_ObjectFile)) {
    error->report(
        Error::ERROR,
        "LLVM cannot emit object files for the target architecture: " + target);
    return false;
  }
  pass.run(*llvm_module);
  out.flush();

  // Determine our output file name
  string output_name = options["output"];
  if (output_name.empty()) {
    output_name = filesystem::path(arguments[0]).stem();
    if (flags["object"]) {
      output_name += ".o";
    }
  }

  // Finish with the object file if requested
  if (flags["object"]) {
    if (rename(object_path, output_name.c_str()) == -1) {
      error->report(Error::ERROR,
                    "Could not move " + string(object_path) + " to " + name);
      return false;
    }
    unlink(object_path);
    return true;
  }

  // Link the object file using the cc command to include the C standard library
  string command = options["linker"] + " " + object_path + " -o " + output_name;
  if (system(command.c_str()) == -1) {
    error->report(Error::ERROR, "Could not execute linker: " + command);
    return false;
  }
  unlink(object_path);
  return true;
}

}

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
#include <llvm/Support/raw_os_ostream.h>
#include <stdlib.h>
#include <unistd.h>

#include "../checker/check.h"
#include "../core/error.h"
#include "../emitter/emit.h"
#include "../emitter/optimize.h"
#include "../parser/parse.h"

namespace compiler::commands {

Build::Build()
    : Command("build", "Build a binary for a program",
              {
                  Option("strict", "Treat compiler warnings as fatal errors"),
                  Option("unoptimized", "Do not optimize the program"),
                  Option("output", "Output binary name", Option::OPTION),
                  Option("target", "Target architecture (see options below)",
                         Option::OPTION),
                  Option("llvm", "Generate LLVM IR assembly code"),
                  Option("object", "Generate an unlinked object file"),
              },
              "path…") {
}

bool Build::execute(const string& executable, map<string, bool>& flags,
                    map<string, string>& options, vector<string>& arguments) {
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

  // Build ASM for target
  llvm::EngineBuilder factory((std::unique_ptr<llvm::Module>(llvm_module)));
  std::string llvm_error;
  auto engine = factory.setErrorStr(&llvm_error).create();
  if (!engine) {
    error->report(nullptr, Error::Level::ERROR, llvm_error);
    return false;
  }
  string target = options["target"];
  if (target.empty()) {
    target = llvm::sys::getDefaultTargetTriple();
  }
  llvm_module->setTargetTriple(target.c_str());
  auto llvm_target =
      llvm::TargetRegistry::lookupTarget(target.c_str(), llvm_error);
  if (!llvm_target) {
    std::cerr << "Invalid LLVM target: " << llvm_error;
    return false;
  }
  auto llvm_machine = llvm_target->createTargetMachine(
      target.c_str(), "generic", "", llvm::TargetOptions(),
      llvm::Optional<llvm::Reloc::Model>());
  llvm_module->setDataLayout(llvm_machine->createDataLayout());
  if (!flags["unoptimized"]) {
    std::cerr << "XXXX" << std::endl;
    emitter::optimize(llvm_module);
  }

  // Determine our output file name
  string name = options["output"];
  if (name.empty()) {
    name = file_name(arguments[0]);
    if (name.size() > 6 && name.substr(name.size() - 6) == ".indie") {
      name = name.substr(0, name.size() - 6);
    }
    if (flags["llvm"]) {
      name += ".ll";
    } else if (flags["object"]) {
      name += ".o";
    }
  }

  // Print the LLVM IR code if requested
  if (flags["llvm"]) {
    std::error_code file_error;
    llvm::raw_fd_ostream out(name.c_str(), file_error, llvm::sys::fs::F_None);
    if (file_error) {
      std::cerr << "Could write file: " << file_error.message() << std::endl;
      return false;
    }
    llvm_module->print(out, nullptr);
    return true;
  }

  // Write the object file
  char object_path[PATH_MAX];
  strcpy(object_path, P_tmpdir);
  strcpy(object_path + strlen(P_tmpdir), "XXXXXXXXXX.o");
  auto object_fd = mkstemps(object_path, 2);
  if (object_fd == -1) {
    std::cerr << "Could not create temporary file: " << object_path
              << std::endl;
    return false;
  }
  close(object_fd);
  std::error_code file_error;
  llvm::raw_fd_ostream out(object_path, file_error, llvm::sys::fs::F_None);
  if (file_error) {
    std::cerr << "Could write file: " << file_error.message() << std::endl;
    return false;
  }
  llvm::legacy::PassManager pass;
  if (llvm_machine->addPassesToEmitFile(pass, out, nullptr,
                                        llvm::CGFT_ObjectFile)) {
    std::cerr
        << "LLVM cannot emit native object files for the target architecture: "
        << target << std::endl;
    return false;
  }
  pass.run(*llvm_module);
  out.flush();

  // Finish with the object file if requested
  if (flags["object"]) {
    if (rename(object_path, name.c_str()) == -1) {
      std::cerr << "Could not move " << object_path << " to " << name
                << std::endl;
      return false;
    }
    unlink(object_path);
    return true;
  }

  std::cerr << "Linking is not yet implemented." << std::endl;
  std::cerr << "Run with -object to generate an object file in the meantime."
            << std::endl;
  return false;
}

void Build::print_secondary_help(std::ostream& out, bool tty) {
  llvm::InitializeAllTargetInfos();
  vector<std::pair<string, string>> targets;
  size_t tab_width = 0;
  for (auto& target : llvm::TargetRegistry::targets()) {
    string name(target.getName());
    targets.push_back(std::make_pair(name, target.getShortDescription()));
    tab_width = std::max(tab_width, name.size() + 4);
  }
  out << "Valid architectures for -target=… (defaults to "
      << llvm::sys::getDefaultTargetTriple() << "):" << std::endl;
  for (auto& item : targets) {
    out << "  " << item.first;
    for (size_t i = item.first.size(); i < tab_width; i++) {
      out << " ";
    }
    out << item.second << std::endl;
  }
  out << std::endl;
}

}

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

#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Module.h>
#include <llvm/Transforms/IPO.h>
#include <llvm/Transforms/IPO/PassManagerBuilder.h>

namespace compiler::emitter {

void optimize(llvm::Module* module) {
  llvm::PassManagerBuilder builder;
  builder.OptLevel = 3;
  builder.Inliner = llvm::createFunctionInliningPass();

  llvm::legacy::FunctionPassManager fpm(module);
  builder.populateFunctionPassManager(fpm);
  for (auto i = module->begin(); i != module->end(); ++i) {
    if (!i->isDeclaration()) {
      fpm.run(*i);
    }
  }
  fpm.doFinalization();

  llvm::legacy::PassManager mpm;
  builder.populateModulePassManager(mpm);
  mpm.run(*module);
}

}

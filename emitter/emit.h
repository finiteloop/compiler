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

#pragma once

#include <llvm/IR/Function.h>
#include <llvm/IR/Module.h>

#include "../parser/ast.h"

namespace compiler::emitter {

// Emits the LLVM IR code for the given module into the given LLVM module. We
// return the generated main function, which can be executed to run the program.
llvm::Function* emit(shared_ptr<parser::Module> ast, llvm::Module* llvm_module);

}

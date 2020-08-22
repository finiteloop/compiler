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

#include "emit.h"

#include <llvm/IR/Verifier.h>

#include "expression.h"

namespace compiler::emitter {

llvm::Function* emit(shared_ptr<parser::Module> ast,
                     llvm::Module* llvm_module) {
  llvm::IRBuilder<> builder(llvm_module->getContext());

  auto printf = llvm::Function::Create(
      llvm::FunctionType::get(builder.getInt32Ty(),
                              {llvm::PointerType::get(builder.getInt8Ty(), 0)},
                              true),
      llvm::Function::ExternalLinkage, "printf", llvm_module);

  auto main = llvm::Function::Create(
      llvm::FunctionType::get(builder.getInt32Ty(), {}, false),
      llvm::Function::ExternalLinkage, "main", llvm_module);
  auto block = llvm::BasicBlock::Create(builder.getContext(), "", main);
  builder.SetInsertPoint(block);

  // Print the result of every expression to stdout with printf
  auto printf_format = builder.CreateGlobalStringPtr("%d\n");
  for (auto expression : ast->expressions) {
    auto value = emit_expression(builder, expression);
    builder.CreateCall(printf, {printf_format, value});
  }

  builder.CreateRet(builder.getInt32(0));
  builder.ClearInsertionPoint();
  llvm::verifyFunction(*main);
  return main;
}

}

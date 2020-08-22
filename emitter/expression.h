
// Copyright 2020 Bret Taylor

#pragma once

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Value.h>

#include "../parser/ast.h"

namespace compiler::emitter {

// Emits the given expression to the given LLVM builder, returning the LLVM
// value that stores the result of the expression.
llvm::Value* emit_expression(llvm::IRBuilder<>& builder,
                             shared_ptr<parser::Expression> expression);

}

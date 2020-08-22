
// Copyright 2020 Bret Taylor

#include "expression.h"

#include <assert.h>

namespace compiler::emitter {

namespace {

class Emitter : private parser::Expression::Handler {
 public:
  Emitter(llvm::IRBuilder<>& builder) : builder_(builder), result_(nullptr) {
  }

  llvm::Value* emit(shared_ptr<parser::Expression> expression) {
    expression->handle(this);
    assert(result_);
    return result_;
  }

 private:
  virtual void handle_binary(const parser::Binary& ast) override {
    auto lhs = emit_expression(builder_, ast.lhs);
    auto rhs = emit_expression(builder_, ast.rhs);
    switch (ast.op) {
      case parser::Binary::ADD:
        result_ = builder_.CreateAdd(lhs, rhs);
        break;
      case parser::Binary::SUBTRACT:
        result_ = builder_.CreateSub(lhs, rhs);
        break;
      case parser::Binary::DIVIDE:
        result_ = builder_.CreateSDiv(lhs, rhs);
        break;
      case parser::Binary::MULTIPLY:
        result_ = builder_.CreateMul(lhs, rhs);
        break;
      case parser::Binary::MOD:
        result_ = builder_.CreateSRem(lhs, rhs);
        break;
      case parser::Binary::SHIFT_LEFT:
        result_ = builder_.CreateShl(lhs, rhs);
        break;
      case parser::Binary::SHIFT_RIGHT:
        result_ = builder_.CreateLShr(lhs, rhs);
        break;
      case parser::Binary::BIT_AND:
        result_ = builder_.CreateAnd(lhs, rhs);
        break;
      case parser::Binary::BIT_OR:
        result_ = builder_.CreateOr(lhs, rhs);
        break;
      case parser::Binary::BIT_XOR:
        result_ = builder_.CreateXor(lhs, rhs);
        break;
    }
  }

  virtual void handle_integer_literal(
      const parser::IntegerLiteral& ast) override {
    result_ = builder_.getInt64(ast.value);
  }

  llvm::IRBuilder<>& builder_;
  llvm::Value* result_;
};

}

// Emits the given expression to the given LLVM builder, returning the LLVM
// value that stores the result of the expression.
llvm::Value* emit_expression(llvm::IRBuilder<>& builder,
                             shared_ptr<parser::Expression> expression) {
  return Emitter(builder).emit(expression);
}

}

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

#include "../core/common.h"
#include "../core/location.h"

namespace compiler::parser {

// A node in the program's abstract syntax tree.
class AST {
 public:
  virtual ~AST() {
  }

  // The file location of this node in the program.
  Location location;

 protected:
  AST(const Location& location) : location(location) {
  }
};

// A node that expresses a value.
class Expression : public AST {
 public:
  class Handler;

  // Dynamically dispatch based on the runtime type of this expression.
  virtual void handle(Handler& handler) = 0;

 protected:
  Expression(const Location& location) : AST(location) {
  }
};

// A binary operation on two expressions.
class Binary : public Expression {
 public:
  enum Operator {
    ADD,
    SUBTRACT,
    DIVIDE,
    MULTIPLY,
    MOD,
    SHIFT_LEFT,
    SHIFT_RIGHT,
    BIT_AND,
    BIT_OR,
    BIT_XOR,
  };

  Binary(const Location& location, shared_ptr<Expression> lhs, Operator op,
         shared_ptr<Expression> rhs)
      : Expression(location), lhs(lhs), op(op), rhs(rhs) {
  }

  void handle(Handler& handler) override;

  shared_ptr<Expression> lhs;
  Operator op;
  shared_ptr<Expression> rhs;
};

// A 64-bit integer constant.
class IntegerLiteral : public Expression {
 public:
  IntegerLiteral(const Location& location, int64_t value)
      : Expression(location), value(value) {
  }

  void handle(Handler& handler) override;

  int64_t value;
};

class Module {
 public:
  Module(const filesystem::path& path) : path(path) {
  }

  filesystem::path path;
  vector<shared_ptr<Expression>> expressions;
};

class Expression::Handler {
 public:
  virtual ~Handler() {
  }

  virtual void handle_binary(Binary&) = 0;
  virtual void handle_integer_literal(IntegerLiteral&) = 0;
};

}

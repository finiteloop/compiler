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
#include "../core/file.h"

namespace compiler::parser {

// A node in the program's abstract syntax tree.
class AST {
 public:
  virtual ~AST();

  // The file location of this node in the program.
  shared_ptr<Location> location;

 protected:
  AST(shared_ptr<Location> location);
};

class Expression : public AST {
 public:
  class Handler;

  // Dynamically dispatch based on the runtime type of this expression.
  virtual void handle(Handler* handler) = 0;

 protected:
  Expression(shared_ptr<Location> location);
};

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

  Binary(shared_ptr<Location> location, shared_ptr<Expression> lhs, Operator op,
         shared_ptr<Expression> rhs);

  virtual void handle(Handler* handler) override;

  shared_ptr<Expression> lhs;
  Operator op;
  shared_ptr<Expression> rhs;
};

class IntegerLiteral : public Expression {
 public:
  IntegerLiteral(shared_ptr<Location> location, int64_t value);

  virtual void handle(Handler* handler) override;

  int64_t value;
};

class Module : public AST {
 public:
  Module(shared_ptr<File> file);

  shared_ptr<File> file;
  vector<shared_ptr<Expression>> expressions;
};

class Expression::Handler {
 public:
  virtual ~Handler();
  virtual void handle_binary(const Binary&) = 0;
  virtual void handle_integer_literal(const IntegerLiteral&) = 0;
};

}

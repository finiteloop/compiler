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

#include "ast.h"

namespace compiler::parser {

AST::AST(shared_ptr<Location> location) : location(location) {
}

AST::~AST() {
}

Expression::Expression(shared_ptr<Location> location) : AST(location) {
}

Binary::Binary(shared_ptr<Location> location, shared_ptr<Expression> lhs,
               Operator op, shared_ptr<Expression> rhs)
    : Expression(location), lhs(lhs), op(op), rhs(rhs) {
}

void Binary::handle(Handler* handler) {
  handler->handle_binary(*this);
}

IntegerLiteral::IntegerLiteral(shared_ptr<Location> location, int64_t value)
    : Expression(location), value(value) {
}

void IntegerLiteral::handle(Expression::Handler* handler) {
  handler->handle_integer_literal(*this);
}

Module::Module(shared_ptr<File> file)
    : AST(make_shared<Location>(file, 1, 1, 1, 1)), file(file) {
}

Expression::Handler::~Handler() {
}

}

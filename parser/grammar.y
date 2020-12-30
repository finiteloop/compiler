/*
 * Copyright 2020 Bret Taylor
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

%{

#include "grammar.h"
#include "scanner.h"

%}

%require "3.3"
%language "c++"
%define api.value.type variant
%define api.namespace { compiler::parser }
%define api.location.type { Location }
%define api.parser.class { Grammar }
%lex-param { void* yyscanner }
%parse-param { void* yyscanner }
%locations

%code requires {
  #include <iostream>
  #include "../core/error.h"
  #include "ast.h"
}

%code provides {
  namespace compiler::parser {
    struct State {
      std::istream& input;
      Position position;
      shared_ptr<Error> error;
      shared_ptr<Module> module;
    };
  }

  typedef compiler::parser::Grammar::semantic_type YYSTYPE;
  typedef compiler::parser::Grammar::location_type YYLTYPE;
  #define YY_EXTRA_TYPE compiler::parser::State*
}

%token<shared_ptr<IntegerLiteral>> IntegerLiteral;

%token OperatorShiftLeft
%token OperatorShiftRight

%nterm<shared_ptr<Binary>> Binary
%nterm<shared_ptr<Expression>> Expression
%nterm<shared_ptr<Module>> Module

%left '+' '-'
%left '*' '/' '%'
%left '&' '|' '^' OperatorShiftLeft OperatorShiftRight

%%

Module: Module Expression '\n' {
  $$ = $1;
  $$->expressions.push_back($2);
} | Module '\n' {
  $$ = $1;
} | {
  auto state = yyget_extra(yyscanner);
  auto module = make_shared<Module>(*state->position.path);
  state->module = module;
  $$ = module;
}

Expression: Binary {
  $$ = $1;
} | IntegerLiteral {
  $$ = $1;
} | '(' Expression ')' {
  $$ = $2;
  $$->location = @$;
}

Binary: Expression '+' Expression {
  $$ = make_shared<Binary>(@$, $1, Binary::ADD, $3);
} | Expression '-' Expression {
  $$ = make_shared<Binary>(@$, $1, Binary::SUBTRACT, $3);
} | Expression '*' Expression {
  $$ = make_shared<Binary>(@$, $1, Binary::MULTIPLY, $3);
} | Expression '/' Expression {
  $$ = make_shared<Binary>(@$, $1, Binary::DIVIDE, $3);
} | Expression '%' Expression {
  $$ = make_shared<Binary>(@$, $1, Binary::MOD, $3);
} | Expression OperatorShiftLeft Expression {
  $$ = make_shared<Binary>(@$, $1, Binary::SHIFT_LEFT, $3);
} | Expression OperatorShiftRight Expression {
  $$ = make_shared<Binary>(@$, $1, Binary::SHIFT_RIGHT, $3);
} | Expression '&' Expression {
  $$ = make_shared<Binary>(@$, $1, Binary::BIT_AND, $3);
} | Expression '|' Expression {
  $$ = make_shared<Binary>(@$, $1, Binary::BIT_OR, $3);
} | Expression '^' Expression {
  $$ = make_shared<Binary>(@$, $1, Binary::BIT_XOR, $3);
}

%%

void compiler::parser::Grammar::error(
    const compiler::Location& location,
    const std::string& message) {
  yyget_extra(yyscanner)->error->report(compiler::Error::ERROR, location, message);
}

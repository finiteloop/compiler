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

#include "../core/common.h"
#include "../core/file.h"
#include "ast.h"
#include "grammar.h"
#include "state.h"

using namespace compiler;
using namespace compiler::parser;

int yylex(YYSTYPE*, YYLTYPE*, void*);

#define YYPARSE_PARAM state
#define YYLEX_PARAM ((State*) state)->state

#define loc(yloc) (make_shared<Location>( \
  ((State*)state)->module->file, yloc.first_line, yloc.first_column, \
  yloc.last_line, yloc.last_column))

#define yyerror(message) \
  ((State*)state)->error->report(loc(yylloc), Error::Level::ERROR, message);

%}

%pure_parser
%locations

%union {
  int64_t integer;
  compiler::parser::Expression* expression;
  compiler::parser::Module* module;
}

%token<integer> T_IntegerLiteral;

%token T_ShiftLeft
%token T_ShiftRight

%type<expression> Binary
%type<expression> Expression
%type<module> Module

%left '+' '-'
%left '*' '/' '%'
%left '&' '|' '^' T_ShiftLeft T_ShiftRight

%%

Module: Module Expression '\n' {
  $$ = $1;
  $$->expressions.push_back(shared_ptr<Expression>($2));
  $$->location = $$->location->merge(loc(@3));
} | Module '\n' {
  $$ = $1;
  $$->location = $$->location->merge(loc(@2));
} | {
  $$ = ((State*)state)->module.get();
}

Expression: Binary {
  $$ = $1;
} | T_IntegerLiteral {
  $$ = new IntegerLiteral(loc(@1), $1);
} | '(' Expression ')' {
  $$ = $2;
  $$->location = loc(@1)->merge(loc(@3));
}

Binary: Expression '+' Expression {
  $$ = new Binary($1->location->merge($3->location), shared_ptr<Expression>($1), Binary::ADD, shared_ptr<Expression>($3));
} | Expression '-' Expression {
  $$ = new Binary($1->location->merge($3->location), shared_ptr<Expression>($1), Binary::SUBTRACT, shared_ptr<Expression>($3));
} | Expression '*' Expression {
  $$ = new Binary($1->location->merge($3->location), shared_ptr<Expression>($1), Binary::MULTIPLY, shared_ptr<Expression>($3));
} | Expression '/' Expression {
  $$ = new Binary($1->location->merge($3->location), shared_ptr<Expression>($1), Binary::DIVIDE, shared_ptr<Expression>($3));
} | Expression '%' Expression {
  $$ = new Binary($1->location->merge($3->location), shared_ptr<Expression>($1), Binary::MOD, shared_ptr<Expression>($3));
} | Expression T_ShiftLeft Expression {
  $$ = new Binary($1->location->merge($3->location), shared_ptr<Expression>($1), Binary::SHIFT_LEFT, shared_ptr<Expression>($3));
} | Expression T_ShiftRight Expression {
  $$ = new Binary($1->location->merge($3->location), shared_ptr<Expression>($1), Binary::SHIFT_RIGHT, shared_ptr<Expression>($3));
} | Expression '&' Expression {
  $$ = new Binary($1->location->merge($3->location), shared_ptr<Expression>($1), Binary::BIT_AND, shared_ptr<Expression>($3));
} | Expression '|' Expression {
  $$ = new Binary($1->location->merge($3->location), shared_ptr<Expression>($1), Binary::BIT_OR, shared_ptr<Expression>($3));
} | Expression '^' Expression {
  $$ = new Binary($1->location->merge($3->location), shared_ptr<Expression>($1), Binary::BIT_XOR, shared_ptr<Expression>($3));
}

%%

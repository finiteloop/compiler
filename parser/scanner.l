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

#include <algorithm>

#include "ast.h"
#include "grammar.h"
#include "state.h"

using namespace compiler;
using namespace compiler::parser;

#define YY_USER_ACTION { \
  State* state = (State*)yyget_extra(yyscanner); \
  yylloc->first_line = state->line; \
  yylloc->last_line = state->line; \
  yylloc->first_column = state->column; \
  yylloc->last_column = state->column + yyleng - 1; \
  state->column += yyleng; \
}

#define YY_INPUT(buffer, result, buffer_size) \
  State* state = ((State*)yyget_extra(yyscanner)); \
  result = std::min(buffer_size, state->data.size()); \
  memcpy(buffer, state->data.data(), result); \
  state->data = state->data.substr(result);

%}

%option reentrant
%option bison-locations
%option bison-bridge
%option stack
%option yylineno
%option noyywrap
%option noyy_top_state
%option noyy_pop_state
%option noyy_push_state
%option noinput
%option nounput

%s N

Integer ([\-]?[0-9]+)

%%

 BEGIN(N);

 /* Whitespace */

<*>[ \t]+ {
  /* Ignore whitespace */
}

 /* Comments */

<N>"#"[^\n]* {
  /* Ignore comments */
}

 /* Operators */

<N>"<<" {
  return T_ShiftLeft;
}
<N>">>" {
  return T_ShiftRight;
}
<N>[+\-*/%&\|\^] {
  return yytext[0];
}

 /* New lines */

<N>\n {
  State* state = (State*)yyget_extra(yyscanner);
  state->line++;
  state->column = 1;
  return '\n';
}

 /* Literals */

<N>{Integer} {
  yylval->integer = strtol(yytext, nullptr, 10);
  return T_IntegerLiteral;
}

 /* Scanner error */

<N>. {
  return yytext[0];
}

%%

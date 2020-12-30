%{

#include <inttypes.h>
#include <utf8.h>

#include "grammar.h"

using namespace compiler::parser;

#define YY_INPUT(buffer, result, size) {\
  auto state = yyget_extra(yyscanner); \
  state->input.read(buffer, size); \
  result = state->input.gcount(); \
}

#define YY_USER_ACTION {\
  auto state = yyget_extra(yyscanner); \
  auto length = utf8::distance(yytext, yytext + yyleng); \
  yylloc->begin = state->position; \
  state->position.column += length; \
  yylloc->end.line = state->position.line; \
  yylloc->end.column = state->position.column - 1; \
}

%}

%option reentrant
%option bison-locations
%option bison-bridge
%option noyywrap
%option noyy_top_state
%option noyy_pop_state
%option noyy_push_state
%option noinput
%option nounput

%%

 /* Comments */
#[^\n]* {}

 /* Inline whitespace */
[ \t\v\f\r]+ {}

 /* New line */
\n {
  auto state = yyget_extra(yyscanner);
  state->position.line++;
  state->position.column = 1;
  return yytext[0];
}

 /* Multi-character symbols */
"<<" { return Grammar::token::OperatorShiftLeft; }
">>" { return Grammar::token::OperatorShiftRight; }

 /* Integer literal */
[-+]?[0-9]+ {
  yylval->emplace<std::shared_ptr<IntegerLiteral>>(
      std::make_shared<IntegerLiteral>(*yylloc, strtol(yytext, nullptr, 10)));
  return Grammar::token::IntegerLiteral;
}

 /* All other symbols */
. {
  return yytext[0];
}

%%

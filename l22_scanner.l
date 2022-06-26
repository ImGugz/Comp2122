%option c++ prefix="l22_scanner_" outfile="l22_scanner.cpp"
%option stack noyywrap yylineno 8bit debug
%{ 
// make relevant includes before including the parser's tab file
#include <string>
#include <vector>
#include <sstream>
#include <cdk/ast/sequence_node.h>
#include <cdk/ast/expression_node.h>
#include <cdk/ast/lvalue_node.h>
#include "l22_parser.tab.h"

// don't change this
#define yyerror LexerError

static bool clear_blocks = false;         // flag for closing inner blocks when the outermost block is closed

static int lLevelsToPop = 0;              // when an inner block is closed, see how many inner blocks to said block must be closed
static std::vector<int> indents;          // known indentation levels

static std::ostringstream strlit;

inline bool isValidIndent(int indent) {
    lLevelsToPop = -1;
    bool indentOk = false;
    for (size_t ix = 0; ix < indents.size(); ix++) {
        if (indent == indents[ix]) {
            lLevelsToPop = indents.size() - ix - 1;
            indentOk = true;
        }
    }
    return indentOk;
}

%}

LETTER       [A-Za-z]
IDENTIFIER   {LETTER}({LETTER}|[[:digit:]|"_"])*

%x X_STRING X_COMMENT X_NEWLINE X_BACKSLASH

%%
   {    
        //yydebug=1; set_debug(1);
        if (clear_blocks) {
          // std:cerr << "POPPING BLOCKS (" << indents.size() << ")" << std::endl;
          indents.pop_back();
          if (indents.size() == 0) { clear_blocks = false; }
          return '}';
        }
        else {
          if (lLevelsToPop > 0) {
            // std:cerr << "POPPING BLOCKS (" << lLevelsToPop << ")" << std::endl;
            lLevelsToPop--;
            indents.pop_back();
            return '}';
          }
        }
  }

  /* ====================================================================== */
  /* ====[                      3.2.2 - INDENTATION                   ]==== */
  /* ====================================================================== */

\n                       yy_push_state(X_NEWLINE); 

<X_NEWLINE>^[ ]+ {
    yy_pop_state();
    if (isValidIndent(yyleng)) {                                    // if a block was closed
          // std:cerr << "CASE 1" << std::endl;
          if (lLevelsToPop == 0) {
            // instructions at the same block or empty block
            // std::cerr << "CASE 1a (levels to pop = 0)" << std::endl;
            return ';';
          }
          else {
            // an indentation block has been closed, send closing block token to parser
            // std:cerr << "CASE 1b (levels to pop = " << lLevelsToPop << ")" << std::endl;
            if (lLevelsToPop > 0) {
              // std::cerr << "POPPING LEVELS (" << lLevelsToPop << ")" << std::endl;
              lLevelsToPop--;
              indents.pop_back();
              return '}';
            }
          }
    }
    else if (indents.size() == 0 || yyleng > indents.back()) {       // a block was opened
          // std:cerr << "CASE 2" << std::endl;
          indents.push_back(yyleng);
          return '{';
    } else {                                                         // error: bad indentation
          // something wrong: bad indent: communicate to user
          // std:cerr << "CASE 4" << std::endl;
          // std:cerr << "bad indent at line " << yylineno;
    }

}

<X_NEWLINE>\n      ;    // ignore successive newline chars
<X_NEWLINE>.       {   
  // std:cerr << "CASE 4 (levels to pop = " << lLevelsToPop << ")" << std::endl;
  yy_pop_state();
  yyless(0);
  // outermost block has been closed 
  if (indents.size() > 0) {
    // std:cerr << " more indentations " << std::endl;
    // std:cerr << "should close " << indents.size() << " blocks at " << yylineno << std::endl;
    indents.pop_back();
    if (indents.size() > 0) { clear_blocks = true; }
    return '}';
  }
  else {
    // std:cerr << " no further indentations " << std::endl;
    return ';'; // top-level declarations
  }
}

  /* ====================================================================== */
  /* ====[                      3.3 - COMMENTS                        ]==== */
  /* ====================================================================== */

[ ]*"..."\n            ;         

";".*\n                { yyless(yyleng - 1); }  /* note that a new block can start after this type of comment  */

"(*"                   yy_push_state(X_COMMENT);
<X_COMMENT>"(*"        yy_push_state(X_COMMENT);
<X_COMMENT>"*)"        yy_pop_state();
<X_COMMENT>.|\n        ; /* ignore content */

  /* ====================================================================== */
  /* ====[                  3.4.1 - CONSTANTS                         ]==== */
  /* ====[                  3.4.2 - GLOBAL SYMBOLS                    ]==== */
  /* ====[                  3.5 - DATA TYPES                          ]==== */
  /* ====================================================================== */

"null"                 return tNULLPTR;

"var"                  return tVAR;
"use"                  return tUSE;
"public"               return tPUBLIC;
"foreign"              return tFOREIGN;

"int"                  return tINT_TYPE;
"double"               return tREAL_TYPE;
"text"                 return tSTRING_TYPE;
"void"                 return tVOID_TYPE;
[\[\]]                 return *yytext;

  /* ====================================================================== */
  /* ====[              5 - FUNCTIONS                                 ]==== */
  /* ====================================================================== */

[():@,]                return *yytext;
"->"                   return tIOTYPES;
"begin"                return tBEGIN; 
"end"                  return tEND; 

  /* ====================================================================== */
  /* ====[                 6.2 - CONDITIONAL INSTRUCTION              ]==== */
  /* ====[                 6.3 - ITERATION INSTRUCTION                ]==== */
  /* ====[                 6.4 - TERMINATION INSTRUCTION              ]==== */
  /* ====[                 6.5 - CONTINUE INSTRUCTION                 ]==== */
  /* ====[                 6.6 - RETURN INSTRUCTION                   ]==== */
  /* ====[                 6.8 - PRINT INSTRUCTION                    ]==== */
  /* ====================================================================== */

"if"                   return tIF; 
"then:"                return tTHEN; 
"elif"                 return tELIF; 
"else:"                return tELSE; 

"while"                return tWHILE;
"do:"                  return tDO;

"again"                return tAGAIN;

"stop"                 return tSTOP;

"return"               return tRETURN;

"write"                return tWRITE;
"writeln"              return tWRITELN;

"sweep"                return tSWEEP;
".."                   return tDOTS;
"with"                 return tWITH;

  /* ====================================================================== */
  /* ====[                 7 - EXPRESSION OPERATORS                   ]==== */
  /* ====================================================================== */

[\[\]()?,]                 return *yytext;
[-+*/%<>?=]                return *yytext;

"=="                       return tEQ;
"!="                       return tNE;
"<="                       return tLE;
">="                       return tGE;
"not"                      return tNOT;
"and"                      return tAND;
"or"                       return tOR;
"input"                    return tINPUT;
"sizeof"                   return tSIZEOF;

  /* ====================================================================== */
  /* ====[                   3.9.1 - INTEGERS                         ]==== */
  /* ====================================================================== */

0[0-6]+              {
  try { 
    yylval.i = std::stoul(yytext, NULL, 7); return tINTEGER;
  } catch (const std::out_of_range& e) {
    yyerror("Integer is out of range");
  }
}
 
0[0-9]+              yyerror("Invalid base 7 number.");
0|[1-9][0-9]*         {
  try { 
    yylval.i = std::stoul(yytext, NULL, 10); 
    return tINTEGER;
  } catch (const std::out_of_range& e) {
    yyerror("Integer is out of range");
  }
}

  /* ====================================================================== */
  /* ====[                   2.7.2 - REAL NUMBERS                     ]==== */
  /* ====================================================================== */

[0-9]*\.[0-9]+([Ee][-+]?[0-9]+)?        { yylval.d = std::stod(yytext); return tREAL; }
[0-9]+\.[0-9]*([Ee][-+]?[0-9]+)?        { yylval.d = std::stod(yytext); return tREAL; }
[0-9]+([Ee][-+]?[0-9]+)                 { yylval.d = std::stod(yytext); return tREAL; }

  /* ====================================================================== */
  /* ====[                   3.8 - IDENTIFIERS                        ]==== */
  /* ====================================================================== */

{IDENTIFIER}  yylval.s = new std::string(yytext); return tIDENTIFIER;

  /* ====================================================================== */
  /* ====[              2.7.3 - STRINGS                               ]==== */
  /* ====================================================================== */

"\""                          yy_push_state(X_STRING);
<X_STRING>\\                  yy_push_state(X_BACKSLASH);
<X_STRING>"\""                {
                                yylval.s = new std::string(strlit.str());
                                strlit.str("");
                                yy_pop_state();
                                return tSTRING;
                              }

<X_STRING>"...\n"             ;
<X_STRING>\0                  yyerror("NULL character in string");               
<X_STRING>\n                  yyerror("Newline in string");          
<X_STRING>.                   strlit << *yytext;
                              
<X_BACKSLASH>t                 strlit << '\t'; yy_pop_state();
<X_BACKSLASH>n                 strlit << '\n'; yy_pop_state();
<X_BACKSLASH>r                 strlit << '\r'; yy_pop_state();
<X_BACKSLASH>\"                strlit << '\"'; yy_pop_state();
<X_BACKSLASH>\\                strlit << '\\'; yy_pop_state();

<X_BACKSLASH>[0-6]{1,3}        {
                                 unsigned long res = std::stoul(yytext, NULL, 7);
                                 if (res > 255) {
                                  yyerror("Invalid 8 bit number.");
                                 }
                                 strlit << (char)(unsigned char)res;
                                 yy_pop_state();
                               }
<X_BACKSLASH>.                 strlit << *yytext; yy_pop_state(); 

  /* ====================================================================== */
  /* ====[                    OTHER CHARACTERS                        ]==== */
  /* ====================================================================== */

<<EOF>>                {
// std:cerr << "AT <<EOF>> OPEN INDENTS ARE " << indents.size() << std::endl;
if (indents.size() > 0) {
  indents.pop_back();
  return '}';
} 
return 0;  // nothing left to do: tell parser to proceed
}

[ ]                           /* ignore remaining white space inside the program */
.                             yyerror(yytext);    /* all other chars: error! */

%%
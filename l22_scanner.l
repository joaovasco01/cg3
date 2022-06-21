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

static bool restart_functions = false;

static int lLevelsToPop = 0, lIndex = 0;
static std::vector<int> indents;   // known indentation levels
static std::vector<int> deferred;  // deferred tokens

static std::ostringstream strlit;

inline bool valid(int indent) {
    lLevelsToPop = lIndex = -1;
    bool indentOk = false;
    for (size_t ix = 0; ix < indents.size(); ix++) {
        if (indent == indents[ix]) {
            lLevelsToPop = indents.size() - ix - 1;
            lIndex = ix;
            indentOk = true;
        }
    }
    return indentOk;
}

// don't change this
#define yyerror LexerError
%}
SPACE     [ \t]
INT       [[:digit:]]+
LETTER  [A-Za-z]


%x X_STRING X_NEWLINE X_COMMENT X_SPECIALSEQ

DIGIT                [0-9]
EXPONENT             [eE][-+]?{DIGIT}+
REAL                 {DIGIT}+{EXPONENT}|({DIGIT}+"."{DIGIT}*|{DIGIT}*"."{DIGIT}+){EXPONENT}?


%%


  {
    yydebug=1; set_debug(1);

    //std::cerr << "################" << std::endl;
    //set_debug(true);
    if (restart_functions) {
      std::cerr << "POPPING FUNCTIONS (" << indents.size() << ")" << std::endl;
      indents.pop_back();
      if (indents.size() == 0) restart_functions = false;
      return '}';
    }
    else {
      if (lLevelsToPop > 0) {
        std::cerr << "POPPING LEVELS (" << lLevelsToPop << ")" << std::endl;
        lLevelsToPop--;
        indents.pop_back();
        return '}';
      }
    }

  }       

 /*-------------------Comments--------------------*/

{SPACE}*"..."\n        ; /* ignore continuation marks */

";".*\n                       ; /* ignore comments */

"(*"                          yy_push_state(X_COMMENT);
<X_COMMENT>"(*"               yy_push_state(X_COMMENT);
<X_COMMENT>"*)"               yy_pop_state();
<X_COMMENT>.|\n               ; /* ignore content */

 /*------------------Identation-----------------*/

\n                   yy_push_state(X_NEWLINE); /* delay decision */

<X_NEWLINE>^[ \t]+  {
    yy_pop_state();

    if (valid(yyleng)) {
          std::cerr << "CASE 1" << std::endl;
          if (lLevelsToPop == 0) {
            // at the same level: ignore index
            // do not return tINDENT;
            std::cerr << "CASE 1a (levels to pop = 0)" << std::endl;
            return ';';
          }
          else {
            // nothing is done here: the scanner will ignore the input while levels > 0
            // and send block-end tokens to the parser
            std::cerr << "CASE 1b (levels to pop = " << lLevelsToPop << ")" << std::endl;
            // at least one has to be returned here...
            if (lLevelsToPop > 0) {
              std::cerr << "POPPING LEVELS (" << lLevelsToPop << ")" << std::endl;
              lLevelsToPop--;
              indents.pop_back();
              return '}';
            }
          }
    }
    else if (indents.size() == 0) {
          // first block to open
          std::cerr << "CASE 2" << std::endl;
          indents.push_back(yyleng);
          return '{';
    }
    else if (yyleng > indents.back()) {
          // this is like the one before, but not the first
          std::cerr << "CASE 3" << std::endl;
          indents.push_back(yyleng);
          return '{';
    }
    else {
          // something wrong: bad indent: communicate to user
          std::cerr << "CASE 4" << std::endl;
          std::cerr << "bad indent at line " << yylineno;
    }

}

<X_NEWLINE>\n      ; // ignore successive newline chars
<X_NEWLINE>.       {
  // in this case, we have a new function starting
  // first, return whatever we are seeing to the input
  yyless(0);
  yy_pop_state();
  if (indents.size() > 0) {
    std::cerr << "should restart " << indents.size() << " functions at " << yylineno << std::endl;
    indents.pop_back();
    if (indents.size() > 0) { restart_functions = true; }
    return '}';
  }
  else return ';'; // top-level declarations
}

 /*-----------------Types----------------------*/

"int"                         return tINT_TYPE;
"double"                      return tDOUBLE_TYPE;
"text"                        return tTEXT_TYPE;
"void"                        return tVOID_TYPE; 

 /*----------------Declarations---------------*/

"foreign"                     return tFOREIGN;
"use"                         return tUSE;
"public"                      return tPUBLIC;
"var"                         return tVAR;


 /*---------------Operators----------------*/

"sizeof"                      return tSIZE_OF;
"and"                         return tAND;
"or"                          return tOR;
"not"                         return tNOT;
">="                          return tGE; 
"<="                          return tLE; 
"=="                          return tEQ; 
"!="                          return tNE; 


 /*--------------Instructions--------------*/

"if"                          return tIF; 
"elif"                        return tELIF;
"while"                       return tWHILE; 
"stop"                        return tSTOP;
"again"                       return tAGAIN;
"write"                       return tWRITE;
"writeln"                     return tWRITELN;
"return"                      return tRETURN;

 /*-------------Other instructions------------*/

"do:"                         return tDO;
"then:"                       return tTHEN;
"else:"                       return tELSE;


 /*------------Expressions------------------*/

"input"                       return tINPUT;
"null"                        return tNULL;

 /*------------Main program-------------------*/

"begin"                       return tBEGIN; 
"end"                         return tEND; 


 /*------------Nao sei o que e---------------*/

"->"                          return tRETURN_TYPE;


 /*-----------Identifier----------------*/

[A-Za-z][A-Za-z0-9_]*  yylval.s = new std::string(yytext); return tIDENTIFIER;

 /*-----------Strings------------------*/

"\""                          yy_push_state(X_STRING);
<X_STRING>"\\"                yy_push_state(X_SPECIALSEQ);
<X_STRING>"\""                {
                                yylval.s = new std::string(strlit.str());
                                strlit.str("");
                                yy_pop_state();
                                return tSTRING;
                              }

<X_STRING>.                   strlit << *yytext;

<X_SPECIALSEQ>n              { strlit << '\n'; yy_pop_state(); }
<X_SPECIALSEQ>r              { strlit << '\r'; yy_pop_state(); }
<X_SPECIALSEQ>t              { strlit << '\t'; yy_pop_state(); }
<X_SPECIALSEQ>"\""           { strlit << '"'; yy_pop_state(); }   //aspa
<X_SPECIALSEQ>"\\"           { strlit << '\\'; yy_pop_state(); }  //backslash

<X_SPECIALSEQ>0[7-9A-Za-z][A-Za-z0-9]* { strlit << '\0'; yy_pop_state(); }  

<X_SPECIALSEQ>[0-6]{1,3}     {
                                strlit << (char)(unsigned char)std::strtoul(yytext, NULL, 7);
                                 yy_pop_state();
                              }

<X_SPECIALSEQ>.			         { strlit << *yytext; yy_pop_state(); }

 /*-----------------Integers----------------------*/

0[0-6]+	          { 
                    try { 
                      yylval.i = std::stoi(yytext, nullptr, 7);
                      return tINTEGER;
                    } catch (std::out_of_range& e) { yyerror("Integer out of range."); }
                  }


[0-9]+            { 
                    try { 
                      yylval.i = std::stoi(yytext, nullptr, 10);
                      return tINTEGER;
                    } catch (std::out_of_range& e) { yyerror("Integer out of range."); }
                  }
                            
 /*-----------------Reais----------------*/  

{REAL}               		 	{ yylval.d = strtod(yytext, nullptr);		
                            if(errno == ERANGE) yyerror(""); 
                            return tREAL;
                          }

 /*-----------Chars----------------*/


[-()\[\]<>=+*:@/%,?.]       return *yytext;




[ ]         ;/* ignore remaining white space inside the program */

.                      yyerror("Unknown character");

%%

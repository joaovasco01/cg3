%{
//-- don't change *any* of these: if you do, you'll break the compiler.
#include <algorithm>
#include <memory>
#include <cstring>
#include <cdk/compiler.h>
#include <cdk/types/types.h>
#include ".auto/all_nodes.h"
#define LINE                         compiler->scanner()->lineno()
#define yylex()                      compiler->scanner()->scan()
#define yyerror(compiler, s)         compiler->scanner()->error(s)
//-- don't change *any* of these --- END!
%}

%parse-param {std::shared_ptr<cdk::compiler> compiler}

%union {
  //--- don't change *any* of these: if you do, you'll break the compiler.
  YYSTYPE() : type(cdk::primitive_type::create(0, cdk::TYPE_VOID)) {}
  ~YYSTYPE() {}
  YYSTYPE(const YYSTYPE &other) { *this = other; }
  YYSTYPE& operator=(const YYSTYPE &other) { type = other.type; return *this; }

  std::shared_ptr<cdk::basic_type> type;        /* expression type */
  //-- don't change *any* of these --- END!

  int                   i;	/* integer value */
  double                d;  /* double value */
  std::string          *s;	/* symbol name or string literal */
  cdk::basic_node      *node;	/* node pointer */
  cdk::sequence_node   *sequence;
  cdk::expression_node *expression; /* expression nodes */
  cdk::lvalue_node     *lvalue;
  std::vector<std::shared_ptr<cdk::basic_type>>  *vetor;
  l22::block_node      *bloco;
  cdk::typed_node       *typenode;
};

%token <i> tINTEGER
%token <s> tIDENTIFIER tSTRING
%token <d> tREAL
%token tWHILE tIF tELIF tTHEN tELSE tDO
%token tBEGIN tEND
%token tAGAIN tSTOP tRETURN
%token tWRITE tWRITELN
%token tINT_TYPE tDOUBLE_TYPE tTEXT_TYPE tVOID_TYPE
%token tFOREIGN tPUBLIC tUSE tVAR
%token tSIZE_OF tRETURN_TYPE tINPUT tNULL 
%token tPRIVATE

%nonassoc tIF tWHILE
%nonassoc tTHEN tDO 
%nonassoc tELIF tELSE

%right '='
%left tOR
%left tAND
%nonassoc tNOT
%left tEQ tNE 
%left tGE tLE '>' '<' 
%left '+' '-'
%left '*' '/' '%'
%nonassoc '?' tUNARY
%nonassoc '[' ']' '(' ')'


%type <node> instruction program declaration elif 
%type <sequence> file declarations expressions instructions variables 
%type <expression> expr function funcCall optional
%type <lvalue> lval
%type <type> type funcType 
%type <s> string 
%type <i> qualifier var /*public*/
%type <vetor> types
%type <bloco> block
%type <typenode> variable

%{
//-- The rules below will be included in yyparse, the main parsing function.
%}
%%

file           : /* empty */  { compiler->ast($$ = new cdk::sequence_node(LINE)); }
               | declarations { compiler->ast($$ = new cdk::sequence_node(LINE, $1)); }
               | program { compiler->ast($$ = new cdk::sequence_node(LINE, $1)); }
               | declarations program { compiler->ast($$ = new cdk::sequence_node(LINE, $2, $1)); }
               ;

program	     : tBEGIN block tEND       { $$ = new l22::program_node(LINE, $2); }
	           ;

declarations   : declaration                 { $$ = new cdk::sequence_node(LINE, $1);     }
               | declarations declaration    { $$ = new cdk::sequence_node(LINE, $2, $1); } 
               | declaration ';'             { $$ = new cdk::sequence_node(LINE, $1);     }
               | declarations declaration ';'    { $$ = new cdk::sequence_node(LINE, $2, $1); } 
               ;


optional       : /*empty*/ { $$ = nullptr;}
               | '=' expr   {$$ = $2;}
               ;

declaration    : qualifier type tIDENTIFIER               { $$ = new l22::variable_declaration_node(LINE,$1, $2, *$3, nullptr); delete $3; } 
              // | public type tIDENTIFIER '=' expr       { $$ = new l22::variable_declaration_node(LINE,$1, $2, *$3, $5); delete $3; } 
               | tPUBLIC type tIDENTIFIER optional        { $$ = new l22::variable_declaration_node(LINE,tPUBLIC, $2, *$3, $4); delete $3; } 
               | type tIDENTIFIER optional               { $$ = new l22::variable_declaration_node(LINE,tPRIVATE, $1, *$2, $3); delete $2; }
               | tPUBLIC var tIDENTIFIER  '=' expr                  { $$ = new l22::variable_declaration_node(LINE, tPUBLIC, nullptr, *$3, $5); delete $3; } 
               | var tIDENTIFIER  '=' expr                  { $$ = new l22::variable_declaration_node(LINE, tPRIVATE, nullptr, *$2, $4); delete $2; } 
               ;


/*public        :{$$ = tPRIVATE; } 
              |              tPUBLIC            {$$ = tPUBLIC;}
              ;*/

qualifier      : tUSE         { $$ = tUSE; }
               | tFOREIGN     { $$ = tFOREIGN; } 
               ;



var            : tVAR         { $$ = tVAR; }
               |              { $$ = '\0'; }
               ;

types          : /*null*/                      { $$ = new std::vector<std::shared_ptr<cdk::basic_type>>(); }
               | type                          { $$ = new std::vector<std::shared_ptr<cdk::basic_type>>(); $$->push_back($1); }
               | types ',' type                { $$ = $1; $$->push_back($3); }
               ;
type           : '[' type ']'      { $$ = cdk::reference_type::create(4,std::shared_ptr<cdk::basic_type>($2));}
               | tINT_TYPE         { $$ = cdk::primitive_type::create(4,cdk::TYPE_INT);}
               | tDOUBLE_TYPE      { $$ = cdk::primitive_type::create(8,cdk::TYPE_DOUBLE);}
               | tTEXT_TYPE        { $$ = cdk::primitive_type::create(4,cdk::TYPE_STRING);}
               | tVOID_TYPE        { $$ = cdk::primitive_type::create(0,cdk::TYPE_VOID);}
               | funcType          { $$ =  $1; }
               ;    


funcType       : type '<' types '>'     { $$ = cdk::functional_type::create(*$3, $1); }
               ;

variable       : type  tIDENTIFIER                 { $$ = new l22::variable_declaration_node(LINE,tPRIVATE, $1, *$2, nullptr); delete $2;}
               ;

variables      : variable                   { $$ = new cdk::sequence_node(LINE, $1); }
               | variables ',' variable     { $$ = new cdk::sequence_node(LINE, $3, $1); }
               ;

       
expressions    : expr                     { $$ = new cdk::sequence_node(LINE, $1);     }
               | expressions ',' expr    { $$ = new cdk::sequence_node(LINE, $3, $1); }
               ;

expr           : tINTEGER                    { $$ = new cdk::integer_node(LINE, $1); }
               | tREAL                       { $$ = new cdk::double_node(LINE, $1); }
               | string                      { $$ = new cdk::string_node(LINE, $1); }
               | tNULL                       { $$ = new l22::nullptr_node(LINE); }
               | '-' expr %prec tUNARY       { $$ = new cdk::neg_node(LINE, $2); }
               | '+' expr %prec tUNARY       { $$ = new l22::identity_node(LINE, $2); }
               | tNOT expr                   { $$ = new cdk::not_node(LINE, $2); }
               | expr '+' expr	          { $$ = new cdk::add_node(LINE, $1, $3); }
               | expr '-' expr	          { $$ = new cdk::sub_node(LINE, $1, $3); }
               | expr '*' expr	          { $$ = new cdk::mul_node(LINE, $1, $3); }
               | expr '/' expr	          { $$ = new cdk::div_node(LINE, $1, $3); }
               | expr '%' expr	          { $$ = new cdk::mod_node(LINE, $1, $3); }
               | expr '<' expr	          { $$ = new cdk::lt_node(LINE, $1, $3); }
               | expr '>' expr	          { $$ = new cdk::gt_node(LINE, $1, $3); }
               | expr tGE expr	          { $$ = new cdk::ge_node(LINE, $1, $3); }
               | expr tLE expr               { $$ = new cdk::le_node(LINE, $1, $3); }
               | expr tNE expr	          { $$ = new cdk::ne_node(LINE, $1, $3); }
               | expr tEQ expr	          { $$ = new cdk::eq_node(LINE, $1, $3); }
               | expr tOR expr               { $$ = new cdk::or_node(LINE, $1, $3); }
               | expr tAND expr              { $$ = new cdk::and_node(LINE, $1, $3); }
               | tINPUT                      { $$ = new l22::read_node(LINE); }
               | tSIZE_OF '(' expr ')'       { $$ = new l22::sizeof_node(LINE, $3); }
               | '(' expr ')'                { $$ = $2; }
               | lval '?'                    { $$ = new l22::address_of_node(LINE, $1);}
               | lval                        { $$ = new cdk::rvalue_node(LINE, $1); }  
               | lval '=' expr               { $$ = new cdk::assignment_node(LINE, $1, $3); }
               | funcCall                    { $$ = $1; }
               | function                    { $$ = $1; }  
               | '[' expr ']'                { $$ = new l22::stack_alloc_node(LINE, $2); }
               ;
 
function : '('')' tRETURN_TYPE type ':' block             { $$ = new l22::function_definition_node(LINE, nullptr, $4, $6);}
         | '(' variables ')'  tRETURN_TYPE type ':' block { $$ = new l22::function_definition_node(LINE, $2, $5, $7);}
         ; 


string : tSTRING          {$$ = $1;}
       | string tSTRING   {$$ = new std::string(*$1 + *$2); delete $1; delete $2;}
       ;

lval : tIDENTIFIER             { $$ = new cdk::variable_node(LINE, *$1); delete $1;}
     | expr '[' expr ']'      { $$ = new l22::index_node(LINE, $1, $3); }  
    // | '@'                     { $$ = new cdk::variable_node(LINE, std::string('@'));}
     ;       


block :  '{' declarations instructions '}'    {$$ = new l22::block_node(LINE, $2, $3);}
      |  '{' instructions '}'                 {$$ = new l22::block_node(LINE, nullptr, $2);}
      ;


funcCall  :  '@' '(' expressions ')'                {$$ = new l22::function_call_node(LINE, nullptr, $3);}
          |  '@' '(' ')'                {$$ = new l22::function_call_node(LINE, nullptr);}
          |  expr '(' expressions ')'  {$$ = new l22::function_call_node(LINE,$1, $3);}
          | expr    '(' ')'       {$$ = new l22::function_call_node(LINE,$1);}
          ;


instructions   : instructions instruction    { $$ = new cdk::sequence_node(LINE, $2, $1);    }
               | instruction                    { $$ = new cdk::sequence_node(LINE, $1);   }
               | instructions ';' instruction   { $$ = new cdk::sequence_node(LINE, $3, $1);    }
               ;


instruction : expr                                                  { $$ = new l22::evaluation_node(LINE, $1); }
            | tWRITE expressions                                     { $$ = new l22::print_node(LINE, $2, false); }
            | tWRITELN expressions                                   { $$ = new l22::print_node(LINE, $2, true); }
            | tAGAIN                                                { $$ = new l22::again_node(LINE); }
            | tSTOP                                                 { $$ = new l22::stop_node(LINE); }
            | tRETURN                                               { $$ = new l22::return_node(LINE, nullptr); }
            | tRETURN expr                                          { $$ = new l22::return_node(LINE, $2); }
            | tIF '(' expr ')' tTHEN block                          { $$ = new l22::if_node(LINE, $3, $6); }
            | tIF '(' expr ')' tTHEN block elif                     { $$ = new l22::if_else_node(LINE, $3, $6, $7); }
            | tWHILE '(' expr ')' tDO block                         { $$ = new l22::while_node(LINE, $3, $6); }
            | block                                                 { $$ = $1; }
            ;


elif        : tELSE block                           { $$ = $2; }
            | tELIF '(' expr ')' tTHEN block elif   { $$ = new l22::if_else_node(LINE, $3, $6, $7); }
            | tELIF '(' expr ')' tTHEN block        { $$ = new l22::if_node(LINE, $3, $6); }
            ;

%%
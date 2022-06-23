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
  double                d;	/* double value */
  std::string          *s;	/* symbol name or string literal */
  cdk::basic_node      *node;	/* node pointer */
  cdk::sequence_node   *sequence;
  cdk::expression_node *expression; /* expression nodes */
  cdk::lvalue_node     *lvalue;
  l22::program_node    *program;  
  l22::declaration_node *decl;
  l22::block_node      *block;
  std::shared_ptr<cdk::basic_type> vartype;
  std::vector<std::shared_ptr<cdk::basic_type>> *types;
  l22::function_definition_node *fndef;
};

%token tNULLPTR
%token tVAR tPRIVATE tUSE tPUBLIC tFOREIGN
%token tINT_TYPE tREAL_TYPE tSTRING_TYPE tVOID_TYPE

%token tIOTYPES tBEGIN tEND

%token tIF tTHEN tELIF tELSE
%token tWHILE tDO tSTOP tAGAIN tRETURN tWRITE tWRITELN

%token tINPUT tSIZEOF

%token <i> tINTEGER 
%token<d> tREAL
%token <s> tIDENTIFIER tSTRING

%type <node> instruction blockinstr elif
%type <program> program
%type <decl> declaration var filedeclaration
%type <sequence> file declarations instructions opt_exprs exprs simple_exprs block_exprs opt_vars vars filedeclarations
%type <expression> expr block_expr simple_expr integer real opt_expr expr_assignment
%type <lvalue> lval 
%type <block> block
%type <vartype> type function_type return_type
%type <types> types
%type <fndef> fundef
%type <s> string

%nonassoc tIF tWHILE
%nonassoc tTHEN tDO
%nonassoc tELIF tELSE

%right '='
%left tOR
%left tAND
%right tNOT
%left tNE tEQ
%left '<' tLE tGE '>'
%left '+' '-'
%left '*' '/' '%'
%right tUNARY
%nonassoc '(' '['


%{
//-- The rules below will be included in yyparse, the main parsing function.
%}
%%

file            : /* empty */                  { compiler->ast($$ = new cdk::sequence_node(LINE)); }
                | filedeclarations             { compiler->ast($$ = $1); }
                |                  program     { compiler->ast($$ = new cdk::sequence_node(LINE, $1)); }
                | filedeclarations program     { compiler->ast($$ = new cdk::sequence_node(LINE, $2, $1)); }
                ;
  
filedeclarations:                  filedeclaration { $$ = new cdk::sequence_node(LINE, $1); }
                | filedeclarations filedeclaration { $$ = new cdk::sequence_node(LINE, $2, $1); }
                ;

filedeclaration : tPUBLIC  type tIDENTIFIER opt_expr        { $$ = new l22::declaration_node(LINE, tPUBLIC, $2, *$3, $4); delete $3; }   
                | tUSE     type tIDENTIFIER ';'             { $$ = new l22::declaration_node(LINE, tUSE, $2, *$3, nullptr); delete $3; }
                | tFOREIGN type tIDENTIFIER ';'             { $$ = new l22::declaration_node(LINE, tFOREIGN, $2, *$3, nullptr); delete $3; }
                | tPUBLIC       tIDENTIFIER expr_assignment { $$ = new l22::declaration_node(LINE, tPUBLIC, nullptr, *$2, $3); delete $2; } 
                | declaration                               { $$ = $1; }
                ; 

program	        : tBEGIN block tEND     { $$ = new l22::program_node(LINE, $2); }
	              ;

block           : '{'              instructions '}' {
                $$ = new l22::block_node(LINE, nullptr, $2);
                }
                | '{' declarations instructions '}' {
                $$ = new l22::block_node(LINE, $2, $3);
                }
                | ';'                                {
                $$ = new l22::block_node(LINE, nullptr, nullptr);
                }
                ;

declarations    :               declaration  { $$ = new cdk::sequence_node(LINE, $1); }
                |  declarations declaration  { $$ = new cdk::sequence_node(LINE, $2, $1); }
                ;

declaration:    type tIDENTIFIER opt_expr        { $$ = new l22::declaration_node(LINE, tPRIVATE, $1, *$2, $3); delete $2; }
           |    tVAR tIDENTIFIER expr_assignment { $$ = new l22::declaration_node(LINE, tPRIVATE, nullptr, *$2, $3); delete $2; }
           ;

opt_expr   : ';'                                          { $$ = nullptr; }
           | expr_assignment                              { $$ = $1; }

expr_assignment :  '=' simple_expr ';'                    { $$ = $2; }
                |  '=' block_expr                         { $$ = $2; }
                ; 
         
instructions    : instruction                     { $$ = new cdk::sequence_node(LINE, $1); }
                | blockinstr                      { $$ = new cdk::sequence_node(LINE, $1); }
                | instruction ';' instructions    { std::reverse($3->nodes().begin(), $3->nodes().end()); $$ = new cdk::sequence_node(LINE, $1, $3); std::reverse($$->nodes().begin(), $$->nodes().end()); }
                | blockinstr instructions         { std::reverse($2->nodes().begin(), $2->nodes().end()); $$ = new cdk::sequence_node(LINE, $1, $2); std::reverse($$->nodes().begin(), $$->nodes().end()); }
                ;

instruction :  simple_expr                        { $$ = new l22::evaluation_node(LINE, $1); }
            |  tWRITE simple_exprs                { $$ = new l22::write_node(LINE, $2); }
            |  tWRITELN simple_exprs              { $$ = new l22::write_node(LINE, $2, true);  }  //TODO: see this
            |  tAGAIN                             { $$ = new l22::again_node(LINE); }       
            |  tSTOP                              { $$ = new l22::stop_node(LINE); }
            |  tRETURN                            { $$ = new l22::return_node(LINE); }
            |  tRETURN simple_expr                { $$ = new l22::return_node(LINE, $2); }
            ;

blockinstr  :  block_expr                         { $$ = new l22::evaluation_node(LINE, $1); }
            |  tWRITE block_exprs                 { $$ = new l22::write_node(LINE, $2); }
            |  tWRITELN block_exprs               { $$ = new l22::write_node(LINE, $2, true);  }  //TODO: see this
            |  tIF '(' expr ')' tTHEN block       { $$ = new l22::if_node(LINE, $3, $6); }
            |  tIF '(' expr ')' tTHEN block elif  { $$ = new l22::if_else_node(LINE, $3, $6, $7); }
            |  tWHILE '(' expr ')' tDO block      { $$ = new l22::while_node(LINE, $3, $6); }
            |  tRETURN block_expr                 { $$ = new l22::return_node(LINE, $2); }
            |  block                              { $$ = $1; }
            ;


elif        : tELSE block                           { $$ = $2; }
            | tELIF '(' expr ')' tTHEN block        { $$ = new l22::if_node(LINE, $3, $6); }
            | tELIF '(' expr ')' tTHEN block elif   { $$ = new l22::if_else_node(LINE, $3, $6, $7); }
            ;

type      :    tINT_TYPE                     { $$ = cdk::primitive_type::create(4, cdk::TYPE_INT);    }
          |    tREAL_TYPE                    { $$ = cdk::primitive_type::create(8, cdk::TYPE_DOUBLE); }
          |    tSTRING_TYPE                  { $$ = cdk::primitive_type::create(4, cdk::TYPE_STRING); }
          |    '[' tVOID_TYPE ']'            { $$ = cdk::reference_type::create(4, cdk::primitive_type::create(4, cdk::TYPE_VOID)); } 
          |    '[' type ']'                  { if ($2->name() == cdk::TYPE_POINTER && 
                                                  cdk::reference_type::cast($2)->referenced()->name() == cdk::TYPE_VOID) {
                                                    $$ = cdk::reference_type::create(4, cdk::primitive_type::create(4, cdk::TYPE_VOID)); 
                                                  } 
                                               else {
                                                $$ = cdk::reference_type::create(4, $2);     
                                               }   
                                             }
          |    function_type                 { $$ = $1;                                               }
          ;

function_type  : tVOID_TYPE '<' '>'          { $$ = cdk::functional_type::create(cdk::primitive_type::create(4, cdk::TYPE_VOID)); }
               | tVOID_TYPE '<' types '>'    { $$ = cdk::functional_type::create(*$3, cdk::primitive_type::create(4, cdk::TYPE_VOID));
                                               delete $3;
                                             }
               | type '<' '>'                { $$ = cdk::functional_type::create($1);          }
               | type '<' types '>'          { $$ = cdk::functional_type::create(*$3, $1); delete $3; }
               ;
                              
types          : type                        { $$ = new std::vector<std::shared_ptr<cdk::basic_type>>(); 
                                               $$->push_back($1); 
                                             }
               | types ',' type              { $$ = $1; 
                                               $$->push_back($3); 
                                             }
               ;

expr           : simple_expr                      { $$ = $1; }
               | block_expr                       { $$ = $1; }
               ;

simple_expr    : integer                          { $$ = $1; }
               | real                             { $$ = $1; }
	             | string                           { $$ = new cdk::string_node(LINE, $1); }
               | tNULLPTR                         { $$ = new l22::nullptr_node(LINE); }
               | '(' expr ')'                     { $$ = $2; }
               | '[' expr ']'                     { $$ = new l22::stack_alloc_node(LINE, $2); }
               | lval '?'                         { $$ = new l22::address_of_node(LINE, $1); }  
               | lval                             { $$ = new cdk::rvalue_node(LINE, $1); }  
               | simple_expr  '(' opt_exprs ')'   { $$ = new l22::function_call_node(LINE, $1, $3); }
               | '@' '(' opt_exprs ')'            { $$ = new l22::function_call_node(LINE, nullptr, $3); }
               | tSIZEOF '(' expr ')'             { $$ = new l22::sizeof_node(LINE, $3); }
               | tINPUT                           { $$ = new l22::input_node(LINE); }
               | '+' simple_expr %prec tUNARY     { $$ = new l22::identity_node(LINE, $2); }
               | '-' simple_expr %prec tUNARY     { $$ = new cdk::neg_node(LINE, $2); } 
               | simple_expr '+' simple_expr	    { $$ = new cdk::add_node(LINE, $1, $3); }
               | simple_expr '-' simple_expr	    { $$ = new cdk::sub_node(LINE, $1, $3); }
               | simple_expr '*' simple_expr	    { $$ = new cdk::mul_node(LINE, $1, $3); }
               | simple_expr '/' simple_expr	    { $$ = new cdk::div_node(LINE, $1, $3); }
               | simple_expr '%' simple_expr	    { $$ = new cdk::mod_node(LINE, $1, $3); }
               | simple_expr '<' simple_expr	    { $$ = new cdk::lt_node(LINE, $1, $3); }
               | simple_expr '>' simple_expr	    { $$ = new cdk::gt_node(LINE, $1, $3); }
               | simple_expr tGE simple_expr	    { $$ = new cdk::ge_node(LINE, $1, $3); }
               | simple_expr tLE simple_expr      { $$ = new cdk::le_node(LINE, $1, $3); }
               | simple_expr tNE simple_expr	    { $$ = new cdk::ne_node(LINE, $1, $3); }
               | simple_expr tEQ simple_expr	    { $$ = new cdk::eq_node(LINE, $1, $3); }
               | block_expr '*' simple_expr	      { $$ = new cdk::mul_node(LINE, $1, $3); }
               | block_expr '/' simple_expr	      { $$ = new cdk::div_node(LINE, $1, $3); }
               | block_expr '%' simple_expr	      { $$ = new cdk::mod_node(LINE, $1, $3); }
               | block_expr '<' simple_expr	      { $$ = new cdk::lt_node(LINE, $1, $3); }
               | block_expr '>' simple_expr	      { $$ = new cdk::gt_node(LINE, $1, $3); }
               | block_expr tGE simple_expr	      { $$ = new cdk::ge_node(LINE, $1, $3); }
               | block_expr tLE simple_expr       { $$ = new cdk::le_node(LINE, $1, $3); }
               | block_expr tNE simple_expr	      { $$ = new cdk::ne_node(LINE, $1, $3); }
               | block_expr tEQ simple_expr	      { $$ = new cdk::eq_node(LINE, $1, $3); }
               | tNOT simple_expr                 { $$ = new cdk::not_node(LINE, $2); }
               | simple_expr tAND simple_expr     { $$ = new cdk::and_node(LINE, $1, $3); }
               | simple_expr tOR simple_expr      { $$ = new cdk::or_node (LINE, $1, $3); }
               | block_expr tAND simple_expr     { $$ = new cdk::and_node(LINE, $1, $3); }
               | block_expr tOR simple_expr      { $$ = new cdk::or_node (LINE, $1, $3); }
               | lval '=' simple_expr             { $$ = new cdk::assignment_node(LINE, $1, $3); }
               ;

block_expr     : '+' block_expr %prec tUNARY         { $$ = new l22::identity_node(LINE, $2); }
               | '-' block_expr %prec tUNARY         { $$ = new cdk::neg_node(LINE, $2); } 
               | block_expr '*' block_expr	         { $$ = new cdk::mul_node(LINE, $1, $3); }
               | block_expr '/' block_expr	         { $$ = new cdk::div_node(LINE, $1, $3); }
               | block_expr '%' block_expr	         { $$ = new cdk::mod_node(LINE, $1, $3); }
               | block_expr '<' block_expr	         { $$ = new cdk::lt_node(LINE, $1, $3); }
               | block_expr '>' block_expr	         { $$ = new cdk::gt_node(LINE, $1, $3); }
               | block_expr tGE block_expr	         { $$ = new cdk::ge_node(LINE, $1, $3); }
               | block_expr tLE block_expr           { $$ = new cdk::le_node(LINE, $1, $3); }
               | block_expr tNE block_expr	         { $$ = new cdk::ne_node(LINE, $1, $3); }
               | block_expr tEQ block_expr	         { $$ = new cdk::eq_node(LINE, $1, $3); }
               | simple_expr '+' block_expr	         { $$ = new cdk::add_node(LINE, $1, $3); }
               | simple_expr '-' block_expr	         { $$ = new cdk::sub_node(LINE, $1, $3); }
               | simple_expr '*' block_expr	         { $$ = new cdk::mul_node(LINE, $1, $3); }
               | simple_expr '/' block_expr	         { $$ = new cdk::div_node(LINE, $1, $3); }
               | simple_expr '%' block_expr	         { $$ = new cdk::mod_node(LINE, $1, $3); }
               | simple_expr '<' block_expr	         { $$ = new cdk::lt_node(LINE, $1, $3); }
               | simple_expr '>' block_expr	         { $$ = new cdk::gt_node(LINE, $1, $3); }
               | simple_expr tGE block_expr	         { $$ = new cdk::ge_node(LINE, $1, $3); }
               | simple_expr tLE block_expr          { $$ = new cdk::le_node(LINE, $1, $3); }
               | simple_expr tNE block_expr	         { $$ = new cdk::ne_node(LINE, $1, $3); }
               | simple_expr tEQ block_expr	         { $$ = new cdk::eq_node(LINE, $1, $3); }
               | tNOT block_expr                     { $$ = new cdk::not_node(LINE, $2); }
               | block_expr tAND block_expr          { $$ = new cdk::and_node(LINE, $1, $3); }
               | block_expr tOR block_expr           { $$ = new cdk::or_node (LINE, $1, $3); }
               | simple_expr tAND block_expr     { $$ = new cdk::and_node(LINE, $1, $3); }
               | simple_expr tOR block_expr      { $$ = new cdk::or_node (LINE, $1, $3); }
               | lval '=' block_expr                 { $$ = new cdk::assignment_node(LINE, $1, $3); }
               | fundef                              { $$ = $1; }
               ;   

opt_exprs      :  /* empty */                     { $$ = new cdk::sequence_node(LINE); }
               |  exprs                           { $$ = $1; }
               ;

exprs          : expr                       { $$ = new cdk::sequence_node(LINE, $1); }   
               | exprs ',' expr             { $$ = new cdk::sequence_node(LINE, $3, $1); }
               ;

simple_exprs   : simple_expr                      { $$ = new cdk::sequence_node(LINE, $1); }   
               | exprs ',' simple_expr            { $$ = new cdk::sequence_node(LINE, $3, $1); }
               ;

block_exprs    : block_expr                      { $$ = new cdk::sequence_node(LINE, $1); }   
               | exprs ',' block_expr            { $$ = new cdk::sequence_node(LINE, $3, $1); }
               ;


fundef         : '(' opt_vars ')' tIOTYPES return_type ':' block    { $$ = new l22::function_definition_node(LINE, $5, $2, $7); }
               ;

opt_vars       :  /* empty */                    { $$ = new cdk::sequence_node(LINE); }
               |  vars                           { $$ = $1; }
               ;

vars           :          var                    { $$ = new cdk::sequence_node(LINE, $1); }   
               | vars ',' var                    { $$ = new cdk::sequence_node(LINE, $3, $1); }
               ;

var            : type tIDENTIFIER                { $$ = new l22::declaration_node(LINE, tPRIVATE, $1, *$2, nullptr); delete $2; }
               ;

return_type    :  type                           { $$ = $1; }
               |  tVOID_TYPE                     { $$ = cdk::primitive_type::create(4, cdk::TYPE_VOID); }
               ;
              
integer         : tINTEGER                       { $$ = new cdk::integer_node(LINE, $1); };
real            : tREAL                          { $$ = new cdk::double_node(LINE, $1); };
string          : tSTRING                        { $$ = $1; }
                | string tSTRING                 { $$ = $1; $$->append(*$2); delete $2; }
                ;

lval :  tIDENTIFIER                     { $$ = new cdk::variable_node(LINE, $1); }
     |  simple_expr  '[' expr ']'       { $$ = new l22::index_node(LINE, $1, $3); }    
     ;

%%
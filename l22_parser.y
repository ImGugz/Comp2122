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
  int                   d;	/* double value */
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
%token tVAR tUSE tPUBLIC tFOREIGN tPRIVATE
%token tINT_TYPE tREAL_TYPE tSTRING_TYPE tVOID_TYPE

%token tIOTYPES tBEGIN tEND

%token tIF tTHEN tELIF tELSE
%token tWHILE tDO tSTOP tAGAIN tRETURN tWRITE tWRITELN

%token tINPUT tSIZEOF

%token <i> tINTEGER
%token<d> tREAL
%token <s> tIDENTIFIER tSTRING

%type <node> instruction blkinstr elif
%type <program> program
%type <decl> declaration var
%type <sequence> file declarations opt_instructions instructions opt_exprs exprs opt_vars vars
%type <expression> expr integer real
%type <lvalue> lval 
%type <block> block
%type <vartype> type function_type return_type
%type <types> types
%type <fndef> fundef
%type <s> string

/* NOTE: Check ambiguities regarding this later */
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


%{
//-- The rules below will be included in yyparse, the main parsing function.
%}
%%

file            : /* empty */              { compiler->ast($$ = new cdk::sequence_node(LINE)); }
                | program                  { compiler->ast($$ = new cdk::sequence_node(LINE, $1)); }
                | declarations program     { compiler->ast($$ = new cdk::sequence_node(LINE, $2, $1)); }
                ;

program	      : tBEGIN block tEND        { $$ = new l22::program_node(LINE, $2); }
	           ;

block           : '{'              opt_instructions '}' {
                $$ = new l22::block_node(LINE, nullptr, $2);
                }
                | '{' declarations opt_instructions '}' {
                $$ = new l22::block_node(LINE, $2, $3);
                }
                ;

declarations    :  declaration               { $$ = new cdk::sequence_node(LINE, $1); }
                |  declarations declaration  { $$ = new cdk::sequence_node(LINE, $2, $1); }
                ;

opt_instructions:  /* empty */          { $$ = new cdk::sequence_node(LINE); }
                |  instructions         { $$ = $1; }
                ;

// TODO: create new rule for optional attribution (this eliminates 2 productions here)
// FIXME: what happens when the program only has declarations??
declaration:   tPUBLIC type tIDENTIFIER '=' expr ';'  { $$ = new l22::declaration_node(LINE, tPUBLIC, $2, *$3, $5); delete $3; }
           |   tPUBLIC type tIDENTIFIER ';'           { $$ = new l22::declaration_node(LINE, tPUBLIC, $2, *$3, nullptr); delete $3; }
           |   type tIDENTIFIER '=' expr ';'          { $$ = new l22::declaration_node(LINE, tPRIVATE, $1, *$2, $4); delete $2; }
           |   type tIDENTIFIER ';'                   { $$ = new l22::declaration_node(LINE, tPRIVATE, $1, *$2, nullptr); delete $2; }
           |   tPUBLIC tIDENTIFIER '=' expr ';'       { $$ = new l22::declaration_node(LINE, tPUBLIC, nullptr, *$2, $4); delete $2; }  
           |   tPUBLIC tVAR tIDENTIFIER '=' expr ';'  { $$ = new l22::declaration_node(LINE, tPUBLIC, nullptr, *$3, $5); delete $3; }
            /* NOTE: is a declaration without 'var' allowed? */
           |   tVAR tIDENTIFIER '=' expr ';'         { $$ = new l22::declaration_node(LINE, tPRIVATE, nullptr, *$2, $4); delete $2; }
           ;

// TODO: try to optimize this at the end, this happens because last instruction of the block does not end in a ';'
// FIXME: empty blocks????
instructions    : instruction                     { $$ = new cdk::sequence_node(LINE, $1);     }
                | blkinstr                        { $$ = new cdk::sequence_node(LINE, $1);     }
                | instruction ';' instructions    { std::reverse($3->nodes().begin(), $3->nodes().end()); $$ = new cdk::sequence_node(LINE, $1, $3); std::reverse($$->nodes().begin(), $$->nodes().end()); }
                | blkinstr instructions           { std::reverse($2->nodes().begin(), $2->nodes().end()); $$ = new cdk::sequence_node(LINE, $1, $2); std::reverse($$->nodes().begin(), $$->nodes().end()); }
                ;

instruction :  expr                               { $$ = new l22::evaluation_node(LINE, $1); }
            |  tWRITE exprs                       { $$ = new l22::write_node(LINE, $2); }
            |  tWRITELN exprs                     { $$ = new l22::write_node(LINE, $2, true);  }
            |  tAGAIN                             { $$ = new l22::again_node(LINE); }       
            |  tSTOP                              { $$ = new l22::stop_node(LINE); }
            |  tRETURN                            { $$ = new l22::return_node(LINE); }
            |  tRETURN expr                       { $$ = new l22::return_node(LINE, $2); }
            ;

// NOTE: these block instructions do not allow for empty if-then-else's and while's!
blkinstr        : tIF '(' expr ')' tTHEN block          { $$ = new l22::if_node(LINE, $3, $6); }
                | tIF '(' expr ')' tTHEN block elif     { $$ = new l22::if_else_node(LINE, $3, $6, $7); }
                | tWHILE '(' expr ')' tDO block         { $$ = new l22::while_node(LINE, $3, $6); }
                | block                                 { $$ = $1; }
                ;

elif            : tELSE block                           { $$ = $2; }
                | tELIF '(' expr ')' tTHEN block        { $$ = new l22::if_node(LINE, $3, $6); }
                | tELIF '(' expr ')' tTHEN block elif   { $$ = new l22::if_else_node(LINE, $3, $6, $7); }

/* NOTE: Is it the parser's responsibility to make sure void types are correctly used ?? */
type      :    tINT_TYPE                     { $$ = cdk::primitive_type::create(4, cdk::TYPE_INT);    }
          |    tREAL_TYPE                    { $$ = cdk::primitive_type::create(8, cdk::TYPE_DOUBLE); }
          |    tSTRING_TYPE                  { $$ = cdk::primitive_type::create(4, cdk::TYPE_STRING); }
          |    '[' tVOID_TYPE ']'            { $$ = cdk::reference_type::create(4, nullptr);          } 
          |    '[' type ']'                  { $$ = cdk::reference_type::create(4, $2);               }
          |    function_type                 { $$ = $1;                                               }
          ;

/* NOTE: not quite sure of this construction... */
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
               | types type                  { $$ = $1; 
                                               $$->push_back($2); 
                                             }
               ;

expr           : integer                          { $$ = $1; }
               | real                             { $$ = $1; }
	          | string                           { $$ = new cdk::string_node(LINE, $1); }
               | tNULLPTR                         { $$ = new l22::nullptr_node(LINE); }
               | '(' expr ')'                     { $$ = $2; }
               | '[' expr ']'                     { $$ = new l22::stack_alloc_node(LINE, $2); }
               | lval '?'                         { $$ = new l22::address_of_node(LINE, $1); }  // NOTE: does this need precedence??
               | '+' expr %prec tUNARY            { $$ = $2; }
               | '-' expr %prec tUNARY            { $$ = new cdk::neg_node(LINE, $2); }
               | expr '+' expr	               { $$ = new cdk::add_node(LINE, $1, $3); }
               | expr '-' expr	               { $$ = new cdk::sub_node(LINE, $1, $3); }
               | expr '*' expr	               { $$ = new cdk::mul_node(LINE, $1, $3); }
               | expr '/' expr	               { $$ = new cdk::div_node(LINE, $1, $3); }
               | expr '%' expr	               { $$ = new cdk::mod_node(LINE, $1, $3); }
               | expr '<' expr	               { $$ = new cdk::lt_node(LINE, $1, $3); }
               | expr '>' expr	               { $$ = new cdk::gt_node(LINE, $1, $3); }
               | expr tGE expr	               { $$ = new cdk::ge_node(LINE, $1, $3); }
               | expr tLE expr                    { $$ = new cdk::le_node(LINE, $1, $3); }
               | expr tNE expr	               { $$ = new cdk::ne_node(LINE, $1, $3); }
               | expr tEQ expr	               { $$ = new cdk::eq_node(LINE, $1, $3); }
               | tNOT expr                        { $$ = new cdk::not_node(LINE, $2); }
               | expr tAND expr                   { $$ = new cdk::and_node(LINE, $1, $3); }
               | expr tOR expr                    { $$ = new cdk::or_node (LINE, $1, $3); }
               | lval                             { $$ = new cdk::rvalue_node(LINE, $1); }  
               | lval '=' expr                    { $$ = new cdk::assignment_node(LINE, $1, $3); }
               | '(' expr ')' '(' opt_exprs ')'   { $$ = new l22::function_call_node(LINE, $2, $5); }
               | fundef                           { $$ = $1; }
               | tSIZEOF '(' expr ')'             { $$ = new l22::sizeof_node(LINE, $3); }
               | tINPUT                           { $$ = new l22::input_node(LINE); }
               ;

opt_exprs      :  /* empty */                     { $$ = new cdk::sequence_node(LINE); }
               |  exprs                           { $$ = $1; }
               ;

exprs          : expr                            { $$ = new cdk::sequence_node(LINE, $1); }   
               | exprs  ',' expr                 { $$ = new cdk::sequence_node(LINE, $3, $1); }
               ;

/* NOTE: see 'return_type' also */
fundef         : '(' opt_vars ')' tIOTYPES return_type ':' block      { $$ = new l22::function_definition_node(LINE, $5, $2, $7); }
               ;

opt_vars       :  /* empty */                    { $$ = new cdk::sequence_node(LINE); }
               |  vars                           { $$ = $1; }
               ;

vars           : var                             { $$ = new cdk::sequence_node(LINE, $1); }   
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
     |  lval   '[' expr ']'             { $$ = new l22::index_node(LINE, new cdk::rvalue_node(LINE, $1), $3); }
     |  '(' expr ')' '[' expr ']'       { $$ = new l22::index_node(LINE, $2, $5); }    
     ;

%%
%error-verbose

%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "e_proc.h"
#include "e_tac.h"

extern FILE * yyin;
extern int yylineno;
int line;

char character;
char token[1000];

int yylex();
void yyerror(char* msg);
%}

%union {
    struct op *operation;
    struct member_def *definition;

    char* name;
    char* string;

    int num_int;
    double num_float;
    char num_char;

    // int data_type;
    int data_type;
    struct var_type *variable_type;

    int index;
}

%token <name> IDENTIFIER
%token <num_int> NUM_INT
%token <num_float> NUM_FLOAT
%token <num_char> NUM_CHAR
%token <string> TEXT
%token <data_type> VOID
%token <data_type> INT
%token <data_type> LONG
%token <data_type> FLOAT
%token <data_type> DOUBLE
%token <data_type> CHAR
%token <index> INDEX
%type <index> index_or_null
%type <variable_type> parameter_type
%type <variable_type> void_type
%type <variable_type> basic_type
%type <operation> program
%type <operation> function_declaration_list
%type <operation> function_declaration
%type <operation> function
%type <operation> function_head
%type <operation> parameter_list
%type <operation> statement_block
%type <operation> declaration_list
%type <operation> declaration
%type <operation> struct_definition
%type <definition> definition_block
%type <definition> definition_list
%type <definition> definition
%type <definition> member_list
%type <definition> add_member
%type <operation> variable_list
%type <operation> statement_list
%type <operation> statement
%type <operation> assign_statement
%type <operation> input_statement
%type <operation> output_statement
%type <operation> return_statement
%type <operation> if_statement
%type <operation> while_statement
%type <operation> for_statement
%type <operation> break_statement
%type <operation> continue_statement
%type <operation> call_statement
%type <operation> statement_or_expression_or_null
%type <operation> assign_statement_or_null
%type <operation> argument_list
%type <operation> expression_list
%type <operation> left_val
%type <operation> right_val
%type <operation> expression
%type <operation> inc_expression
%type <operation> dec_expression
%type <operation> expression_or_null
%type <operation> const_val
%type <operation> add_identifier
%type <operation> array_identifier
%type <operation> existed_identifier

%token EQ NE LT LE GT GE
%token IF ELSE WHILE FOR BREAK CONTINUE INPUT OUTPUT RETURN STRUCT

%left INC DEC
%left EQ NE LT LE GT GE
%left '+' '-'
%left '*' '/'
%nonassoc NEGATIVE

%%

/**************************************/
/************ func&program ************/
/**************************************/
program : function_declaration_list { $$ = process_program($1); }

function_declaration_list : function_declaration { $$ = cpy_op($1); }
| function_declaration_list function_declaration { $$ = cat_list($1,$2); }
;

function_declaration : function { $$ = cpy_op($1); }
| declaration { $$ = cpy_op($1); }
| struct_definition { $$ = cpy_op($1); }
;

function : function_head statement_block { reset_table(OUT_LOCAL_TABLE); $$ = process_function($1,$2); }
| error {}
;

function_head : basic_type IDENTIFIER '(' parameter_list ')' { $$ = process_function_head($1,$2,$4); }
| void_type IDENTIFIER '(' parameter_list ')' { $$ = process_function_head($1,$2,$4); }
;

parameter_list : parameter_type IDENTIFIER { reset_table(INTO_LOCAL_TABLE); $$ = process_parameter_list_head($1,$2); }
| parameter_list ',' parameter_type IDENTIFIER { $$ = process_parameter_list($1,$3,$4); }
| { reset_table(INTO_LOCAL_TABLE); $$ = new_op(); }
;

/**************************************/
/**************** stat ****************/
/**************************************/
struct_definition : STRUCT IDENTIFIER definition_block { $$ = process_struct_definition($2,$3); }

definition_block : '{' definition_list '}' ';' { $$ = $2; }

definition_list : definition { $$ = $1; }
| definition_list definition { $$ = cat_def($1,$2); }
;

definition : basic_type member_list ';' { $$ = process_definition($1,$2); }

member_list : add_member { $$ = $1; }
| member_list ',' add_member { $$ = cat_def($1,$3); }
;

add_member : IDENTIFIER index_or_null { $$ = add_member_def_raw($1,$2); }

statement_block: '{' declaration_list statement_list '}' { $$ = cat_list($2,$3); }

declaration_list : { $$ = new_op(); }
| declaration_list declaration { $$ = cat_list($1,$2); }
;

declaration : basic_type variable_list ';' { $$ = process_declaration($1,$2); }

variable_list : add_identifier { $$ = cpy_op($1); }
| variable_list ',' add_identifier { $$ = cat_list($1,$3); }
;

statement_list : statement { $$ = cpy_op($1); }  
| statement_list statement { $$ = cat_list($1,$2); }
;

statement : assign_statement ';' { $$ = cpy_op($1); }
| input_statement ';' { $$ = cpy_op($1); }
| output_statement ';' { $$ = cpy_op($1); }
| call_statement ';' { $$ = cpy_op($1); }
| return_statement ';' { $$ = cpy_op($1); }
| if_statement { $$ = cpy_op($1); }
| while_statement { $$ = cpy_op($1); }
| for_statement { $$ = cpy_op($1); }
| break_statement ';' { $$ = cpy_op($1); }
| continue_statement ';' { $$ = cpy_op($1); }
| statement_block { $$ = cpy_op($1); }
| error { $$ = new_op(); }
;

assign_statement : left_val '=' right_val { $$ = process_assign($1,$3); }
| left_val '=' assign_statement { $$ = process_assign($1,$3); }
;

input_statement : INPUT IDENTIFIER { $$ = process_input($2); }

output_statement : OUTPUT IDENTIFIER { $$ = process_output_variable($2); }
| OUTPUT TEXT { $$ = process_output_text($2); }
;

return_statement : RETURN right_val { $$ = process_return($2); }

if_statement : IF '(' right_val ')' statement_block { $$ = process_if_only($3,$5); }
| IF '(' right_val ')' statement_block ELSE statement_block { $$ = process_if_else($3,$5,$7); }
;

while_head : WHILE { block_initialize(); }

for_head : FOR { block_initialize(); }

while_statement : while_head '(' right_val ')' statement_block { $$ = process_while($3,$5); }

for_statement : for_head '(' assign_statement_or_null ';' expression_or_null ';' statement_or_expression_or_null ')' statement_block 
                { $$ = process_for($3,$5,$7,$9); }

break_statement : BREAK { $$ = process_break(); }

continue_statement : CONTINUE { $$ = process_continue(); }

call_statement : IDENTIFIER '(' argument_list ')' { $$ = process_call($1,$3); }

statement_or_expression_or_null : statement { $$ = cpy_op($1); }
| expression { $$ = cpy_op($1); }
| { $$ = new_op(); }
;

assign_statement_or_null : assign_statement { $$ = cpy_op($1); }
| { $$ = new_op(); }
;

argument_list  : { $$ = new_op(); }
| expression_list { $$ = process_argument_list($1); }
;

/*************************************/
/**************** exp ****************/
/*************************************/
expression_list : expression_list ',' right_val { $$ = process_expression_list($1,$3); }
|  right_val { $$ = process_expression_list_head($1); }
;

left_val : '*' expression { $$ = process_derefer_put($2); }
| '*' existed_identifier { $$ = process_derefer_put($2); }
| array_identifier { $$ = process_derefer_put($1); }
| existed_identifier { $$ = cpy_op($1); }
;

right_val : expression { $$ = cpy_op($1); }
| '*' expression { $$ = process_derefer_get($2); }
| '&' expression { $$ = process_reference($2); }
| '*' existed_identifier { $$ = process_derefer_get($2); }
| '&' existed_identifier { $$ = process_reference($2); }
| array_identifier { $$ = process_derefer_get($1); }
| existed_identifier { $$ = cpy_op($1); }
| const_val { $$ = cpy_op($1); }
;

expression : inc_expression { $$ = cpy_op($1); }
| dec_expression { $$ = cpy_op($1); }
| right_val '+' right_val { $$ = process_calculate($1,$3,TAC_PLUS); }
| right_val '-' right_val { $$ = process_calculate($1,$3,TAC_MINUS); }
| right_val '*' right_val { $$ = process_calculate($1,$3,TAC_MULTIPLY); }		
| right_val '/' right_val { $$ = process_calculate($1,$3,TAC_DIVIDE); }		
| right_val EQ right_val { $$ = process_calculate($1,$3,TAC_EQ); }
| right_val NE right_val { $$ = process_calculate($1,$3,TAC_NE); }
| right_val LT right_val { $$ = process_calculate($1,$3,TAC_LT); }
| right_val LE right_val { $$ = process_calculate($1,$3,TAC_LE); }	
| right_val GT right_val { $$ = process_calculate($1,$3,TAC_GT); }	
| right_val GE right_val { $$ = process_calculate($1,$3,TAC_GE); }	
| '-' right_val %prec NEGATIVE { $$ = process_calculate(NUM_ZERO,$2,TAC_MINUS); } // hjj: 统一性起见，不再有单独的negative处理了
| '(' right_val ')' { $$ = cpy_op($2); }
| call_statement { $$ = cpy_op($1); }
;

inc_expression : INC left_val { $$ = process_inc($2,INC_HEAD); }
| left_val INC { $$ = process_inc($1,INC_TAIL); }
;

dec_expression : DEC left_val { $$ = process_dec($2,DEC_HEAD); }
| left_val DEC { $$ = process_dec($1,DEC_TAIL); }
;

expression_or_null : expression { $$ = cpy_op($1); }
| { $$ = new_op(); }
;

const_val : NUM_INT { $$ = process_int($1); }
| NUM_FLOAT { $$ = process_float($1); }
| NUM_CHAR { $$ = process_char($1); }
;

// 这么写是因为改成star_or_null的形式会出现神秘的无法解析全局变量的冲突
add_identifier : IDENTIFIER index_or_null { $$ = process_add_identifier($1,$2) }

array_identifier : existed_identifier INDEX { $$ = process_array_identifier($1,$2); }

existed_identifier : IDENTIFIER '.' IDENTIFIER { $$ = process_instance_member($1,$3); }
| IDENTIFIER { $$ = process_identifier($1); }
;

parameter_type : basic_type '&' { $$ = new_var_type($1->data_type,REF_VAR); }
| basic_type { $$ = $1; }
;

void_type : VOID { $$ = new_var_type(DATA_VOID,NOT_PTR); }

basic_type : INT { $$ = new_var_type(DATA_INT,NOT_PTR); }
| LONG { $$ = new_var_type(DATA_LONG,NOT_PTR); }
| FLOAT { $$ = new_var_type(DATA_FLOAT,NOT_PTR); }
| DOUBLE { $$ = new_var_type(DATA_DOUBLE,NOT_PTR); }
| CHAR { $$ = new_var_type(DATA_CHAR,NOT_PTR); }
| INT '*' { $$ = new_var_type(DATA_INT,PTR_VAR); }
| LONG '*' { $$ = new_var_type(DATA_LONG,PTR_VAR); }
| FLOAT '*' { $$ = new_var_type(DATA_FLOAT,PTR_VAR); }
| DOUBLE '*' { $$ = new_var_type(DATA_DOUBLE,PTR_VAR); }
| CHAR '*' { $$ = new_var_type(DATA_CHAR,PTR_VAR); }
| STRUCT IDENTIFIER { $$ = process_struct_type($2,NOT_PTR); }
| STRUCT IDENTIFIER '*' { $$ = process_struct_type($2,PTR_VAR); }
;

index_or_null : INDEX { $$ = $1; }
| { $$ = NO_INDEX; }
;

%%

/* these come from Flex/Bison */
extern char *yytext;     /* the text of the current token */
extern int yylineno;     /* current line number, if you did %option yylineno */
extern int yylex(void);  /* the lexer */

void yyerror(char* msg) 
{
    int tok = yychar;                   /* the last lookahead token code */
    const char *tname = "UNKNOWN";
    if (tok >= 0 && tok < YYNTOKENS)    /* YYNTOKENS is generated by Bison */
        tname = yytname[tok];

    printf("Syntax error at line %d: %s\n", yylineno, msg, tname, yytext);
    exit(0);
}
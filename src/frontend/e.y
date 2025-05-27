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
    struct op* operation;
    // struct var_list* variable_list;
    char* name;
    char* string;
    int num_int;
    double num_float;
    char num_char;
    int data_type;
}

%token <name> IDENTIFIER
%token <num_int> NUM_INT
%token <num_float> NUM_FLOAT
%token <num_char> NUM_CHAR
%token <string> TEXT
%token <data_type> INT
%token <data_type> LONG
%token <data_type> FLOAT
%token <data_type> DOUBLE
%token <data_type> CHAR
%type <name> left_val
%type <name> right_val
%type <data_type> complex_type
%type <data_type> basic_type
%type <operation> program
%type <operation> function_declaration_list
%type <operation> function_declaration
%type <operation> function
%type <operation> function_head
%type <operation> parameter_list
%type <operation> block
%type <operation> declaration_list
%type <operation> declaration
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
%type <operation> expression
%type <operation> inc_expression
%type <operation> dec_expression
%type <operation> expression_or_null

%token EQ NE LT LE GT GE
%token IF ELSE WHILE FOR BREAK CONTINUE INPUT OUTPUT RETURN

%left INC DEC
%left EQ NE LT LE GT GE
%left '+' '-'
%left '*' '/'
%nonassoc NEGATIVE

%%

/**************************************/
/**************** func ****************/
/**************************************/
program : function_declaration_list
                            {
                                $$ = process_program($1);
                            }

function_declaration_list : function_declaration
                            {
                                $$ = cpy_op($1);
                            }
| function_declaration_list function_declaration
                            {
                                $$ = cat_list($1,$2);
                            }
;

function_declaration : function
                            {
                                $$ = cpy_op($1);
                            }
| declaration 
                            {
                                $$ = cpy_op($1);
                            }
;

function : function_head '(' parameter_list ')' block
                            {
	                            reset_table(OUT_LOCAL_TABLE); 
                                $$ = process_function($1,$3,$5);
                            }
| error {}
;

function_head : complex_type IDENTIFIER
                            {
                                $$ = process_function_head($1,$2);
	                            reset_table(INTO_LOCAL_TABLE); 
                            }
;

parameter_list : complex_type IDENTIFIER               
                            {
                                $$ = process_parameter_list_head($1,$2);
                            }
| parameter_list ',' complex_type IDENTIFIER               
                            {
                                $$ = process_parameter_list($1,$3,$4);
                            }
|                           {
                                $$ = new_op();
                            }
;

/**************************************/
/**************** stat ****************/
/**************************************/
block: '{' declaration_list statement_list '}'					
                            {
                                $$ = cat_list($2,$3);
                            }
;

declaration_list :
                            {
                                $$ = new_op();
                            }
| declaration_list declaration
                            {
                                $$ = cat_list($1,$2);
                            }
;

declaration : complex_type variable_list ';'
                            {
                                $$ = process_declaration($1,$2);
                            }
;

variable_list : IDENTIFIER
                            {
                                $$ = process_variable_list_end($1);
                            }
| variable_list ',' IDENTIFIER
                            {
                                $$ = process_variable_list($1,$3);
                            }
;

statement_list : statement			
                            {
                                $$ = cpy_op($1);
                            }  
| statement_list statement  
                            {
                                $$ = cat_list($1,$2);
                            }
;

statement : assign_statement ';'
                            {
                                $$ = cpy_op($1);
                            }
| input_statement ';' 
                            {
                                $$ = cpy_op($1);
                            }
| output_statement ';' 
                            {
                                $$ = cpy_op($1);
                            }
| call_statement ';'
                            {
                                $$ = cpy_op($1);
                            }
| return_statement ';' 
                            {
                                $$ = cpy_op($1);
                            }
| if_statement
                            {
                                $$ = cpy_op($1);
                            }
| while_statement 
                            {
                                $$ = cpy_op($1);
                            }
| for_statement 
                            {
                                $$ = cpy_op($1);
                            }
| break_statement ';'           
                            {
                                $$ = cpy_op($1);
                            }
| continue_statement ';'           
                            {
                                $$ = cpy_op($1);
                            }
| block 
                            {
                                $$ = cpy_op($1);
                            }
| error
                            {
                                $$ = new_op();
                            }
;

assign_statement : left_val '=' expression	
                            {
                                $$ = process_assign($1,$3);
                            }
| left_val '=' assign_statement 
                            {
                                $$ = process_assign($1,$3);
                            }
;

input_statement : INPUT IDENTIFIER
                            {
                                $$ = process_input($2);
                            }
;

output_statement : OUTPUT IDENTIFIER
                            {
                                $$ = process_output_variable($2);
                            }
| OUTPUT TEXT
                            {
                                $$ = process_output_text($2);
                            }
;

return_statement : RETURN expression
                            {
                                $$ = process_return($2);
                            }
;


if_statement : IF '(' expression ')' block
                            {
                                $$ = process_if_only($3,$5);
                            }
| IF '(' expression ')' block ELSE block
                            {
                                $$ = process_if_else($3,$5,$7);
                            }
;

while_head : WHILE 
                            {
                                block_initialize();
                            }

for_head : FOR
                            {
                                block_initialize();
                            }

while_statement : while_head '(' expression ')' block
                            {
                                $$ = process_while($3,$5);
                            }
;

for_statement : for_head '(' assign_statement_or_null ';' expression_or_null ';' statement_or_expression_or_null ')' block
                            {
                                $$ = process_for($3,$5,$7,$9);
                            }
;

break_statement : BREAK
                            {
                                $$ = process_break();
                            }

continue_statement : CONTINUE
                            {
                                $$ = process_continue();
                            }

call_statement : IDENTIFIER '(' argument_list ')'
                            {
                                $$ = process_call($1,$3);
                            }
;

statement_or_expression_or_null : statement
                            {
                                $$ = cpy_op($1);
                            }
| expression
                            {
                                $$ = cpy_op($1);
                            }
|
                            {
                                $$ = new_op();
                            }
;

assign_statement_or_null : assign_statement
                            {
                                $$ = cpy_op($1);
                            }
|
                            {
                                $$ = new_op();
                            }
;

argument_list  :
                            {
                                $$ = new_op();
                            }
| expression_list
                            {
                                $$ = process_argument_list($1);
                            }
;

/*************************************/
/**************** exp ****************/
/*************************************/
expression_list : expression_list ',' expression
                            {
                                $$ = process_expression_list($1,$3);
                            }
|  expression
                            {
                                $$ = process_expression_list_head($1);
                            }
;

expression : inc_expression
                            {
                                $$ = cpy_op($1);
                            }
| dec_expression
                            {
                                $$ = cpy_op($1);
                            }
| expression '+' expression	
                            { 
                                $$ = process_calculate($1,$3,TAC_PLUS);
                            }
| expression '-' expression				
                            { 
                                $$ = process_calculate($1,$3,TAC_MINUS);
                            }
| expression '*' expression		
                            { 
                                $$ = process_calculate($1,$3,TAC_MULTIPLY);
                            }		
| expression '/' expression		
                            { 
                                $$ = process_calculate($1,$3,TAC_DIVIDE);
                            }		
| expression EQ expression				
                            { 
                                $$ = process_calculate($1,$3,TAC_EQ);
                            }
| expression NE expression				
                            { 
                                $$ = process_calculate($1,$3,TAC_NE);
                            }
| expression LT expression			
                            { 
                                $$ = process_calculate($1,$3,TAC_LT);
                            }
| expression LE expression			
                            { 
                                $$ = process_calculate($1,$3,TAC_LE);
                            }	
| expression GT expression			
                            { 
                                $$ = process_calculate($1,$3,TAC_GT);
                            }	
| expression GE expression			
                            { 
                                $$ = process_calculate($1,$3,TAC_GE);
                            }	
| '(' expression ')'				
                            { 
                                $$ = cpy_op($2);
                            }
| '-' expression  %prec NEGATIVE
                            {
                                $$ = process_negative($2);
                            }

| call_statement
                            {
                                $$ = cpy_op($1);
                            }
| NUM_INT					
                            {
                                $$ = process_int($1);
                            }
| NUM_FLOAT
                            {
                                $$ = process_float($1);
                            }
| NUM_CHAR          
                            {
                                $$ = process_char($1);
                            }
| right_val
                            {
                                $$ = process_rightval($1);
                            }
;

inc_expression : INC IDENTIFIER
                            {
                                $$ = process_inc($2,INC_HEAD);
                            }
| IDENTIFIER INC
                            {
                                $$ = process_inc($1,INC_TAIL);
                            }
;

dec_expression : DEC IDENTIFIER
                            {
                                $$ = process_dec($2,DEC_HEAD);
                            }
| IDENTIFIER DEC
                            {
                                $$ = process_dec($1,DEC_TAIL);
                            }
;

expression_or_null : expression
                            {
                                $$ = cpy_op($1);
                            }
| 
                            {
                                $$ = new_op();
                            }
;

left_val : '*' IDENTIFIER
                            {
                                $$ = (char*)process_dereference($2);
                            }
| IDENTIFIER
                            {
                                $$ = $1;                               
                            }
;

right_val : '*' IDENTIFIER
                            {
                                $$ = (char*)process_dereference($2);
                            }
| '&' IDENTIFIER
                            {
                                $$ = (char*)process_reference($2);
                            }
| IDENTIFIER
                            {
                                $$ = $1;                               
                            }
;

complex_type : basic_type
                            {
                                $$ = $1;
                            }
| basic_type '*'
                            {
                                $$ = $1 + PTR_OFFSET;
                            }
;

basic_type : INT
                            {
                                $$ = DATA_INT;
                            }
| LONG
                            {
                                $$ = DATA_LONG;
                            }
| FLOAT
                            {
                                $$ = DATA_FLOAT;
                            }
| DOUBLE
                            {
                                $$ = DATA_DOUBLE;
                            }
| CHAR                      
                            {
                                $$ = DATA_CHAR;
                            }

%%

void yyerror(char* msg) 
{
	printf("%s: line %d\n", msg, yylineno);
	exit(0);
}
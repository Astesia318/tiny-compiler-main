%{
#include <stdlib.h>
#include "e.y.h"
#include "e_tac.h"
#define MAX 100

%}

%option yylineno

ID 							[_a-zA-Z][_a-zA-Z0-9]*

INT_CONST					{DEC_CONST}|{OCT_CONST}|{HEX_CONST}
DEC_CONST					([1-9][0-9]*)
OCT_CONST					(0[0-7]*)	
HEX_CONST					((0x|0X)[0-9a-fA-F]+)

FLOAT_CONST					{DEC_FLOAT_CONST}|{HEX_FLOAT_CONST}
DEC_FLOAT_CONST				((([0-9]+\.[0-9]+)|([0-9]+\.){EXP_PART}?)|([0-9]+{EXP_PART}))
EXP_PART					([eE][-+]?[0-9]+)
HEX_FLOAT_CONST				((0x|0X)(([0-9a-fA-F]*\.[0-9a-fA-F]+)|([0-9a-fA-F]+\.))|([0-9a-fA-F]+){BIN_EXP_PART})
BIN_EXP_PART				([pP][-+]?[0-9]+)

CHAR_CONST					\'[^\']\'
STRING_CONST				\"[^\"]*\"

SINGLE_COMMENT				"//".*
MULTI_COMMENT				"/*"([^\*]|(\*)*[^\*/])*(\*)*"*/"

%%

"void"  					{ return VOID; }
"int" 						{ return INT; }
"long"  					{ return LONG; }
"float"  					{ return FLOAT; }
"double"  					{ return DOUBLE; }
"char"						{ return CHAR; }
"struct"  					{ return STRUCT; }
"input"  					{ return INPUT; }
"output"  					{ return OUTPUT; }
"return"  					{ return RETURN; }
"break"  					{ return BREAK; }
"continue"	 	 			{ return CONTINUE; }
"if"  						{ return IF; }
"else"  					{ return ELSE; }
"while"  					{ return WHILE; }
"for"  						{ return FOR; }
"->"  						{ return PSTRUCT_ACCESS; }

{ID}						{ yylval.name = strdup(yytext); return IDENTIFIER; }
{INT_CONST} 				{ yylval.num_int = atoi(yytext); return NUM_INT; }
{FLOAT_CONST} 				{ yylval.num_float = strtod(yytext,NULL); return NUM_FLOAT; }
{CHAR_CONST} 				{ yylval.num_char = strdup(yytext)[1]; return NUM_CHAR; }
{STRING_CONST} 				{ yylval.string = strdup(yytext); return TEXT; }

"++"					 	{ return INC; }
"--"  						{ return DEC; }
"=="  						{ return EQ; }
"!="  						{ return NE; }
"<="  						{ return LE; }
"<"  						{ return LT; }
">="  						{ return GE; }
">"  						{ return GT; }

[ \t\r\n]

{SINGLE_COMMENT} 			{/*ignored*/}
{MULTI_COMMENT}				{/*ignored*/}

. 	{  return *yytext;  }

%%

int yywrap()
{
	return 1;
}


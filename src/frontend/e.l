%{
#include <stdlib.h>
#include "e.y.h"
#include "e_tac.h"
#define MAX 100

%}

%option yylineno

%%

"int"  {  return INT;  }

"long"  {  return LONG;  }

"float"  {  return FLOAT;  }

"double"  {  return DOUBLE;  }

"input"  {  return INPUT;  }

"output"  {  return OUTPUT;  }

"return"  {  return RETURN;  }

"break"  {  return BREAK;  }

"continue"  {  return CONTINUE;  }

"if"  {  return IF;  }

"then"  {  return THEN;  }

"else"  {  return ELSE;  }

"fi"  {  return FI;  }

"while"  {  return WHILE;  }

"for"  {  return FOR;  }

"do"  {  return DO;  }

"done"  {  return DONE;  }

[A-Za-z]([A-Za-z]|[0-9])*  {  yylval.name = strdup(yytext); return IDENTIFIER;  }

[0-9]+  {  yylval.num_int = atoi(yytext); return NUM_INT;  }

[0-9]+\.[0-9]+  {  yylval.num_float = strtod(yytext,NULL); return NUM_FLOAT;  }

\"[^\"]*\" {  yylval.string = strdup(yytext); return TEXT;  }

"++"  {  return INC; }

"--"  {  return DEC; }

"=="  {  return EQ;  }

"!="  {  return NE;  }

"<="  {  return LE;  }

"<"  {  return LT;  }

">="  {  return GE;  }

">"  {  return GT;  }

[ \t\r\n]

"//".*  {/*ignored*/}
"/*"([^\*]|(\*)*[^\*/])*(\*)*"*/"  {/*ignored*/}

. 	{  return *yytext;  }

%%

int yywrap()
{
	return 1;
}


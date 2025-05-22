%{
#include "asm.y.h"
%}

%option yylineno

%%

"ADD"  {  return ADD;  }

"SUB"  {  return SUB;  }

"MUL"  {  return MUL;  }

"DIV"  {  return DIV;  }

"TST"  {  return TST;  }

"STO"  {  return STO;  }

"LOD"  {  return LOD;  }

"JMP"  {  return JMP;  }

"JEZ"  {  return JEZ;  }

"JLZ"  {  return JLZ;  }

"JGZ"  {  return JGZ;  }

"DBN"  {  return DBN;  }

"DBS"  {  return DBS;  }

"IN"  {  return IN;  }

"OUTN"  {  return OUTN;  }

"OUTS"  {  return OUTS;  }

"NOP"  {  return NOP;  }

"END"  {  return END;  }

[0-9]*	{
	yylval.number = atoi(yytext);
	return INTEGER;
}

R[0-9]*  {
	yylval.number = atoi(yytext+1);
	return REG;
}

[A-Za-z]([A-Za-z]|[0-9])*  {  
	yylval.string = strdup(yytext);
	return LABEL;
}

[ \t\r\n]|#.*

.  {  return *yytext;  }

%%

int yywrap()
{
	return 1;
}


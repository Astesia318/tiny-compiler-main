#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "e_tac.h"
#include "o_wrap.h"

extern int yyparse();
extern FILE *yyin;

FILE *source_file, *tac_file, *obj_file;

int main(int argc, char *argv[]) {
	if (argc != 2) {
		printf("usage: %s filename\n", argv[0]);
		exit(0);
	}

	char *input = argv[1];
	source_file = fopen(input, "r");
	if (source_file == NULL) {
		printf("open file %s failed\n", argv[1]);
		exit(0);
	}
	yyin = source_file;

	char *output = strdup(input);
	output[strlen(output) - 1] = 'x';
	tac_file = fopen(output, "w");
	if (tac_file == NULL) {
		printf("open file tac.txt failed\n");
		exit(0);
	}

	output[strlen(output) - 1] = 's';
	obj_file = fopen(output, "w");
	if (obj_file == NULL) {
		printf("open file tac.txt failed\n");
		exit(0);
	}

	init_tac();
	yyparse();
	source_to_tac(tac_file, tac_head);
	printf("program compiled to tac!\n");
	tac_to_obj();
	printf("tac converted to obj!\n");

	fclose(source_file);
	fclose(tac_file);

	return 0;
}
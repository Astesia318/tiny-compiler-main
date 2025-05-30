struct student {
	int number, score;
	char *name;
};

struct teacher {
	int number, salary;
	char *name;
};

struct cls {
	int number;
	struct student students[30];
	struct teacher teacher[5];
	struct teacher *advisor;
};

// void init_class(struct cls &clas, int number, struct teacher *advisor) {
// 	clas.number = number;
// 	clas.advisor = advisor;
// }

// void init_student(struct student &s, int number, int score, char *name) {
// 	s.number = number;
// 	s.score = score;
// 	s.name = name;
// }
int global;

int main() {
	// struct cls classes[10];
	// struct teacher *advisor;
	char a;
	char *p;
	p = &a;
	a = 'a';
	*p = 'b';
	global = 10;

	// int i, j;
	// for (i = 0; i < 10; i = i + 1) {
	// 	init_class(classes[i], i, advisor);
	// 	for (j = 0; j < 30; j = j + 1) {
	// 		init_student(classes[i].students[j], j, 100, name);
	// 	}
	// }

	return 0;
}
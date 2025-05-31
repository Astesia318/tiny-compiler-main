struct student {
	int number, score;
	char *name;
};

struct teacher {
	int number, salary;
	char *name;
};

struct cls {
	int number;                   // 4
	struct student students[30];  // 30*12
	struct teacher teacher[5];    // 5*12
	struct teacher *advisor;      // 4
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

int main() {
	struct cls classes[10];
	struct teacher *advisor;
	char name[20];

	int i, j;
	for (i = 0; i < 10; i = i + 1) {
		classes[i].number = i;
		classes[i].advisor = advisor;
		for (j = 0; j < 30; j = j + 1) {
			classes[i].students[j].score = 100;
			classes[i].students[j].name = name;
		}
	}

	return 0;
}
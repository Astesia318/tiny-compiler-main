struct student {
	int number, score;
	char *name;
};

struct teacher {
	int number, salary;
	char *name;
};

struct cls {
	int number;                  // 4
	struct student students[2];  // 2*12
	struct teacher teacher[3];   // 3*12
	struct teacher *advisor;     // 4
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
	int i, j;
	for (i = 0; i < 10; i = i + 1) {
		for (j = 0; j < 20; j = j + 1) {
			continue;
			int x = 10;
		}
	}
	return 0;
}
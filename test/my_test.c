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
	int x;
	struct cls *classes[2];
	struct cls class_instance;
	classes[1] = &class_instance;
	classes[1]->students[1].number = 1234;
	x = classes[1]->students[1].number;
	output x;
	return 0;
}
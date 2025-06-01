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
//     clas.number = number;
//     clas.advisor = advisor;
// }

// void init_student(struct student &s, int number, int score, char *name) {
//     s.number = number;
//     s.score = score;
//     s.name = name;
// }

int main() {
	struct teacher *advisor;
	struct teacher advisor_instence;
	int x;
	advisor = &advisor_instence;
	advisor->number = 100;
	advisor_instence.salary = 200;
	x = advisor->number;
	output x;
	return 0;
}
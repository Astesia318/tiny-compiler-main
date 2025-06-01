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
	struct student students[3];
	struct teacher teacher[5];
	struct teacher *advisor;
};

void init_class(struct cls &clas, int number, struct teacher *advisor) {
	clas.number = number;
	clas.advisor = advisor;
}

void init_student(struct student &s, int number, int score, char *name) {
	s.number = number;
	s.score = score;
	s.name = name;
}

int main() {
	struct cls classes[5];
	struct teacher *advisor;
	char name[20];
	int x;
	int i, j;

	for (i = 0; i < 5; i = i + 1) {
		init_class(classes[i], i, advisor);
		for (j = 0; j < 3; j = j + 1) {
			init_student(classes[i].students[j], j, 100 * (j + 1) + i, name);
		}
	}
	for (i = 0; i < 5; i = i + 1) {
		x = classes[i].number;
		output x;
		for (j = 0; j < 3; j = j + 1) {
			x = classes[i].students[j].score;
			output x;
		}
	}
	return 0;
}
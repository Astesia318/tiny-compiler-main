int random() {
	int a, b, c;
	a = 1;
	b = 2;
	c = 3;
	return a + b + c;
}

int main() {
	int target,guess;
	target = random();
	input guess;
	for(;guess!=target;){
		output "wrong!";
		input guess;
	}
	output "right!";
	return 0;
}
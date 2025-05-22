int fib(int i) {
	if (i == 0) {
		return 1;
	}
	if (i == 1) {
		return 1;
	}
	return fib(i - 1) + fib(i);
}

int main() {
	int n, sum;
	input n;
	sum = fib(n);
	return 0;
}
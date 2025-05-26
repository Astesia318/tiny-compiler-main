int add(int *x, int *y) { return 1; }  // return *x + *y; }
int main() {
	int a, b, c;
	int *d;
	a = 1;
	b = 2;
	// c = add(&a, &b);
	d = &a;
	*d = 3;
	return 0;
}
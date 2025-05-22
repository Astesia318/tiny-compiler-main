int add(int x,int y){
	return x+y;
}

int main() {
	int a, b, c;
	a = 10;
	b = 20;
	// c = 30;
	a = a + b;
	a = add(a,b);
	output a;
	output "\n";
	return 0;
}
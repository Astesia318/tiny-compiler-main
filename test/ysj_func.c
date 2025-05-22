int add(int x,int y)
{
	return x+y;
}


int main() 
{
	int a,b,c;
	output "waiting for input\n";
	input a;
	input b;
	c=add(a,b);
	output "going to output\n";
	output c;
	output "\n";
	return 0;
}

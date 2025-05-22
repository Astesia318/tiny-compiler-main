int g;
int alongname;
int fib(int a)
{
    if(a<=2){
        return 1;
    }
    return fib(a - 1) + fib(a - 2);
}
int main()
{
    int a,b, g;
    a = 1;
    b = 3;
    g = fib(10);
    a = g;
    return 0;
}
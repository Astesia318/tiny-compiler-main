char global;
int main()
{
    char a;
    int b;
    long c;

    a = 'b';
    global = a;
    c = a+global;
    return 0;
}
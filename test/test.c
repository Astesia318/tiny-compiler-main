int global;
char func(char a)
{
    return a;
}
int main()
{
    int a;
    int b;
    char x;
    char temp;

    a = 10;
    b = 12;
    x = 'a';
    b = 'x' + 'y';
    temp = func('d');
    while (x >temp)
    {
        b = '1';
        if(a<='2'){
            break;
        }
    }

    return 0;
}
// int global;
float func(float a)
{
    float b;
    b = a;
    return b;
}
float main()
{
    float x;
    float y;
    float z;
    int a;
    char c;
    x = 2.6;
    y = 3.4;
    a = x+y;//f2i
    a = func(a);//i2f f2i
    c = a;
    a = c;  
    c = x;
    return z;
}
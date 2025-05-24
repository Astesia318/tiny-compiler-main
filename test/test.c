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
    a = 0;
    x = 2.6;
    y = 3.4;
    z = x + 2.1;
    z = x - 2.1;
    z = x * 2.1;
    z = x / 2.1;
    z = x < 2.1;
    z = x > 2.1;
    z = x >= 2.1;
    z = x <= 2.1;
    z = x == 2.1;
    z = x != 2.1;
    if(a==1){
        x = a;
        a = x;
    }
    else{
        a = 1;
    }
    return z;
}
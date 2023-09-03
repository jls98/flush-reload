#include <stdio.h>

int gcd_simple(int number1, int number2)
{
    if (number1 < 1 || number2 < 1)
        return 0;
    int gcd = 1;

    for (int i = 1; i <= number1 && i <= number2; ++i)
    {
        if (number1 % i == 0 && number2 % i == 0)
        {
            gcd = i;
        }
    }
    return gcd;
}

int gcd_euklidean(int number1, int number2)
{
    int t, a = number1, b = number2;
    while(b != 0)
    {
        t = b;
        b = a % b;
        a = t;
    }
    return a;
}

int main() 
{
    
    int a = 24, b = 18, c;
    c = gcd_simple(a, b);
    printf("The GCD of %i and %i is %i!\n", a, b, c);
    return 0;
}
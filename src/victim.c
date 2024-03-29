#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int gcd_simple(int number1, int number2)
{
    if (number1 < 1 || number2 < 1) return 0;
    int gcd = 1;
    for (int i = 1; i <= number1 && i <= number2; ++i)
    {
        if (number1 % i == 0 && number2 % i == 0) gcd = i;
        
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

int main(int argc, char *argv[]) 
{
    if(argc != 4) return 1;
    int mode, a = strtol(argv[1], NULL, 10) != 0 ? strtol(argv[1], NULL, 10) : 1, b = strtol(argv[2], NULL, 10) != 0 ? strtol(argv[2], NULL, 10) : 1, c;
    
	//mode = *argv[3] == 0 || *argv[3] == 1 ? (unsigned int) *argv[3] : ((unsigned)time(NULL)) % 2; 
    mode = *argv[3] % 2;
    c = mode ? gcd_simple(a, b) : gcd_euklidean(a, b);
    //c = mode ? gcd_simple(a, b) : gcd_simple(a, b);
    //c = mode ? gcd_euklidean(a, b) : gcd_euklidean(a, b);
    
    printf("The GCD of %i and %i is %i! And mode is %i\n", a, b, c, mode);
    return 0;
}

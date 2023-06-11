#include <stdio.h>

int square_and_multiply(int base, int exponent, int modulus) {
    long long result = 1;
    base = base % modulus;

    while (exponent > 0) 
    {
        if (exponent % 2 == 1) 
        {
            result = (result * base) % modulus;
        }
        base = (base * base) % modulus;
        exponent = exponent / 2;
    }

    return result;
}

int main() 
{
    
    int (*fPtrSAM)() = &square_and_multiply;
    int (*fPtrMain)() = &main;
    printf("Adresse der Funktion SAM: %p, main %p\n", fPtrSAM, fPtrMain);

    int base = 17;
    int exponent = 0b1110001100101111110001100101111;
    int modulus = 37;

    int result = square_and_multiply(base, exponent, modulus);
    printf("%d\n", result);  // Output: 2

    return 0;
}

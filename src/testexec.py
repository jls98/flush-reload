def square_and_multiply(base, exponent, modulus):
    result = 1
    base = base % modulus

    while exponent > 0:
        if exponent % 2 == 1:
            breakpoint()
            result = (result * base) % modulus
        breakpoint()
        base = (base * base) % modulus
        exponent = exponent // 2

    return result

secret = 0b111000110010111
base = 17
modulus = 37

print(square_and_multiply(base, secret, modulus))
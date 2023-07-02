#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "operations.h"
#include "pi.h"

/*
 * Polynome:
 * a(n) = 2
 * b(n) = 1
 * p(n) = n
 * q(n) = 2*n + 1
 */

/*
 * Implementation of the polynomial p(n) = n
 */
struct bignum p(size_t n)
{
    struct bignum res;
    bignumInit(&res, n);
    return res;
}

/*
 * Recursive definition of P(n1, n2);
 */
struct bignum P(size_t n1, size_t n2)
{
    struct bignum res;
    size_t nm = (n1 + n2) / 2;

    if (n1 == n2 - 1)
    {
        res = p(n1);
    }
    else
    {
        struct bignum num0;
        struct bignum num1;
        num0 = P(n1, nm);
        num1 = P(nm, n2);
        karazMult(&num0, &num1, &res);

        bignumFree(&num0);
        bignumFree(&num1);
    }
    return res;
}

/*
 * Implementation of the polynomial q(n) = 2n + 1;
 * shift 1 bit to the left and then add 1
 */
struct bignum
q(size_t n)
{
    struct bignum temp1, temp2, res;
    bignumInit(&temp1, n);
    bignumInit(&temp2, n + 1);
    bignumAdd(&temp2, &temp1, &res);
    bignumFree(&temp1);
    bignumFree(&temp2);
    return res;
}

/*
 * Recursive definition of Q(n1, n2)
 * similar to P(size_t n1, size_t n2)
 */
struct bignum Q(size_t n1, size_t n2)
{
    struct bignum res;

    if (n1 == n2 - 1)
    {
        res = q(n1);
    }
    else
    {
        size_t nm = (n1 + n2) / 2;
        struct bignum num0;
        struct bignum num1;
        num0 = Q(n1, nm);
        num1 = Q(nm, n2);

        karazMult(&num0, &num1, &res);

        bignumFree(&num0);
        bignumFree(&num1);
    }
    return res;
}

/*
 * Recursive definition of T(n1, n2)
 */
struct bignum
T(size_t n1, size_t n2)
{
    struct bignum res;
    size_t nm = (n1 + n2) / 2;

    if (n1 == n2 - 1)
    {
        res = p(n1);

        struct bignum temp;
        bignumCopy(&res, &temp);
        memcpy(&temp.numbers[0], &res.numbers[0], res.length * (sizeof(uint32_t)));
        bignumFree(&res);
        lShift(&temp, 1, &res);
        bignumFree(&temp);
    }
    else
    {
        struct bignum num0;
        struct bignum num1;
        struct bignum num2;
        struct bignum num3;
        num2 = Q(nm, n2);
        num3 = T(n1, nm);

        karazMult(&num2, &num3, &num0);

        bignumFree(&num2);
        bignumFree(&num3);

        num2 = P(n1, nm);
        num3 = T(nm, n2);

        karazMult(&num2, &num3, &num1);

        bignumFree(&num2);
        bignumFree(&num3);

        bignumAdd(&num0, &num1, &res);

        bignumFree(&num0);
        bignumFree(&num1);
    }
    return res;
}

/*
 * Returns the approximation of pi with precision of s binary subone places
 * by computing 2 + T(1, n) / Q(1, n) * B(1, n)
 * B(1, n) = 1
 */
struct bignum
comp_pi(size_t s)
{

    struct bignum res;

    if (s == 0)
    {
        bignumInit(&res, 3);
        return res;
    }

    struct bignum Tuso, Mauso;
    Tuso = T(1, 4 * s);
    Mauso = Q(1, 4 * s);

    res = newtonDiv(&Tuso, &Mauso, 4 * s);
    bignumFree(&Tuso);
    bignumFree(&Mauso);

    // Cut the unprecise places in last block
    s *= 4;
    s %= 32;
    if (s != 0)
    {
        res.numbers[0] = (res.numbers[0] >> (32 - s)) << (32 - s);
    }

    res.numbers = realloc(res.numbers, (res.length + 1) * sizeof(uint32_t));
    checkAllocationError(res.numbers);
    res.numbers[res.length - 1] += 2;

    return res;
}

/*
This Version of pi uses a formula resulted from applying Euler's Transform on Leibniz Series
the formula is: pi= 2 + 1/3 (2 + (2/5 (2 + (3/7 (2 + 4/9 (2 + ... ) ) ) ) ) )
the result is in decimal reppresentation.
Qwelle und prrof: https://codegolf.stackexchange.com/questions/18865/stupid-restrictions-and-desert/18988#18988
*/
void version2_Dec(size_t n, int last_itr)
{
    size_t prec = n + 1;
    char str[prec + 10];
    uint64_t d = 0, h = 1e9; // long_division is used, in base 1000000000
    uint32_t b, digits = 30 * ((prec + 25) / 9), e = 0, g;
    size_t counter = 0;
    uint32_t *remainder = (uint32_t *)malloc(4 * digits);
    checkAllocationError(remainder);
    digits -= 30;
    b = digits;
    while (b)
    { // applying a formula derived from Leibniz Series
        while (--b)
        {
            uint32_t temp;
            if (e)
            {
                temp = remainder[b];
            }
            else
            {
                temp = 2;
            }
            d = d * b + h * temp;
            g = 2 * b - 1;
            remainder[b] = d % g;
            d /= g;
        }
        uint32_t res = e + d / h;

        sprintf(str + counter, e ? "%09u" : "%u.", res);
        if (e)
        {
            counter += 9;
        }
        else
        {
            counter += 2;
        }

        e = d % h;
        digits -= 30;
        b = digits;
    }
    str[prec + 1] = '\0';
    if (last_itr)
    {
        printf("Result: %s\n", str);
    }

    free(remainder);
}

/*
 * this functions applays the Bailey–Borwein–Plouffe BBF formula by calculating 4S(1,n)−2S(4,n)−S(5,n)−S(6,n)
*/
void version2_Hex(size_t n, int last_itr)
{
    if (last_itr)
    {
        printf("Result: 3.");
    }
    int rest = n%5;
    
    for (int i = 0; i < (int)n-rest; i+=5)
    {
        apply(i, last_itr, 0);
    }
    for (int i= (int)n-rest; i < (int)n; i++)
    {
        apply(i, last_itr, 1);
    }
    if (last_itr)
    {
        printf("\n");
    }
}
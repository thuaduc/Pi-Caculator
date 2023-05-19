#include <stdlib.h>
#include "operations.h"
#include "pi.h"


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
        karazubaMult(&num0, &num1, &res);

        bignumFree(&num0);
        bignumFree(&num1);
    }
    return res;
}

/*
 * Implementation of the polynomial q(n) = 2n + 1;
 * shift 1 bit to the left and then add 1
 */
struct bignum q(size_t n)
{
    struct bignum temp1, temp2, res;
    bignumInit(&temp1, n);
    bignumInit(&temp2, n+1);
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
        
        karazubaMult(&num0, &num1, &res);

        bignumFree(&num0);
        bignumFree(&num1);
    }
    return res;
}

/*
 * Recursive definition of T(n1, n2)
 */
struct bignum T(size_t n1, size_t n2)
{
    struct bignum res;
    size_t nm = (n1 + n2) / 2;

    if (n1 == n2 - 1)
    {
        res = p(n1);

        struct bignum temp;
        temp.length = res.length;
        temp.subone = res.subone;
        temp.numbers = malloc(res.length*(sizeof(uint32_t)));
        memcpy(&temp.numbers[0], &res.numbers[0], res.length*(sizeof(uint32_t)));

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

        karazubaMult(&num2, &num3, &num0);

        bignumFree(&num2);
        bignumFree(&num3);

        num2 = P(n1, nm);
        num3 = T(nm, n2);

        karazubaMult(&num2, &num3, &num1);

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
struct bignum comp_pi(size_t s)
{

    struct bignum res;

    if (s == 0)
    {
        bignumInit(&res, 3);
        return res;
    }

    struct bignum Tuso, Mauso;
    Tuso = T(1, s + 1);
    Mauso = Q(1, s + 1);

    printf("N before Div: "); bignumPrint(&Tuso);
    printf("D before Div: "); bignumPrint(&Mauso);
    
    newtonDiv(&Tuso, &Mauso, s, &res);
    bignumFree(&Tuso);
	bignumFree(&Mauso);

    // NewtonDiv already returns a result cut to the right amount of blocks, so only the unprecise places have to be cut here
    s %= 32;
    if (s != 0)
    {
        res.numbers[0] = (res.numbers[0] >> (32 - s)) << (32 - s);
    }

    res.numbers = realloc(res.numbers, (res.length + 1) * sizeof(uint32_t));
    res.numbers[res.length-1] += 2;
    //res.length++;
    //bignumPrint( &res );
    //reduceSize(&res,1);
    printf("Pi = 2 + X* T_reduced : "); bignumPrint( &res );
    printf("----------------------------------\n");
    //bignumPrintHex(&res, 10);
    //bignumPrintDec(&res, 1);

}

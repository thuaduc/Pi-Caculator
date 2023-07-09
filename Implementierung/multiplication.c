#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <inttypes.h>
#include <string.h>
#include "operations.h"

size_t getKarazubaFactors(const struct bignum *x, const struct bignum *y, struct bignum *x0, struct bignum *x1, struct bignum *y0, struct bignum *y1);
void baseShiftL(struct bignum *x, size_t blocks);
void extendLengthToSubone(struct bignum *x);
void splitAndPadd(const struct bignum *a, struct bignum *a0, struct bignum *a1, size_t blocksize);

/**
 * karazuba Multiplication
 * res = x*y
 */
void karazMult(const struct bignum *x, const struct bignum *y, struct bignum *res)
{
    // base case of the recursion
    if((x->length == 0) || (y->length == 0)){
        bignumInit(res, 0);
        return;
    }
    else if ((x->length == 1) && (y->length == 1))
    {
        if (x->numbers[0] == 0 || y->numbers[0] == 0)
        {
            bignumInit(res, 0);
        }
        else
        {
            uint64_t simple_prod = ((uint64_t)x->numbers[0]) * ((uint64_t)y->numbers[0]);
            bignumInit(res, simple_prod);
            res->subone = x->subone + y->subone;
        }
        return;
    }

    // save the position of the decimal point for the end
    // it is ignoreed it in the multiplicationa and inserted at the end
    size_t res_subone = x->subone + y->subone;

    // split the numbers in factors x = x0 + b * x1
    struct bignum x0, x1, y0, y1;
    size_t base_length = getKarazubaFactors(x, y, &x0, &x1, &y0, &y1);
    // replace multiplication with base by shift
    // The multiplication formula:
    // x0y0 + x1y1*b^2 + [(x0+x1)(y0+y1) - x0y0 - x1y1] * b = p0 + p1 *b^2 + [sum_x*sum_y - p0 - p1]*b

    struct bignum t0, t1, t2;
    bignumAdd(&x1, &x0, &t0); // t0 <- x1+x0
    bignumAdd(&y1, &y0, &t1); // t1 <- y1+y0
    karazMult(&t0, &t1, &t2); // t2 <- (x0 + x1)(y0*y1)
    bignumFree(&t1);
    bignumFree(&t0);

    struct bignum p0, p1;

    karazMult(&y0, &x0, &p0); // Product of lower factors: p0 = x0y0
    bignumFree(&x0);
    bignumFree(&y0);

    karazMult(&y1, &x1, &p1); // Product of higher factors: p1 = x1y1
    bignumFree(&x1);
    bignumFree(&y1);

    // sum_x*sum_y - p0 - p1
    // unsigned Subtraction because always: sum_x*sum_y <=  p0 + p1
    bignum_uSub(&t2, &p0);
    bignum_uSub(&t2, &p1);

    baseShiftL(&t2, base_length);     // [sum_x*sum_y - p0 - p1]*b
    baseShiftL(&p1, base_length * 2); // p1 * b^2
    bignumAdd(&t2, &p0, &t1);         // t1 <- result + p0
    bignumAdd(&t1, &p1, &t0);         // t0 <- result + p1*b^2
    bignumFree(&p0);
    bignumFree(&p1);

    bignumFree(&t2);
    bignumFree(&t1);
    res->numbers = t0.numbers;
    res->length = t0.length;

    // res = &temp;
    res->subone = res_subone;
    if (res->subone > res->length)
        extendLengthToSubone(res);
}

/**
 * Get the Karazuba Factors
 *
 * Input: x, y
 * Results in:
 * x0  lower half of x
 * x1  upper half of x
 * y0
 * y1
 */
size_t getKarazubaFactors(const struct bignum *x, const struct bignum *y, struct bignum *x0, struct bignum *x1, struct bignum *y0, struct bignum *y1)
{
    size_t greater_length = x->length > y->length ? x->length : y->length;
    size_t blocksize; // size of the factors (rounded up half of the og size)
    if (greater_length % 2 == 0)
        blocksize = greater_length / 2;
    else
        blocksize = (greater_length / 2) + 1;

    splitAndPadd(x, x0, x1, blocksize);
    splitAndPadd(y, y0, y1, blocksize);
    return blocksize;
}

/**
 * the number, viewed as an integer, is shifted by blocks to the left
 */
void baseShiftL(struct bignum *x, size_t blocks)
{
    if (blocks == 0)
        return;
    if (x->length == 1 && x->numbers[0] == 0)
        return;
    uint32_t *more_numbers = malloc((blocks + x->length) * sizeof(uint32_t));
    if (!more_numbers)
    {
        // error handling
        fprintf(stderr, "Error while allocation memory! (baseShiftL)");
        exit(EXIT_FAILURE);
    }

    memcpy(&more_numbers[blocks], &x->numbers[0], sizeof(uint32_t) * x->length);
    memset(&more_numbers[0], 0, sizeof(uint32_t) * blocks);
    free(x->numbers);
    x->numbers = more_numbers;
    x->length += blocks;
    // x->subone ignored
}

// length > subone + 1 has to apply (because there always needs to be a whole part)
void extendLengthToSubone(struct bignum *x)
{
    uint32_t *newNumbers = calloc(x->subone, sizeof(uint32_t));
    if (newNumbers == NULL)
    {
        fprintf(stderr, "Error allocation memory. (extendLengthToSubone)");
        exit(EXIT_FAILURE);
    }
    memcpy(newNumbers, &x->numbers[0], sizeof(uint32_t) * x->length);
    size_t diff = x->subone - x->length;
    memset(&newNumbers[x->length], 0, sizeof(uint32_t) * diff);
    free(x->numbers);
    x->numbers = newNumbers;
    x->length = x->subone;
}

/**
 * splits bignum in two blocksize sized chunks (eventually padds with 0)
 *
 * condition: a->length <= blocksize*2
 */
void splitAndPadd(const struct bignum *a, struct bignum *a0, struct bignum *a1, size_t blocksize)
{
    a0->numbers = calloc(blocksize, sizeof(uint32_t));
    a1->numbers = calloc(blocksize, sizeof(uint32_t));
    if (a0->numbers == NULL || a1->numbers == NULL)
    {
        fprintf(stderr, "Error allocation memory. (splitAndPadd)");
        exit(EXIT_FAILURE);
    }

    a0->length = blocksize;
    a1->length = blocksize;

    if (a->length > blocksize)
    {
        // fill a0 (no padding)
        memcpy(a0->numbers, &a->numbers[0], sizeof(uint32_t) * blocksize);

        // fill a1 (eventual padding)
        memcpy(a1->numbers, &a->numbers[blocksize], sizeof(uint32_t) * (a->length - blocksize));
        for (uint32_t i = a->length; i < 2 * blocksize; i++)
            a1->numbers[i - blocksize] = 0x0;
    }
    else
    {
        // fill a0 (eventual padding)
        memcpy(a0->numbers, &a->numbers[0], sizeof(uint32_t) * a->length);
        for (uint32_t i = a->length; i < blocksize; i++)
            a0->numbers[i] = 0x0;
        // fill a1 (only padding)
        memset(a1->numbers, 0, blocksize);
    }

    a0->subone = 0;
    a1->subone = 0;
}

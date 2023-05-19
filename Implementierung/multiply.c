#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <inttypes.h>
#include <string.h>
#include "operations.h"

/*
 * Copies an array x to the y starting at begin up to end(inclusive).
 * Adapts subnum accordingly.
 */
void copy(struct bignum *x, struct bignum *y, int begin, int end)
{
    y->length = 0;
    y->subone = 0;

    y->numbers = calloc(end - begin + 2, sizeof(uint32_t));
    if (y->numbers == NULL)
    {
        fprintf(stderr, "Error while allocation memory!");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i <= end - begin; i++)
    {
        y->numbers[i] = x->numbers[begin + i];
        y->length++;
    }
}

/*
 * Shifts the numbers of the given bignum to the left by n blocks
 */
void lShift32(struct bignum *x, size_t n)
{
    if (x->subone > 0)
    {
        size_t subone = x->subone;
        x->subone = subone > n ? subone - n : 0;
        n = x->subone > n ? 0 : n - subone;
    }

    x->numbers = realloc(x->numbers, (x->length + n) * sizeof(uint32_t));
    if (x->numbers == NULL)
    {
        fprintf(stderr, "Error while allocation memory!");
        exit(EXIT_FAILURE);
    }

    if (n > 0)
    {
        // Shifts blocks to the left
        for (size_t i = x->length; i > 0; i--)
        {
            x->numbers[n + i - 1] = x->numbers[i - 1];
        }
        // Sets new blocks to zero
        for (size_t i = 0; i < n; i++)
        {
            x->numbers[i] = 0;
        }
    }
    x->numbers[x->length + n - 1] != 0 ? x->length += n : 1;
}

/*
 * Aligns two number arrays to same length by 0-extending the smaller one.
 * Assures that both arrays are of an even length using 0-extension.
 */
void karazMultPreparation(struct bignum *x, struct bignum *y)
{
    // aligning to the same precision
    int32_t subone_diff = x->subone - y->subone;

    x->subone = 0;
    y->subone = 0;

    // checking if x->subone is greater that y->subone
    // the number with the smaller precision gets shifted left until there is a 'zero extention' to the right end of the other number
    if (subone_diff > 0)
    {
        lShift32(y, subone_diff);
    }
    else if (subone_diff < 0)
    {
        subone_diff *= -1;
        lShift32(x, subone_diff);
    }

    // aligning to the same length
    int32_t offset = x->length - y->length;

    // checking if x->length is greater than y->length
    if (offset > 0)
    {
        // y->numbers has to be 0-extended
        // allocating memory for the 0's
        y->numbers = (uint32_t *)realloc(y->numbers, (y->length + offset + 1) * sizeof(uint32_t));
        if (y->numbers == NULL)
        {
            fprintf(stderr, "Error while allocation memory!");
            exit(EXIT_FAILURE);
        }
        while (offset > 0)
        {
            // extending and setting length
            y->numbers[y->length] = 0x00000000;
            y->length++;
            offset--;
        }
    }
    else if (offset < 0)
    {
        // x->numbers has to be 0-extended
        // allocating memory for the 0's
        x->numbers = (uint32_t *)realloc(x->numbers, ((int32_t)x->length - offset + 1) * sizeof(uint32_t));
        if (x->numbers == NULL)
        {
            fprintf(stderr, "Error while allocation memory!");
            exit(EXIT_FAILURE);
        }
        while (offset < 0)
        {
            x->numbers[x->length] = 0x00000000;
            x->length++;
            offset++;
        }
    }

    // aligning to the even length
    if ((x->length % 2) == 1)
    {
        x->length++;

        // extending and setting length
        x->numbers[x->length - 1] = 0x00000000;
    }

    if ((y->length % 2) == 1)
    {
        y->length++;

        y->numbers[y->length - 1] = 0x00000000;
    }
}

void karazubaMult(const struct bignum *x, const struct bignum *y, struct bignum *res)
{
    // recursion base case: multiplication when both lengths are 1
    if ((x->length == 1) && (y->length == 1))
    {
        if (x->numbers[0] == 0 || y->numbers[0] == 0)
        {
            bignumInit(res, 0);
            return res;
        }

        uint64_t product = ((uint64_t)x->numbers[0]) * ((uint64_t)y->numbers[0]);
        bignumInit(res, product);
        res->subone = x->subone + y->subone;
        return;
    }

    struct bignum x_cpy, y_cpy;

    copy(x, &x_cpy, 0, x->length - 1);
    copy(y, &y_cpy, 0, y->length - 1);

    x_cpy.subone = x->subone;
    y_cpy.subone = y->subone;

    // calling function to assure, that both bignum structs have the same even length
    karazMultPreparation(&x_cpy, &y_cpy);

    struct bignum x0, x1, y0, y1;

    copy(&x_cpy, &x1, x_cpy.length / 2, x_cpy.length - 1);
    copy(&x_cpy, &x0, 0, (x_cpy.length / 2) - 1);
    copy(&y_cpy, &y1, y_cpy.length / 2, y_cpy.length - 1);
    copy(&y_cpy, &y0, 0, (y_cpy.length / 2) - 1);

    bignumFree(&x_cpy);
    bignumFree(&y_cpy);

    struct bignum mx, my;
    bignumAdd(&x1, &x0, &mx);
    bignumAdd(&y1, &y0, &my);

    // Stores the amount of blocks needed to calculate number * 2^2 by shifting number left
    size_t powerOfTwo = x1.length;
    // Stores the amount of blocks needed to calculate number * 2^2m by shifting number left
    size_t powerOfTwoMulTwo = 2 * x1.length;

    // Used to temporally store the result of the operations in order to retain the pointers of the operands so they can be freed if need be
    struct bignum temp;

    // Multiplying mx*my = (x0 + x1)*(y0*y1) and storing the result in mx
    karazubaMult(&mx, &my, &temp);
    bignumFree(&mx);
    mx = temp;

    // x0 * y0
    karazubaMult(&y0, &x0, &temp);
    bignumFree(&x0);
    x0 = temp;

    // x1 * y1
    karazubaMult(&x1, &y1, &temp);
    bignumFree(&x1);
    x1 = temp;

    // (x0 + x1)*(y0*y1) - x0*y0 - x1*y1
    bignumSubV1(&mx, &x0, &temp);
    bignumFree(&mx);
    mx = temp;

    bignumSubV1(&mx, &x1, &temp);
    ;
    bignumFree(&mx);
    mx = temp;

    // 2^m * ((x0 + x1)*(y0*y1) - x0*y0 - x1*y1)
    lShift32(&mx, powerOfTwo);
    // x1*y1 * 2^2m
    lShift32(&x1, powerOfTwoMulTwo);

    // x0*y0 + 2^m * ((x0 + x1)*(y0*y1) - x0*y0 - x1*y1)
    bignumAdd(&mx, &x0, &temp);
    bignumFree(&mx);
    mx = temp;

    // x0*y0 + 2^m * ((x0 + x1)*(y0*y1) - x0*y0 - x1*y1) + 2^2m x1*y1
    bignumAdd(&mx, &x1, &temp);
    bignumFree(&mx);
    mx = temp;

    mx.subone = x->subone > y->subone ? 2 * x->subone : 2 * y->subone;
    if (mx.subone > mx.length)
    {
        mx.numbers = realloc(mx.numbers, mx.subone * sizeof(uint32_t));
        checkAllocationError(mx.numbers);

        for (size_t i = mx.length; i < mx.subone; i++)
        {
            mx.numbers[i] = 0;
        }
        mx.length = mx.subone;
    }

    bignumFree(&x0);
    bignumFree(&x1);
    bignumFree(&y0);
    bignumFree(&y1);
    bignumFree(&my);

    return mx;
}

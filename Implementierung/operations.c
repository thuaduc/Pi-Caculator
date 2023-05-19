#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <inttypes.h>
#include <string.h>
#include "operations.h"

void bignumInit(struct bignum *num, uint64_t n)
{
    num->numbers = calloc(2, sizeof(uint32_t));
    if (num->numbers == NULL)
    {
        fprintf(stderr, "Error while allocation memory!");
        exit(EXIT_FAILURE);
    }
    num->numbers[0] = n % 0x100000000;
    num->length = 1;
    if (n > UINT32_MAX)
    {
        num->numbers[1] = n >> 32;
        num->length++;
    }
    num->subone = 0;
}

/*
 * Free bignum allocated memory
 */
void bignumFree(struct bignum *num)
{
    free(num->numbers);
}

void checkAllocationError(uint32_t *numbers)
{
    if (numbers == NULL)
    {
        fprintf(stderr, "Error while re/allocating memory!\n");
        exit(EXIT_FAILURE);
    }
}

void bignumPrint(const struct bignum *num)
{
    for (int i = num->length - 1; i >= 0; i--)
    {
        if (num->subone - 1 == (size_t)i)
        {
            printf(",");
        }
        printf("%08x ", num->numbers[i]);
    }
    printf("\n");
}

void bignumPreparation(const struct bignum *x, const struct bignum *y,
                       struct bignum *a, struct bignum *b)
{
    size_t greater_integer = x->length - x->subone > y->length - y->subone ? x->length - x->subone : y->length - y->subone;
    size_t greater_factional = x->subone > y->subone ? x->subone : y->subone;

    a->length = greater_integer + greater_factional;
    b->length = greater_integer + greater_factional;

    a->subone = greater_factional;
    b->subone = greater_factional;

    a->numbers = calloc(a->length, sizeof(uint32_t));
    b->numbers = calloc(b->length, sizeof(uint32_t));

    if (x->subone > y->subone)
    {
        memcpy(&a->numbers[0], &x->numbers[0], x->subone * sizeof(uint32_t));
        memcpy(&b->numbers[x->subone - y->subone], &y->numbers[0], y->subone * sizeof(uint32_t));
    }
    else if (y->subone > x->subone)
    {
        memcpy(&b->numbers[0], &y->numbers[0], y->subone * sizeof(uint32_t));
        memcpy(&a->numbers[y->subone - x->subone], &x->numbers[0], x->subone * sizeof(uint32_t));
    }
    else
    {
        memcpy(&a->numbers[0], &x->numbers[0], x->subone * sizeof(uint32_t));
        memcpy(&b->numbers[0], &y->numbers[0], y->subone * sizeof(uint32_t));
    }

    memcpy(&a->numbers[a->subone], &x->numbers[x->subone], greater_integer * sizeof(uint32_t));
    memcpy(&b->numbers[b->subone], &y->numbers[y->subone], greater_integer * sizeof(uint32_t));
}

void bignumAddV1(const struct bignum *x, const struct bignum *y, struct bignum *res)
{

    struct bignum a, b;
    bignumPreparation(x, y, &a, &b);

    res->subone = a.subone;
    res->length = a.length + 1;
    res->numbers = calloc(res->length, sizeof(uint32_t));

    uint64_t carry = 0;
    for (int i = 0; i < (int)a.length; i++)
    {
        uint64_t sum = (uint64_t)a.numbers[i] + (uint64_t)b.numbers[i] + carry;
        if (sum > UINT32_MAX)
        {
            res->numbers[i] = sum - UINT32_MAX - 1;
            carry = 1;
        }
        else
        {
            res->numbers[i] = a.numbers[i] + b.numbers[i] + carry;
            carry = 0;
        }
    }

    if (carry)
    {
        res->numbers[res->length - 1] = 1;
    }
    else
    {
        realloc(&res->numbers[0], (res->length - 1) * sizeof(uint32_t));
        if (res->numbers == NULL)
        {
            fprintf(stderr, "Error while reallocating memory!\n");
            return EXIT_FAILURE;
        }
        res->length--;
    }

    bignumFree(&a);
    bignumFree(&b);
}

/*
 * add two bignums and return the result
 */
void bignumAdd(const struct bignum *x, const struct bignum *y, struct bignum *result)
{
    size_t greater_length = x->length > y->length ? x->length : y->length;
    size_t greater_subone = x->subone > y->subone ? x->subone : y->subone;

    struct bignum res;

    res.numbers = calloc(greater_length + 1, sizeof(uint32_t));
    if (res.numbers == NULL)
    {
        fprintf(stderr, "Error while allocation memory!");
        exit(EXIT_FAILURE);
    }
    res.length = 0;

    uint64_t sum;
    uint32_t carry = 0;
    size_t offset = 0;
    size_t x_index = 0;
    size_t y_index = 0;

    if (x->subone > y->subone)
    {
        size_t deff = x->subone - y->subone;
        memcpy(&res.numbers[0], &x->numbers[0], deff * 4);
        /*
        for(int i =0 ; i < deff; i++){
            res.numbers[i] = x->numbers[x_index++];
        }
        */
        res.length += deff;
        x_index = deff;
    }
    else if (x->subone < y->subone)
    {
        size_t deff = y->subone - x->subone;
        memcpy(&res.numbers[0], &y->numbers[0], deff * 4);
        /*
        for(int i =0 ; i < deff; i++){
            res.numbers[i] = y->numbers[y_index++];
        }
        */
        res.length += deff;
        y_index = deff;
    }

    for (size_t i = 0; x_index < x->length && y_index < y->length; i++)
    {
        if (x_index >= x->length)
        {
            sum = ((uint64_t)y->numbers[y_index]) + carry;
            y_index++;
        }
        else if (y_index >= y->length)
        {
            sum = ((uint64_t)x->numbers[x_index]) + carry;
            x_index++;
        }
        else
        {
            sum = ((uint64_t)x->numbers[x_index] + (uint64_t)y->numbers[y_index]) + carry;
            x_index++;
            y_index++;
        }
        carry = sum > UINT32_MAX ? 1 : 0;
        if (i == 0 && x_index == y_index && i < greater_subone && sum % 0x100000000 == 0)
        {
            offset++;
            continue;
        }
        res.numbers[i - offset] = sum % 0x100000000;
        res.length++;
    }

    if (carry)
    {
        res.numbers[res.length] = 1;
        res.length++;
    }

    // The number of sub one blocks is equal to that of the addend with more sub one places minus the blocks that are omitted
    res.subone = greater_subone - offset;

    return res;
}

/*
 * reduce the bignum size to desired precision in blocks
 */
void reduceSize(struct bignum *num, size_t prec)
{
    if (num->subone > prec)
    {
        size_t offset = num->subone - prec;

        for (size_t i = 0; i < num->length - offset; i++)
        {
            num->numbers[i] = num->numbers[i + offset];
        }

        num->length -= offset;
        num->subone -= offset;
    }
}

/*
 * normalizes the given number to a number between [0.5,1] to be used in Newton-Raphson-formula, ommits unused subone blocks, stores the result in destenation
 * this methos can only be applied on integer numbers. if it's not the case, the program terminates.
 */
size_t normalize(const struct bignum *x, struct bignum *dest)
{
    if (x->subone > 0)
    {
        fprintf(stderr, "DEBUG: this method accepts integer values only!\n");
        exit(EXIT_FAILURE);
    }

    // convert all blocks to subone blocks, applying left shifts until the most significant subone bit is set to get a number between [0.5,1]
    // n represents the number of left shifts
    size_t n = 0;
    size_t last_block = x->length;
    if (x->length != 1 || x->numbers[0] != 0)
    {
        while (x->numbers[last_block - 1] == 0 && last_block != 1)
        {
            last_block -= 1;
        }
        while ((x->numbers[last_block - 1] << n & 0x80000000) == 0)
        {
            n++;
        }

        dest->numbers = calloc(last_block, sizeof(uint32_t));
        if (dest->numbers == NULL)
        {
            fprintf(stderr, "Error while allocation memory!");
            exit(EXIT_FAILURE);
        }

        // overflow represents the bits lost by shifting left, overflow will be added to the next block to restore the value
        uint32_t overflow = 0;

        uint32_t val;

        // Offset represents the number of unnecessary blocks to be removed
        size_t offset = 0;

        // if n=0 then no shifts required, so the blocks will be copied directly to the destenation
        if (n == 0)
        {
            memcpy(&dest->numbers[0], &x->numbers[0], last_block);
        }
        else
        {
            for (size_t i = 0; i < last_block; i++)
            {
                uint32_t temp = x->numbers[i] >> (32 - n);
                val = x->numbers[i] << n;
                val += overflow;
                overflow = temp;

                // remove unnessesary blocks
                if (val == 0 && i == offset)
                {
                    offset++;
                }
                else
                {
                    dest->numbers[i - offset] = val;
                }
            }
        }

        dest->length = last_block - offset;
    }

    // the total right shifts needed will be stored in n.
    n = (last_block - x->subone) * 32 - n;
    dest->subone = dest->length;
    return n;
}

/**
 * @brief print bignum number Array with decimal seperator
 *
 * @param bignum
 */
void printArr(struct bignum *num)
{
    printf("Array [");
    for (size_t i = num->length - 1; i > 0; i--)
    {
        printf("%u ", num->numbers[i]);
        if (num->subone == i)
            printf(",");
    }
    printf("%u]\n", num->numbers[0]);
}

/*
 * Print bignum number in decimal form
 */
void bignumPrintDec(const struct bignum *x, size_t totalDigits)
{
    struct bignum result;
    result.numbers = malloc(x->length * sizeof(uint32_t));
    memcpy(result.numbers, x->numbers, x->length * sizeof(uint32_t));
    result.length = x->length;
    result.subone = x->subone;
    size_t looptimes = totalDigits - 1;

    while (looptimes > 0)
    {
        struct bignum a;
        struct bignum b;
        a = lShift(&result, 3);
        b = lShift(&result, 1);
        bignumFree(&result);
        result = bignumAdd(&a, &b);
        bignumFree(&a);
        bignumFree(&b);
        looptimes--;
    }

    printf("result after times 10. %d\n", result.subone);
    bignumPrint(&result);

    size_t len = result.length;
    size_t bufcounter = 0;

    uint8_t *buffer = (uint8_t *)calloc(totalDigits, sizeof(uint8_t));
    uint32_t *num = (uint32_t *)calloc(len, sizeof(uint32_t));
    uint32_t *num2 = (uint32_t *)calloc(len, sizeof(uint32_t));
    bignumCopy(&result, num);

    while (1)
    {
        binaryConverter(buffer, num, num2, bufcounter++, len);
        if (bufcounter == totalDigits)
            break;

        binaryConverter(buffer, num2, num, bufcounter++, len);
        if (bufcounter == totalDigits)
            break;
    }

    for (int i = totalDigits - 1; i >= 0; i--)
    {
        printf("%" PRIu8 "", buffer[i]);
    }
    printf("\n");

    free(buffer);
    free(num);
    free(num2);
}

void bignumPrintHex(const struct bignum *num, size_t totalDigits)
{
    size_t all_blocks = totalDigits / 8;
    totalDigits %= 8;
    bool comma_set = false;

    if (all_blocks > num->subone || (all_blocks == num->subone && totalDigits != 0))
    {
        fprintf(stderr, "DEBUG: printResult cannot print more precise than the number actually is!");
        exit(EXIT_FAILURE);
    }

    for (int i = num->length - 1; i >= (int)num->subone - (int)all_blocks; i--)
    {
        // Printing the highest significant places without leading zeroes, if they are not subone
        if ((size_t)i == num->length - 1 && num->subone < num->length)
        {
            printf("%x", num->numbers[num->length - 1]);
        }
        else
        {
            if (num->subone - 1 == (size_t)i)
            {
                printf(",");
                comma_set = true;
            }
            printf("%08x", num->numbers[i]);
        }
    }

    if (totalDigits != 0)
    {
        if (!comma_set)
        {
            printf(",");
        }
        printf("%x", num->numbers[num->subone - all_blocks - 1] >> (32 - totalDigits * 4));
    }
    printf("\n");
}

/*
 * Shift bignum n bits to the right and return result, method for newton raphson division
 */
void rShift(const struct bignum *num, size_t n, struct bignum *res)
{
    size_t shifted_blocks = n / 32;
    size_t shifted_bits = n % 32;

    struct bignum res;
    res.length = num->length + 1;
    res.subone = num->subone - shifted_blocks;
    res.numbers = (uint32_t *)calloc(res.length, sizeof(uint32_t));

    binaryShiftRight(res.numbers, num->numbers, num->length, shifted_bits);

    if (res.numbers[res.length - 1] == 0)
    {
        realloc(res.numbers, (res.length - 1) * sizeof(uint32_t));
        res.length--;
    }
    return res;
}

/*
 * Shift bignum n bits to the left and return result, method for newton raphson division
 */
void lShift(const struct bignum *num, size_t n, struct bignum *res)
{
    size_t shifted_blocks = n / 32;
    size_t shifted_bits = n % 32;

    struct bignum res;
    res.length = num->length + 1;
    res.subone = num->subone + shifted_blocks;
    res.numbers = (uint32_t *)calloc(res.length, sizeof(uint32_t));

    binaryShiftLeft(res->numbers, num->numbers, num->length, shifted_bits);

    if (res->numbers[res->length - 1] == 0)
    {
        realloc(res->numbers, (res->length - 1) * sizeof(uint32_t));
        res->length--;
    }
}

// test A greater B
/**
 * @brief compares two bignums
 *
 * @param A
 * @param B
 * @return uint8_t
 * 1 (true) if A > B
 * 0 (false) if A < B
 * 2 if equal
 *
 */
uint8_t bignumCompare(const struct bignum *A, const struct bignum *B)
{
    size_t wh_la = A->length - A->subone;
    size_t wh_lb = B->length - B->subone;
    if (wh_la > wh_lb)
    {
        return true;
    }
    else if (wh_la < wh_lb)
        return false;
    else
    {
        for (size_t i = 1; i <= min(A->length, B->length); i++)
        {
            if (A->numbers[A->length - i] > B->numbers[B->length - i])
                return true;
            else if (A->numbers[A->length - i] < B->numbers[B->length - i])
                return false;
        }
        return 2; // if equal
    }
}

/*
 * return the bigger uint32
 */
uint32_t max(uint32_t a, uint32_t b)
{
    return a > b ? a : b;
}

/*
 * return the smaller uint32
 */
uint32_t min(uint32_t a, uint32_t b)
{
    return a < b ? a : b;
}

void bignumcopy(struct bignum *src, struct bignum *res)
{
    res->length = src->length;
    res->subone = src->subone;
    res->numbers = malloc(res->length * sizeof(uint32_t));
    memcpy(&res->numbers[0], &src->numbers[0], res->length * sizeof(uint32_t));
}

struct bignum newtonDiv(const struct bignum *T, const struct bignum *M, size_t prec)
{
    // Number of considered blocks needed to assure the required precision
    size_t cons_blocks = prec / 32;
    if (prec % 32 != 0)
    {
        cons_blocks++;
    }

    // Reduces the denominator to be between 0.5 and 1 then right shifts the Numerator by the same amount needed for reduce
    struct bignum T_reduced;
    struct bignum M_reduced;

    size_t n = normalize(M, &M_reduced);
    rShift(T, n, &T_reduced);

    reduceSize(&T_reduced, cons_blocks * 2);
    reduceSize(&M_reduced, cons_blocks * 2);

    printf("T reduced: ");
    bignumPrint(&T_reduced);
    printf("M reduced: ");
    bignumPrint(&M_reduced);

    // magic0 is an aproximation for 48/17, magic1 for 32/17
    struct bignum magic0;
    magic0.length = M_reduced.subone + 1;
    magic0.subone = magic0.length - 1;

    struct bignum magic1;
    magic1.length = M_reduced.subone + 1;
    magic1.subone = magic1.length - 1;

    magic0.numbers = calloc(magic0.length, sizeof(uint32_t));
    magic1.numbers = calloc(magic1.length, sizeof(uint32_t));

    if (magic0.numbers == NULL || magic1.numbers == NULL)
    {
        fprintf(stderr, "Error while allocation memory!");
        exit(EXIT_FAILURE);
    }

    // Fills the magic up to the same precision as D_reduced
    for (size_t i = 0; i < magic0.length - 1; i++)
    {
        magic0.numbers[i] = 0xd2d2d2d2;
        magic1.numbers[i] = 0xe1e1e1e1;
    }
    magic0.numbers[magic0.length - 1] = 0x2;
    magic1.numbers[magic1.length - 1] = 0x1;

    // Stores the aproximation of the reciprocal
    struct bignum x;

    // Initial value for x wich is an aproximation of 48/17 - 32/17 * D_reduced
    x = karazubaMult(&M_reduced, &magic1);
    //
    // reduceSize(&x, cons_blocks * 2);
    //
    bignumFree(&magic1);

    bignumSubV1(&magic0, &x, &magic1);
    bignumFree(&x);
    bignumFree(&magic0);

    x = magic1;

    // Required steps to achieve the required precision
    int steps = ceil(log((prec + 1) / (log(17) / log(2))) / log(2));
    printf("steps: %d\n", steps);

    struct bignum two;
    bignumInit(&two, 2);

    // x = x * (2 - x * D_reduced)
    for (int i = 0; i < steps; i++)
    {
        printf("x: ");
        bignumPrint(&x);
        struct bignum temp0;
        temp0 = karazubaMult(&M_reduced, &x);
        //
        reduceSize(&temp0, cons_blocks * 2);
        //

        printf("x * m_reduce: ");
        bignumPrint(&temp0);

        struct bignum temp1;
        bignumSubV1(&two, &temp0, &temp1);

        printf("\n");
        printf("2 - x * m_reduce: ");
        bignumPrint(&temp1);
        bignumFree(&temp0);

        temp0 = karazubaMult(&x, &temp1);
        //
        reduceSize(&temp0, cons_blocks * 2);
        //

        bignumFree(&temp1);
        bignumFree(&x);

        x = temp0;

        /* printf("x: ");
        bignumPrint(&x);*/

        printf("----------------------------------\n");
    }
    bignumFree(&two);

    // The actual quotient is then computed with N * x
    printf("x last mul: ");
    bignumPrint(&x);
    printf("T_reduced: ");
    bignumPrint(&T_reduced);
    struct bignum res = karazubaMult(&T_reduced, &x);
    // bignumPrint( &res );
    bignumFree(&x);
    bignumFree(&T_reduced);
    bignumFree(&M_reduced);
    return res;
}
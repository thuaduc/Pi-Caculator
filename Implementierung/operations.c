#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <inttypes.h>
#include <string.h>
#include "operations.h"

#define MAX32_MASK 0x100000000

/*
 * Initializes the given bignum with the value n;
 * allocates an additional 32 bit block for further operations
 */
void bignumInit(struct bignum *num, uint64_t n)
{
    num->numbers = calloc(2, sizeof(uint32_t));
    checkAllocationError(num->numbers);
    num->numbers[0] = n % MAX32_MASK;
    num->length = 1;
    if (n > UINT32_MAX)
    {
        num->numbers[1] = n >> 32;
        num->length++;
    }
    num->subone = 0;
}

void checkAllocationError(uint32_t *numbers)
{
    if (numbers == NULL)
    {
        fprintf(stderr, "Error while re/allocating memory!\n");
        exit(EXIT_FAILURE);
    }
}

/*
 * Unsigned subtraction x = x - y
 */
int bignum_uSub(struct bignum *x, const struct bignum *y)
{

    long int offset;
    offset = x->subone - y->subone;
    // differenciate if minuend or subtrahend has a longer fractional part
    bool minuend_sub_longer = offset >= 0;

    if (!minuend_sub_longer)
    {
        // Make the array bigger -> length of x +
        size_t nl = (x->length - offset);
        uint32_t *more_numbers = malloc(nl * sizeof(uint32_t));
        checkAllocationError(more_numbers);
        memcpy(&more_numbers[-offset], &x->numbers[0], sizeof(uint32_t) * x->length);
        free(x->numbers);
        x->numbers = more_numbers;
        x->length = nl;
        x->subone -= offset;
    }

    bool carry = false;
    size_t i = 0;

    if (minuend_sub_longer == 0)
    { // subtrahend has more decimals after point --> 0-y
        (x->numbers)[0] = MAX32_MASK - (y->numbers)[0];
        carry = 1;
        for (i = 1; i < (long unsigned int)(-offset); i++)
        {
            (x->numbers)[i] = MAX32_MASK - (y->numbers)[i] - carry;
        }
        offset = 0;
    }

    while (i < y->length)
    {
        if ((x->numbers)[i + offset] >= (y->numbers)[i] + carry)
        {
            (x->numbers)[i + offset] = ((x->numbers)[i + offset] - (y->numbers)[i] - carry);
            carry = 0;
        }
        else
        {
            (x->numbers)[i + offset] = MAX32_MASK + (x->numbers)[i + offset] - (y->numbers)[i] - carry;
            carry = 1;
        }
        i++;
    }

    while (i < x->length && carry)
    {
        if ((x->numbers)[i] > 0)
        {
            (x->numbers)[i]--;
            carry = 0;
        }
        else
        {
            (x->numbers)[i] = MAX32_MASK - 1;
        }
        i++;
    }
    if (carry)
    {
        // error subtraction should be unsigned
        fprintf(stderr, "Subtraction should be unsigned.\n");
        return EXIT_FAILURE;
    }
    if (!minuend_sub_longer)
        x->subone = y->subone;

    while (x->numbers[x->length - 1] == 0 && x->length > 1 && x->length >= x->subone)
        x->length--;

    return EXIT_SUCCESS;
}

/*
 * Signed subtraction of two bignums. returns absolute value
 */
void bignum_signedSub(const struct bignum *x, const struct bignum *y, struct bignum *res)
{
    // x bigger
    if (bignumCompare(x, y))
    {
        bignumCopy(x, res); // create copy of x for result
        if (bignum_uSub(res, y))
        {
            printf("sub x - y:");
            exit(EXIT_FAILURE);
        }
    }
    else // y bigger
    {
        bignumCopy(y, res); // create copy of x for result
        if (bignum_uSub(res, x))
        {
            printf("sub y - x:");
            exit(EXIT_FAILURE);
        }
    }
}

/*
 * Free bignum allocated memory
 */
void bignumFree(struct bignum *num)
{
    if (num == NULL)
    {
        fprintf(stderr, "Allocation already be freed or not exits.\n");
        exit(EXIT_FAILURE);
    }
    free(num->numbers);
}

/*
 *  Prints bignum, meant for debugging
 */
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

/*
 * add two bignums and return the result
 */
void bignumAdd(const struct bignum *x, const struct bignum *y, struct bignum *result)
{
    size_t greater_length = x->length > y->length ? x->length : y->length;
    size_t greater_subone = x->subone > y->subone ? x->subone : y->subone;

    // allocate memory for result with the greater_length + one additional block for potential carry
    result->numbers = calloc(greater_length + 1, sizeof(uint32_t));
    checkAllocationError(result->numbers);
    result->length = 0;

    // the result of addition will be stored in this variable, if there's an overflow, the rest will be saved in the highst 32 bits of sum
    uint64_t sum;

    // indicates if the carry flag is set. 1-> true 0 -> false
    uint32_t carry = 0;
    size_t offset = 0;
    size_t x_index = 0;
    size_t y_index = 0;

    // if x->subone has more sub one blocks than y->subone, then copy these blocks to the result using memcpy
    if (x->subone > y->subone)
    {
        size_t deff = x->subone - y->subone;
        memcpy(&result->numbers[0], &x->numbers[0], deff * 4);
        result->length += deff;
        x_index = deff;
    }
    else if (x->subone < y->subone) // if y->subone has more sub one blocks than x->subone, then copy these blocks to the result using memcpy
    {
        size_t deff = y->subone - x->subone;
        memcpy(&result->numbers[0], &y->numbers[0], deff * 4);
        result->length += deff;
        y_index = deff;
    }
    size_t i;
    // The addition loop
    for (i = 0; x_index < x->length || y_index < y->length; i++)
    {
        // if x finished before y, then copy the rest of y in sum
        if (x_index >= x->length)
        {
            sum = ((uint64_t)y->numbers[y_index]) + carry;
            y_index++;
        } // if y finished before x, then copy the rest of y in sum
        else if (y_index >= y->length)
        {
            sum = ((uint64_t)x->numbers[x_index]) + carry;
            x_index++;
        }
        else
        { // add x + y + carry to sum
            sum = ((uint64_t)x->numbers[x_index] + (uint64_t)y->numbers[y_index]) + carry;
            x_index++;
            y_index++;
        }

        carry = sum > UINT32_MAX ? 1 : 0;

        //  If the sum of two sub one blocks is zero and the result block was the first in res, the value of the new block gets omitted
        if (i - offset == 0 && i < greater_subone && sum % MAX32_MASK == 0)
        {
            offset++;
            continue;
        }
        result->numbers[i - offset] = sum % MAX32_MASK;
        result->length++;
    }

    if (carry)
    {
        result->numbers[result->length] = 1;
        result->length++;
    }

    // removing the number of blocks ,ommited during the addition, from the total subone blocks
    result->subone = greater_subone - offset;
}

/*
 * Reduce the bignum size to desired precision in blocks
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
 * Check if bignum is integer
 * return 1 if true, 0 if false
 */
int checkInteger(struct bignum *result)
{
    if (result->subone == 0)
    {
        return 1;
    }
    else
    {
        for (size_t i = 0; i < result->subone; i++)
        {
            if (result->numbers[i] != 0)
            {
                return 0;
            }
        }
    }
    return 1;
}

/*
 * Convert bignum number in decimal form
 */
size_t bignumConvDec(const struct bignum *x, uint8_t **bufferRes)
{
    // result store fractional part of pi
    struct bignum result;
    result.numbers = malloc((x->length - 1) * sizeof(uint32_t));
    checkAllocationError(result.numbers);
    result.length = x->length - 1;
    result.subone = x->subone;

    memcpy(result.numbers, x->numbers, result.length * sizeof(uint32_t));

    // since convert algo only work with integer, multiply pi by ten till result is integer
    while (checkInteger(&result) == 0)
    {
        struct bignum a;
        struct bignum b;
        lShift(&result, 3, &a);
        lShift(&result, 1, &b);
        bignumFree(&result);
        bignumAdd(&a, &b, &result);
        bignumFree(&a);
        bignumFree(&b);
    }

    size_t len = result.length;
    size_t bufcounter = 0;
    size_t bufferLength = 10 * result.length;

    // buffer save result digits by digits
    uint8_t *buffer = (uint8_t *)calloc(bufferLength, sizeof(uint8_t));
    uint32_t *num = (uint32_t *)calloc(len, sizeof(uint32_t));
    uint32_t *num2 = (uint32_t *)calloc(len, sizeof(uint32_t));
    if (buffer == NULL)
    {
        fprintf(stderr, "Error while re/allocating memory!\n");
        exit(EXIT_FAILURE);
    }
    checkAllocationError(num);
    checkAllocationError(num2);
    memcpy(num, result.numbers, result.length * sizeof(uint32_t));

    while (1)
    {
        // convert hex to dec by compute divide by ten method
        binaryConverter(buffer, num, num2, bufcounter++, len);
        if (bufcounter == bufferLength)
            break;

        binaryConverter(buffer, num2, num, bufcounter++, len);
        if (bufcounter == bufferLength)
            break;
    }

    // find position where there is no leading 0
    for (int i = (int)bufferLength - 1; i >= 0; i--)
    {
        if (buffer[i])
        {
            bufferLength = i;
            break;
        }
    }

    *bufferRes = buffer;
    free(num);
    free(num2);
    bignumFree(&result);
    return bufferLength;
}

/*
 * Print bignum in decimal form
 */
void bignumPrintDec(const struct bignum *x, size_t totalDigits)
{
    uint8_t *buffer;
    size_t bufferLength = bignumConvDec(x, &buffer);

    printf("%" PRId32 ",", x->numbers[x->length - 1]);
    for (size_t i = bufferLength; i > bufferLength - totalDigits; i--)
    {
        printf("%" PRIu8 "", buffer[i]);
    }
    printf("\n");

    free(buffer);
}

/*
 * Print bignum number in hexadecimal form
 */
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
            printf("%X", num->numbers[num->length - 1]);
        }
        else
        {
            if (num->subone - 1 == (size_t)i)
            {
                printf(",");
                comma_set = true;
            }
            printf("%08X", num->numbers[i]);
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

    res->length = num->length + 1;
    // shift block by increasing subone
    res->subone = num->subone + shifted_blocks + 1;
    res->numbers = (uint32_t *)calloc(res->length, sizeof(uint32_t));
    checkAllocationError(res->numbers);
    binaryShiftRight(res->numbers, num->numbers, num->length, shifted_bits);

    if (res->numbers[res->length - 1] == 0)
    {
        if (res->subone == res->length)
        {
            res->subone--;
        }
        res->length--;
    }
}

/*
 * Shift bignum n bits to the left and return result, method for newton raphson division
 * work for n < 32
 */
void lShift(const struct bignum *num, size_t n, struct bignum *res)
{
    res->length = num->length + 1;
    // shift block by decreasing subone
    res->subone = num->subone;
    res->numbers = (uint32_t *)calloc(res->length, sizeof(uint32_t));
    checkAllocationError(res->numbers);
    binaryShiftLeft(res->numbers, num->numbers, num->length, n);

    if (res->numbers[res->length - 1] == 0)
    {
        res->length--;
    }
}

/*
 * Returns length of bignum without leading zeros
 */
size_t actualLength(const struct bignum *A)
{
    size_t res = A->length;
    while ((res > 1) && (A->numbers[res - 1] == 0) && res > A->subone)
    {
        res--;
    }
    return res;
}

/*
 * Compares two bignums
 * 1 (true) if A > B
 * 0 (false) if A < B
 * 2 if equal
 */
uint8_t bignumCompare(const struct bignum *A, const struct bignum *B)
{
    size_t A_len = actualLength(A);
    size_t B_len = actualLength(B);
    if (A_len - A->subone > B_len - B->subone)
    {
        return true;
    }
    else if (A_len - A->subone < B_len - B->subone)
        return false;
    else
    {
        for (size_t i = 1; i <= min(A_len, B_len); i++)
        {
            if (A->numbers[A_len - i] > B->numbers[B_len - i])
                return true;
            else if (A->numbers[A_len - i] < B->numbers[B_len - i])
                return false;
        }
        return 2; // if equal
    }
}

/*
 * return the smaller uint32
 */
uint32_t min(uint32_t a, uint32_t b)
{
    return a < b ? a : b;
}

/*
 * Creates a copy of a bignum
 */
void bignumCopy(const struct bignum *src, struct bignum *res)
{
    res->length = src->length;
    res->subone = src->subone;
    uint32_t *newNumbers = (uint32_t *)malloc(res->length * sizeof(uint32_t));
    checkAllocationError(newNumbers);
    memcpy(&newNumbers[0], &(src->numbers[0]), res->length * sizeof(uint32_t));

    // first saved in newNumbers Variable, to avoid memory problems
    res->numbers = newNumbers;
}

/*
 * Reduce bignum to range 0.5 - 1
 * Return number of bit needed to shift
 */
int reduceToOne(const struct bignum *src, struct bignum *res)
{
    int count = 0;
    for (int i = (int)src->length - 1; i >= 0; i--)
    {
        if (src->numbers[i] == 0)
        {
            continue;
        }
        else
        {
            uint32_t k = src->numbers[i];
            while (k != 0)
            {
                k = k >> 1;
                count++;
            }
            rShift(src, count, res);
            res->subone += i;
            count += i * 32;
            break;
        }
    }
    return count;
}

/*
 * Newton Division method
 * Compute T/M and return result
 */
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

    size_t n = (size_t)reduceToOne(M, &M_reduced);
    rShift(T, n, &T_reduced);

    reduceSize(&T_reduced, cons_blocks * 2);
    reduceSize(&M_reduced, cons_blocks * 2);

    // magic0 is an aproximation for 48/17, magic1 for 32/17
    struct bignum magic0;
    magic0.length = M_reduced.subone + 1;
    magic0.subone = magic0.length - 1;

    struct bignum magic1;
    magic1.length = M_reduced.subone + 1;
    magic1.subone = magic1.length - 1;

    magic0.numbers = calloc(magic0.length, sizeof(uint32_t));
    magic1.numbers = calloc(magic1.length, sizeof(uint32_t));

    checkAllocationError(magic0.numbers);
    checkAllocationError(magic1.numbers);

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
    karazMult(&M_reduced, &magic1, &x);
    bignumFree(&magic1);

    bignum_signedSub(&magic0, &x, &magic1);
    bignumFree(&x);
    bignumFree(&magic0);

    x = magic1;

    // Required steps to achieve the required precision
    int steps = ceil(log((prec + 1) / (log(17) / log(2))) / log(2));

    struct bignum two;
    bignumInit(&two, 2);

    for (int i = 0; i < steps; i++)
    {
        struct bignum temp0;
        karazMult(&M_reduced, &x, &temp0);
        reduceSize(&temp0, cons_blocks);

        struct bignum temp1;
        bignum_signedSub(&two, &temp0, &temp1);
        bignumFree(&temp0);

        karazMult(&x, &temp1, &temp0);
        reduceSize(&temp0, cons_blocks);

        bignumFree(&temp1);
        bignumFree(&x);

        x = temp0;
    }
    bignumFree(&two);

    struct bignum res;
    karazMult(&T_reduced, &x, &res);
    reduceSize(&res, cons_blocks);
    bignumFree(&x);
    bignumFree(&T_reduced);
    bignumFree(&M_reduced);
    return res;
}

/*
 * This functions applays the Bailey–Borwein–Plouffe BBF formula by calculating 4S(1,n)−2S(4,n)−S(5,n)−S(6,n)
 * Source: https://www.experimentalmath.info/bbp-codes/bbp-alg.pdf 
 */
void apply(int id, int last_itr, int rest)
{
    double pid, s1, s2, s3, s4;
    char chx[16];

    /*  id is the digit position.  Digits generated follow immediately
    after id. */

    s1 = sum(1, id);
    s2 = sum(4, id);
    s3 = sum(5, id);
    s4 = sum(6, id);
    pid = 4. * s1 - 2. * s2 - s3 - s4;
    pid = pid - (int)pid + 1.;
    tohex(pid, 16, chx);
    if (last_itr)
    {
        if (rest)
        {
            printf("%1.1s", chx);
        }
        else
        {
            printf("%5.5s", chx);
        }
    }
}

/*  
 * This returns, in chx, the first nhx hex digits of the fraction of x. 
 */
void tohex(double x, int nhx, char chx[])
{
    int i;
    double y;
    char hx[] = "0123456789ABCDEF";

    y = fabs(x);

    for (i = 0; i < nhx; i++)
    {
        y = 16. * (y - floor(y));
        chx[i] = hx[(int)y];
    }
}

/*  
 * This routine evaluates the series  sum_k 16^(id-k)/(8*k+m)
 * using the modular exponentiation technique. 
 */
double sum(int m, int id)
{
    int k;
    double ak, p, s, t;
    double eps = 1e-17;
    s = 0.;

    /*  Sum the series up to id. */
    for (k = 0; k < id; k++)
    {
        ak = 8 * k + m;
        p = id - k;
        t = power(p, ak);
        s = s + t / ak;
        s = s - (int)s;
    }

    /*  Compute a few terms where k >= id. */
    for (k = id; k <= id + 100; k++)
    {
        ak = 8 * k + m;
        t = pow(16., (double)(id - k)) / ak;
        if (t < eps)
            break;
        s = s + t;
        s = s - (int)s;
    }
    return s;
}

/*  
 * power = 16^p mod ak.  This routine uses the left-to-right binary
 * exponentiation scheme. 
 */
double power(double p, double ak)
{
    int i, j;
    double p1, pt, r;
    static double tp[25];
    static int tp1 = 0;

    // If this is the first call to power, fill the power of two table tp.
    if (tp1 == 0)
    {
        tp1 = 1;
        tp[0] = 1.;

        for (i = 1; i < 25; i++)
            tp[i] = 2. * tp[i - 1];
    }

    if (ak == 1.)
        return 0.;

    //  Find the greatest power of two less than or equal to p.
    for (i = 0; i < 25; i++)
        if (tp[i] > p)
            break;

    pt = tp[i - 1];
    p1 = p;
    r = 1.;

    // Perform binary exponentiation algorithm modulo ak.
    for (j = 1; j <= i; j++)
    {
        if (p1 >= pt)
        {
            r = 16. * r;
            r = r - (int)(r / ak) * ak;
            p1 = p1 - pt;
        }
        pt = 0.5 * pt;
        if (pt >= 1.)
        {
            r = r * r;
            r = r - (int)(r / ak) * ak;
        }
    }

    return r;
}

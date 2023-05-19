#ifndef OPERATIONS_H
#define OPERATIONS_H

#include <stddef.h>
#include <stdint.h>

/*
 * Representation of big numbers used in computations, subone stores the amount of blocks used to represent subone places
 * Numbers are stored in little-endian format
 */
struct bignum
{
    uint32_t *numbers;
    size_t length;
    size_t subone;
};

/*
 * Initializes the given bignum with the value n;
 * allocates an additional 32 bit block for further operations
 */
void bignumInit(struct bignum *num, uint64_t n);

/*
 * Free bignum allocated memory
 */
void bignumFree(struct bignum *num);

/*
 * Print bignum block by block
 * Method for debug purpose
 */
void bignumPrint(const struct bignum *num);

/*
 * Print bingum in hexadecimal form
 */
void bignumPrintHex(const struct bignum *num, size_t totalDigits);

/*
 * Print bignum number in decimal form
 */
void bignumPrintDec(const struct bignum *num, size_t totalDigits);

/*
 * copy whole bignum
 */
void bignumCopy(struct bignum *src, struct bignum *res);

/*
 * Cut bignum fractional partto desired size
 * Method for optimizing newton division
 */
void reduceSize(struct bignum *num, size_t prec);

/*
 * convert binary to decimal by "devide and conquer" algorithms
 * result is saved digit by digit in buffer
 */
void binaryConverter(uint8_t *buffer, uint32_t *num, uint32_t *num2, int bufferIndex, int len);

/*
 * Helping method to shift bits to the right 
 */
void binaryShiftRight(uint32_t *dst, const uint32_t *src, size_t length, size_t bits);

/*
 * Helping method to shift bits to the left 
 */
void binaryShiftLeft(uint32_t *dst, const uint32_t *src, size_t length, size_t bits);

/*
 * Reduced bignum to value between 0.5 and 1
 * Method for newton div
 */
int reduceToOne(const struct bignum *x, struct bignum *dest);

/*
 * Shift bignum n bits to the right and return result
 * Method for newton raphson division
 */
void rShift(const struct bignum *num, size_t n, struct bignum *res);

/*
 * Shift bignum n bits to the left and return result
 * Method for newton raphson division
 */
void lShift(const struct bignum *num, size_t n, struct bignum *res);

/*
 * check after calling malloc, calloc, realloc
 */
void checkAllocationError(uint32_t *numbers);

void bignumSubV1(const struct bignum *x, const struct bignum *y, struct bignum *res);

struct bignum bignumAdd(struct bignum *x, struct bignum *y);

/*
 * add two bignums and return the result
 */
void bignumAdd(const struct bignum *x, const struct bignum *y, struct bignum *result);

struct bignum newtonDiv(struct bignum *x, struct bignum *y, size_t prec);

void karazubaMult(const struct bignum *x, const struct bignum *y, struct bignum *res);

void newtonDiv(const struct bignum *T, const struct bignum *M, size_t prec, struct bignum *res); 

void bignum_uSub(struct bignum *x, const struct bignum *y);

/*
 * compares two bignums
 * 1 (true) if A > B
 * 0 (false) if A < B
 * 2 if equal
 */
uint8_t bignumCompare(const struct bignum *A, const struct bignum *B);

/*
 * return the smaller uint32
 */
uint32_t min(uint32_t a, uint32_t b);

/*
 * return the bigger uint32
 */
uint32_t max(uint32_t a, uint32_t b);

#endif

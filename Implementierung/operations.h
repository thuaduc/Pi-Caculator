#ifndef OPERATIONS_H
#define OPERATIONS_H

#include <stddef.h>
#include <stdint.h>

/*
 * Representation of big numbers used in computations, subone stores the amount of blocks used to represent the fractional part, length the overall amout of base 32 digits;
 * Numbers are stored in little-endian format;
 * 0,xxx is stored as ,xxx without the leading zero;
 */
struct bignum
{
    uint32_t *numbers;
    size_t length;
    size_t subone;
};

void bignumInit(struct bignum *num, uint64_t n);

void bignumFree(struct bignum *num);

void checkAllocationError(uint32_t *numbers);

uint8_t bignumCompare(const struct bignum *A, const struct bignum *B);

void bignumCopy(const struct bignum *src, struct bignum *res);

void bignumPrint(const struct bignum *num);

void bignumPrintHex(const struct bignum *num, size_t totalDigits);

void bignumPrintDec(const struct bignum *num, size_t totalDigits);

size_t bignumConvDec(const struct bignum *x, uint8_t **bufferRes);

void binaryConverter(uint8_t *buffer, uint32_t *num, uint32_t *num2, int bufferIndex, int len);

void binaryShiftRight(uint32_t *dst, const uint32_t *src, size_t length, size_t bits);

void binaryShiftLeft(uint32_t *dst, const uint32_t *src, size_t length, size_t bits);

void rShift(const struct bignum *num, size_t n, struct bignum *res);

void lShift(const struct bignum *num, size_t n, struct bignum *res);

// arithmatic bignum operations
void bignum_signedSub(const struct bignum *x, const struct bignum *y, struct bignum *res);

int bignum_uSub(struct bignum *x, const struct bignum *y);

void bignumAdd(const struct bignum *x, const struct bignum *y, struct bignum *result);

void karazMult(const struct bignum *x, const struct bignum *y, struct bignum *res);

struct bignum newtonDiv(const struct bignum *T, const struct bignum *M, size_t prec);

uint32_t min(uint32_t a, uint32_t b);

void apply(int id, int last_itr, int rest);

void tohex (double x, int nhx, char chx[]);

double sum (int m, int id);

double power (double p, double ak);

#endif

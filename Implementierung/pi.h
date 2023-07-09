#ifndef PI_H
#define PI_H

#include <stddef.h>
#include <stdint.h>

struct bignum p(size_t n);

struct bignum P(size_t n1, size_t n2);

struct bignum q(size_t n);

struct bignum Q(size_t n1, size_t n2);

struct bignum T(size_t n1, size_t n2);

struct bignum comp_pi(size_t s);

void version2_Dec(size_t n, int last_itr);

void version2_Hex(size_t n, int last_itr);

#endif

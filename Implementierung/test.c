#include "operations.c"


void testshiftright()
{
    size_t len = 2;
    struct bignum num1 = {
        .length = len,
        .subone = 0,
        .numbers = (uint32_t *)malloc(len*sizeof(uint32_t))
    };

    num1.numbers[0] = 1;
    num1.numbers[1] = 1000;

    struct bignum num2 = {
        .length = 1,
        .subone = 0,
        .numbers = malloc(1*sizeof(uint32_t))
    };

    num2.numbers[0] = 300;

    struct bignum res;

    newtonDiv(&num2, &num1, 10, &res);

    //bignumPrint(&res);

    bignumFree(&num1);
    bignumFree(&num2);
    bignumFree(&res);

}

void test_reduce(){
    struct bignum num1 = {
        .length = 1,
        .subone = 0,
        .numbers = malloc(1*sizeof(uint32_t))
    };

    num1.numbers[0] = 0x0000006e;

    bignumPrint(&num1);
    bignumPrint(&num2);

    free(num1.numbers);
    free(num2.numbers);
}

void testconvert()
{
    struct bignum pi = comp_pi(44);
    printf("Required Hex numbers: "); 
    bignumPrint(&pi);
    bignumFree(&pi);
}

testshiftleft()
{  
    size_t len = 2;
    struct bignum num1 = {
        .length = len,
        .subone = 0,
        .numbers = (uint32_t *)malloc(len*sizeof(uint32_t))
    };

    num1.numbers[0] = 1;
    num1.numbers[1] = 1;

    struct bignum num2 = lShift(&num1, 2);

    num1.numbers[0] = 0x2a000000;
    num1.numbers[1] = 0xb8787878;
    num1.numbers[2] = 1;

    num2.numbers[0] = 0x01640000;
    num2.numbers[1] = 0x93e96969;
    num2.numbers[2] = 1;

    printf("num 1: "); bignumPrint(&num1);
    printf("num 2: "); bignumPrint(&num2);

    struct bignum res;
    karazubaMult(&num1, &num2, &res);
    
    free(num1.numbers);
    free(num2.numbers);
}

int main()
{
    testconvert();
    return 0;
}
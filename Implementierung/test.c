#include "operations.h"
#include "pi.h"
#include <stdlib.h>
#include <stdio.h>
#define PI_FILE "pi.txt"

/**
 *  compare the decimal digits of the given bignum pi with a file
 */
void test_correctness(const struct bignum *calculatedPi, size_t precision)
{
    printf("Correctness testing of first %ld decimal digits: ", precision);
    // Test the integer part of the calculated Pi
    if (calculatedPi->numbers[calculatedPi->length - 1] != 3 && calculatedPi->length - 1 == calculatedPi->subone)
    {
        printf("Failed (Pi is not correct at position 0. Should have value 3,... but is %08x)\n", calculatedPi->numbers[0]);
        exit(EXIT_FAILURE);
    }

    // convert pi to decimal digits
    uint8_t *buffer;
    size_t bufferLength = bignumConvDec(calculatedPi, &buffer);

    // open the file with the comparison
    FILE *file = fopen(PI_FILE, "r");
    if (file == NULL)
    {
        perror("\nError in opening file. (test_correctness)");
        exit(EXIT_FAILURE);
    }

    unsigned int loadedNumber; // current number loaded from file
    for (size_t i = bufferLength; i > bufferLength - precision; i--)
    {
        int x = fscanf(file, "%1u", &loadedNumber);
        if (x == 0)
        {
            printf("fscanf returnded nothing.(test_correctness)");
            exit(EXIT_FAILURE);
        }
        // compare the loaded and calculated decimal digit
        if (buffer[i] != loadedNumber)
        {
            printf("Failed (Pi is not correct at position %lu. Was %8u but should be %8u)\n", i, buffer[i], loadedNumber);
            exit(EXIT_FAILURE);
        }
    }

    printf("Success\n");
    fclose(file);
    free(buffer);
}
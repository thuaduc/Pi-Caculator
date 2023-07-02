#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h>
#include <errno.h>
#include <getopt.h>
#include <time.h>

#include "pi.h"
#include "operations.h"

void test_correctness(const struct bignum *calculatedPi, size_t precision);

#define VERSION_MAX 2 // highest version-id  or  number of versions - 1

/**
 * @brief
 *
 */
void usage()
{
    printf("./main.o\n"
           "---------------------------Usage--------------------------------------\n"
           "-d <int>    Output of n decimal places after the decimal point. Takes precedence over -h\n"
           "-h<int>     Output of n hexadecimal places after the decimal point (don't add space between option and argument or it will be ignored)\n"
           "-B<int>     Option to print running time with optional argument for the numer of repeats of the function call. \"-B3 is set as default\" (don't add space between option and argument or it will be ignored)\n"
           "-h          Print description of all options and ends program\n"
           "--help      Print description of all options and ends program\n"
           "-V <int>    choose implementation:\n"
           "    V0      main implementaition (is set as default)\n"
           "    V1      comparison implementation \n"
           "    V2      test correctness of requested amount of decimal places \n" 
           "----------------------------------------------------------------------\n");
}

struct bignum mainImplementation(size_t s)
{
    return comp_pi(s+1);
}

void secondImplementation(size_t s, int hex, int last_itr)
{
    if(hex) {
        version2_Hex(s, last_itr);
    }else {
        version2_Dec(s, last_itr);
    }
}

/**
 * @brief function parses the options of the program
 *
 * @param h
 * @param d
 * @param version
 * @param B
 * @param argc
 * @param argv
 * @return int: EXIT_SUCCESS or EXIT_FAILTURE
 */
int parseOptions(int *h, int *d, int *version, int *B, int argc, char **argv)
{
    int opt;
    char *endptr;
    errno = 0;
    int numArg=0;

    static struct option long_options[] = {
        {"help", no_argument, 0, 'h'},
    };
    int option_index = 0;

    while ((opt = getopt_long(argc, argv, "d:V:h::B::", long_options, &option_index)) != -1)
    {
        endptr = NULL;
        errno = 0;

        switch (opt)
        {
        case 'h':
            if (optarg != 0)
            {
                *h = strtol(optarg, &endptr, 10);
                if (*h <= 0 || endptr == optarg || *endptr != '\0')
                {
                    fprintf(stderr, "-h %s is not allowed. Only a value > 0 is valid.\n", optarg);
                    return EXIT_FAILURE;
                }
                else if (errno == ERANGE)
                {
                    fprintf(stderr, "%s over - or underflows double \n", optarg);
                    return EXIT_FAILURE;
                }
            }
            else
            {
                usage();
                return EXIT_SUCCESS;
            }
            break;

        case 'd': // n dezimalen Nachkommastellen. Hat Vorrang vor -h
            *d = strtol(optarg, &endptr, 10);
            if (*d <= 0 || endptr == optarg || *endptr != '\0')
            {
                fprintf(stderr, "-d %s is not allowed. Only a value > 0 is valid.\n", optarg);
                return EXIT_FAILURE;
            }
            else if (errno == ERANGE)
            {
                fprintf(stderr, "%s over - or underflows double \n", optarg);
                return EXIT_FAILURE;
            }
            break;
        case 'B':
            if (optarg)
            {
                *B = strtol(optarg, &endptr, 10);
                if (*B <= 0 || endptr == optarg || *endptr != '\0')
                {
                    fprintf(stderr, "-B %s is not allowed. Only a value > 0 is valid.\n", optarg);
                    return EXIT_FAILURE;
                }
                else if (errno == ERANGE)
                {
                    fprintf(stderr, "%s over - or underflows double \n", optarg);
                    return EXIT_FAILURE;
                }
            }
            else
                *B = 3;
            break;
        case 'V':
            *version = strtol(optarg, &endptr, 10);
            if (*version > VERSION_MAX || *version < 0 || endptr == optarg || *endptr != '\0')
            {
                fprintf(stderr, "-V %s is not allowed. Please choose a valid version. (use -h/--help for information)\n", optarg);
                return EXIT_FAILURE;
            }
            else if (errno == ERANGE)
            {
                fprintf(stderr, "%s over - or underflows double \n", optarg);
                return EXIT_FAILURE;
            }
            break;

        default:
            fprintf(stderr, "Wrong Argument %s. (use -h/--help for information)\n", argv[0]);
            return EXIT_FAILURE;
            break;
        }
        numArg++;
    }

    if (*d == 0 && *h == 0)
    {
        fprintf(stderr, "Argument(s) missing: Please set the number of decimal places -d <int> or -h <int>\n");
        return EXIT_FAILURE;
    }else if (numArg < argc-1){
        fprintf(stderr, "too many Arguments. (use -h/--help for information)\n");
        return EXIT_FAILURE;
    }
    else
        return 2;
}

void benchmarking(int version, int iterations, int number_of_digits, int is_hex)
{

    // avoiding free on uninitialized
    struct timespec start;
    struct timespec end;
    struct bignum res;
    double time = 0;
    double avg_time=0;
    bignumInit(&res, 1);

    switch (version)
    {
    case 1:
        clock_gettime(CLOCK_MONOTONIC, &start);
        for (size_t i = 0; i < (size_t)iterations; i++)
        {
            secondImplementation(number_of_digits, is_hex, (int)i == iterations - 1);
        }
        clock_gettime(CLOCK_MONOTONIC, &end);
        time = end.tv_sec - start.tv_sec + 1e-9 * (end.tv_nsec - start.tv_nsec);
        avg_time = time / iterations;
        printf("\n---------------------------------\n");
        printf("Displaying runtime of computing %d places\n", number_of_digits);
        printf("Iterations: %d\n", iterations);
        printf("Runtime: %lf\n", time);
        printf("Average runtime: %lf\n", avg_time);
        break;

    case 0:
        clock_gettime(CLOCK_MONOTONIC, &start);
        for (size_t i = 0; i < (size_t)iterations; i++)
        {
            bignumFree(&res); // avoiding memorey leak
            res = mainImplementation(number_of_digits);
        }
        clock_gettime(CLOCK_MONOTONIC, &end);
        time = end.tv_sec - start.tv_sec + 1e-9 * (end.tv_nsec - start.tv_nsec);
        avg_time = time / iterations;
        printf("Displaying runtime of computing %d places\n", number_of_digits);
        printf("Iterations: %d\n", iterations);
        printf("Runtime: %lf\n", time);
        printf("Average runtime: %lf\n", avg_time);
        printf("------------------------------------\n");
        printf("Done calculate, printing result ....\n");
        printf("------------------------------------\n");

        if(is_hex){
            printf("Result in hex: ");
	        bignumPrintHex(&res, number_of_digits);
        }else{
            printf("Result in dec: ");
	        bignumPrintDec(&res, number_of_digits);
        }
	    bignumFree(&res);
        break;

    default:
        break;
    }
    
}

int main(int argc, char **argv)
{
    int h = 0, d = 0, version = 0, B = 0;
    int parseState = parseOptions(&h, &d, &version, &B, argc, argv);
    if (parseState == EXIT_FAILURE)
        return EXIT_FAILURE;
    else if (parseState == EXIT_SUCCESS)
        return EXIT_SUCCESS;

    printf("Parsed Parameters: h: %d, d: %d, Version: %d, B: %d\n", h, d, version, B);

    struct bignum result;

    // todo
    switch (version)
    {
    case 0:
        printf("==> Execute version 0\n");
        if (!d)
        { // the result is in hex
            if (B)
            { // benshmark is required
                benchmarking(version, B, h, 1);
            }
            else
            {
                printf("Printing %d hexadecimal places after comma...\n", h);
                mainImplementation(h);
                printf("Result: ");
                bignumPrintHex(&result, h);
            }
        }
        else
        { // the result is in dec
            if (B)
            { // benshmark is required
                benchmarking(version, B, d, 0);
            }
            else
            {
                printf("Printing %d decimal places after comma...\n", d);
                mainImplementation(d);
                printf("Result: ");
                bignumPrintDec(&result, d);
            }
            
        }
        break;
    case 1:
        printf("==> Execute version 1\n");
        if (h)
        { // the result is in hex
            if (B)
            { // benchmark is required
                benchmarking(version, B, h, 1);
            }
            else
            {
                printf("Printing %d hexadecimal places after comma...\n", h);
                secondImplementation(h, 1,1);
            }

        }
        else
        { // the result is in dec
            if (B)
            { // benshmark is required
                benchmarking(version, B, d, 0);
            }
            else
            {
                printf("Printing %d decimal places after comma...\n", d);
                secondImplementation(d,0,1);
            }

        }
        break;
    case 2:
        printf("==> Execute version 2\n");
        if(!d){
            fprintf(stderr, "testing is only ment for decimal places. (use -h/--help for information)\n");
            return EXIT_FAILURE;
        }
        struct bignum res = comp_pi(d);
        test_correctness(&res, d);
        break;
    default:
        break;
    }

    bignumFree(&result);
    return EXIT_SUCCESS;
}

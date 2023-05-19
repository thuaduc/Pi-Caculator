#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h>
#include <errno.h>
#include <getopt.h>
#define VERSION_MAX 1 // highest version-id  or  number of versions - 1

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
           "-B<int>     Option to print running time with optional argument for the numer of repeats of the function call (don't add space between option and argument or it will be ignored)\n"
           "-h          Print description of all options and ends program\n"
           "--help      Print description of all options and ends program\n"
           "-V <int>    choose implementation:\n"
           "    V0      main implementaition (is set as default)\n"
           "    V1      comparison ...  \n" // todo
           "----------------------------------------------------------------------\n");
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
            }
            else if (errno == ERANGE)
            {
                fprintf(stderr, "%s over - or underflows double \n", optarg);
                return EXIT_FAILURE;
            }
            else
                *B = 1;
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
    }

    if (*d == 0 && *h == 0)
    {
        fprintf(stderr, "Argument(s) missing: Please set the number of decimal places -d <int> or -h <int>\n");
        return EXIT_FAILURE;
    }
    else
        return 2;
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

    // todo
    switch (version)
    {
    case 0:
        printf("==> Execute implementation 0\n");
        /* code */
        break;
    case 1:
        printf("==> Execute implementation 1\n");
        /* code */
        break;

    default:
        break;
    }

    return EXIT_SUCCESS;
}
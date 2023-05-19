# Project README

## Overview
This project is part of the "Praktikum Grundlagen Rechnerarchitektur" course at TUM. It involves approximating the mathematical constant π using efficient algorithms. The goal is to calculate π with configurable precision and evaluate the solution's quality scientifically.

## Task Description
The task is to implement an efficient algorithm to approximate the mathematical constant π with arbitrary precision. The algorithm will utilize the Karatsuba multiplication and Binary Splitting techniques.

### Karatsuba Multiplication
The Karatsuba algorithm allows efficient multiplication of large numbers by recursively dividing them into smaller parts. It reduces the number of required multiplications and improves performance.

### Binary Splitting
Binary Splitting is a technique used to efficiently calculate mathematical sequences. It utilizes polynomials and integer arithmetic operations to perform calculations. The approach avoids division operations except for the final step.

### Framework Program Options
The framework program supports the following command-line options:

- `-d <int>`: Output n decimal places after the decimal point. Overrides the `-h` option.
- `-h <int>`: Output n hexadecimal places after the decimal point.
- `-B <int>`: Print running time with an optional argument for the number of function call repeats.
- `-h`: Print the description of all options and exit.
- `--help`: Print the description of all options and exit.
- `-V <int>`: Choose the implementation:
  - `V0`: Main implementation (default).
  - `V1`: Comparison implementation.

## Getting Started
To begin working on this project, follow these steps:

1. Clone the project repository from Git.
2. Compile the program with `make`.
3. Run the program as `./main`.

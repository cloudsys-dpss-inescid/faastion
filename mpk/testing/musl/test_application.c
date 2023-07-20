#include <stdio.h>

// Function to calculate the factorial of a number
unsigned long long factorial(int n) {
    if (n == 0 || n == 1) {
        return 1;
    } else {
        return n * factorial(n - 1);
    }
}

int main() {
    int number = 5; // Change this value to calculate the factorial of a different number

    // Calculate the factorial of the given number
    unsigned long long result = factorial(number);

    // Print the result
    printf("Factorial of %d is %llu\n", number, result);

    return 0;
}

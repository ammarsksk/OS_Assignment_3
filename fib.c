#include <stdio.h>

// Recursive function to calculate Fibonacci numbers
int fibonacci(int n) {
    if (n <= 1)
        return n;  // Base cases: F(0) = 0, F(1) = 1
    return fibonacci(n - 1) + fibonacci(n - 2);  // Recursive call
}

int main() {
    int n = 40;

    printf("Fibonacci number at position %d is: %d\n", n, fibonacci(n));
    return 0;
}
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <stdbool.h>
#include "sieve.h"

void sieveofe(int c)
{
	printf("%d \n\r", c);
	//int n;
	int n = 20;
	int prime[n+1];
	//Loading the array with numbers from 1 to n
	for(int i = 1; i <= n; i++)
	{
		prime[i] = i;
	}
	//Start with least prime number, which is 2.
	//No need to check for numbers greater than square root of n.
	//They will be already marked.
	for(int i = 2; i*i <= n; i++)
	{
		if(prime[i] != -1)
		{
			//Mark all the multiples of i as -1.
			for(int j = 2*i; j <=n ; j += i)
				prime[j] = -1;
		}
	}
	printf("Prime numbers are: \n\r");
	for(int i=2; i <= n; i++)
	{
		if(prime[i] != -1)
		{
			printf("%d \n\r", i);
		}
	}
}

void simpleSieve(int limit, bool prime[]) {
    for (int p = 2; p * p <= limit; p++) {
        if (prime[p]) {
            for (int i = p * p; i <= limit; i += p)
                prime[i] = false;
        }
    }
}

// Segmented Sieve Algorithm
void segmentedSieve(int n) {
    int segmentSize = 100000; // Segment size
    int sqrtN = (int)sqrt(n);

    // Array to store primes up to sqrtN
    bool* prime = malloc((sqrtN + 1) * sizeof(bool));
    for (int i = 0; i <= sqrtN; i++) prime[i] = true;
    simpleSieve(sqrtN, prime);

    // Array to mark non-primes in the current segment
    bool* isPrime = malloc(segmentSize * sizeof(bool));

    // Count of primes
    int count = 0;

    // Process each segment
    for (int low = 1; low <= n; low += segmentSize) {
        int high = low + segmentSize;
        if (high > n + 1) high = n + 1;  // Adjust high for the last segment

        // Initialize the segment as all primes
        for (int i = 0; i < segmentSize && low + i <= n; i++) {
            isPrime[i] = true;
        }

        // Mark non-primes in the current segment
        for (int i = 2; i <= sqrtN; i++) {
            if (prime[i]) {
                // Find the minimum number in the current segment that is a multiple of i
                int loLim = fmax(i * i, (low + i - 1) / i * i);
                if (loLim >= high) continue;
                for (int j = loLim; j < high; j += i) {
                    if (j >= low) {
                        isPrime[j - low] = false;
                    }
                }
            }
        }

        // Count primes in the current segment
        int segmentCount = 0;
        for (int i = 0; i < segmentSize && low + i <= n; i++) {
            if (isPrime[i] && (low + i) > 1) {  // Avoid counting 1 which is not a prime
                segmentCount++;
                count++;
            }
        }

        // Print the segment range and total primes so far
        printf("%d,%d\n\r", high - 1, count);
    }

    free(prime);
    free(isPrime);
    printf("Total number of primes less than %d: %d\n\r", n, count);
}



void π(int n) {

    printf("\n\r - up to %d\n\r", n);
    segmentedSieve(n);
    // return 0;
}

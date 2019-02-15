#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define count_left_zero_bits(x) __builtin_clz(x)


/*
EXPLANATION:
-- The factorial number system.
Its functionality will be best explained with an example.
Let's say that we have the (ordered) string: 0123456789 and we want to find its 1000000th lexicographic permutation (the original PE problem).
We know that the first couple permutations start with 0 (which is first in lexicographic order). But how many?
Well, if we fix 0 at the start, then the question becomes: How many ways do we have to order the next 9 digits. And that is 9! = 362880.
So, we have (the first) 9! permutations that start with 0. That means that the 1000000th is not one of them. Let's see the next interval-set of permutaions.
That is, permutations that start with 1. Again, we have if we fix 1 on the start, we have 9! factorial ways to order the next 9 digits.
So, from the 362880 up to (and including) the 2 * 9! = (2 * 362880) = 725760th permutation, they start with 1. It's apparent that the 1000000th is not one of
them either. The third set is from 725761 up to (and including) 3 * 9! = 1088640. It's apparent that our target permutation is in this interval. So, we just
found the first digit, which is 2.
Now, let's go to find the second. We know that as a first digit we have the 2. That means that the digits that have remained to use next are: 0,1,3,4,5,6,7,8,9
in that lexicographic order. So, we know that 2 is fixed in the first position. If we also fix 1 in the second, there are 8! ways to order the next 8.
So, permutations from 725761 up to (and including) 725760 + 8! are ones with 2 as a first digit and 0 as a second. In the same manner, permutations from
725760 + 8! + 1 up to (and including) 725760 + 2 * 8! have 2 as a first digit and 1 as second.
Basically, using that logic, we find all the digits of the number. We find for every digit, in which interval is our target, until we have figured out all
the digits.
*/

/*
Notes on implementation: This is actually a simple version of an actual (useful) permutation compressor, as it handles up to 32 bits permutations.
To make it a strong permutation compressor we have to:
a) Represent an arbitrary number of symbols (easy)
b) Implement some arbitrary-length arithmetic (not very easy but still duable)
c) Code is mediocre so we could make it better.
*/


#define NUM_DIGITS 8
char original_digits[] = "01234567";

int factorial[NUM_DIGITS + 1];

int fact(int n) {
	if(n < 0)
		return 1;
	int res = 1;
	for(int i = 1; i <= n; ++i)
		res *= i;
	return res;
}

void printNthLexPermutation(uint64_t n) {
	uint64_t sum = 0;
	char digits[NUM_DIGITS];
	for(int i = 0; i < NUM_DIGITS; ++i)
		digits[i] = original_digits[i];
	uint64_t target = n;
	for(int i = 0; i < NUM_DIGITS - 1; ++i) {   // for each digit (except the last one)
		int current_fact = factorial[NUM_DIGITS - 1 - i];   // ways to order the remaining digits
		int j = 1;
		while((sum + j * current_fact) < target)    // find the correct interval
			++j;
		int k = j;
		sum += (k - 1) * current_fact;   // start of the interval

		// find the jth (not already used) digit
		for(j = 0; j < NUM_DIGITS; ++j) {
			if(digits[j] != '\0')   // if not used
				--k;
			if(k == 0)   // we found it
				break;
		}
		printf("%c", digits[j]);
		digits[j] = '\0';    // mark it as used
	}

	// find what is the last digit (not yet used)
	for(int i = 0; i < NUM_DIGITS; ++i) {
		if(digits[i] != '\0') {
			printf("%c", digits[i]);
			break;
		}
	}
	printf("\n");
}

uint32_t compute_term(char perm[]) {
    uint32_t res = 0;
	char digits[NUM_DIGITS];
	for(int i = 0; i < NUM_DIGITS; ++i)
		digits[i] = original_digits[i];
    for(int i = 0; i < NUM_DIGITS - 1; ++i) {   // for each digit (except the last one)
        int current_fact = factorial[NUM_DIGITS - 1 - i];   // ways to order the remaining digits
		int digits_skipped = 0;
        // find number of digits skipped before the one used.
        for(int j = 0; digits[j] != perm[i]; ++j) {
			if(digits[j] != '\0')
				++digits_skipped;
		}
		res += digits_skipped * current_fact;
		digits[perm[i] - original_digits[0]] = '\0';
    }
    return 1+res;
}


int main(void) {
	// initialize factorials
    int temp;
    factorial[0] = temp = 1;
	for(int i = 1; i <= NUM_DIGITS; ++i) {
        temp = factorial[i] = temp * i;
	}

    char perm[] = "03652174";

    // Compute bits that we will need
    int32_t num_perms = factorial[NUM_DIGITS];
    printf("We need at most %d bits.\n", (int)ceil(log2(num_perms)));
    printf("Alternatively, we need at most %d bits\n", 32 - count_left_zero_bits(num_perms));

    // compress step
	uint32_t n = compute_term(perm);
	printf("%s compressed to %d bits\n", perm, 32 - count_left_zero_bits(n));
    // decompress step
    printf("Decompress: ");
	printNthLexPermutation(n);
}

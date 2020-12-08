#ifndef CEXT_XORSHIFT_H
#define CEXT_XORSHIFT_H
/**
 * PRNG based on Xorshift
 * @return A pseudo-random number on the interval [0 .. UINT_MAX)
 */
unsigned int xor_rand();

/**
 * PRNG based on Xorshift
 * @return A pseudo-random number on the interval [INT_MIN .. INT_MAX)
 */
int xor_rand_int32();
/**
 * PRNG based on Xorshift
 * @return A pseudo-random number on the interval [0.0 .. 1.0)
 */
double xor_rand_double();

/**
 * Sets the seed for xor_rand
 * @param seed A number on the interval [0 .. UINT_MAX)
 * @return The previous state of the PRNG.
 */
unsigned int xor_srand(unsigned int seed);

int rand_sample(const int arr[], int arr_len);

#endif //CEXT_XORSHIFT_H

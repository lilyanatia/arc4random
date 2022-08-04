#include <stdint.h>

uint32_t arc4random(void);

void arc4random_buf(void *buf, size_t nbytes);

uint32_t arc4random_uniform(uint32_t upper_bound);

void arc4random_double_buf(double *buf, size_t length);

void arc4random_float_buf(float *buf, size_t length);

#include <avr/io.h>

FUSES = {
    .low = LFUSE_DEFAULT | 0x3f,
    .high = HFUSE_DEFAULT & ~(1 << 4)
};

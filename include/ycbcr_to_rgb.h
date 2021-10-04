#include <stdint.h>


void ycbcr_to_rgb(uint8_t *r, uint8_t *g, uint8_t *b, uint8_t *y, uint8_t *cb,
                    uint8_t *cr, uint32_t len);

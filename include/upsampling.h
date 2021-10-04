#include <stdint.h>


// sur-échantillonnage horizontal
void upsample_H(uint8_t *pixels, uint8_t *pixels_TL, uint8_t *pixels_TR);


// sur-échantillonnage vertical
void upsample_V(uint8_t *pixels, uint8_t *pixels_TL, uint8_t *pixels_BL);


// sur-échantillonnage horizontal et vertical
void upsample_H_and_V(uint8_t *pixels, uint8_t *pixels_TL,
                        uint8_t *pixels_TR, uint8_t *pixels_BL, uint8_t *pixels_BR);

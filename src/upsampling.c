#include <stdint.h>
#include "upsampling.h"


// sur-échantillonnage horizontal
void upsample_H(uint8_t *pixels, uint8_t *pixels_L, uint8_t *pixels_R)
{

    // left block
    for (uint8_t y = 0; y < 8; y++) {
        for (uint8_t x = 0; x < 4; x++) {
            pixels_L[x*2+8*y] = pixels[x+8*y];
            pixels_L[x*2+1+8*y] = pixels[x+8*y];
        }
    }

    // right block
    for (uint8_t y = 0; y < 8; y++) {
        for (uint8_t x = 0; x < 4; x++) {
            pixels_R[x*2+8*y] = pixels[x+4+8*y];
            pixels_R[x*2+1+8*y] = pixels[x+4+8*y];
        }
    }
}


// sur-échantillonnage vertical
void upsample_V(uint8_t *pixels, uint8_t *pixels_T, uint8_t *pixels_B)
{

    // top block
    for (uint8_t y = 0; y < 4; y++) {
        for (uint8_t x = 0; x < 8; x++) {
            pixels_T[x+8*y*2] = pixels[x+8*y];
            pixels_T[x+8+8*y*2] = pixels[x+8*y];
        }
    }

    // bottom block
    for (uint8_t y = 0; y < 4; y++) {
        for (uint8_t x = 0; x < 8; x++) {
            pixels_B[x+8*y*2] = pixels[x+8*(y+4)];
            pixels_B[x+8+8*y*2] = pixels[x+8*(y+4)];
        }
    }
}


// sur-échantillonnage horizontal et vertical
void upsample_H_and_V(uint8_t *pixels, uint8_t *pixels_TL,
                        uint8_t *pixels_TR, uint8_t *pixels_BL, uint8_t *pixels_BR)
{

    // top left block
    for (uint8_t y = 0; y < 4; y++) {
        for (uint8_t x = 0; x < 4; x++) {
            pixels_TL[x*2+8*y*2] = pixels[x+8*y];
            pixels_TL[x*2+1+8*y*2] = pixels[x+8*y];
            pixels_TL[x*2+8*(y*2+1)] = pixels[x+8*y];
            pixels_TL[x*2+1+8*(y*2+1)] = pixels[x+8*y];
        }
    }

    // top right block
    for (uint8_t y = 0; y < 4; y++) {
        for (uint8_t x = 0; x < 4; x++) {
            pixels_TR[x*2+8*y*2] = pixels[x+4+8*y];
            pixels_TR[x*2+1+8*y*2] = pixels[x+4+8*y];
            pixels_TR[x*2+8*(y*2+1)] = pixels[x+4+8*y];
            pixels_TR[x*2+1+8*(y*2+1)] = pixels[x+4+8*y];
        }
    }

    // bottom left block
    for (uint8_t y = 0; y < 4; y++) {
        for (uint8_t x = 0; x < 4; x++) {
            pixels_BL[x*2+8*y*2] = pixels[x+8*(y+4)];
            pixels_BL[x*2+1+8*y*2] = pixels[x+8*(y+4)];
            pixels_BL[x*2+8*(y*2+1)] = pixels[x+8*(y+4)];
            pixels_BL[x*2+1+8*(y*2+1)] = pixels[x+8*(y+4)];
        }
    }

    // bottom right block
    for (uint8_t y = 0; y < 4; y++) {
        for (uint8_t x = 0; x < 4; x++) {
            pixels_BR[x*2+8*y*2] = pixels[x+4+8*(y+4)];
            pixels_BR[x*2+1+8*y*2] = pixels[x+4+8*(y+4)];
            pixels_BR[x*2+8*(y*2+1)] = pixels[x+4+8*(y+4)];
            pixels_BR[x*2+1+8*(y*2+1)] = pixels[x+4+8*(y+4)];
        }
    }
}

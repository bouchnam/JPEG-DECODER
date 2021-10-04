#include <stdint.h>
#include <math.h>

#define M_PI 3.14159265358979323846


// Coefficient C de la transformée en cosinus discrète inverse.
float C(uint8_t i)
{
    if (i == 0) {
        return 1/sqrt(2);
    } else {
        return 1;
    }
}


// Valeur saturée entre 0 et 255.
void saturated(float *value)
{
    if (*value < 0) {
        *value = 0;
    } else if (*value > 255) {
        *value = 255;
    }
}


// Idct using row-column decomposition
void idct(uint8_t *pixels, int16_t *frequences)
{
    float sum1[64];
    float idctcos[64];
    
    // remplir la liste par les cos
    for (uint8_t x = 0; x < 8; x++) {
        for (uint8_t y = 0; y < 8; y++) {
          idctcos[x+8*y] = C(y)*cosf((2*x+1)*y*M_PI/16);
        }
      }

    // Idc 1D ,parcours de chaque colone 
    for (uint8_t i = 0; i < 8; i++) {
        for (uint8_t y = 0; y < 8; y++) {
            float sum = 0;
            for (uint8_t v = 0; v < 8; v++){
              sum += frequences[v * 8 + i] * idctcos[v * 8 + y];
            }
            sum1[y * 8 + i] =  sum;
        }
    }

    // Idct 1D,parcours de chaque ligne
    for (uint8_t i = 0; i < 8; i++) {
        for (uint8_t x = 0; x < 8; x++) {
            float sum = 0;
            for (uint8_t u = 0; u < 8; u++) {
              sum += sum1[i * 8 + u] * idctcos[u * 8 + x];
            }
            float pixel_value = 128 + 0.25*sum;
            saturated(&pixel_value);
            pixels[i * 8 + x] = (uint8_t) pixel_value;
        }
    }
}

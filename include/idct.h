#include <stdint.h>


// Coefficient C de la transformée en cosinus discrète inverse.
extern float C(uint8_t i);


// Valeur saturée entre 0 et 255.
extern void saturated(float *value);


// Transformée en cosinus discrète inverse.
extern void idct(uint8_t *pixels, int16_t *frequences);

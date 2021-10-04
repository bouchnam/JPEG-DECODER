#include <stdint.h>


// Retourne la valeur d'un élément à partir de son indice de classe de magnitude.
int16_t magnitudeindex_to_value(uint8_t *index, uint8_t m);


// Quantification inverse.
void iquant(int16_t *vector, uint8_t *quant_table);

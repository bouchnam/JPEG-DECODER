#include <stdint.h>
#include <math.h>


// Retourne la valeur d'un élément à partir de son indice de classe de magnitude.
int16_t magnitudeindex_to_value(uint8_t *index, uint8_t m)
{
    int16_t value = 0;
    if (index[0] == 1) {
        for (uint8_t i = 0; i < m; i++) {
            value += index[i]*pow(2,m-i-1);
        }
    } else {
        value = -pow(2, m);
        for (uint8_t i = 1; i < m; i++) {
            value += index[i]*pow(2,m-i-1);
        }
        value+=1;
    }
    return value;
}


// Quantification inverse.
void iquant(int16_t *vector, uint8_t *quant_table)
{
    for (uint8_t i = 0; i <= 63; i++) {
        vector[i] *= quant_table[i];
    }
}

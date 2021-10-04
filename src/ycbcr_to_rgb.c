#include <stdint.h>
#include "idct.h"


// Calcule les valeurs RGB à partir des valeurs YCbCr.
void ycbcr_to_rgb(uint8_t *r, uint8_t *g, uint8_t *b, uint8_t *y, uint8_t *cb,
                    uint8_t *cr, uint32_t len)
{
    for (uint32_t i = 0; i < len; i++) {
        float r_value = y[i] -0.0009267*(cb[i]-128)+1.4016868*(cr[i]-128);
        float g_value = y[i] -0.3436954*(cb[i]-128)-0.7141690*(cr[i]-128);
        float b_value = y[i] +1.7721604*(cb[i]-128)+0.0009902*(cr[i]-128);

        // valeur saturée entre 0 et 255
        saturated(&r_value);
        saturated(&g_value);
        saturated(&b_value);

        r[i] = (uint8_t) r_value;
        g[i] = (uint8_t) g_value;
        b[i] = (uint8_t) b_value;
    }
}

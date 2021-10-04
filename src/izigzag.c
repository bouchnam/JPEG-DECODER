#include <stdint.h>
#include <stdbool.h>
#include <string.h>


// Zig-zag inverse.
void izigzag(int16_t *vector)
{
    uint8_t nombre_diag = 7;
    uint8_t L[64]={0};
    bool Towards_the_top = true;
    uint8_t couple = 0;
    uint8_t couple2 = 63;
    uint8_t compteur = 1;
    uint8_t compteur2 = 62;
    L[0] = 0;
    L[63] = 63;

    for (uint8_t i = 0; i < nombre_diag; i++) {
        if (Towards_the_top) {

            for (uint8_t elt = 0; elt < i; elt++) {
                couple-=8;
                couple+=1;
                L[compteur] = couple;
                compteur +=1;
            }

            couple+=1;
            L[compteur] = couple;
            compteur +=1;

            for (uint8_t elt = 0; elt < i; elt++) {
                couple2+=8;
                couple2-=1;
                L[compteur2] = couple2;
                compteur2 -=1;
            }

            couple2-=1;
            L[compteur2] = couple2;
            compteur2 -=1;
            Towards_the_top = false;

        } else {

            for (uint8_t elt = 0; elt < i; elt++) {
                couple+=8;
                couple-=1;
                L[compteur] = couple;
                compteur +=1;
            }

            couple+=8;
            L[compteur] = couple;
            compteur +=1;

            for (uint8_t elt = 0; elt < i; elt++) {
                couple2-=8;
                couple2+=1;
                L[compteur2] = couple2;
                compteur2 -=1;
            }

            couple2-=8;
            L[compteur2] = couple2;
            compteur2 -=1;
            Towards_the_top = true;
        }
    }

    for (uint8_t i = 0; i < 6; i++) {
        L[compteur] = L[compteur-1] +7;
        compteur+=1;
    }

   int16_t copie[64];
   memcpy(copie, vector, sizeof copie);

   for(uint8_t i = 0; i < 64; i++) {
       vector[L[i]] = copie[i];
   }
}

#include <stdint.h>
#include "jpeg_reader.h"
#include "bitstream.h"



// Détermine le nouveau nom du fichier avec la bonne extension
void get_new_filename(const char *filename, char *new_filename, uint8_t nb_components);


/*
Décode un bloc de 8x8 et place les valeurs des pixels dans un vecteur de 64
éléments.
*/
void decode_block(struct jpeg_desc *jdesc, struct bitstream *stream,
                    uint8_t component, int16_t *last_Y_DC_value,
                    int16_t *last_Cb_DC_value, int16_t *last_Cr_DC_value,
                    uint8_t *pixels);

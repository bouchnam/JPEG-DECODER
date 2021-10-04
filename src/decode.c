#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "jpeg_reader.h"
#include "bitstream.h"
#include "huffman.h"
#include "magnitude_iquant.h"
#include "izigzag.h"
#include "idct.h"


// Détermine le nouveau nom du fichier avec la bonne extension
void get_new_filename(const char *filename, char *new_filename, uint8_t nb_components)
{
    uint8_t length = strlen(filename);
    uint8_t new_length = 0;

    if (filename[length-2]=='e') {
        new_length = length -5;
    } else {
        new_length = length -4;
    }
    for (uint8_t i = 0; i < new_length; i++) {
        new_filename[i] = filename[i];
    }
    new_filename[new_length] = '\0';

    // P5 pour pgm et P6 pour ppm
    if (nb_components == 1) {
        strcat(new_filename, ".pgm");
    } else {
        strcat(new_filename, ".ppm");
    }
}


/*
Décode un bloc de 8x8 et place les valeurs des pixels dans un vecteur de 64
éléments.
*/
void decode_block(struct jpeg_desc *jdesc, struct bitstream *stream,
                    uint8_t component, int16_t *last_Y_DC_value,
                    int16_t *last_Cb_DC_value, int16_t *last_Cr_DC_value,
                    uint8_t *pixels)
{

    int16_t vector[64]={0};



    // coefficient DC
    uint8_t component_id =  jpeg_get_frame_component_id(jdesc, component) -1;
    uint8_t huffman_DC_index = jpeg_get_scan_component_huffman_index(jdesc, DC, component_id);

    struct huff_table *huff_table_DC = jpeg_get_huffman_table(jdesc, DC, huffman_DC_index);
    // classe de magnitude
    int16_t coeff_value = 0;
    int8_t m = huffman_next_value(huff_table_DC, stream);
    if (m == 0) {
        coeff_value = 0;
    } else {
        uint32_t bits;
        bitstream_read(stream, m, &bits, true);
        // indice dans la classe de magnitude
        uint8_t index[m];
        for (uint8_t i = 0; i < m; i++) {
            index[m-i-1] = bits & 0x01;
            bits = bits >> 1;
        }

        coeff_value = magnitudeindex_to_value(index, m);
    }
    if (component == 0) {
        vector[0] = coeff_value + *last_Y_DC_value;
        *last_Y_DC_value = vector[0];
    } else if (component == 1) {
        vector[0] = coeff_value + *last_Cb_DC_value;
        *last_Cb_DC_value = vector[0];
    } else {
        vector[0] = coeff_value + *last_Cr_DC_value;
        *last_Cr_DC_value = vector[0];
    }



    // coefficients AC
    uint8_t huffman_AC_index = jpeg_get_scan_component_huffman_index(jdesc, AC, component_id);
    struct huff_table *huff_table_AC = jpeg_get_huffman_table(jdesc, AC, huffman_AC_index);

    for (uint8_t i = 1; i<= 63; i++) {
        uint8_t value = huffman_next_value(huff_table_AC, stream);

        if (value == 0xf0) {
            // code ZRL : 16 zeros
            i += 15;
            continue;
        } else if (value == 0) {
            // End of block
            break;
        }

        uint8_t m = value & 0x0f;
        uint8_t nb_zeros = value >> 4;

        while (nb_zeros != 0) {
            nb_zeros--;
            i++;
        }

        uint32_t bits;
        bitstream_read(stream, m, &bits, true);
        uint8_t index[m];

        for (uint8_t j = 0; j < m; j++) {
            index[m-j-1] = bits & 0x01;
            bits = bits >> 1;
        }

        int16_t coeff_value = magnitudeindex_to_value(index, m);
        vector[i] = coeff_value;
    }



    uint8_t quant_index = jpeg_get_frame_component_quant_index(jdesc, component);
    uint8_t *quant_table = jpeg_get_quantization_table(jdesc, quant_index);

    // quantification inverse
    iquant(vector, quant_table);

    // zig-zag inverse
    izigzag(vector);

    // transformée en cosinus discrète inverse
    idct(pixels, vector);
}

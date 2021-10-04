#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#include "jpeg_reader.h"
#include "bitstream.h"
#include "huffman.h"
#include "decode.h"
#include "upsampling.h"
#include "ycbcr_to_rgb.h"
#include "ppm.h"


// convertie une image jpeg en image pgm ou ppm
void jpeg_to_ppm(const char *filename)
{

    struct jpeg_desc *jdesc = jpeg_read(filename);
    struct bitstream  *stream = jpeg_get_bitstream(jdesc);

    uint16_t height = jpeg_get_image_size(jdesc, V);
    uint16_t width = jpeg_get_image_size(jdesc, H);

    uint8_t nb_components = jpeg_get_nb_components(jdesc);
    uint8_t size_MCU_H = jpeg_get_frame_component_sampling_factor(jdesc, H, 0);
    uint8_t size_MCU_V = jpeg_get_frame_component_sampling_factor(jdesc, V, 0);
    uint8_t C_sampling_factor_H = jpeg_get_frame_component_sampling_factor(jdesc, H, 1);
    uint8_t C_sampling_factor_V = jpeg_get_frame_component_sampling_factor(jdesc, V, 1);

    uint32_t nb_blocks_H = size_MCU_H*ceil((float) width / (8*size_MCU_H));
    uint32_t nb_blocks_V = size_MCU_V*ceil((float) height / (8*size_MCU_V));
    uint32_t nb_MCU_H = ceil((float) nb_blocks_H / size_MCU_H);
    uint32_t nb_MCU_V = ceil((float) nb_blocks_V / size_MCU_V);

    int16_t last_Y_DC_value = 0;
    int16_t last_Cb_DC_value = 0;
    int16_t last_Cr_DC_value = 0;

    uint8_t blocks[nb_blocks_V][nb_blocks_H][nb_components][64];

    char new_filename[50];

    get_new_filename(filename, new_filename, nb_components);

    uint8_t Y_to_decode = 1;
    if (size_MCU_H==2 && size_MCU_V==2) {
        Y_to_decode = 4;
    } else if ((size_MCU_H==2 && size_MCU_V==1) || (size_MCU_H==1 && size_MCU_V==2)) {
        Y_to_decode = 2;
    }

    uint8_t C_to_decode = 1;
    if ((size_MCU_H==2 && size_MCU_V==1) || (size_MCU_H==1 && size_MCU_V==2)) {
        if (C_sampling_factor_H == size_MCU_H && C_sampling_factor_V == size_MCU_V) {
            C_to_decode = 2;
        }
    } else if (size_MCU_H==2 && size_MCU_V==2) {
        if (C_sampling_factor_H==2 && C_sampling_factor_V==2) {
            C_to_decode = 4;
        } else if ((C_sampling_factor_H==2 && C_sampling_factor_V==1) ||
                (C_sampling_factor_H==1 && C_sampling_factor_V==2)) {
            C_to_decode = 2;
        }
    }

    for (uint32_t y = 0; y < nb_MCU_V; y++) {

        for (uint32_t x = 0; x < nb_MCU_H; x++) {

            // Y
            for (uint8_t i = 0; i < Y_to_decode; i++) {

                uint8_t component = 0;
                uint8_t pixels[64]={0};


                decode_block(jdesc, stream, component, &last_Y_DC_value,
                                &last_Cb_DC_value, &last_Cr_DC_value, pixels);


                if (size_MCU_V ==1) {
                    for (uint8_t j = 0; j < 64; j++) {
                        blocks[y][x*size_MCU_H+i][component][j] = pixels[j];
                    }
                } else if (size_MCU_H ==1) {
                    for (uint8_t j = 0; j < 64; j++) {
                        blocks[y*size_MCU_V+i][x][component][j] = pixels[j];
                    }
                } else {
                    if (i < 2) {
                        for (uint8_t j = 0; j < 64; j++) {
                            blocks[y*size_MCU_V][x*size_MCU_H+i][component][j] = pixels[j];
                        }
                    } else {
                        for (uint8_t j = 0; j < 64; j++) {
                            blocks[y*size_MCU_V+1][x*size_MCU_H+i-2][component][j] = pixels[j];
                        }
                    }
                }
            }


            if (nb_components == 1) {
                continue;
            }

            for (uint8_t k = 0; k < C_to_decode; k++) {

                // Cb Cr
                for (uint8_t component= 1; component < 3; component++) {

                    uint8_t pixels[64]={0};

                    decode_block(jdesc, stream, component, &last_Y_DC_value,
                                    &last_Cb_DC_value, &last_Cr_DC_value, pixels);


                    uint8_t pixels_TL[64]={0}; // top left block
                    uint8_t pixels_TR[64]={0}; // top right block
                    uint8_t pixels_BL[64]={0}; // bottom left block
                    uint8_t pixels_BR[64]={0}; // bottom right block

                    if (size_MCU_H == 1 && size_MCU_V == 1) {

                        for (uint8_t j = 0; j < 64; j++) {
                            blocks[y][x][component][j] = pixels[j];
                        }

                    } else if (size_MCU_H == 2 && size_MCU_V == 1) {
                        upsample_H(pixels, pixels_TL, pixels_TR);

                        for (uint8_t j = 0; j < 64; j++) {
                            blocks[y][x*size_MCU_H][component][j] = pixels_TL[j];
                            blocks[y][x*size_MCU_H+1][component][j] = pixels_TR[j];
                        }

                    } else if (size_MCU_H == 1 && size_MCU_V == 2) {
                        upsample_V(pixels, pixels_TL, pixels_BL);

                        for (uint8_t j = 0; j < 64; j++) {
                            blocks[y*size_MCU_V][x][component][j] = pixels_TL[j];
                            blocks[y*size_MCU_V+1][x][component][j] = pixels_BL[j];
                        }

                    } else {
                        upsample_H_and_V(pixels, pixels_TL, pixels_TR, pixels_BL,
                                            pixels_BR);

                        for (uint8_t j = 0; j < 64; j++) {
                            blocks[y*2][x*2][component][j] = pixels_TL[j];
                            blocks[y*2][x*2+1][component][j] = pixels_TR[j];
                            blocks[y*2+1][x*2][component][j] = pixels_BL[j];
                            blocks[y*2+1][x*2+1][component][j] = pixels_BR[j];
                        }
                    }
                }
            }
        }
    }

    uint8_t *Y_values = (uint8_t *) calloc(height*width, 1);
    uint8_t *Cb_values = (uint8_t *) calloc(height*width, 1);
    uint8_t *Cr_values = (uint8_t *) calloc(height*width, 1);
    uint8_t *R_values = (uint8_t *) calloc(height*width, 1);
    uint8_t *G_values = (uint8_t *) calloc(height*width, 1);
    uint8_t *B_values = (uint8_t *) calloc(height*width, 1);

    if (nb_components == 1) {

        for (uint32_t y = 0; y < nb_blocks_V; y++) {
            for (uint8_t j = 0; j < 8; j++) {
                for (uint32_t x = 0; x < nb_blocks_H; x++) {
                    for (uint8_t k = 0; k < 8; k++) {
                        if ((k+8*x)+width*(j+8*y) >= height*width) {
                            break;
                        }
                        Y_values[(k+8*x)+width*(j+8*y)] = blocks[y][x][0][k+8*j];
                    }
                }
            }
        }

    } else {

        for (uint32_t y = 0; y < nb_blocks_V; y++) {
            for (uint8_t j = 0; j < 8; j++) {
                for (uint32_t x = 0; x < nb_blocks_H; x++) {
                    for (uint8_t k = 0; k < 8; k++) {
                        if ((k+8*x)+width*(j+8*y) >= height*width) {
                            break;
                        }
                        Y_values[(k+8*x)+width*(j+8*y)] = blocks[y][x][0][k+8*j];
                        Cb_values[(k+8*x)+width*(j+8*y)] = blocks[y][x][1][k+8*j];
                        Cr_values[(k+8*x)+width*(j+8*y)] = blocks[y][x][2][k+8*j];
                    }
                }
            }
        }
    }

    if (nb_components == 3) {
        ycbcr_to_rgb(R_values, G_values, B_values, Y_values, Cb_values, Cr_values,
                        (uint32_t ) height*width);
        create_ppm(new_filename, R_values, G_values, B_values, height, width);
    } else {
        create_pgm(new_filename, Y_values, height, width);
    }

    free(Y_values);
    free(Cb_values);
    free(Cr_values);
    free(R_values);
    free(G_values);
    free(B_values);


    jpeg_close(jdesc);
}


int main(int argc, char **argv)
{
    if (argc != 2) {
	fprintf(stderr, "Usage: %s fichier.jpeg\n", argv[0]);
	return EXIT_FAILURE;
    }

    const char *filename = argv[1];
    jpeg_to_ppm(filename);

    return 0;
}

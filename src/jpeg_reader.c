#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "bitstream.h"
#include "huffman.h"
#include "jpeg_reader.h"


struct jpeg_desc {
    const char *filename;
    struct Sec_APP0 *sec_app0;
    struct Sec_DQT *sec_dqt;
    struct Sec_SOF0 *sec_sof;
    struct Sec_DHT *sec_dht;
    struct Sec_SOS *sec_sos;
    struct bitstream *stream;
};

struct Sec_APP0 {
    uint32_t marker_APP0;
    uint32_t length;
    uint32_t *data;
};

//struct Sec_Com{};
struct Sec_DQT {
    uint32_t marker_DQT;
    uint32_t length;
    uint32_t precision;
    uint8_t **quant_table;
    uint32_t quant_index;
    uint32_t nb_quant_tables;
};

struct Sec_SOF0 {
    uint32_t marker_SOF ;
    uint32_t length;
    uint32_t precision ;
    uint32_t height ;
    uint32_t width ;
    uint32_t nb_components;
    uint32_t iC_y;
    uint32_t Y_sampling_factor_H;
    uint32_t Y_sampling_factor_V;
    uint32_t iq_y;
    uint32_t iC_Cb;
    uint32_t Cb_sampling_factor_H;
    uint32_t Cb_sampling_factor_V;
    uint32_t iq_Cb;
    uint32_t iC_Cr;
    uint32_t Cr_sampling_factor_H;
    uint32_t Cr_sampling_factor_V;
    uint32_t iq_Cr;
};

struct Sec_DHT {
    uint32_t marker_DHT;
    uint32_t length;
    uint32_t type_AC_DC;
    uint32_t index;
    struct huff_table **table_Ac;
    struct huff_table **table_Dc;
};

struct Sec_SOS {
    uint32_t Marker_SOS;
    uint32_t length;
    uint8_t *iC;
    uint8_t *ih_DC;
    uint8_t *ih_AC;
};


// general
struct jpeg_desc *jpeg_read(const char *filename)
{
    //allocation de memeoire au jpeg
    struct jpeg_desc *jdesc = malloc(sizeof(struct jpeg_desc));
    jdesc ->filename = filename;
    struct bitstream *stream = bitstream_create(filename);
    if (stream == NULL) {
        exit(1);
    }
    jdesc ->stream = stream;
    uint32_t byte = 0;

    //lecture de la section APP0
    struct Sec_APP0 *app0 =  malloc(sizeof(struct Sec_APP0));
    bitstream_read(stream,16,&byte,false);
    if (byte != 0xffd8) {
        fprintf(stderr, "[src/jpeg_reader.c] Error : Invalid marker, expected 0xffd8, \
got 0x%x\n", byte);
        exit(1);
    }
    //   printf("%d",byte);
    //on lit les deux bit de marqueur app0
    bitstream_read(stream,16,&byte,false);
    while (byte>>4 == 0xffe) {
        (app0->marker_APP0) = byte;
        uint32_t erreur = 0;
        erreur =  app0->marker_APP0 - 65504;
        if ( 1<=erreur && erreur <=15){
	       fprintf(stderr, "[src/jpeg_reader.c] Application APP mode APP%u is \
not handled by this decoder (APP0 only)\n", erreur);
	       exit(1);
        } else if (erreur == 0){
              //la longueur de la seciton APP0
              bitstream_read(stream,16,&(app0->length),false);
              //on lit JFIF\0 (5octes)
              bitstream_read(stream,32,&byte,false);
              bitstream_read(stream,8,&byte,false);
              //on lit les données du app0
              uint32_t reste = 0;
              reste = app0->length -7 ;
              //le reste des bits
              app0->data = malloc(sizeof(uint32_t)* reste);
              for (uint32_t k =0;k<reste;k++) {
                  bitstream_read(stream,8,&(app0->data)[k],false);
              }
        }
        bitstream_read(stream,16,&byte,false);
    }

    // on passe a la setion commentaire s'il existe
    if (byte == 0xfffe) {
        bitstream_read(stream,16,&byte,false);
        //byte ici c'est la longueur de la section commentaire
        uint32_t stock_comment = 0;
        for (uint32_t k=0;k<byte-2;k++) {
            bitstream_read(stream,8,&stock_comment,false);
        }
        bitstream_read(stream,16,&byte,false);
   }

   // on passe a la section DQT
    struct Sec_DQT *dqt =  malloc(sizeof(struct Sec_DQT));
    dqt ->quant_table = malloc(4*sizeof(uint8_t *));
    for (uint8_t i = 0; i < 4; i++) {
        dqt ->quant_table[i] = NULL;
    }

    while (byte == 0xffdb) {
        dqt->marker_DQT = byte;
        //longueur de la section DQT
        bitstream_read(stream, 16, &(dqt->length), false);
        //la precision
        //dans le reste on eneleve 3bytes
        uint32_t reste_DQT = 0;
        reste_DQT = dqt->length -2;
        while (reste_DQT > 0) {
            bitstream_read(stream, 4, &(dqt->precision), false);
            bitstream_read(stream, 4, &(dqt->quant_index), false);
            (dqt ->quant_table)[dqt->quant_index] = malloc(sizeof(uint32_t)*reste_DQT);
            for (uint32_t i=0; i < 64; i++) {
                bitstream_read(stream, 8, &byte, false);
                (dqt->quant_table)[dqt->quant_index][i] = byte;
            }
            dqt->nb_quant_tables +=1;
            reste_DQT -= 65;
        }
        bitstream_read(stream, 16, &byte, false);
    }

    // on passe a la section SOF
    struct Sec_SOF0 *sof =  malloc(sizeof(struct Sec_SOF0));
    sof->marker_SOF = byte;
    if (sof ->marker_SOF != 0xffc0){
        fprintf(stderr, "[src/jpeg_reader.c] Only JPEG baseline sequential, DCT, Huffman, 8 bits \
is handled by this decoder\n");
        exit(1);
    }
    //longueur de la section SOF
    bitstream_read(stream, 16, &(sof->length), false);
    //precision
    bitstream_read(stream, 8, &(sof->precision), false);
    //la hauteur
    bitstream_read(stream, 16, &(sof->height), false);
    //la largeur
    bitstream_read(stream, 16, &(sof->width), false);
    //nombre des composantes
    bitstream_read(stream, 8, &(sof->nb_components), false);
    //cas d'une image gris
    bitstream_read(stream, 8, &(sof->iC_y), false);
    bitstream_read(stream, 4, &(sof->Y_sampling_factor_H), false);
    bitstream_read(stream, 4, &(sof->Y_sampling_factor_V), false);
    bitstream_read(stream, 8, &(sof->iq_y), false);

    if (sof->nb_components !=1) {
        bitstream_read(stream, 8, &(sof->iC_Cb), false);
        bitstream_read(stream, 4, &(sof->Cb_sampling_factor_H), false);
        bitstream_read(stream, 4, &(sof->Cb_sampling_factor_V), false);
        bitstream_read(stream, 8, &(sof->iq_Cb), false);
        bitstream_read(stream, 8, &(sof->iC_Cr), false);
        bitstream_read(stream, 4, &(sof->Cr_sampling_factor_H), false);
        bitstream_read(stream, 4, &(sof->Cr_sampling_factor_V), false);
        bitstream_read(stream, 8, &(sof->iq_Cr), false);
    }

    //on passe a la section DHT
    struct Sec_DHT *dht =  malloc(sizeof(struct Sec_DHT));
    dht ->table_Dc = malloc(4* sizeof(struct huff_table *));
    dht ->table_Ac = malloc(4* sizeof(struct huff_table *));
    for (uint8_t i = 0; i < 4; i++) {
        dht ->table_Dc[i] = NULL;
    }
    for (uint8_t i = 0; i < 4; i++) {
        dht ->table_Ac[i] = NULL;
    }
    bitstream_read(stream, 16, &byte, false);

    while (byte == 0xffc4) {
        bitstream_read(stream,16,&(dht ->length),false);
        uint32_t reste_DHT = (dht ->length) -2;
        while (reste_DHT > 0) {
            //3bits doivent avoir 000
            bitstream_read(stream,3,&byte,false);
            if (byte != 0) {
                return NULL;
            }
            uint16_t nb_bytes_read = 0;
            //on stocck dans byte le type du table
            bitstream_read(stream,1,&(dht ->type_AC_DC),false);
            //si l'indice =0 donc le type de la table est DC
            if (dht->type_AC_DC== 0) {
                bitstream_read(stream,4,&(dht ->index),false);
                (dht ->table_Dc)[dht ->index] = huffman_load_table(stream,&nb_bytes_read);
            }
            else {
                bitstream_read(stream,4,&(dht ->index),false);
                (dht ->table_Ac)[dht ->index] = huffman_load_table(stream,&nb_bytes_read);
            }
            reste_DHT -= nb_bytes_read + 1;
        }
        bitstream_read(stream,16,&byte,false);
    }

    // on passe a la section SOS
    struct Sec_SOS *sos =  malloc(sizeof(struct Sec_SOS));
    sos ->Marker_SOS = byte;
    bitstream_read(stream,16,&(sos->length),false);
    //on stock le nombre de composantes
    bitstream_read(stream,8,&byte,false);
    sos->iC = malloc(sizeof(uint8_t)* sof->nb_components);
    sos->ih_AC = malloc(sizeof(uint8_t)* sof->nb_components);
    sos->ih_DC = malloc(sizeof(uint8_t)* sof->nb_components);

    for (uint32_t i=0;i<(sof->nb_components);i++) {
        bitstream_read(stream,8,&byte,false);
        (sos->iC)[i] = byte;
        bitstream_read(stream,4,&byte,false);
        (sos->ih_DC)[i] = byte;
        bitstream_read(stream,4,&byte,false);
        (sos->ih_AC)[i] = byte;
        byte = 0;
    }

    jdesc ->sec_sos = sos;
    jdesc ->sec_sof = sof;
    jdesc ->sec_app0 = app0;
    jdesc ->sec_dqt = dqt;
    jdesc ->sec_dht = dht;
    return jdesc;

}


void jpeg_close(struct jpeg_desc *jpeg)
{
    // on libere la section APP0
    free((jpeg->sec_app0)->data);
    free(jpeg->sec_app0);
    // on libere la section DQT
    for (uint8_t k=0;k<4;k++) {
        if (((jpeg->sec_dqt)->quant_table)[k] != NULL) {
            free(((jpeg->sec_dqt)->quant_table)[k]);
        }
    }
    free(jpeg->sec_dqt->quant_table);
    free(jpeg->sec_dqt);
    //on libere la section sof
    free(jpeg->sec_sof);
    //on libere la section dht
    for (uint8_t k=0;k<4;k++) {
        if ((((jpeg->sec_dht)->table_Dc)[k] != NULL)) {
            huffman_free_table(((jpeg->sec_dht)->table_Dc)[k]);
        }
        if (((jpeg->sec_dht)->table_Ac)[k] != NULL) {
            huffman_free_table(((jpeg->sec_dht)->table_Ac)[k]);
        }
    }
    free((jpeg->sec_dht)->table_Ac);
    free((jpeg->sec_dht)->table_Dc);
    free(jpeg->sec_dht);
    //on libere la section sos
    free((jpeg->sec_sos)->iC);
    free((jpeg->sec_sos)->ih_AC);
    free((jpeg->sec_sos)->ih_DC);
    free(jpeg->sec_sos);
    bitstream_close(jpeg->stream);
    free(jpeg);
}


char *jpeg_get_filename(const struct jpeg_desc *jpeg)
{
    char *filename = NULL;
    strcpy(filename, jpeg->filename);
    return filename;
}

// access to stream, placed just at the beginning of the scan raw data
struct bitstream *jpeg_get_bitstream(const struct jpeg_desc *jpeg)
{
    uint32_t byte =0 ;
    for (uint8_t k=0;k<3;k++) {
        bitstream_read(jpeg->stream,8,&byte,false);
    }
    return jpeg->stream;
}


// from DQT
uint8_t jpeg_get_nb_quantization_tables(const struct jpeg_desc *jpeg)
{
    return (jpeg->sec_dqt)->nb_quant_tables;
}

uint8_t *jpeg_get_quantization_table(const struct jpeg_desc *jpeg, uint8_t index)
{
    return (jpeg->sec_dqt)->quant_table[index];
}


// from DHT.....
uint8_t jpeg_get_nb_huffman_tables(const struct jpeg_desc *jpeg, enum sample_type acdc)
{
    uint8_t compteur =0;
    if (acdc == 0) {
        for (uint32_t k=0;k<4;k++) {
            if ((jpeg->sec_dht)->table_Ac[k] != NULL) {
                compteur+=1;
            }
        }
    } else {
        for (uint32_t k=0;k<4;k++) {
            if ((jpeg->sec_dht)->table_Dc[k] != NULL) {
            compteur+=1;
            }
        }
    }
    return compteur;
}


struct huff_table *jpeg_get_huffman_table(const struct jpeg_desc *jpeg,
                                enum sample_type acdc, uint8_t index)
{
    if (acdc ==0) {
        return ((jpeg->sec_dht)->table_Dc)[index];
    } else {
        return ((jpeg->sec_dht)->table_Ac)[index];
    }
}



// from Frame Header SOF0
uint16_t jpeg_get_image_size(struct jpeg_desc *jpeg, enum direction dir)
{
    if (dir == 0) {
        return (jpeg->sec_sof)->width;
    }
    else {
        return (jpeg->sec_sof)->height;
    }
}


uint8_t jpeg_get_nb_components(const struct jpeg_desc *jpeg)
{
    return (jpeg ->sec_sof)->nb_components;
}



uint8_t jpeg_get_frame_component_id(const struct jpeg_desc *jpeg,
                                    uint8_t frame_comp_index)
{
    if (frame_comp_index ==0) {
        return (jpeg->sec_sof)->iC_y;
    }
    else if (frame_comp_index ==1) {
        return (jpeg->sec_sof)->iC_Cb;
    }
    else {
        return (jpeg->sec_sof)->iC_Cr;
    }
}




uint8_t jpeg_get_frame_component_sampling_factor(const struct jpeg_desc *jpeg,
                            enum direction dir,uint8_t frame_comp_index)
{
    if (dir == 0) {
        if (frame_comp_index ==0) {
            return (jpeg->sec_sof)->Y_sampling_factor_H;
        } else if (frame_comp_index ==1) {
            return (jpeg->sec_sof)->Cb_sampling_factor_H;
        } else {
            return (jpeg->sec_sof)->Cr_sampling_factor_H;
        }
    } else {
        if (frame_comp_index ==0) {
            return (jpeg->sec_sof)->Y_sampling_factor_V;
        }
        else if (frame_comp_index ==1) {
            return (jpeg->sec_sof)->Cb_sampling_factor_V;
        } else {
            return (jpeg->sec_sof)->Cr_sampling_factor_V;
        }
    }
}




uint8_t jpeg_get_frame_component_quant_index(const struct jpeg_desc *jpeg,
                                            uint8_t frame_comp_index)
{
    if (frame_comp_index ==0) {
        return (jpeg->sec_sof)->iq_y;
    } else if (frame_comp_index ==1) {
        return (jpeg->sec_sof)->iq_Cb;
    } else {
        return (jpeg->sec_sof)->iq_Cr;
    }
}



// from Scan Header SOS
uint8_t jpeg_get_scan_component_id(const struct jpeg_desc *jpeg,uint8_t scan_comp_index)
{
    if (scan_comp_index ==0) {
        return (jpeg->sec_sof)->iC_y;
    } else if (scan_comp_index ==1) {
        return (jpeg->sec_sof)->iC_Cb;
    } else {
        return (jpeg->sec_sof)->iC_Cr;
    }
}



uint8_t jpeg_get_scan_component_huffman_index(const struct jpeg_desc *jpeg,
                    enum sample_type sample_type,uint8_t scan_comp_index)
{
    if (sample_type ==0){
        return ((jpeg->sec_sos)->ih_DC)[scan_comp_index];
    } else {
        return ((jpeg->sec_sos)->ih_AC)[scan_comp_index];
   }
}


/*
int main(int argc, char **argv)
{
    if (argc != 2) {frame_comp_index
	fprintf(stderr, "Usage: %s fichier.jpeg\n", argv[0]);
	return EXIT_FAILURE;
    }
    const char *filename = argv[1];
    struct jpeg_desc *jpeg  = jpeg_read(filename);
    printf("%s\n", jpeg_get_filename(jpeg));
    printf("%d\n", jpeg_get_nb_quantization_tables(jpeg));
    printf("%d\n", jpeg_get_nb_components(jpeg));
    jpeg_close(jpeg);
    return 0;
}
*/

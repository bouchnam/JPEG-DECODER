#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include "bitstream.h"


struct bitstream {
  FILE *file;
  uint8_t buffer; // octect en cours de lecture
  uint8_t position; // position du bit dans buffer
};


// la fonction crée un flux, positionné au début du fichier filename
struct bitstream *bitstream_create(const char *filename)
{
    FILE *file = fopen(filename, "rb");
    struct bitstream *stream = malloc(sizeof(struct bitstream));

    if (file == NULL){
        fprintf(stderr, "[src/bitstream.c] Unable to create stream from file\n");
        return NULL;
    }

    fread(&stream->buffer, sizeof(uint8_t), 1, file);
    if (stream->buffer == 0) {
        return NULL;
    }
    stream->file = file;
    stream->position = 0;
    return stream;
}



// La fonction bitstream_read permet de lire des bits et d'avancer dans le flux
uint8_t bitstream_read(struct bitstream *stream, uint8_t nb_bits, uint32_t *dest,
                        bool discard_byte_stuffing)
{
    if (nb_bits > 32){
        return 0;
    }

    if (nb_bits == 0){
        return 0;
    }


    uint8_t nbr_bits_read = 0;
    *dest = 0;

    for (uint32_t i = 0; i < nb_bits; i++) {
        write_new_bit(stream, dest);
        nbr_bits_read += 1;

        if (stream->position == 8) {
            // byte stuffing
            if (!discard_byte_stuffing){
                fread(&stream->buffer, sizeof(uint8_t), 1, stream->file);
                stream->position = 0;
            } else {
                if (stream->buffer == 0xff) {
                    fread(&stream->buffer, sizeof(uint8_t), 1, stream->file);
                    stream->position = 0;
                    if (stream->buffer == 0x00) {
                        fread(&stream->buffer, sizeof(uint8_t), 1, stream->file);
                        stream->position = 0;
                    }
                } else {
                    fread(&stream->buffer, sizeof(uint8_t), 1, stream->file);
                    stream->position = 0;
                }
            }
        }
    }
    return nbr_bits_read;

}


// fonction retourne True si le flux a été entièrement parcouru el false sinon.
bool bitstream_is_empty(struct bitstream *stream)
{
    if (!feof(stream->file) && stream->position == 8) {
        return true;
    } else {
        return false;
    }
}

// fonction permettant d'ecrire le prochain bit et le stock dans dest
void write_new_bit(struct bitstream *stream, uint32_t *dest)
{
    uint8_t new_bit;
    *dest = (*dest << 1);
    new_bit = stream->buffer >> (7 - stream->position) & 1; // on recupere le premier bit non lus dans le buffer
    *dest = *dest + new_bit; // on ajoute le nouveau bit a droite
    stream->position += 1;
}


// la fonction sert à fermer le fichier et à désallouer proprement le flux
void bitstream_close(struct bitstream *stream)
{
    fclose(stream->file);
    free(stream);
}




/*int main() {
    struct bitstream *stream = bitstream_create("albert.jpg");
    //printf("%x\n", stream->buffer);
    //printf("%x\n", stream->suivant);
    uint32_t bits;
    bitstream_read(stream, 2, &bits, false);
    //printf("%u\n", nb_read);
    bitstream_close(stream);
    return 0;
}*/

#include <stdint.h>
#include <stdio.h>


// crée une image pgm et écrit les valeurs des pixels
void create_pgm(const char *new_filename,  uint8_t *pixels, uint16_t height,
                uint16_t width)
{

    FILE *file;
    file = fopen(new_filename, "wb");

    fprintf(file, "P5\n%u %u\n255\n", width, height);

    for (uint32_t y = 0; y < (uint32_t) height*width; y++) {
        fwrite(&pixels[y], 1, 1, file);
    }

    fclose(file);
}


// crée une image ppm et écrit les valeurs rgb des pixels
void create_ppm(const char *new_filename, uint8_t *r, uint8_t *g, uint8_t *b,
                 uint16_t height, uint16_t width)
{

    FILE *file;
    file = fopen(new_filename, "wb");

    fprintf(file, "P6\n%u %u\n255\n", width, height);

    for (uint32_t y = 0; y < (uint32_t) height*width; y++) {
        fwrite(&r[y], 1, 1, file);
        fwrite(&g[y], 1, 1, file);
        fwrite(&b[y], 1, 1, file);
    }

    fclose(file);
}

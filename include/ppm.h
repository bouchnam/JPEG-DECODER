#include <stdint.h>


// crée une image pgm et écrit les valeurs des pixels
void create_pgm(const char *new_filename,  uint8_t *pixels, uint16_t height,
                uint16_t width);


// crée une image ppm et écrit les valeurs rgb des pixels
void create_ppm(const char *new_filename, uint8_t *r, uint8_t *g, uint8_t *b,
                 uint16_t height, uint16_t width);

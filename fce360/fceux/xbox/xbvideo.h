#include <xtl.h>

void FCEUD_GetPalette(unsigned char i, unsigned char *r, unsigned char *g,
        unsigned char *b) {
    *r = pcpalette[i].r;
    *g = pcpalette[i].g;
    *b = pcpalette[i].b;
}
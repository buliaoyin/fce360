// prototypes
#include <stdint.h>
void hq2x_32( unsigned char * srcPtr, DWORD srcPitch, unsigned char * dstPtr, int width, int height);
void hq3x_32( uint32_t * sp, uint32_t * dp, int Xres, int Yres );
void hqxInit(void);
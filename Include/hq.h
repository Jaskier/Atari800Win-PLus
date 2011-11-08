#include "config.h"

extern int   LUT16to32[65536];
extern int   RGBtoYUV[65536];
extern int   YUV1, YUV2;

#define Ymask 0x00FF0000
#define Umask 0x0000FF00
#define Vmask 0x000000FF
#define trY   0x00300000
#define trU   0x00000700
#define trV   0x00000006

void InitLUTs(void);
void hq2x_32( unsigned char * pIn, unsigned char * pOut, int Xres, int Yres, int BpL );
void hq3x_32( unsigned char * pIn, unsigned char * pOut, int Xres, int Yres, int BpL );
void hq4x_32( unsigned char * pIn, unsigned char * pOut, int Xres, int Yres, int BpL );


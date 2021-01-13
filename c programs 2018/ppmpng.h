#ifndef PPMPNG_H_
#define PPMPNG_H_

typedef struct {
	 unsigned char rgb[3];//red,green,blue;
} PPMPixel;

typedef struct {
	 int x, y;
	 PPMPixel *data;
} PPMImage;

#define RGB_COMPONENT_COLOR 255

PPMImage *readPPM(const char *filename);

void writePPM(const char *filename, PPMImage *img);

#endif
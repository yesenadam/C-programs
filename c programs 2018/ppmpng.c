#include<stdio.h>
#include<stdlib.h>
#include "ppmpng.h"

#define RGB_COMPONENT_COLOR 255

PPMImage *readPPM(const char *filename) {
	char buff[16];
	PPMImage *img;
	FILE *fp;
	int c, rgb_comp_color;
	fp = fopen(filename, "rb");
	if (!fp) {
	  fprintf(stderr, "Unable to open file '%s'\n", filename);
	  exit(1);
	}
	if (!fgets(buff, sizeof(buff), fp)) {	//read image format
	  perror(filename);
	  exit(1);
	}
	if (buff[0] != 'P' || buff[1] != '6') { //check the image format
		 fprintf(stderr, "Invalid image format (must be 'P6')\n");
		 exit(1);
	}
	//alloc memory for image
	img = (PPMImage *)malloc(sizeof(PPMImage));
	if (!img) {
		 fprintf(stderr, "Unable to allocate memory\n");
		 exit(1);
	}
	//check for comments
	c = getc(fp);
	while (c == '#') {
	while (getc(fp) != '\n') ;
		 c = getc(fp);
	}
	ungetc(c, fp);
	//read image size information
	if (fscanf(fp, "%d %d", &img->x, &img->y) != 2) {
		 fprintf(stderr, "Invalid image size (error loading '%s')\n", filename);
		 exit(1);
	}
	//read rgb component
	if (fscanf(fp, "%d", &rgb_comp_color) != 1) {
		 fprintf(stderr, "Invalid rgb component (error loading '%s')\n", filename);
		 exit(1);
	}
	//check rgb component depth
	if (rgb_comp_color!= RGB_COMPONENT_COLOR) {
		 fprintf(stderr, "'%s' does not have 8-bits components\n", filename);
		 exit(1);
	}
	while (fgetc(fp) != '\n') ;
	//memory allocation for pixel data
	img->data = (PPMPixel*)malloc(img->x * img->y * sizeof(PPMPixel));
	if (!img) {
		 fprintf(stderr, "Unable to allocate memory\n");
		 exit(1);
	}
	//read pixel data from file
	if (fread(img->data, 3 * img->x, img->y, fp) != img->y) {
		 fprintf(stderr, "Error loading image '%s'\n", filename);
		 exit(1);
	}
	fclose(fp);
	return img; 
}

void writePPM(const char *filename, PPMImage *img) {
	FILE *fp;
	fp = fopen(filename, "wb");
	if (!fp) {
		 fprintf(stderr, "Unable to open file '%s'\n", filename);
		 exit(1);
	}
	fprintf(fp, "P6\n"); //image format
	fprintf(fp, "%d %d\n",img->x,img->y);	  //image size
	fprintf(fp, "%d\n",RGB_COMPONENT_COLOR);	// rgb component depth
	fwrite(img->data, 3 * img->x, img->y, fp);	  // pixel data
	fclose(fp);
}

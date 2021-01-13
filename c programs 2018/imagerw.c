#include "ppmpng.h"

void WritetoCArray(PPMImage *img){//, unsigned char *a) {
	int i=0,x,y,k;
	for (y=0;y<HEI;y++){
		for (x=0;x<WID;x++){
			for (k=0;k<3;k++)
				a[x][y][k]=img->data[i].rgb[k];
			i++;
		}	
	}
}

void WriteBack(PPMImage *img) {
	int i=0,x,y,k;
	for (y=0;y<HEI;y++){
		for (x=0;x<WID;x++){
			for (k=0;k<3;k++)
				img->data[i].rgb[k]=a[x][y][k];
			i++;
		}	
	}
}

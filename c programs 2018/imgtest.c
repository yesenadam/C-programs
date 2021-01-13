//imgtest.c
//22 sept 2018
//
//compile with:
//bash$ c99 -o imgtest ppmpng.c imgtest.c
//bash$ ./imgtest

/* TO DO:
- enable loading different images into different arrays, not just a[][][].

*/
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "ppmpng.h"
#define WID 2048 //must be exact size of PNG image
#define HEI 2048
char fname[50]="tiger2048"; //PNG only.
unsigned char a[WID][HEI][3];

void DoStuff() {
//image processing on a[][][] array
printf("sizeof array : %zu\n", sizeof a);
int i,k;
	for (i=0;i<WID;i++){
		for (k=0;k<3;k++) a[i][i][k]=255;
	}
}

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

int main() {
//how I want to write it:
	char pngtopnmstr[80],pnmtopngstr[80], pngname[50],outpngname[50];
	PPMImage *image;
	sprintf(pngname, "%s.png", fname);
	sprintf(outpngname, "%s-test.png", fname);
	sprintf(pngtopnmstr, "pngtopnm %s >temp.ppm", pngname);
	sprintf(pnmtopngstr,"pnmtopng temp.ppm >%s",outpngname);
//imread(fn,a)
	system(pngtopnmstr);
	image = readPPM("temp.ppm");
	WritetoCArray(image);
	DoStuff();
//	imsave(fn,a)
	WriteBack(image);
	writePPM("temp.ppm",image);
	system(pnmtopngstr);
}
#include "libmathsart.h"
#include <math.h>
//#include <stdlib.h>
#include <stdio.h>

//NB this is for elsewhere, not for here:
//typedefstruct {
//    int width;
//    int height;
//    image_ptr input;
//    image_ptr output;
//} diffuse_s;

//from #diffusion.pyx

//#uses version of the algorithm in:
//#"A GPU Laplacian Solver for Diffusion Curves and Poisson Image Editing"

//outputs a number of frames... after about 30 it's usually near-finished...
//maybe just output 1? need these tools to be versatile as possible...
//but still, need to e.g. for frame 25, do all stages... it's wasteful to run
//through all stages over and over.... hmm.
//
//
	typedef struct {
		int x;
		int y; 
		} intpt;
		
int WID, HEI, HALFWID, HALFHEI, MAXPTS, NUMLOOPS;
float XYRATIO, INITRAD, MAXT;
int sum[3], denoms,numpts=0;
char* fname;

//void FindDistAndSetInitPixel(int x, int y, //intpt* list, float** dist,
// intpt list[], int WID, int HEI,float dist[WID][HEI],
//    image_ptr cin, image_ptr car) {// nogil:
////#improved: 1. dont use list.. search down columns. (so dont do this in a function like this)
////#use nearest pt from above pixel!... dist to this point is at most 1 further than that.
////#(tricky if the point nearest to prev is the current one....)
////#2. use lookup table for pyth dist.

void ma_diffuse(diffuse_s d) {
//it has 3 arrays, cin, car, output. ..and input
//seems like OUTPUT is just numpy version of car.
//seems like INPUT is just numpy version of cin.
HEI=d.height;
WID=d.width;
HALFWID=WID/2;
HALFHEI=HEI/2;
MAXPTS=600000;//#10000 #max coloured points allowed in input image
NUMLOOPS=30;
XYRATIO=((float) WID)/((float) HEI);//1280.0/720.0
INITRAD=5.0;
MAXT=355.0;//#35.0#350.0
	float dist[WID][HEI];
	image_ptr cin=d.input;
	image_ptr car=  ma_init_image(WID,HEI);
	image_ptr out=d.output;
	float dfrac=0.92;
	int col[6];
	intpt list[MAXPTS]; //make this width*height, so never overflows
    int i,j,k;//
    int intd, loops;
    int which,xis,yj,num;
    float sqd,minsqdist;
	printf("Starting..\n");
	//so cin is all (0,0,0) except for the coloured pixels (which will stay the same)
	//cin never changes after initial set.
	ma_write_png_file("initpts.png", cin);
	//make list of coloured points
	printf("Making list\n");
	for (i=0;i<WID;i++) 
		for (j=0;j<HEI;j++) 
			if (cin[i][j][0]+cin[i][j][1]+cin[i][j][2]>0) { //coloured pix
				list[numpts]=(intpt) {i,j};
				numpts+=1;
			}
	printf("%d\n",numpts);
	printf("Make voronoi\n");
	for (i=0;i<WID;i++) 
		for (j=0;j<HEI;j++) {
			for (k=0;k<3;k++) 
				car[i][j][k]=cin[i][j][k];
			if (car[i][j][0]>0)
				continue;
			if (car[i][j][1]>0)
				continue;
			if (car[i][j][2]>0)
				continue;
            minsqdist=10000000;
            for (num=0;num<numpts;num++) { //cycle through list of coloured pts in cin
                xis=i-list[num].x;
                xis*=xis;
                if (list[num].x>i && minsqdist<xis) //already too far right to find min pt so quit
                    break;
                yj=j-list[num].y;
                sqd=xis+yj*yj;
                if (sqd<minsqdist) {
                    minsqdist=sqd;
                    which=num;
                }
            }
            dist[i][j]=sqrt(minsqdist);
            for (k=0;k<3;k++) 
                car[i][j][k]=cin[list[which].x][list[which].y][k];
        } //j
	printf("Output\n");
	ma_write_png_file("diff0.png",car);
//	misc.imsave('/Users/admin/diffusion/diff0.png',output) //car
	for (loops=1;loops<NUMLOOPS+1;loops++){ // in xrange(1,NUMLOOPS+1):
		printf("%d %g\n",loops,dfrac);
		for (i=0;i<WID;i++) 
			for (j=0;j<HEI;j++) {
				if (cin[i][j][0]>0)
					continue;
				if (cin[i][j][1]>0)
					continue;
				if (cin[i][j][2]>0)
					continue;
				intd=(int) dfrac*dist[i][j];
				sum[0]=0; 
				sum[1]=0; 
				sum[2]=0;
				denoms=0; //will be 4 if all circle pts are within img
				if (i+intd<WID) {
					for (k=0;k<3;k++) 
                        sum[k]+=car[i+intd][j][k];
                    denoms+=1;
				}
//					AddPt(i+intd,j, car);
				if (i-intd>=0) {
					for (k=0;k<3;k++) 
                        sum[k]+=car[i-intd][j][k];
                    denoms+=1;
				}
//					AddPt(i-intd,j, car);
				if (j+intd<HEI) {
					for (k=0;k<3;k++) 
                        sum[k]+=car[i][j+intd][k];
                    denoms+=1;
				}
//					AddPt(i,j+intd, car);
				if (j-intd>=0) {
					for (k=0;k<3;k++) 
                        sum[k]+=car[i][j-intd][k];
                    denoms+=1;
				}
//					AddPt(i,j-intd, car);
				//write average to output
				for (k=0;k<3;k++) 
					out[i][j][k]=sum[k]/denoms;
            } //j
            
             sprintf(fname,"diff%03d.png",loops);
//		fn='/Users/admin/diffusion/diff%03d.png' % loops
	ma_write_png_file(fname,out);
//		misc.imsave(fn,output)
		dfrac*=0.95;
		for (i=0;i<WID;i++) //NB!! make a library func for copying image arrays.
		for (j=0;j<HEI;j++) {
			for (k=0;k<3;k++) 
				car[i][j][k]=out[i][j][k];
    } //loops
}
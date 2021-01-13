#include <stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>
#include "ppmpng.h"

#define _(x,y) for (x=0;x<(y);x++)
typedef struct {
	int col[3]; //colour of the superpixel
	int x,y;	//loc of superpixel centre
	int numpts; //num of pts in superpixel
	int minx,maxx,miny,maxy; //stores the extremes of superpixels
} cen;
enum {WID=513,HEI=513, GAP=20,  //720,HEI=960};
	SMOOTHED=1}; //1=weighted average of each pix, 0=just use val of swapped closest
PPMImage *image;
char name2[100];
unsigned char a[WID][HEI][3]={};
char fn[50] = "~/tiger-09copy"; //adamtree";

float sq(int n) { return n*n; }

void readpng(char *fn) {
	int i = 0,x,y,k;
	char pngtopnmstr[80];
	sprintf(pngtopnmstr, "pngtopnm %s.png >temp.ppm", fn);
	system(pngtopnmstr);
	image = readPPM("temp.ppm");
	_(y,HEI)
		for (x=0;x<WID;x++,i++)
			_(k,3) a[x][y][k] = image->data[i].rgb[k];
}	

void savepng(char *fn) {
	char pnmtopngstr[80]; 
	int i = 0,x,y,k;
	_(y,HEI) for (x=0;x<WID;x++,i++)
		_(k,3) image->data[i].rgb[k] = a[x][y][k];
	writePPM("temp.ppm",image);
	sprintf(pnmtopngstr,"pnmtopng temp.ppm >%s.png",fn);
	system(pnmtopngstr);
}
int main(){
//char fn[50] = "~/adamtree";
	readpng(fn); //**** NB fix this!! dont need to read image. just add
	//mallocs from ppmpng.c
	int wid,hei,c,superpixels; 
	FILE *f, *fopen();
	f=fopen("data.txt","r");
	fscanf(f,"%d,%d,%d\n",&superpixels,&wid,&hei);
	if (wid!=WID || hei!=HEI){
		printf("WID or HEI constants dont match vals in file.");
		return 0;
	}
	cen SP[superpixels];
	_(c,superpixels) {
		fscanf(f,"%d,%d,%d,%d,%d\n",&SP[c].x,&SP[c].y,
			&SP[c].col[0],&SP[c].col[1],&SP[c].col[2]);
	}
	fclose(f);
//I guess, save with different combinations.....
	//maybe have a NEAREST_N, e.g. 6, saves EVERY combination up to N=6?
	//e.g. [0,1] (i.e. normal), [1,0], [0,1,2] (normal), [0,2,1] etc
	//n! I guess... sum of N!
	//need closest N SPs to each pixel, in order.
	//Q. test all, or only those within some dist? well...
	//I guess within 3 or 4x starting radius should find them all, amply.
	//but depends on N. hmm N=1, R=1. N=4, R=2. ok, R=sqrt(N) should be fine.

	int i,j,k,e,r,p,N=6; //number of closest SPs to find
	//NB doesnt work for N<4..why not????????????
	int closest[N]; //clo[y]=yth closest SP to pixel
	float dist[N], d,denom,temp,fcol;
	int searchRad=4;//4;
	int cl; // <==== 'cl'th closest used as closest below.
	//NB sort SPs by increasing x, then can make search faster.
	_(cl,N) { //for now, save each different "clth closest as closest" pic
		_(i,wid) {// means "for (i=0;i<wid;i++)"
			printf("%d %d\n",cl,i);
			_(j,hei) {
				_(k,N) dist[k]=6000000;
				_(c,superpixels) {
					//only check those kind of near i,j.
					//like for N=6,within 3x GAP radius should be plenty.
					if (SP[c].x-i>searchRad*GAP || i-SP[c].x>searchRad*GAP ||
						SP[c].y-j>searchRad*GAP || j-SP[c].y>searchRad*GAP)
						continue;
					d=sq(i-SP[c].x)+sq(j-SP[c].y);
					_(r,N-1) 
						if (d<dist[r]) {
							for (e=N-1;e>r;e--) {
								dist[e]=dist[e-1]; //e.g. d5=d4
								closest[e]=closest[e-1];
							}
							dist[r]=d;
							closest[r]=c;
							break;
				}		}
				if (dist[N-1]==6000000) {
					printf("Increase SP search radius!");
					return 1;
				}
				if (SMOOTHED) {
					denom=0;
					_(p,N) {
						dist[p] = dist[p]>0.1 ? 1/dist[p] : 10; //so centre pixels draw ok.
						denom += dist[p];
					}
					//swap 0th closest and cl-th closest
					if (cl>0) {
						temp=closest[0];
						closest[0]=closest[cl];
						closest[cl]=temp;
					}
					_(k,3) {
						fcol=0;
						_(p,N) fcol+=dist[p]*SP[closest[p]].col[k];
						fcol/=denom;
						a[i][j][k]=round(fcol);
					}
				} else
					_(k,3) a[i][j][k]=SP[closest[cl]].col[k];
		}	}	
	snprintf(name2,sizeof(name2),"~/shufvor-%dof%d",cl,N);
	savepng(name2);
}	}

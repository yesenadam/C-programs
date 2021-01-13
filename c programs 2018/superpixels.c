#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>
#include "ppmpng.h"
#define WID 2048 //must be exact size of PNG image
#define HEI 2048
char fn[50]="~/tiger2048"; //PNG only.
unsigned char a[WID][HEI][3],b[WID][HEI][3];
PPMImage *image;
#define MAXK 5000 //not going to be more than this
#define LOOPS 50//100//100
#define BOUNDARIES 1 //0=don't draw superpixel boundaries in black. 1=draw
enum {M=60, //max colour distance allowed in a SP.
//max colour distance.. I think 3*255^2 with raw RGB and sq dist
//increasing M makes physical distance more important than colour dist
//i.e. more regular, compact SPs
	GAP=30, //init gap between SP centres
	SEARCHRADMULT=1, //1=usual search of a 2Sx2S square
	MAXDIFF=5000 //for final merging part
	};
typedef struct {
	int col[3];
	int x,y;
	int numpts;
	int minx,maxx,miny,maxy //stores the extremes of superpixels
} cen;
typedef struct{
	float minDist;
	int closestCen;
} ci;
cen cent[MAXK];
cen *Ct;
ci cenInfo[WID][HEI],connectionMap[WID][HEI];
int NUMCENTS;
char adjMat[MAXK][MAXK];
int searchRad=SEARCHRADMULT*GAP;
enum {	DELETED=-1,NEIGHBOUR=1,ERASED=-1};

void readpng(char *fn) {
	int i=0,x,y,k;
	char pngtopnmstr[80];
	sprintf(pngtopnmstr, "pngtopnm %s.png >temp.ppm", fn);
	system(pngtopnmstr);
	image = readPPM("temp.ppm");
	for (y=0;y<HEI;y++){
		for (x=0;x<WID;x++){
			for (k=0;k<3;k++)
				a[x][y][k]=image->data[i].rgb[k];
			i++;
		}	
	}
}

void savepng(char *fn) {
	char pnmtopngstr[80];
	int i=0,x,y,k;
	for (y=0;y<HEI;y++){
		for (x=0;x<WID;x++){
			for (k=0;k<3;k++)
				image->data[i].rgb[k]=a[x][y][k];
			i++;
		}	
	}
	writePPM("temp.ppm",image);
	sprintf(pnmtopngstr,"pnmtopng temp.ppm >%s.png",fn);
	system(pnmtopngstr);
}

void EraseConnectedPixels(int thisSP,int i,int j) {
	//turns all points to -1 except those unreachable from SP centres.
	if (i<1 || i>WID-2 || j<1 || j>HEI-2)
		return;
	if (connectionMap[i][j].closestCen==ERASED)
		return;
	if 	(connectionMap[i][j].closestCen!=thisSP) //only process if in same pixel
		return;
	connectionMap[i][j].closestCen=ERASED;
	EraseConnectedPixels(thisSP,i+1,j);
	EraseConnectedPixels(thisSP,i-1,j);
	EraseConnectedPixels(thisSP,i,j+1);
	EraseConnectedPixels(thisSP,i,j-1);
}

void EmptyCentreData() {
	int c,k;
	for (c=0;c<NUMCENTS;c++) {
		Ct=&cent[c];
		for (k=0;k<3;k++)
			Ct->col[k]=0; //do this without a loop?
		Ct->numpts=0;
		Ct->x=0;
		Ct->y=0;
}	}

void CalcClusterCentresAndColours() {
	int i,j,cc,k,c;
	for (i=0;i<WID;i++) {
		for (j=0;j<HEI;j++) {
			cc=cenInfo[i][j].closestCen;
			Ct=&cent[cc];
			for (k=0;k<3;k++)
				Ct->col[k]+=b[i][j][k];
			Ct->numpts+=1;
			Ct->x+=i;
			Ct->y+=j;
		}
	}
	for (c=0;c<NUMCENTS;c++) {
		Ct=&cent[c];
		Ct->x/=Ct->numpts;
		Ct->y/=Ct->numpts;
		for (k=0;k<3;k++)
			Ct->col[k]/=Ct->numpts;
}	}

float sq(int n) {
	return n*n;
}
int isq(int n) {
	return n*n;
}
float fsq(float n) {
	return n*n;
}

void AssignDistsToPixelsNearCentres() {
	float fM=(float)M;
	float fsearchRad=(float)SEARCHRADMULT*GAP;//searchRad;
	float mOverSSq=fsq(fM/fsearchRad); //*(fM/fsearchRad);

	int c,i,j,k;
	float squaredColourDist,squaredDist,dist;
	for (i=0;i<WID;i++)
		for (j=0;j<HEI;j++)
			cenInfo[i][j].minDist=3000000;
	for (c=0;c<NUMCENTS;c++) {
		Ct=&cent[c];
		for (i=Ct->x-searchRad;i<Ct->x+searchRad+1;i++) {
			if (i<0 || i>=WID)
				continue;
			for (j=Ct->y-searchRad;j<Ct->y+searchRad+1;j++) {
				if (j<0)
					continue;
				if (j>=HEI)
					break; //go to next row
				squaredColourDist=0;
				for (k=0;k<3;k++)
					squaredColourDist+=sq(b[i][j][k]-Ct->col[k]);
				squaredDist=sq(i-Ct->x)+sq(j-Ct->y);
				dist=squaredColourDist+squaredDist*mOverSSq;
				if (dist<cenInfo[i][j].minDist) {
					cenInfo[i][j].minDist=dist;
					cenInfo[i][j].closestCen=c;
}	}	}	}	}

void MovePixelToSPWithPhysicallyClosestCentre(int i, int j) {
	float dist=2000000, sqDist=0;
	int c;
	for (c=0;c<NUMCENTS;c++) {
		sqDist=sq(i-cent[c].x)+sq(j-cent[c].y);
		if (sqDist<dist) {
			dist=sqDist;
			cenInfo[i][j].closestCen=c;
	}	}
	cenInfo[i][j].minDist=sqDist; //doesnt matter if last frame.
}

int Dist(int i, int j) {
	return isq(cent[i].col[0]-cent[j].col[0])+
		isq(cent[i].col[1]-cent[j].col[1])+
		isq(cent[i].col[2]-cent[j].col[2]);
}

void MakeJIntoIPixels(int i,int j) {
	int p,q,c,num=0;
	float sum[3]={0,0,0};
	for (p=0;p<WID;p++) {
		for (q=0;q<HEI;q++) {
			if (cenInfo[p][q].closestCen==j) {
				cenInfo[p][q].closestCen=i;
				for (c=0;c<3;c++)
					sum[c]+=b[p][q][c];
				num+=1;
				continue;
			}
			if (cenInfo[p][q].closestCen==i) {
				for (c=0;c<3;c++)
					sum[c]+=b[p][q][c];
				num+=1;
	}	}	}
	for (c=0;c<3;c++)
		cent[i].col[c]=round(sum[c]/num);
	cent[i].numpts=num;
}


void DoStuff() {
//image processing on a[][][] array
	char name2[100];
	float dist;
	int i,j,c=0,loop,k,x,y,thisSP,cC,diff;
	savepng("testpic.png");
	for (i=0;i<WID;i++)
		for (j=0;j<HEI;j++)
			for (k=0;k<3;k++)
				b[i][j][k]=a[i][j][k];
	//NB in this version, do stuff to a array, keep "b" for reference.
	//assign initial centres
	for (i=GAP/2;i<WID-GAP/2;i+=GAP)
		for (j=GAP/2;j<HEI-GAP/2;j+=GAP) {
			if (c>=MAXK) {
				printf("ERROR, increase MAXK");
				return;
			}
			cent[c].x=i;
			cent[c].y=j;
			for (k=0;k<3;k++)
				cent[c].col[k]=a[i][j][k];
			c+=1;
		}
	NUMCENTS=c;
	printf("%d\n",NUMCENTS);
	for (loop=0;loop<LOOPS;loop++) {
		printf("%d\n",loop);
		AssignDistsToPixelsNearCentres();
		EmptyCentreData();
		CalcClusterCentresAndColours();
		for (i=0;i<WID;i++)
			for (j=0;j<HEI;j++)
				for (k=0;k<3;k++)
					a[i][j][k]=cent[cenInfo[i][j].closestCen].col[k];			
		snprintf(name2,sizeof(name2),"~/superpixels-tiger-%d",loop);
		savepng(name2);
	}
//move unconnected pixels to nearest-centre sp for last pic.
	//copy to new connectionMap array
	for (i=0;i<WID;i++)
		for (j=0;j<HEI;j++)
			connectionMap[i][j]=cenInfo[i][j];		
	//start at each centre, erase all group nums, recursively fill..put -1
	for (c=0;c<NUMCENTS;c++) {
		x=cent[c].x;
		y=cent[c].y;
		//change that to: loc=cent[c] (both are type pt : [x,y])
		thisSP=connectionMap[x][y].closestCen;
		EraseConnectedPixels(thisSP,x,y);
	}
		//now all non- -1 pixels will be unconnected. 
	for (i=0;i<WID;i++)
		for (j=0;j<HEI;j++) {
			if (connectionMap[i][j].closestCen>-1) //i.e. unconnected
				MovePixelToSPWithPhysicallyClosestCentre(i,j);
			cC=cenInfo[i][j].closestCen;
			if (BOUNDARIES) { 	//make sp boundaries black
				if (i>0 && cenInfo[i-1][j].closestCen!=cC) {
					for (k=0;k<3;k++)
						a[i][j][k]=0;
					continue;
				}
				else
					if (j>0 && cenInfo[i][j-1].closestCen!=cC) {
						for (k=0;k<3;k++)
							a[i][j][k]=0;
						continue;
					}
			}
			for (k=0;k<3;k++)
				a[i][j][k]=cent[cC].col[k];			
		}
	loop+=1;
	snprintf(name2,sizeof(name2),"/Users/admin/superpixels-tig%d-f%d-G%dM%d",WID,loop,GAP,M);
	savepng(name2);
	//make adjacency matrix, progressively merge SPs closer than a certain
	//rising amount with each other..
//	np.ndarray[np.uint8_t,ndim = 2] adjMat
//	adjMat = np.zeros([NUMCENTS,NUMCENTS],dtype=np.uint8)
//NB adjMat indices are 'backwards' currently. but array is symmetrical across
//diagonal, so shouldnt matter (?)
	for (i=0;i<WID;i++)
		for (j=0;j<HEI;j++) {
			cC=cenInfo[i][j].closestCen;
			if (i>0 && cenInfo[i-1][j].closestCen!=cC) {
				adjMat[cC][cenInfo[i-1][j].closestCen]=1;
				adjMat[cenInfo[i-1][j].closestCen][cC]=1;
			}
			if (j>0 && cenInfo[i][j-1].closestCen!=cC) {
				adjMat[cC][cenInfo[i][j-1].closestCen]=1;
				adjMat[cenInfo[i][j-1].closestCen][cC]=1;
			}
		}
	for (diff=100;diff<MAXDIFF;diff+=100) {
		printf("%d\n",diff);
		for (i=0;i<NUMCENTS;i++) {
			if (adjMat[i][i]==DELETED) //has been deleted
				continue;
			//ok, so not deleted.
			//find all adjacent groups, see if colour is close
			for (j=i+1;j<NUMCENTS;j++) {
				if (adjMat[j][i]!=NEIGHBOUR) //isnt neighbour
					continue;
				if (adjMat[j][j]==DELETED) //has been deleted
					continue;
				dist=Dist(i,j);
				if (dist<diff) {//ok, close neighbour found, so delete
					//1. make all j pixels into i
					MakeJIntoIPixels(i,j);
					//2. add neighbours of j to i in adjMat
					for (c=0;c<NUMCENTS;c++) {
						if (c==j)
							adjMat[j][j]=DELETED; //delete
						if (adjMat[c][i]==0 && adjMat[c][j]==NEIGHBOUR)
							adjMat[c][i]=adjMat[c][j]; //add sp if not already there
					//3. delete entries for j in adjMat
						adjMat[c][j]=0;
		}	}	}	}
		for (i=0;i<WID;i++)
			for (j=0;j<HEI;j++)
				for (k=0;k<3;k++)
					a[i][j][k]=cent[cenInfo[i][j].closestCen].col[k];			
		snprintf(name2,sizeof(name2),"superpixels-tiger-merged%d",diff);
		savepng(name2);
}	}



int main() {
	readpng(fn);
	printf("sizeof array : %zu\n", sizeof a);
	DoStuff();
	savepng(fn);
	return 0;
}

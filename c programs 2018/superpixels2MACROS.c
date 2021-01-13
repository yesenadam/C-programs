//superpixels2
//this version stores the min and max x and y of each SP for the merging part.
//ie to make it faster.
#define FOR(var,end) for (var=0;var<end;var++)
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>
#include "ppmpng.h"
#define WID 1704 //512 //2048 //must be exact size of PNG image
#define HEI 1511 //512 // 2048
char fn[50]="~/Tara-orig"; //~/tiger2048"; //PNG only.
unsigned char a[WID][HEI][3],b[WID][HEI][3];
PPMImage *image;
#define MAXK 6400 //1000 //not going to be more than this
#define LOOPS 350 //100//100
#define BOUNDARIES 1 //0=don't draw superpixel boundaries in black. 1=draw
enum {M=60, //max colour distance allowed in a SP.
//max colour distance.. I think 3*255^2 with raw RGB and sq dist
//increasing M makes physical distance more important than colour dist
//i.e. more regular, compact SPs
	GAP=20, //30, //init gap between SP centres
	SEARCHRADMULT=1, //1=usual search of a 2Sx2S square
	MAXDIFF=5000 //for final merging part
	};
typedef struct {
	int col[3];
	int x,y;
	int numpts;
	int minx,maxx,miny,maxy; //stores the extremes of superpixels
} cen;
typedef struct{
	float minDist;
	int closestCen;
} ci;
cen cent[MAXK];
cen *Ct, *Ci, *Cj;
ci cenInfo[WID][HEI],connectionMap[WID][HEI];
int NUMCENTS,searchRad=SEARCHRADMULT*GAP;
char adjMatrix[MAXK][MAXK];
enum {	DELETED=-1,NEIGHBOUR=1,ERASED=-1};

void readpng(char *fn) {
	int i=0,x,y,k;
	char pngtopnmstr[80];
	sprintf(pngtopnmstr, "pngtopnm %s.png >temp.ppm", fn);
	system(pngtopnmstr);
	image = readPPM("temp.ppm");
	FOR(y,HEI){
		FOR(x,WID){
			FOR(k,3)
				a[x][y][k]=image->data[i].rgb[k];
			i++;
}	}	}	

void savepng(char *fn) {
	char pnmtopngstr[80];
	int i=0,x,y,k;
	FOR(y,HEI){
		FOR(x,WID){
			FOR(k,3)
				image->data[i].rgb[k]=a[x][y][k];
			i++;
	}	}	
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
	FOR(c,NUMCENTS) {
		Ct=&cent[c];
		FOR(k,3)
			Ct->col[k]=0; //do this without a loop?
		Ct->numpts=0;
		Ct->x=0;
		Ct->y=0;
}	}

void CalcClusterCentresAndColours() {
	int i,j,k,c;
	FOR(i,WID) 
		FOR(j,HEI) {
			Ct=&cent[cenInfo[i][j].closestCen];
			FOR(k,3)
				Ct->col[k]+=b[i][j][k];
			Ct->numpts+=1;
			Ct->x+=i;
			Ct->y+=j;
		}
	FOR(c,NUMCENTS) {
		Ct=&cent[c];
		Ct->x/=Ct->numpts;
		Ct->y/=Ct->numpts;
		FOR(k,3)
			Ct->col[k]/=Ct->numpts;
}	}

float sq(int n) { return n*n; }

int isq(int n) { return n*n; }

float fsq(float n) { return n*n; }

void AssignDistsToPixelsNearCentres() {
//	float fM=;
	//float fsearchRad=;
	float mOverSSq=fsq(((float)M) / ((float)SEARCHRADMULT*GAP));
	int c,i,j,k;
	float squaredColourDist,squaredDist,dist;
	FOR(i,WID)
		FOR(j,HEI)
			cenInfo[i][j].minDist=3000000;
	FOR(c,NUMCENTS) {
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
				FOR(k,3)
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
	FOR(c,NUMCENTS) {
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
	cen *Ci=&cent[i], *Cj=&cent[j];
	//so search in area including all i and j pixels
	//new SP i will include area of j
	if (Cj->minx<Ci->minx) Ci->minx=Cj->minx; 
	if (Cj->maxx>Ci->maxx) Ci->maxx=Cj->maxx; 
	if (Cj->miny<Ci->miny) Ci->miny=Cj->miny; 
	if (Cj->maxy>Ci->maxy) Ci->maxy=Cj->maxy; 
	//j will disappear, so not important to set it.	
	for (p=Ci->minx;p<Ci->maxx+1;p++) 
		for (q=Ci->miny;q<Ci->maxy+1;q++) {
//	FOR(p,WID) 
//		FOR(q,HEI) {
			if (cenInfo[p][q].closestCen==j)
				cenInfo[p][q].closestCen=i;
			if (cenInfo[p][q].closestCen==i) {
				FOR(c,3)
					sum[c]+=b[p][q][c];
				num+=1;
		}	}
	FOR(c,3)
		Ci->col[c]=round(sum[c]/num);
	Ci->numpts=num;
}


void DoStuff() {
//image processing on a[][][] array
	char name2[100];
	float dist;
	int i,j,c=0,loop,k,x,y,thisSP,cC,diff,SP;
//	savepng("testpic.png");
	FOR(i,WID)
		FOR(j,HEI)
			FOR(k,3)
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
			FOR(k,3)
				cent[c].col[k]=a[i][j][k];
			c+=1;
		}
	NUMCENTS=c;
	printf("%d\n",NUMCENTS);
	FOR(loop,LOOPS) {
		printf("%d, ",loop);
		AssignDistsToPixelsNearCentres();
		EmptyCentreData();
		CalcClusterCentresAndColours();
		FOR(i,WID)
			FOR(j,HEI)
				FOR(k,3)
					a[i][j][k]=cent[cenInfo[i][j].closestCen].col[k];
		//save every 10th one
		if (loop%10>0)
			continue;
		snprintf(name2,sizeof(name2),"~/superpixels-tiger-%d",loop);
		savepng(name2);
	}
//move unconnected pixels to nearest-centre sp for last pic.
	//copy to new connectionMap array
	FOR(i,WID)
		FOR(j,HEI)
			connectionMap[i][j]=cenInfo[i][j];		
	//start at each centre, erase all group nums, recursively fill..put -1
	FOR(c,NUMCENTS) {
		x=cent[c].x;
		y=cent[c].y;
		//change that to: loc=cent[c] (both are type pt : [x,y])
		thisSP=connectionMap[x][y].closestCen;
		EraseConnectedPixels(thisSP,x,y);
	}
		//now all non- -1 pixels will be unconnected. 
	FOR(i,WID)
		FOR(j,HEI) {
			if (connectionMap[i][j].closestCen>-1) //i.e. unconnected
				MovePixelToSPWithPhysicallyClosestCentre(i,j);
			cC=cenInfo[i][j].closestCen;
			if (BOUNDARIES) { 	//make sp boundaries black
				if (i>0 && cenInfo[i-1][j].closestCen!=cC) {
					FOR(k,3)
						a[i][j][k]=0;
					continue;
				}
				else
					if (j>0 && cenInfo[i][j-1].closestCen!=cC) {
						FOR(k,3)
							a[i][j][k]=0;
						continue;
			}		}
			FOR(k,3)
				a[i][j][k]=cent[cC].col[k];			
		}
	loop+=1;
	snprintf(name2,sizeof(name2),
		"/Users/admin/superpixels-tig%d-f%d-G%dM%d",WID,loop,GAP,M);
	savepng(name2);
	//make adjacency matrix, progressively merge SPs closer than a certain
	//rising amount with each other..
//NB adjMatrix indices are 'backwards' currently. but array is symmetrical across
//diagonal, so shouldnt matter (?)
	int SPabove, SPToTheLeft;
	FOR(i,WID)
		FOR(j,HEI) {
			cC=cenInfo[i][j].closestCen;
			SPToTheLeft=cenInfo[i-1][j].closestCen;
			if (i>0 && SPToTheLeft!=cC) {
				adjMatrix[cC][SPToTheLeft]=1;
				adjMatrix[SPToTheLeft][cC]=1;
			}
			SPabove=cenInfo[i][j-1].closestCen;
			if (j>0 && SPabove!=cC) {
				adjMatrix[cC][SPabove]=1;
				adjMatrix[SPabove][cC]=1;
			}
		}
	//NEW BIT for v2
	//Find min and max x and y for all SPs.
	//then alter MakeJIntoIPixels to update i info with j's extremes.
	//first init
	FOR(c,NUMCENTS) {
		cent[c].minx=WID;
		cent[c].maxx=-1;
		cent[c].miny=HEI;
		cent[c].maxy=-1;
	}
	//now set vals
	FOR(i,WID)
		FOR(j,HEI) {
			SP=cenInfo[i][j].closestCen;
			if (cent[SP].minx>i) cent[SP].minx=i;
			if (cent[SP].maxx<i) cent[SP].maxx=i;
			if (cent[SP].miny>j) cent[SP].miny=j;
			if (cent[SP].maxy<j) cent[SP].maxy=j;
		}
	for (diff=100;diff<MAXDIFF;diff+=100) {
		printf("%d\n",diff);
		FOR(i,NUMCENTS) {
			if (adjMatrix[i][i]==DELETED) //if has been deleted
				continue;
			//ok, so not deleted.
			//find all adjacent groups, see if colour is close
			for (j=i+1;j<NUMCENTS;j++) {
				if (adjMatrix[j][i]!=NEIGHBOUR) //isnt neighbour
					continue;
				if (adjMatrix[j][j]==DELETED) //has been deleted
					continue;
				dist=Dist(i,j);
				if (dist<diff) {//ok, close neighbour found, so delete
					//1. make all j pixels into i
					MakeJIntoIPixels(i,j);
					//2. add neighbours of j to i in adjMatrix
					FOR(c,NUMCENTS) {
						if (c==j)
							adjMatrix[j][j]=DELETED; //delete
						if (adjMatrix[c][i]==0 && adjMatrix[c][j]==NEIGHBOUR)
							adjMatrix[c][i]=adjMatrix[c][j]; //add sp if not already there
					//3. delete entries for j in adjMatrix
						adjMatrix[c][j]=0;
		}	}	}	}
		FOR(i,WID)
			FOR(j,HEI)
				FOR(k,3)
					a[i][j][k]=cent[cenInfo[i][j].closestCen].col[k];			
		snprintf(name2,sizeof(name2),"superpixels-tiger-merged%d",diff);
		savepng(name2);
}	}

int main() {
	readpng(fn);
	printf("sizeof array : %zu\n", sizeof a);
	DoStuff();
	return 0;
}

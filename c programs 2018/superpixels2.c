//superpixels2
//this version stores the min and max x and y of each SP for the merging part.
//ie to make it faster.

//****** idea: do shuffled voronoi colouring!!!!!!..........

//TO DO: Measure sum of dist SPs move each frame, stop when < certain amount.
#define _(x,y) for (x=0;x<(y);x++)
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>
#include "ppmpng.h"
char fn[50] = "~/tiger-09copy"; //Tara-orig"; //~/adamtree"; // //~/tiger2048"; //PNG only.
char tag[20] = "tiger"; //adamtree"; // //short name to use in filenames
PPMImage *image;
enum {WID = 513, HEI = 513, MAXK = 10500, LOOPS = 100, YES = 1, NO = 0,
	M = 50, //max colour distance allowed in a SP. <- no not really. hay ecuaciones:
//	dist = squaredColourDist+squaredDist*mOverSSq;
	//float mOverSSq = fsq(((float)M) / ((float)SEARCHRADMULT*GAP));
//max colour distance.. I think 3*255^2 with raw RGB and sq dist
//increasing M makes physical distance more important than colour dist
//i.e. more regular, compact SPs
	GAP = 5, //init gap between SP centres
	SEARCHRADMULT = 1, //1 = usual search of a 2Sx2S square
	MAXDIFF = 5000, //for final merging part
	BOUNDARIES = YES, //0 = don't draw superpixel boundaries in black. 1 = draw};
	MERGE = NO, // merge SPs with squared total colour diff < MAXDIFF
	SAVE_SUPERPIXELS = YES}; //write SP data to file
unsigned char a[WID][HEI][3],b[WID][HEI][3]; 
typedef struct {
	int col[3]; //colour of the superpixel
	int x,y;	//loc of superpixel centre
	int numpts; //num of pts in superpixel
	int minx,maxx,miny,maxy; //stores the extremes of superpixels
} cen;
typedef struct {
	float minDist; //dist from pixel to nearest superpixel centre
	int closestCen; // the cent[] number of that nearest SP, = the pixel colour
	int status;	//used to eliminate stray points unconnected with main SP body.
} ci;
ci cenInfo[WID][HEI];
cen cent[MAXK];
cen *Ci, *Cj;
int superpixels,searchRad = SEARCHRADMULT*GAP;
char adjM[MAXK][MAXK];
enum { DELETED = -1,ADJ = 1,ERASED = -1,NOT_ADJ = 0 };

void readpng(char *fn) {
	int i = 0,x,y,k;
	char pngtopnmstr[80];
	sprintf(pngtopnmstr, "pngtopnm %s.png >temp.ppm", fn);
	system(pngtopnmstr);
	image = readPPM("temp.ppm");
	_(y,HEI)
		for (x=0;x<WID;x++,i++)
			_(k,3)
				a[x][y][k] = image->data[i].rgb[k];
}	

void savepng(char *fn) {
	char pnmtopngstr[80];
	int i = 0,x,y,k;
	_(y,HEI)
		for (x=0;x<WID;x++,i++)
			_(k,3)
				image->data[i].rgb[k] = a[x][y][k];
	writePPM("temp.ppm",image);
	sprintf(pnmtopngstr,"pnmtopng temp.ppm >%s.png",fn);
	system(pnmtopngstr);
}

void EraseConnectedPixels(int thisSP,int i,int j) {
	//turns all points to ERASED except those unreachable from SP centres.
	if (cenInfo[i][j].status == ERASED || cenInfo[i][j].status != thisSP ||
		i<1 || i>WID-2 || j<1 || j>HEI-2)
			return;
	cenInfo[i][j].status = ERASED;
	EraseConnectedPixels(thisSP,i+1,j);
	EraseConnectedPixels(thisSP,i-1,j);
	EraseConnectedPixels(thisSP,i,j+1);
	EraseConnectedPixels(thisSP,i,j-1);
}

void CalcClusterCentresAndColours() {
	int i,j,k,c,thisSP;
	_(i,WID) 
		_(j,HEI) {
			thisSP=cenInfo[i][j].closestCen;
			_(k,3)
				cent[thisSP].col[k] += b[i][j][k];
			cent[thisSP].numpts += 1;
			cent[thisSP].x += i;
			cent[thisSP].y += j;
		}
	_(c,superpixels) {
		cent[c].x /= cent[c].numpts;
		cent[c].y /= cent[c].numpts;
		_(k,3)
			cent[c].col[k] /= cent[c].numpts;
}	}

float sq(int n) { return n*n; }

int isq(int n) { return n*n; }

float fsq(float n) { return n*n; }

void AssignDistsToPixelsNearCentres() {
	float mOverSSq = fsq(((float)M) / ((float)SEARCHRADMULT*GAP));
	int c,i,j,k;
	float squaredColourDist,squaredDist,dist;
	_(i,WID) _(j,HEI)
		cenInfo[i][j].minDist = 3000000;
	_(c,superpixels) {
		for (i=cent[c].x-searchRad;i<cent[c].x+searchRad+1;i++) {
			if (i < 0 || i >= WID) continue;
			for (j=cent[c].y-searchRad;j<cent[c].y+searchRad+1;j++) {
				if (j<0) continue;
				if (j>=HEI) break; //go to next row
				for (squaredColourDist=0,k=0;k<3;k++)
					squaredColourDist += sq(b[i][j][k]-cent[c].col[k]);
				squaredDist = sq(i-cent[c].x)+sq(j-cent[c].y);
				dist = squaredColourDist+squaredDist*mOverSSq;
				if (dist<cenInfo[i][j].minDist) 
					cenInfo[i][j] = (ci) {.closestCen=c, .minDist=dist};
}	}	}	}	

void MovePixelToSPWithPhysicallyClosestCentre(int i, int j) {
	float dist = 2000000, sqDist = 0;
	int c;
	_(c,superpixels) {
		sqDist = sq(i-cent[c].x)+sq(j-cent[c].y);
		if (sqDist<dist) {
			dist = sqDist;
			cenInfo[i][j].closestCen = c;
	}	}
	cenInfo[i][j].minDist = sqDist; //doesnt matter if last frame.
}

int Dist(int i, int j) {
	return isq(cent[i].col[0]-cent[j].col[0])+
		isq(cent[i].col[1]-cent[j].col[1])+
		isq(cent[i].col[2]-cent[j].col[2]);
}

void MergeBIntoA(int i,int j) {
	int p,q,c,num = 0;
	float sum[3] = {0,0,0};
	cen *Ci = &cent[i], *Cj = &cent[j];
	//so search in area including all i and j pixels
	//new SP i will include area of j
	if (Cj->minx<Ci->minx) Ci->minx = Cj->minx; 
	if (Cj->maxx>Ci->maxx) Ci->maxx = Cj->maxx; 
	if (Cj->miny<Ci->miny) Ci->miny = Cj->miny; 
	if (Cj->maxy>Ci->maxy) Ci->maxy = Cj->maxy; 
	//j will disappear, so not important to set it.	
	for (p=Ci->minx;p<Ci->maxx+1;p++) 
		for (q=Ci->miny;q<Ci->maxy+1;q++) {
			if (cenInfo[p][q].closestCen == j)
				cenInfo[p][q].closestCen = i;
			if (cenInfo[p][q].closestCen == i) {
				_(c,3) sum[c] += b[p][q][c];
				num++;
		}	}
	_(c,3) Ci->col[c] = round(sum[c]/num);
	Ci->numpts = num;
}

int WasDeleted(int s){ 
//i.e. I set adjM[i][i] to DELETED when SP i is merged into another.
	return (adjM[s][s] == DELETED);
}

void SetAsAdjacent(int g, int h){
	adjM[g][h] = ADJ;
	adjM[h][g] = ADJ;
}

void DoStuff() { //image processing on the a[][][] array
	char name2[100];
	float dist;
	int i,j,c,loop,k,x,y,thisSP,testSP,diff,SPabove, SPToTheLeft;
	memcpy(b,a,sizeof(b));
	//do stuff to "a" array, keep "b" for reference.
	//assign initial centres
	for (superpixels=0,i=GAP/2;i<WID-GAP/2;i+=GAP)
		for (j=GAP/2;j<HEI-GAP/2;j+=GAP) {
			if (superpixels >= MAXK) {
				printf("ERROR, increase MAXK");
				return;
			}
			cent[superpixels++] = (cen) {.x = i, .y = j, 
				.col = {a[i][j][0],a[i][j][1],a[i][j][2]}};
		}
	printf("%d\n",superpixels);
	_(loop,LOOPS) {
		AssignDistsToPixelsNearCentres();
		_(c,superpixels) cent[c] = (cen) {};
		CalcClusterCentresAndColours();
		_(i,WID) _(j,HEI) _(k,3)
			a[i][j][k] = cent[cenInfo[i][j].closestCen].col[k];
		if (loop%10>0) continue;
		printf("%d\n",loop);
		snprintf(name2,sizeof(name2),"~/sp/superpixels-%s-%d",tag,loop);
		savepng(name2);
	}
//move unconnected pixels to nearest-centre sp for last pic.
	//copy to new connectionMap array. NB connection map is used to
	//find pixels isolated from the main SP to remove them.
	_(i,WID) _(j,HEI) 
		cenInfo[i][j].status = cenInfo[i][j].closestCen;
	//start at each centre, erase all group nums, recursively fill..put ERASED
	_(c,superpixels) {
		x = cent[c].x;
		y = cent[c].y;
		EraseConnectedPixels(cenInfo[x][y].status,x,y); //hmm now not sure about this alg!
	}
		//now all non-ERASED pixels will be unconnected to their superpixel. 
	_(i,WID) 
		_(j,HEI) {
			if (cenInfo[i][j].status != ERASED) //i.e. unconnected
				MovePixelToSPWithPhysicallyClosestCentre(i,j);
			thisSP = cenInfo[i][j].closestCen;
			if (BOUNDARIES) { 	//make sp boundaries black
				if ((i>0 && cenInfo[i-1][j].closestCen != thisSP) ||
					(j>0 && cenInfo[i][j-1].closestCen != thisSP)) {
						_(k,3) a[i][j][k] = 0;
						continue;
			}	}
			_(k,3) a[i][j][k] = cent[thisSP].col[k];			
		}
	loop++;
	snprintf(name2,sizeof(name2),
		"~/sp/superpixels-%s%d-f%d-G%dM%d",tag,WID,loop,GAP,M);
	savepng(name2);
	if (MERGE) {
		_(c,superpixels) {
			cent[c].minx = WID;
			cent[c].maxx = -1;
			cent[c].miny = HEI;
			cent[c].maxy = -1;
		}
		//make adjacency matrix, progressively merge SPs closer than a certain
		//rising amount with each other..
	//NB adjM indices are 'backwards' currently. but array is symmetrical across
	//diagonal, so shouldnt matter (?)
		_(i,WID)
			_(j,HEI) {
				thisSP = cenInfo[i][j].closestCen;
				SPToTheLeft = cenInfo[i-1][j].closestCen;
				if (i>0 && SPToTheLeft != thisSP) 
					SetAsAdjacent(thisSP,SPToTheLeft);
				SPabove = cenInfo[i][j-1].closestCen;
				if (j>0 && SPabove != thisSP)
					SetAsAdjacent(thisSP,SPabove);
		//Find min and max x and y for all SPs.
				if (cent[thisSP].minx>i) cent[thisSP].minx = i;
				if (cent[thisSP].maxx<i) cent[thisSP].maxx = i;
				if (cent[thisSP].miny>j) cent[thisSP].miny = j;
				if (cent[thisSP].maxy<j) cent[thisSP].maxy = j;
			}
		for (diff=100;diff<MAXDIFF;diff+=100) {
			printf("%d\n",diff);
			_(thisSP,superpixels) {
				if (WasDeleted(thisSP))
					continue;
				//find all adjacent SPs, see if colour is close
				for (testSP=thisSP+1;testSP<superpixels;testSP++) {
					if (adjM[testSP][thisSP] != ADJ || WasDeleted(testSP)) 
						continue;
					dist = Dist(thisSP,testSP);
					if (dist<diff) {//ok, close neighbour found, so delete/merge
						MergeBIntoA(thisSP,testSP);
						// add neighbours of testSP to thisSP in adjM
						_(c,superpixels) {
							if (c == testSP)
								adjM[testSP][testSP] = DELETED; 
							if (adjM[c][thisSP] == NOT_ADJ && 
								adjM[c][testSP] == ADJ)
									adjM[c][thisSP] = adjM[c][testSP]; 
							adjM[c][testSP] = NOT_ADJ;
			}	}	}	}
			_(i,WID) _(j,HEI) _(k,3)
				a[i][j][k] = cent[cenInfo[i][j].closestCen].col[k];			
			snprintf(name2,sizeof(name2),"~/sp/superpixels-%s-merged%d",tag,diff);
			savepng(name2);
	}	}
	if (SAVE_SUPERPIXELS) {
		//NO! just write SP info to file.
		//much quicker to test just taking nums from file!
		//write another program to do the shuffled Vor.
		
		//I guess, save with different combinations.....
		//maybe have a NEAREST_N, e.g. 6, saves EVERY combination up to N=6?
		//e.g. [0,1] (i.e. normal), [1,0], [0,1,2] (normal), [0,2,1] etc
		//n! I guess... sum of N!
		//need closest N SPs to each pixel, in order.
		//Q. test all, or only those within some dist? well...
		//I guess within 3 or 4x starting radius should find them all, amply.
		//but depends on N. hmm N=1, R=1. N=4, R=2. ok, R=sqrt(N) should be fine.
		
		//write "a" array to "b"
		
		// go through SPs, delete from list those with 0 pixels. (clean up)
		
		//then shuffle.
		
		//write new colours to "a" array
		//first count how many "used" superpixels there are
		//because number must be written on first line.
		int xx,yy,c0,c1,c2,t=0; //,wid=WID,hei=HEI; //not sure if pointer to enum works
		_(c,superpixels) 
			if (cent[c].numpts>0)
				t++;
		FILE *f, *fopen();
		f=fopen("data.txt","wa");
		fprintf(f,"%d,%d,%d\n",t,WID,HEI);
		_(c,superpixels) {
			if (cent[c].numpts==0)
				continue;
			xx=cent[c].x;
			yy=cent[c].y;
			c0=cent[c].col[0];
			c1=cent[c].col[1];
			c2=cent[c].col[2];
			fprintf(f,"%d,%d,%d,%d,%d\n",xx,yy,c0,c1,c2);
		}
		fclose(f);
}	}

int main() {
	readpng(fn);
	printf("sizeof array : %zu\n", sizeof a);
	DoStuff();
	return 0;
}

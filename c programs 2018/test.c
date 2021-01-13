#include <stdio.h>
int main(){
float dist=0.2;
dist=dist>0.1 ? 1/dist : 10;
/*		if (dist>0.1)*/
/*							dist=1/dist;*/
/*						else*/
/*							dist=10; //so centre pixels draw ok.*/
	printf("%f\n",dist);
}
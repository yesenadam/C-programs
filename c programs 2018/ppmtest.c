#include <stdio.h>
#include <stdlib.h>

int main(){
   FILE *imageFile;
   int x,y,pixel,height=3,width=2;

   imageFile=fopen("image.ppm","wb");
   if(imageFile==NULL){
      perror("ERROR: Cannot open output file");
      exit(EXIT_FAILURE);
   }

   fprintf(imageFile,"P6\n");               // P6 filetype
   fprintf(imageFile,"%d %d\n",width,height);   // dimensions
   fprintf(imageFile,"255\n");              // Max pixel

   unsigned char pix[]={200,200,200, 100,100,100, 0,0,0, 255,0,0, 0,255,0, 0,0,255};
   fwrite(pix,1,18,imageFile);
   fclose(imageFile);
system("pnmtopng image.ppm >imageppmtest.png");
}
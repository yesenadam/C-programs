//libmathsart.c - my maths art library! 
//v1.0 Adam Ponting Feb 2020

/* png read/write parts based on the original short example of using libpng, which said:
 * Copyright 2002-2011 Guillaume Cottenceau and contributors.
 *
 * This software may be freely redistributed under the terms
 * of the X11 license.
 *
 */
 
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#define PNG_DEBUG 3
//#define MA_DEBUG //uncomment to print verbose messages

#include <png.h>

#include "libmathsart.h"

void abort_(const char * s, ...)
{
        va_list args;
        va_start(args, s);
        vfprintf(stderr, s, args);
        fprintf(stderr, "\n");
        va_end(args);
        abort();
}

// having functions to open and close and read/write text to files would be useful too.
// what other forms of input/output might I use? Well, first implement the ones
// I use in the maths art progs Ive already done!!

// Implementation:
// row_ptrs is automatically malloced and freed each time a png is loaded,
// and each time it's saved.
// the 3D array holding the image is only freed manually.
// so you can save an image, make a minimal change, save again etc.

int width, height; 

#if 0

Maths Art (ma) interfaces:

image_ptr   ma_init_image(int A_width, int A_height)
void        ma_zero_image(int A_width, int A_height,image_ptr imageArray)
void        ma_read_png_file(char* filename, image_ptr imageArray)
image_ptr   ma_init_array_and_read_png_file    (char* filename) 
//need that if don't yet know the size. it sets width and height vars.
void        ma_write_png_file(char* filename, image_ptr imageArray)
void        ma_free_image(image_ptr imageArray)

void ma_combined_edge_difference(combined_edge_difference_s ced)

#endif

// also need to be able to make arbitrary sized 1D and 2D arrays.. i.e. runtime-sized...
//..uh cant I? as long as theyre in a function.... or file scope....? see 21st C C - mostly
//dont need malloc so much nowadays.

//******** internal functions *******************************

void PrintErrorMsg() {
    fprintf(stderr, "Out of memory");
	exit(0);
}

//need a function that takes the image_ptr of an existing image and returns width, height etc.
//could build a table while theyre built, i.e. in getImageHandle, a struct like
//int array_info_size //number of arrays with stored info, initially 0.
//
//struct array_info
//    int image_ptr_address
//    int width
//    int height
//then search through them, if not found ERROR.
//but it SHOULD always be found! if using an assigned image pointer....
//also store NUMBER of images...
//when one is freed, slide the others down... uh or use better data structure
//where that's not necessary! like linked list...
//p.s. have i EVER used a linked list?!


image_ptr getImageHandle(int M, int N, int O) { //internal
    int i,j;
	image_ptr imageArray = (image_ptr)malloc(M * sizeof(unsigned char**));
	if (imageArray == NULL) PrintErrorMsg();
	for (i = 0; i < M; i++)	{
		imageArray[i] = (unsigned char**)malloc(N * sizeof(unsigned char*));
		if (imageArray[i] == NULL) PrintErrorMsg();
		for (j = 0; j < N; j++)		{
			imageArray[i][j] = (unsigned char*)malloc(O * sizeof(unsigned char));
			if (imageArray[i][j] == NULL) PrintErrorMsg();
		}
	}
	return imageArray;
	//but doesnt that way of doing it mean.. that the block isnt together? and slower?
//	..or maybe that's how arrays are (automatically) assigned memory anyway?!
}

//******** external functions *******************************

/* use this to set up an array to save as an image, without loading an image from disk.
 */
 
image_ptr ma_init_image(int A_width, int A_height) {
    width=A_width;
    height=A_height;
    return getImageHandle(width, height,3);
}

void ma_zero_image(int A_width, int A_height,image_ptr imageArray) {
    int i,j,k;
    for (i=0;i<A_width;i++)
        for (j=0;j<A_height;j++)
            for (k=0;k<3;k++)
                imageArray[i][j][k]=0;
}


/** returns a handle to the (width x height x 3) array containing the image.
 */
 
 //**** Should put back those commented-out lines.
 //store the png_ptr and/or info_ptr in the table of struct im gonna make...
 //then when saving, use that to save... (?? learn more about all that)
 image_ptr        ma_init_array_and_read_png_file    (char* filename) {
     //png_byte color_type;
    //    int number_of_passes;
    png_structp png_ptr;
    png_infop info_ptr;
    int rowbytes;
    png_byte bit_depth;
    int x,y;
    unsigned char header[8];    // 8 is the maximum size that can be checked
    FILE *fp = fopen(filename, "rb");
    if (!fp) abort_("[ma_read_png_file] File %s could not be opened for reading", filename);
    fread(header, 1, 8, fp);
    if (png_sig_cmp(header, 0, 8))
        abort_("[ma_read_png_file] File %s is not recognized as a PNG file", filename);
    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) abort_("[ma_read_png_file] png_create_read_struct failed");
    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) abort_("[ma_read_png_file] png_create_info_struct failed");
    if (setjmp(png_jmpbuf(png_ptr))) abort_("[ma_read_png_file] Error during init_io");
    png_init_io(png_ptr, fp);
    png_set_sig_bytes(png_ptr, 8);
    png_read_info(png_ptr, info_ptr);
    width = png_get_image_width(png_ptr, info_ptr);
    height = png_get_image_height(png_ptr, info_ptr);
//    color_type = png_get_color_type(png_ptr, info_ptr);
    bit_depth = png_get_bit_depth(png_ptr, info_ptr);
//    number_of_passes = png_set_interlace_handling(png_ptr);
    png_read_update_info(png_ptr, info_ptr);
    /* read file */
    if (setjmp(png_jmpbuf(png_ptr)))
            abort_("[ma_read_png_file] Error during read_image");

    png_bytep * row_ptrs;
    row_ptrs = (png_bytep*) malloc(sizeof(png_bytep) * height);
    if (bit_depth == 16)
        abort_("[ma_read_png_file] Error: PNG is 16 byte, not currently set up for that.");
    //        rowbytes = width*6; //8; //AP: I think this was set up to work ONLY with RGBA.
   // else //now is set up to work ONLY with RGB.
    //AP: Test color_type to be flexible about that, supongo.
    rowbytes = width*3; //4 if RGBA 8 byte. Assume is 8 byte RGB for now.
    for (y=0; y<height; y++)
            row_ptrs[y] = (png_byte*) malloc(rowbytes);

    png_read_image(png_ptr, row_ptrs);
    fclose(fp);
    #ifdef MA_DEBUG
        printf("[ma_read_png_file] File closed\n");
    #endif
    image_ptr imageArray = getImageHandle(width,height,3);
    #ifdef MA_DEBUG
        printf("[ma_read_png_file] Do Malloc done\n");
    #endif
    for (y=0; y<height; y++) {
        png_byte* row = row_ptrs[y];
        for (x=0; x<width; x++) {
            png_byte* ptr = &(row[x*3]);
            imageArray[x][y][0]=ptr[0];
            imageArray[x][y][1]=ptr[1];
            imageArray[x][y][2]=ptr[2];
        }
    }
    #ifdef MA_DEBUG
        printf("[ma_read_png_file] Array copy done\n");
    #endif
    for (y=0; y<height; y++) free(row_ptrs[y]);
    free(row_ptrs);
    return imageArray;
}

 
void ma_read_png_file(char* filename, image_ptr imageArray) {
    //png_byte color_type;
    //    int number_of_passes;
    png_structp png_ptr;
    png_infop info_ptr;
    int rowbytes;
    png_byte bit_depth;
    int x,y;
    unsigned char header[8];    // 8 is the maximum size that can be checked
    FILE *fp = fopen(filename, "rb");
    if (!fp) abort_("[ma_read_png_file] File %s could not be opened for reading", filename);
    fread(header, 1, 8, fp);
    if (png_sig_cmp(header, 0, 8))
        abort_("[ma_read_png_file] File %s is not recognized as a PNG file", filename);
    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) abort_("[ma_read_png_file] png_create_read_struct failed");
    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) abort_("[ma_read_png_file] png_create_info_struct failed");
    if (setjmp(png_jmpbuf(png_ptr))) abort_("[ma_read_png_file] Error during init_io");
    png_init_io(png_ptr, fp);
    png_set_sig_bytes(png_ptr, 8);
    png_read_info(png_ptr, info_ptr);
    width = png_get_image_width(png_ptr, info_ptr);
    height = png_get_image_height(png_ptr, info_ptr);
//    color_type = png_get_color_type(png_ptr, info_ptr);
    bit_depth = png_get_bit_depth(png_ptr, info_ptr);
//    number_of_passes = png_set_interlace_handling(png_ptr);
    png_read_update_info(png_ptr, info_ptr);
    /* read file */
    if (setjmp(png_jmpbuf(png_ptr)))
            abort_("[ma_read_png_file] Error during read_image");

    png_bytep * row_ptrs;
    row_ptrs = (png_bytep*) malloc(sizeof(png_bytep) * height);
    if (bit_depth == 16)
        abort_("[ma_read_png_file] Error: PNG is 16 byte, not currently set up for that.");
    //        rowbytes = width*6; //8; //AP: I think this was set up to work ONLY with RGBA.
   // else //now is set up to work ONLY with RGB.
    //AP: Test color_type to be flexible about that, supongo.
    rowbytes = width*3; //4 if RGBA 8 byte. Assume is 8 byte RGB for now.
    for (y=0; y<height; y++)
            row_ptrs[y] = (png_byte*) malloc(rowbytes);

    png_read_image(png_ptr, row_ptrs);
    fclose(fp);
    #ifdef MA_DEBUG
        printf("[ma_read_png_file] File closed\n");
    #endif
//    image_ptr imageArray = getImageHandle(width,height,3);
//    #ifdef MA_DEBUG
//        printf("[ma_read_png_file] Do Malloc done\n");
//    #endif
    for (y=0; y<height; y++) {
        png_byte* row = row_ptrs[y];
        for (x=0; x<width; x++) {
            png_byte* ptr = &(row[x*3]);
            imageArray[x][y][0]=ptr[0];
            imageArray[x][y][1]=ptr[1];
            imageArray[x][y][2]=ptr[2];
        }
    }
    #ifdef MA_DEBUG
        printf("[ma_read_png_file] Array copy done\n");
    #endif
    for (y=0; y<height; y++) free(row_ptrs[y]);
    free(row_ptrs);
//    return imageArray;
}


void ma_write_png_file(char* filename, image_ptr imageArray) {
    png_structp png_ptr;
    png_infop info_ptr;
    int rowbytes;
    int x,y,k;
    FILE *fp = fopen(filename, "wb");
    if (!fp) abort_("[ma_write_png_file] File %s could not be opened for writing", filename);
    png_bytep * row_ptrs;
        row_ptrs = (png_bytep*) malloc(sizeof(png_bytep) * height);
   // if (bit_depth == 16)
 //           rowbytes = width*6; //8; //AP: I think this was set up to work ONLY with RGBA.
//    else //now is set up to work ONLY with RGB.
    //AP: Test color_type to be flexible about that, supongo.
            rowbytes = width*3; //4;
    for (y=0; y<height; y++)
            row_ptrs[y] = (png_byte*) malloc(rowbytes);
    printf("Start row->3D copy\n");        
    for (y=0; y<height; y++) {
        png_byte* row = row_ptrs[y];
        for (x=0; x<width; x++) {
            for (k=0;k<3;k++)
                row[x*3+k]=imageArray[x][y][k];
        }
    }
    printf("End row->3D copy\n");        
    /* initialize stuff */
    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) abort_("[ma_write_png_file] png_create_write_struct failed");
    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) abort_("[ma_write_png_file] png_create_info_struct failed");
    if (setjmp(png_jmpbuf(png_ptr))) abort_("[ma_write_png_file] Error during init_io");
    png_init_io(png_ptr, fp);
    /* write header */
    if (setjmp(png_jmpbuf(png_ptr))) abort_("[ma_write_png_file] Error during writing header");
    png_set_IHDR(png_ptr, info_ptr, width, height,
                 8, 2, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE); //the 2 was a 6..
//6 means RGBA, see this bit from png.h:

///* These describe the color_type field in png_info. */
///* color type masks */
//#define PNG_COLOR_MASK_PALETTE    1
//#define PNG_COLOR_MASK_COLOR      2
//#define PNG_COLOR_MASK_ALPHA      4

///* color types.  Note that not all combinations are legal */
//#define PNG_COLOR_TYPE_GRAY 0
//#define PNG_COLOR_TYPE_PALETTE  (PNG_COLOR_MASK_COLOR | PNG_COLOR_MASK_PALETTE)
//#define PNG_COLOR_TYPE_RGB        (PNG_COLOR_MASK_COLOR)
//#define PNG_COLOR_TYPE_RGB_ALPHA  (PNG_COLOR_MASK_COLOR | PNG_COLOR_MASK_ALPHA) //<---- 2 or 4 =6
//#define PNG_COLOR_TYPE_GRAY_ALPHA (PNG_COLOR_MASK_ALPHA)
///* aliases */
//#define PNG_COLOR_TYPE_RGBA  PNG_COLOR_TYPE_RGB_ALPHA
//#define PNG_COLOR_TYPE_GA  PNG_COLOR_TYPE_GRAY_ALPHA
    png_write_info(png_ptr, info_ptr);
    /* write bytes */
    if (setjmp(png_jmpbuf(png_ptr))) abort_("[ma_write_png_file] Error during writing bytes");
    png_write_image(png_ptr, row_ptrs);
    /* end write */
    if (setjmp(png_jmpbuf(png_ptr))) abort_("[ma_write_png_file] Error during end of write");
    png_write_end(png_ptr, NULL);
    fclose(fp);
    
    /* cleanup heap allocation */
    for (y=0; y<height; y++) free(row_ptrs[y]);
    free(row_ptrs);
}

void ma_free_image(image_ptr imageArray) {
    int i,j;
	for (i = 0; i < width; i++) 	{
		for (j = 0; j < height; j++)
			free(imageArray[i][j]);
		free(imageArray[i]);
	}
	free(imageArray);
}

//==================
//adaptation of combinededge.pyx - edge detection
//returns a pointer to altered version of pic.

//uhh... needs to store height and width! and associate with image_ptr/image somehow....
//could have an array that stores the image_ptr and the vals of width, height..
//mainly need to return the width and height, given the image_ptr.. 
//i guess a hash table would be overkill?!! just have a struct, and search through them..
//i (famous last words) dont imagine i'll be dealing with more than a few images at a time.

//int SecDiff(int a, int b, int c) {
//	return (a-b)-(b-c);
//}

void ma_threshhold(threshhold_s t) {
    int i,j,sum;
    for (i=0;i<width;i++) 
        for (j=0;j<height;j++) {
            sum=t.img[i][j][0]+t.img[i][j][1]+t.img[i][j][2];
            if (sum<=t.min_colour_sum) {
                t.img[i][j][0]=0;
                t.img[i][j][1]=0;
                t.img[i][j][2]=0;
            }
        }
}

void ma_combined_edge_difference(combined_edge_difference_s ced) {

//so maybe use a struct like in 21st C C for the parameters?!

//#secondedge.pyx
//#simple edge-detection.
//#BUT uses 2nd difference...
//
//#for each pixel, measure x-1,y-1 - x+1,y+1 etc.. all 4 directions. then use the abs(largest)
//#of those differences, (LD+255)/2 to convert -255->+255 to 0-255. yup./*
//DEF use_abs=1 #1=just use pos values, abs() of second difference. white on black*/
/*#0= white and black on grey.*/
/*DEF NEG=1 #0=normal, 1=negative image*/
//#fn="sydney1" #.png only
    uint8 k,indexOfMax;
	int it,val,val2,ad,col,i,j,centrex2,max,hei, wid, diff[4];//, bigmax=-1000;
	hei = ced.height;//img.shape[0]
	wid=ced.width;//img.shape[1]
	if (ced.ampl==0) ced.ampl=1;
	printf("Start edge diff\n");
	image_ptr src_img=ced.src_img;
	image_ptr dest_img=ced.dest_img;
	int use_abs=ced.use_abs;
	int neg=ced.negative_image;
    ma_zero_image(ced.width, ced.height,ced.dest_img);
    for (col=0;col<3;col++) {
    printf("Do colour %d\n",col);
	    for (j=1;j<hei-1;j++) {	
            for (i=1;i<wid-1;i++) {
				diff[0]=src_img[i+1][j+1][col]-src_img[i-1][j-1][col];
				diff[1]=src_img[i+1][j][col]-src_img[i-1][j][col];
				diff[2]=src_img[i][j+1][col]-src_img[i][j-1][col];
				diff[3]=src_img[i+1][j-1][col]-src_img[i-1][j+1][col]; 
				max=(diff[0]>=0) ? diff[0] : -diff[0]; //abs
				indexOfMax=0;
				for (k=1;k<4;k++) { // in xrange(4):
					ad = (diff[k]>=0) ? diff[k] : -diff[k]; //abs
					if (ad>max) {
						max=ad;
						indexOfMax=k;
					}
				}	
				val = (use_abs) ? ced.ampl*abs(diff[indexOfMax]) : 
				    ced.ampl*diff[indexOfMax]/2+255/2;
//				val = (diff[indexOfMax]+256)%256; //) : (diff[indexOfMax]+255)/2;
				//*****!!!! CHANGE! these can use the differences of things just calculated...
				//its just those
				
				//plus the way use_abs is worked out is weird....
				//its not just abs 2nd, but for both... not sure how much it makes sense..
				//needs a better alg, i.e. not just better, different, with different options.
				
				//SecDiff is return (a-b)-(b-c);
				//which is a+c-2*b --> but the b is the same for all 4! so subtract after.
				diff[0]=src_img[i+1][j+1][col]+src_img[i-1][j-1][col];
				diff[1]=src_img[i+1][j][col]+src_img[i-1][j][col];
				diff[2]=src_img[i][j+1][col]+src_img[i][j-1][col];
				diff[3]=src_img[i+1][j-1][col]+src_img[i-1][j+1][col];
                centrex2=2*src_img[i][j][col];
				max=abs(diff[0]-centrex2);
				indexOfMax=0;
				for (k=1;k<4;k++) { // in xrange(4):
					ad=abs(diff[k]-centrex2);
					if (ad>max) {
						max=ad;
						indexOfMax=k;
					}
				}
				val2 = (use_abs) ? ced.ampl*abs(diff[indexOfMax]-centrex2)/2 : 
				    ced.ampl*(diff[indexOfMax]-centrex2)/4+255/2;
				if (use_abs)
				    it =  (val>val2) ? val : val2;
				else
				    it =  (abs(128-val)>abs(128-val2)) ? val : val2;
				    
				//the line above doesnt make sense!
				//no wonder use_abs=0 looks so boring!!
				//if use_abs is off, it should test for max DIST FROM 255/2, NOT
				//max value.....
				dest_img[i][j][col] = (neg==yes) ? 255-it : it;
			} //for i	
		} // for j
    } //for (col	
    	printf("End edge diff\n");

}
//=============================================
#if 0

//points/params  --> image

ma_triangular_dist()
ma_circle_inversion()
ma_minsky()
ma_thue_morse()
ma_wang_tiles()
ma_truchet_tiles()
ma_world()

//image --> image

ma_rgb_ribbon()
ma_stipple()
ma_image_blob()
ma_diffusion()
ma_image_squares()
ma_seam_carve()
ma_hough_transform()

// image --> data

ma_superpixel()

//2 images --> image

ma_bidirectional_similarity()
ma_seamless_clone()

// data --> video images

ma_2D_CA()
ma_life()
ma_larger_than_life()
ma_IFS()

#endif

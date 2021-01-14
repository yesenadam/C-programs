#include "../bin/libmathsart/libmathsart.h"

//#include "libmathsart.c"

// compile in ~ with 
//gcc -Wall matest.c -lpng

#if 0

Maths Art (ma) interfaces:

image_ptr   ma_init_image       (int A_width, int A_height)
void        ma_zero_image       (int A_width, int A_height,image_ptr imageArray)
void        ma_read_png_file    (char* filename, image_ptr imageArray)
image_ptr        ma_init_array_and_read_png_file    (char* filename)
void        ma_write_png_file   (char* filename, image_ptr imageArray)
void        ma_free_image       (image_ptr imageArray)

void ma_combined_edge_difference(int A_width, int A_height, uint8 abs2nd, uint8 neg,
    image_ptr src_img, image_ptr dest_img) {

really need to store the width, height etc somewhere, for each image.
Then wont size to put width and height in ma_zero_image params etc.
The program shouldnt forget how big the images are!!

#endif

image_ptr A,B; 

//typedef struct {
//    int width;
//    int height;
////    int use_second_difference; //yes or no, default = no
//    int use_abs; //yes or no, deafult = no
//    int negative_image; //yes or no, default = no
//    image_ptr src_img;
//    image_ptr dest_img;
//} combined_edge_difference_s;
//typedef struct {
//    image_ptr img;
//    int min_colour_sum; //pixels with R+G+B less than thesee get set to 0.
//} threshhold_s;
//threshhold_s t = { .img = B, .min_colour_sum=100 };

int main(int argc, char **argv) {
//    int x,y;
//    if (argc != 3) abort_("Usage: program_name <file_in> <file_out>");
    if (argc != 2) abort_("Usage: program_name <file_in>");
//    A = ma_init_image(width,height); 
    A=ma_init_array_and_read_png_file(argv[1]);
    //it's lax, but works (for now):
    //(I intend to use Hansen's Table to store vals for each image, like wid & hei))
    //with the img ptr as key, so can then say ma_get_image_width(A) which returns width.
    
    //initing an image makes global variables width & height ,
    //which hang around until the next
    //read_png or write_png. so use these to make B:
    
    //if I have a struct of ma_init_image, could have
//    .zero=yes/no, and..uh.... well, would that be faster than zeroing separately? not much..
    B = ma_init_image(width, height);

    combined_edge_difference_s ced = {
        .width=width, .height=height, 
        .use_abs = yes,
//        .ampl = 3,
        .negative_image = no, 
        .src_img = A, .dest_img = B };
        //COULD get ma_c_e_d to use current vals of width & height if no vals given in struct..
    ma_combined_edge_difference(ced); 

    threshhold_s t = { .img = B, .min_colour_sum=100 };
    ma_threshhold(t);

    ma_write_png_file("combedgediff-abs1-neg0.png",B);

    diffuse_s d = {
        .width=width;
        .height=height;
        .input=B;
        .output=A;
    };
    ma_diffuse(d);

//    ma_free_image(A); //dont think these are needed at the end of program!
//    ma_free_image(B);

    return 0;
}

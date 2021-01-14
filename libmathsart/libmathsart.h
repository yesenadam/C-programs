#ifndef LIBMATHSART_H
#define LIBMATHSART_H

#define yes 1 
#define no 0

typedef unsigned char uint8;
typedef unsigned char*** image_ptr;

extern int width, height; 
 
typedef struct {  
    int width;
    int height;
//    int use_second_difference; //yes or no, default = no
    int use_abs; //yes or no, deafult = no
    int negative_image; //yes or no, default = no
    float ampl; //amplifies amount of difference. 1 = no amplification. default=1
    image_ptr src_img;
    image_ptr dest_img;
} combined_edge_difference_s;

typedef struct {
    image_ptr img;
    int min_colour_sum; //pixels with R+G+B less than thesee get set to 0.
} threshhold_s;

typedef struct {
    int width;
    int height;
    image_ptr input;
    image_ptr output;
} diffuse_s;

extern void abort_(const char * s, ...);

extern image_ptr   ma_init_image(int A_width, int A_height);
extern void        ma_zero_image(int A_width, int A_height,image_ptr imageArray);
extern void        ma_read_png_file(char* filename, image_ptr imageArray);
extern image_ptr        ma_init_array_and_read_png_file    (char* filename);
extern void        ma_write_png_file(char* filename, image_ptr imageArray);
extern void        ma_free_image(image_ptr imageArray);
//hmm is extern needed here for funcs?!
void ma_combined_edge_difference(combined_edge_difference_s ced);
void ma_threshhold(threshhold_s t);
void ma_diffuse(diffuse_s d);
#endif
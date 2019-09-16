#ifndef IMG_H
#define IMG_H

// Pixel type definition
typedef struct pixel pixel_type;

// Functions declarations
int load_image(FILE *fp);
int write_image(FILE *outf, FILE *in);
void filter_grayscale();
void filter_red_balloon();
int close_to_red(unsigned int red, unsigned int green, unsigned int blue);

#endif
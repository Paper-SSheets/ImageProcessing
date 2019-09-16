#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

// Global Definitions
#define IMG_ROWS 900
#define IMG_COLS 900

// Global Variables
char input_file_name[] = "cy.bmp";
char output_file_name[] = "cy2.bmp";
uint16_t bpp;
uint32_t pixel_arr_loc;
int row, col;

typedef struct pixel {
	unsigned int r, g, b, a;
} pixel_type;

pixel_type in[IMG_ROWS][IMG_COLS];
pixel_type out[IMG_ROWS][IMG_COLS];

// Functions declarations
int load_image(FILE *fp);
int write_image(FILE *outf, FILE *in);
void filter_grayscale();
void filter_red_balloon();
int close_to_red(unsigned int red, unsigned int green, unsigned int blue);

// Main Function
int main() {

	FILE *in = fopen(input_file_name, "rb");
	FILE *out = fopen(output_file_name, "wb");
	load_image(in);

//	filter_red_balloon();
	filter_grayscale();
	
	write_image(out, in);
	fclose(in);
	fclose(out);
	
	return 0;
}

int load_image(FILE *fp) {
	unsigned char signature[2];
	uint32_t file_size;
	uint32_t dib_size;
	uint16_t comp_method;

	if(fp == NULL) {
		printf("Unable to open file.\n");
		return 1;
	}

	fread(signature, 1, 2, fp);
	printf("sig %c %c\n", signature[0], signature[1]);
	if(signature[0] != 'B' || signature[1] != 'M') {
		printf("Not a valid BMP file.\n");
		return 1;
	}

	fseek(fp, 2, SEEK_SET);
	fread(&file_size, 4, 1, fp);
	printf("file size %u\n", file_size);

	fseek(fp, 10, SEEK_SET);
	fread(&pixel_arr_loc, 4, 1, fp);
	printf("pixel array location %u\n", pixel_arr_loc);

	fseek(fp, 14, SEEK_SET);
	fread(&dib_size, 4, 1, fp);
	printf("dib size %u\n", dib_size);
	if(dib_size != 40) {
		printf("BMP version not supported.\n");
		return 1;
	}

	fseek(fp, 28, SEEK_SET);
	fread(&bpp, 2, 1, fp);
	printf("bits per pixel %u\n", bpp);
	if(bpp != 32) {
		printf("Bits per pixel not supported.\n");
		return 1;
	}

	fseek(fp, 30, SEEK_SET);
	fread(&comp_method, 2, 1, fp);
	printf("compression method %u\n", comp_method);
	if(comp_method != 0) {
		printf("Compression method not supported.\n");
		return 1;
	}

	int bytes_per_row = IMG_COLS * bpp / 8;
	int padding = 0;
	if(bytes_per_row % 4 != 0) {
		padding = 4 - bytes_per_row % 4;
	}
	printf("input padding %u\n", padding);
	fseek(fp, pixel_arr_loc, SEEK_SET);
	int row;
	int col;
	for(row = IMG_ROWS - 1; row >= 0; row--) {
		for(col = 0; col < IMG_COLS; col++) {
			fread(&in[row][col].b, 1, 1, fp);
			fread(&in[row][col].g, 1, 1, fp);
			fread(&in[row][col].r, 1, 1, fp);
			fread(&in[row][col].a, 1, 1, fp);
			out[row][col].b = in[row][col].b;
			out[row][col].g = in[row][col].g;
			out[row][col].r = in[row][col].r;
			out[row][col].a = in[row][col].a;
			fseek(fp, padding, SEEK_CUR);
		}
	}
	return 0;
}

int write_image(FILE *outf, FILE *in) {
	int c;

	rewind(outf);
	rewind(in);

	c = fgetc(in);
	while(c != EOF) {
		fputc(c, outf);
		c = fgetc(in);
	}

	int bytes_per_row = IMG_COLS * bpp / 8;
	int padding = 0;
	if(bytes_per_row % 4 != 0) {
		padding = 4 - bytes_per_row % 4;
	}
	int row;
	int col;
	printf("output padding %u\n", padding);
	fseek(outf, pixel_arr_loc, SEEK_SET);
	for(row = IMG_ROWS - 1; row >= 0; row--) {
		for( col = 0; col < IMG_COLS; col++) {
			fwrite(&out[row][col].b, 1, 1, outf);
			fwrite(&out[row][col].g, 1, 1, outf);
			fwrite(&out[row][col].r, 1, 1, outf);
			fwrite(&out[row][col].a, 1, 1, outf);
			fseek(outf, padding, SEEK_CUR);
		}
	}
	return 0;
}

void filter_grayscale() {

	for(row=0; row<IMG_ROWS; row++) {
		for(col=0; col<IMG_COLS; col++) {
			if(in[row][col].r > in[row][col].g && in[row][col].r > in[row][col].b) {
			//	out[row][col].r = in[row][col].r;
				out[row][col].g = in[row][col].r;
				out[row][col].b = in[row][col].r;
			} else if(in[row][col].g > in[row][col].r && in[row][col].g > in[row][col].b) {
				out[row][col].r = in[row][col].g;
			//	out[row][col].g = in[row][col].g;
				out[row][col].b = in[row][col].g;
			} else if(in[row][col].b > in[row][col].r && in[row][col].b > in[row][col].g) {
				out[row][col].r = in[row][col].g;
				out[row][col].g = in[row][col].g;
			//	out[row][col].b = in[row][col].g;
			}
		}
	}
	
}

void filter_red_balloon() {
	
	for(row=0; row<IMG_ROWS; row++) {
		for(col=0; col<IMG_COLS; col++) {
			if(close_to_red(in[row][col].r, in[row][col].g, in[row][col].b) == 0) {
				if(in[row][col].r > in[row][col].g && in[row][col].r > in[row][col].b) {
					out[row][col].g = in[row][col].r;
					out[row][col].b = in[row][col].r;
				} else if(in[row][col].g > in[row][col].r && in[row][col].g > in[row][col].b) {
					out[row][col].r = in[row][col].g;
					out[row][col].b = in[row][col].g;
				} else if(in[row][col].b > in[row][col].r && in[row][col].b > in[row][col].g) {
					out[row][col].r = in[row][col].g;
					out[row][col].g = in[row][col].g;
				}
			} else {
				out[row][col].r = in[row][col].r;
				out[row][col].g = in[row][col].g;
				out[row][col].b = in[row][col].b;
			}
		}
	}
	
}

int close_to_red(unsigned int red, unsigned int green, unsigned int blue) {
	const double R = 255.0;
	const double G = 0.0;
	const double B = 0.0;
	const double R_tolerance = 125.0;
	const double G_B_tolerance = 50.0;
	
	if(fabs((double)red-255.0) < R_tolerance && fabs((double)green-0.0) < G_B_tolerance && fabs((double)blue-0.0) < G_B_tolerance) {
		return 1;
	} else {
		return 0;
	}
}
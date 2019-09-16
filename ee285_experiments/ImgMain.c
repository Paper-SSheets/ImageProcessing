#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "Img.h"

// Global Variables
char input_file_name[] = "cy.bmp";
char output_file_name[] = "cy2.bmp";

// Main function
int main() {

	FILE *in = fopen(input_file_name, "rb");
	FILE *out = fopen(output_file_name, "wb");
	load_image(in);

	filter_red_balloon();
//	filter_grayscale();
	
	write_image(out, in);
	fclose(in);
	fclose(out);
	
	return 0;
}
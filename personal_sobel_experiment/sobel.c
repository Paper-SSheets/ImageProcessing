#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

// @author Steven Marshall Sheets - SMSheets@IAState.edu

// gcc -g -lm -o sobel.out sobel.c
// ./sobel.out

// https://www.youtube.com/watch?v=uihBwtPIBxM

/* Do not modify write_pgm() or read_pgm() */
int write_pgm(char *file, void *image, uint32_t x, uint32_t y) {
    FILE *o;

    if (!(o = fopen(file, "w"))) {
        perror(file);
        return -1;
    }

    fprintf(o, "P5\n%u %u\n255\n", x, y);

    /* Assume input data is correctly formatted. *
    *  There's no way to handle it, otherwise.   */

    if (fwrite(image, 1, x * y, o) != (x * y)) {
        perror("fwrite");
        fclose(o);
        return -1;
    }

    fclose(o);

    return 0;
}

/* A better implementation of this function would read the image dimensions *
 * from the input and allocate the storage, setting x and y so that the     *
 * user can determine the size of the file at runtime.  In order to         *
 * minimize complication, I've written this version to require the user to  *
 * know the size of the image in advance.                                   */
int read_pgm(char *file, void *image, uint32_t x, uint32_t y) {
    FILE *f;
    char s[80];
    unsigned i, j;

    if (!(f = fopen(file, "r"))) {
        perror(file);
        return -1;
    }

    if (!fgets(s, 80, f) || strncmp(s, "P5", 2)) {
        fprintf(stderr, "Expected P6\n");
        return -1;
    }

    /* Eat comments */
    do {
        fgets(s, 80, f);
    } while (s[0] == '#');

    if (sscanf(s, "%u %u", &i, &j) != 2 || i != x || j != y) {
        fprintf(stderr, "Expected x and y dimensions %u %u\n", x, y);
        fclose(f);
        return -1;
    }

    /* Eat comments */
    do {
        fgets(s, 80, f);
    } while (s[0] == '#');

    if (strncmp(s, "255", 3)) {
        fprintf(stderr, "Expected 255\n");
        fclose(f);
        return -1;
    }

    if (fread(image, 1, x * y, f) != x * y) {
        perror("fread");
        fclose(f);
        return -1;
    }

    fclose(f);

    return 0;
}

int main(int argc, char *argv[]) {
    int8_t image[1024][1024];
    int8_t out[1024][1024];

    /* Example usage of PGM functions */
    /* This assumes that motorcycle.pgm is a pgm image of size 1024 x 1024 */
    //read_pgm("motorcycle.pgm", image, 1024, 1024);

    char filename[50];    // Thank you Le Chen for the help!
    printf("Please enter the file name with the file extension: \n");
    scanf("%s", filename);
    read_pgm(filename, image, 1024, 1024);

    const int matrix_length = 3;
    const int x_transform[3][3] = {{-1, 0, 1},
                                   {-2, 0, 2},
                                   {-1, 0, 1}};
    const int y_transform[3][3] = {{-1, -2, -1},
                                   {0,  0,  0},
                                   {1,  2,  1}};

    int r, c, j, i;
    for (r = 0; r < 1024; r++) {
        for (c = 0; c < 1024; c++) {
            int x_accumulator = 0;    // For some odd reason, declaring these on the same
            int y_accumulator = 0;    // line produces drastically different results.

            for (j = 0; j < matrix_length; j++) {
                for (i = 0; i < matrix_length; i++) {
                    int row = r + (j - ceil(matrix_length / 2));
                    int col = c + (i - ceil(matrix_length / 2));

                    x_accumulator += x_transform[j][i] * image[row][col];
                    y_accumulator += y_transform[j][i] * image[row][col];
                }
            }

            out[r][c] = sqrt((x_accumulator * x_accumulator) + (y_accumulator * y_accumulator));
        }
    }

    /* After processing the image and storing your  *
     * output in "out", write to motorcycle.edge.pgm. 	*/
    int length = strlen(filename);
    int x = 0;
    char file2[50];    // Thanks Le Chen!
    while (x < strlen(filename) - 4) {
        file2[x] = filename[x];
        x++;
    }

    write_pgm(strcat(file2, ".edge.pgm"), out, 1024, 1024);

    return 0;
}
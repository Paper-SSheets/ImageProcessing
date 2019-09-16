#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

/* Do not modify write_pgm() or read_pgm() */
int write_pgm(char *file, void *image, uint32_t x, uint32_t y) {
    FILE *o;

    if (!(o = fopen(file, "w"))) {
        perror(file);

        return -1;
    }

    fprintf(o, "P5\n%u %u\n255\n", x, y);

    /* Assume input data is correctly formatted. *
     * There's no way to handle it, otherwise.   */

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
        fprintf(stderr, "Expected P5\n");

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
    uint8_t image[1024][1024];
    int16_t x_filter[3][3] = {{-1, 0, 1},
                              {-2, 0, 2},
                              {-1, 0, 1}};
    int16_t y_filter[3][3] = {{-1, -2, -1},
                              {0,  0,  0},
                              {1,  2,  1}};
    uint8_t out[1024][1024];
    int16_t x_out, y_out, xy_out;
    int x, y;
    char *s;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <PGM file>\n", argv[0]);

        return -1;
    }

    read_pgm(argv[1], image, 1024, 1024);
    for (y = 0; y < 1024; y++) {
        for (x = 0; x < 1024; x++) {
            x_out = y_out = 0;
            if (x != 0 && y != 0 && x != 1023 && y != 1023) {
                x_out = ((((int16_t) image[y - 1][x - 1]) * x_filter[0][0]) +
                         (((int16_t) image[y - 1][x]) * x_filter[0][1]) +
                         (((int16_t) image[y - 1][x + 1]) * x_filter[0][2]) +
                         (((int16_t) image[y][x - 1]) * x_filter[1][0]) +
                         (((int16_t) image[y][x]) * x_filter[1][1]) +
                         (((int16_t) image[y][x + 1]) * x_filter[1][2]) +
                         (((int16_t) image[y + 1][x - 1]) * x_filter[2][0]) +
                         (((int16_t) image[y + 1][x]) * x_filter[2][1]) +
                         (((int16_t) image[y + 1][x + 1]) * x_filter[2][2]));
                y_out = ((((int16_t) image[y - 1][x - 1]) * y_filter[0][0]) +
                         (((int16_t) image[y - 1][x]) * y_filter[0][1]) +
                         (((int16_t) image[y - 1][x + 1]) * y_filter[0][2]) +
                         (((int16_t) image[y][x - 1]) * y_filter[1][0]) +
                         (((int16_t) image[y][x]) * y_filter[1][1]) +
                         (((int16_t) image[y][x + 1]) * y_filter[1][2]) +
                         (((int16_t) image[y + 1][x - 1]) * y_filter[2][0]) +
                         (((int16_t) image[y + 1][x]) * y_filter[2][1]) +
                         (((int16_t) image[y + 1][x + 1]) * y_filter[2][2]));
            }
            xy_out = round(sqrt((x_out * x_out) + (y_out * y_out)));
            if (xy_out > 255) {
                xy_out = 255;
            }
            out[y][x] = xy_out;
        }
    }

    /* Construct a string from the input image name with ".edge" stuck between *
     * the name and the extension.  5 bytes for ".edge" and the NULL byte.     */
    s = malloc(strlen(argv[0] + 6));
    strcpy(s, argv[1]);
    /* BUG: Fix it for a little challenge, but there's no bounty for this.  If  *
     * the input file name doesn't end with .pgm, or if it contains extra dots, *
     * this will not construct a correct output file name.  Correct solution is *
     * to iterate backwards through the file name and ensure that you've found  *
     * the last dot and that the file has the appropriate extension (which      *
     * could require a larger allocation).                                      */
    *strchr(s, '.') = '\0';
    strcat(s, ".edge.pgm");
    write_pgm(s, out, 1024, 1024);
    free(s);

    return 0;
}

/*
	I construct an output file name from the input file name.  This construction is slightly buggy, but you can fix it if you want (no bug bounty, though).
    See the comment in the source code for details. I don't use a loop for the convolution.  Instead, I implement the whole convolution in a single expression.
    The reason is entirely one of performance.  That the expanded, hard-coded expression is much, much, much faster than the little 3x3 loop that you probably wrote.
    Go ahead and time them to confirm.  You can time an application running in the shell in Lunux by running it using the "time" utility, e.g., "time sobel
    motorcycle.pgm".  This timer is not good enough for real scientific analysis, but it's plenty good enough for something like this. I cast all
    of my index operations into the input image to int16_t (a 16-bit, signed integer type) to avoid the sign and overflow issues that lead to "grainy" output.
*/
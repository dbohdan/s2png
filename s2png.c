/*
 *   s2png - "something to png" copyright (c) 2006 k0wax
 *   Updates copyright (c) 2013 by dbohdan
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sysexits.h>

#include "gd.h"
#include "gdfontt.h"
#include "rc4.h"

#define VERSION_STR ("0.04")
#define BANNER_HEIGHT 8
#define MODE_ENCODE 1
#define MODE_DECODE 2

#define RC4_DROP_N 3072

#define DEFAULT_BANNER ("This image contains binary data. \
To extract it get s2png on GitHub.")

void init_rc4(char *password, rc4_key *key)
{
    if (password != NULL) {
        int n;
        char seed[256];
        n = pass_hash(password, seed);
        prepare_key(seed, n, key);
        drop_n(RC4_DROP_N, key);      
    }
}

int png_to_file(char *finfn, char *foutfn, char *password)
{
    FILE *fin, *fout;
    struct stat fin_stat;
    fin = fopen(finfn, "rb");
    gdImagePtr im = gdImageCreateFromPng(fin);
    fclose(fin);

    int c = gdImageGetPixel(im, gdImageSX(im) - 1, gdImageSY(im) - 1);
    int data_size = (gdImageRed(im, c) << 8*2) +
                    (gdImageGreen(im, c) << 8*1) + (gdImageBlue(im, c));

    fout = fopen(foutfn, "wb");
    unsigned char buf[3];
    long written_bytes = 0;
    int x, y;
    int nb = 0;

    rc4_key key;
    init_rc4(password, &key);


    for(y=0; y < gdImageSY(im); y++) {
        for(x=0; x < gdImageSX(im); x++) {
            c = gdImageGetPixel(im, x, y);
            buf[0] = gdImageRed(im, c);
            buf[1] = gdImageGreen(im, c);
            buf[2] = gdImageBlue(im, c);
            if (password != NULL) {
                rc4(buf, 3, &key);
            }
            if (written_bytes >= data_size) {
                break; /* FIXME */
            } else {
                nb = (written_bytes + 3 > data_size ?
                     data_size - written_bytes : 3);
                written_bytes += fwrite(buf, 1, nb, fout);
            }
        }
    }
    fclose(fout);

    gdImageDestroy(im);
    return 1;
}

int file_to_png(char *finfn, char *foutfn, int image_width, int make_square,
              char *banner, char *password)
{
    FILE *fin, *fout;
    struct stat fin_stat;
    struct stat fout_stat;
    gdImagePtr im = NULL;

    fin = fopen(finfn, "rb");
    fstat(fileno(fin), &fin_stat);
    int data_size = fin_stat.st_size; /* st_size is off_t, which is int. */
    int banner_height = (strcmp(banner, "") == 0 ? 0 : BANNER_HEIGHT);

    if (make_square) {
        /* Solve (x - banner_height) * x = data_size / 3.0 for x. */
        image_width = ceil(0.5 * sqrt(4 * (float) data_size / 3.0 + 
                           (float) banner_height * (float) banner_height)
                           + (float) banner_height);
    }

    int image_height = ceil(((float) data_size / (float) image_width / 3.0) 
                            + (float) banner_height);

    /* printf("%d %d\n", image_width, image_height); */

    im = gdImageCreateTrueColor(image_width, image_height);

    unsigned char buf[3];
    long bytes_read = 0;
    long total_bytes = 0;
    int x = 0;
    int y = 0;

    rc4_key key;
    init_rc4(password, &key);

    while ((bytes_read = fread(buf, 1, 3, fin)) > 0) {
        if (password != NULL) {
            rc4(buf, 3, &key);
        }
        total_bytes += bytes_read;
        gdImageSetPixel(im, x, y, gdImageColorAllocate(im, buf[0], buf[1],
                        buf[2]));

        if (x + 1 < image_width) {
            x++;
        } else {
            x = 0;
            y++;
        }
    }

    fclose(fin);

    char *s = banner;
    gdImageFilledRectangle(im, 0, gdImageSY(im) - banner_height,
                           image_width - 1, gdImageSY(im) + banner_height,
                           gdImageColorAllocate(im, 255, 255, 255));
    gdImageString(im, (gdFontPtr) gdFontGetTiny(), 5,
                  gdImageSY(im) - banner_height, s,
                  gdImageColorAllocate(im, 0, 0, 0));
    /* store data_size in the last pixel */
    gdImageSetPixel(im, gdImageSX(im) - 1, gdImageSY(im) - 1,
                    gdImageColorAllocate(im, (data_size & 0xff0000) >> 8*2,
                    (data_size & 0xff00) >> 8*1, data_size & 0xff));

    fout = fopen(foutfn, "wb");
    gdImagePng(im, fout);
    fclose(fout);

/*
    int c = gdImageGetPixel(im, gdImageSX(im) - 1, gdImageSY(im) - 1);
    int ds = (gdImageRed(im, c) << 8*2) + (gdImageGreen(im, c) << 8*1) +
              (gdImageBlue(im, c));
    printf("debug: ds %d, data_size %d\n", ds, data_size);
    //printf("c: %d %d %d\n", (data_size & 0xff0000) >> 8*2,
             (data_size & 0xff00) >> 8*1, data_size & 0xff);
    //printf("d: %d %d %d\n", (gdImageRed(im, c) << 8*2),
             (gdImageGreen(im, c) << 8*1), (gdImageBlue(im, c)));
*/

    gdImageDestroy(im);

    stat(foutfn, &fout_stat);
    return (int) fout_stat.st_size;
}

int is_png_file(char *filename)
{
    char png_sign[8] = {0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a};
    char buf[8];
    size_t res;

    FILE *fp = fopen(filename, "rb");
    res = fread(buf, 8, 1, fp);
    fclose(fp);

    if (memcmp(buf, png_sign, 8) == 0)
        return 1;

    return 0;
}

void usage()
{
    printf("s2png (\"something to png\") version %s\n", VERSION_STR);
    printf("usage: s2png [-h] [-o filename] [-w width (600) | -s] [-b text]\n\
             [-p password] file\n");
}

void help()
{
    usage();
    printf("\n\
  -h            display this message and quit\n\
  -o filename   output the converted data (image or binary) to filename\n\
  -w width      set the width of PNG image output (600 by default)\n\
  -s            make the output image roughly square\n\
  -b text       custom banner text (\"\" for no banner)\n\
  -p password   encrypt/decrypt the output with password using RC4\n\
                (Warning: do not use this if you need actual security!)\n\
\n\
See README.md for details.\n");
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        usage();
        return EX_USAGE;
    }

    /* opts */
    char *in_fn = NULL;
    char *out_fn = NULL;
    char *banner_text = DEFAULT_BANNER;
    char *password = NULL;
    int image_width = 600;
    int make_square = 0;

    int ch;
    while ((ch = getopt(argc, argv, "w:o:hsb:p:")) != -1) {
        switch (ch) {
            case 'w':
                image_width = atoi(optarg);
                break;
            case 's':
                make_square = 1;
                break;
            case 'o':
                out_fn = optarg;
                break;
            case 'b':
                banner_text = optarg;
                break;
            case 'p':
                password = optarg;
                break;
            case 'h':
                help();
                return EX_OK;
                break;
        }

        /*
        argc -= optind;
        argv += optind;
        */
    }

    in_fn = argv[argc - 1];
    if (access(in_fn, R_OK) != 0) {
        printf("error: can't open file `%s'\n", in_fn);
        return EX_NOINPUT;
    }

    int mode = is_png_file(in_fn) ? MODE_DECODE : MODE_ENCODE;

    if (out_fn == NULL) {
        if (mode == MODE_DECODE) {
            if (strcasecmp((in_fn + strlen(in_fn) - 4), ".png") == 0) {
                strncpy(out_fn, in_fn, strlen(in_fn) - 4);
            } else {
                strcpy(out_fn, in_fn);
                strcat(out_fn, ".orig");
                fprintf(stderr, "warn: file `%s' will be saved as `%s'\n",
                        in_fn, out_fn);
            }
        } else if (mode == MODE_ENCODE) {
            out_fn = calloc(strlen(in_fn) + 4, sizeof(char));
            strcpy(out_fn, in_fn);
            strcat(out_fn, ".png");
        }
    }

    /* printf("in_fn: %s\nout_fn = %s\n", in_fn, out_fn); */

    /* check opts */
    char *err = calloc(1, sizeof(char) * 255);
    if (!image_width && mode == MODE_ENCODE) {
        strcpy(err, "image width not set or invalid");
    } else if (!in_fn) {
        strcpy(err, "no input file");
    } else if (!out_fn) {
        strcpy(err, "no output file");
    }

    if (strlen(err) > 0) {
        fprintf(stderr, "%s, see -h option for help\n", err);
        return EX_USAGE;
    }


    if (mode == MODE_ENCODE) {
        file_to_png(in_fn, out_fn, image_width, make_square, banner_text,
                    password);
    } else if (mode == MODE_DECODE) {
        png_to_file(in_fn, out_fn, password);
    } else {
        fprintf(stderr, "internal error: unknown mode\n");
        return EX_SOFTWARE;
    }

    return EX_OK;
}

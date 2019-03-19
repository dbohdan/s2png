/*
 *   s2png - "stuff to png"
 *   Copyright (c) 2006 k0wax
 *   Copyright (c) 2013, 2014, 2015, 2016, 2017, 2018, 2019 dbohdan
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 */
#define _POSIX_C_SOURCE 200112L

#include <getopt.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sysexits.h>
#include <unistd.h>

#include "gd.h"
#include "gdfontt.h"
#include "rc4.h"

#define VERSION_STR ("0.8.0")
#define BANNER_HEIGHT 8
#define MODE_NONE 0
#define MODE_ENCODE 1
#define MODE_DECODE 2

#define RC4_DROP_N 3072
#define MAX_FILE_SIZE 0xFFFFFF
#define MAX_IMAGE_WIDTH 0xFFFF

#define DEFAULT_WIDTH 600
#define DEFAULT_BANNER ("This image contains binary data. \
To extract it get s2png on GitHub.")

bool init_rc4(char *password, struct rc4_key *key)
{
    size_t n;
    uint8_t seed[256];
    bool valid;

    if (password == NULL) {
        return true;
    }

    valid = pass_hash(password, seed, &n);
    if (!valid) {
        fprintf(stderr, "error: password is not a hexadecimal string\n");
        return false;
    }
    if (n == 0) {
        fprintf(stderr, "error: password is empty\n");
        return false;
    }
    prepare_key(seed, n, key);
    drop_n(RC4_DROP_N, key);

    return true;
}

int png_to_file(char *fin_fn, char *fout_fn, char *password)
{
    FILE *fin, *fout;

    fin = fopen(fin_fn, "rb");
    gdImagePtr im = gdImageCreateFromPng(fin);
    fclose(fin);

    /* Has libgd been able to read and interpret fin? */
    if (im == NULL) {
        fprintf(stderr, "error: file `%s' is not readable as PNG\n", fin_fn);
        return EX_DATAERR;
    }

    /* Get the data size stored in the bottom right pixel of the image. */
    uint32_t c = gdImageGetPixel(im, gdImageSX(im) - 1, gdImageSY(im) - 1);
    size_t data_size = (gdImageRed(im, c) << 8*2) +
                    (gdImageGreen(im, c) << 8*1) + (gdImageBlue(im, c));

    fout = fopen(fout_fn, "wb");
    if (fout == NULL) {
        fprintf(stderr, "error: can't open file `%s' for output\n", fout_fn);
        return EX_CANTCREAT;
    }

    uint8_t buf[3];
    size_t written_bytes = 0;
    size_t nb = 0;
    int32_t x;
    int32_t y;

    struct rc4_key key;
    if (!init_rc4(password, &key)) {
        return EX_DATAERR;
    };

    /* For each pixel of the image... */
    for (y = 0; y < gdImageSY(im); y++) {
        for (x = 0; x < gdImageSX(im); x++) {
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
    return EX_OK;
}

int file_to_png(char *fin_fn, char *fout_fn, uint32_t image_width,
                bool make_square, char *banner, char *password)
{
    FILE *fin, *fout;
    struct stat fin_stat;
    gdImagePtr im = NULL;

    fin = fopen(fin_fn, "rb");
    fstat(fileno(fin), &fin_stat);
    size_t data_size = fin_stat.st_size;
    if (data_size > MAX_FILE_SIZE) {
        fprintf(stderr,
                "error: file `%s' too large to encode (over %u bytes)\n",
                fin_fn, MAX_FILE_SIZE);
        return EX_DATAERR;
    }

    /* If given no banner text hide the banner. */
    uint32_t banner_height = (strcmp(banner, "") == 0 ? 0 : BANNER_HEIGHT);

    if (make_square) {
        /* Solve for x: (x - banner_height) * x = data_size / 3.0 */
        image_width = ceil(0.5 * sqrt(4 * (float) data_size / 3.0 +
                           (float) banner_height * (float) banner_height)
                           + (float) banner_height);
    }

    uint32_t image_height = ceil(((float) data_size / (float) image_width / 3.0)
                            + (float) banner_height);

    /* Prevent data corruption on images with no banner. On such images
       the bottom right pixel would be used for data and then overwritten
       by the file size pixel. */
    if (banner_height == 0 &&
        image_width * image_height * 3 - data_size <= 2) {
        image_width++;
    }

    im = gdImageCreateTrueColor(image_width, image_height);

    uint8_t buf[3];
    size_t bytes_read = 0;
    size_t total_bytes = 0;
    uint32_t x = 0;
    uint32_t y = 0;

    struct rc4_key key;
    if (!init_rc4(password, &key)) {
        return EX_DATAERR;
    }

    /* Read the input file in sets of three bytes. */
    while ((bytes_read = fread(buf, 1, 3, fin)) > 0) {
        if (password != NULL) {
            rc4(buf, 3, &key);
        }
        total_bytes += bytes_read;
        gdImageSetPixel(im, x, y, gdImageColorAllocate(im, buf[0], buf[1],
                        buf[2]));

        /* Arrange the input data on the 2D grid of the image. */
        if (x + 1 < image_width) {
            x++;
        } else {
            x = 0;
            y++;
        }
    }

    fclose(fin);

    /* Add a banner at the bottom of the image. */
    gdImageFilledRectangle(im, 0, gdImageSY(im) - banner_height,
                           image_width - 1, gdImageSY(im) + banner_height,
                           gdImageColorAllocate(im, 255, 255, 255));
    gdImageString(im, (gdFontPtr) gdFontGetTiny(), 5,
                  gdImageSY(im) - banner_height, (unsigned char *) banner,
                  gdImageColorAllocate(im, 0, 0, 0));
    /* Store the data_size in the bottom right ("last") pixel. */
    gdImageSetPixel(im, gdImageSX(im) - 1, gdImageSY(im) - 1,
                    gdImageColorAllocate(im, (data_size & 0xff0000) >> 8*2,
                    (data_size & 0xff00) >> 8*1, data_size & 0xff));

    /* Save the created image in a PNG file. */
    fout = fopen(fout_fn, "wb");
    if (fout == NULL) {
        fprintf(stderr, "error: can't open file `%s' for output\n",
                fout_fn);
        return EX_CANTCREAT;
    }
    gdImagePng(im, fout);
    fclose(fout);

    gdImageDestroy(im);

    return EX_OK;
}

bool is_png_file(char *filename)
{
    const uint8_t png_sign[8] = {0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a,
                                 0x1a, 0x0a};
    uint8_t buf[8];
    size_t res;

    FILE *fp = fopen(filename, "rb");
    res = fread(buf, sizeof(buf), 1, fp);
    fclose(fp);

    if (res > 0) {
        if (memcmp(buf, png_sign, sizeof(png_sign)) == 0) {
            return true;
        }
    }

    return false;
}

void usage()
{
    printf("s2png (\"stuff to png\") version %s\n", VERSION_STR);
    printf("usage: s2png [-h] [-o filename] [-w width (%u) | -s] [-b text]\n\
             [-p password] [-e | -d] file\n", DEFAULT_WIDTH);
}

void help()
{
    usage();
    printf("\
\n\
Store any data in a PNG image.\n\
This version can encode files of up to %u bytes.\n\
\n\
  -h            display this message and quit\n\
  -o filename   output the encoded or decoded data to filename\n\
  -w width      set the width of the PNG image output (%u by default)\n\
  -s            make the output image roughly square\n\
  -b text       custom banner text (\"\" for no banner)\n\
  -p password   encrypt/decrypt the output with a hexadecimal password\n\
                using RC4\n\
                (Warning: do not use this if you want actual secrecy!)\n\
Normally s2png detects which operation to perform by the file type. You can\n\
override this behavior with the following switches:\n\
  -e            force encoding mode\n\
  -d            force decoding mode\n\
\n\
See README.md for further details.\n", MAX_FILE_SIZE, DEFAULT_WIDTH);
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        usage();
        return EX_USAGE;
    }

    char *in_fn = NULL;
    char *out_fn = NULL;
    char *banner_text = DEFAULT_BANNER;
    char *password = NULL;
    uint32_t image_width = DEFAULT_WIDTH;
    bool make_square = false;
    uint8_t mode = MODE_NONE;

    /* Process the command line options. */
    int ch;
    while ((ch = getopt(argc, argv, "w:o:hsb:p:ed")) != -1) {
        switch (ch) {
            case 'w':
                image_width = atoi(optarg);
                break;
            case 's':
                make_square = true;
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
            case 'e':
                mode = MODE_ENCODE;
                break;
            case 'd':
                mode = MODE_DECODE;
                break;
            case 'h':
                help();
                return EX_OK;
                break;
        }
    }

    in_fn = argv[argc - 1];

    struct stat stat_buf;
    /* Does the input file exist? */
    if (stat(in_fn, &stat_buf) != 0) {
        fprintf(stderr, "error: can't access input file `%s'\n", in_fn);
        return EX_NOINPUT;
    }

    /* Is the input path a file or a directory? */
    if (S_ISDIR(stat_buf.st_mode)) {
        fprintf(stderr, "error: input path `%s' is a directory, not a file\n",
                in_fn);
        return EX_DATAERR;
    }

    /* If we weren't explicitly told what operation to perform through
       a command line option we decide based on the file type. */
    if (mode == MODE_NONE) {
        mode = is_png_file(in_fn) ? MODE_DECODE : MODE_ENCODE;
    }

    /* If no output file name is given we generate one. */
    if (out_fn == NULL) {
        out_fn = calloc(strlen(in_fn) + strlen(".orig"), sizeof(char));
        if (mode == MODE_DECODE) {
            if (strcasecmp((in_fn + strlen(in_fn) - 4), ".png") == 0) {
                strncpy(out_fn, in_fn, strlen(in_fn) - 4);
            } else {
                strcpy(out_fn, in_fn);
                strcat(out_fn, ".orig");
                fprintf(stderr, "warning: file `%s' will be saved as `%s'\n",
                        in_fn, out_fn);
            }
        } else if (mode == MODE_ENCODE) {
            strcpy(out_fn, in_fn);
            strcat(out_fn, ".png");
        }
    }

    /* Check for invalid image width. */
    if ((mode == MODE_ENCODE) &&
        ((image_width == 0) || (image_width > MAX_IMAGE_WIDTH))) {
        fprintf(stderr, "error: invalid image width; "
                "must be between 1 and %u\n", MAX_IMAGE_WIDTH);
        return EX_USAGE;
    }

    /* Perform the chosen operation. */
    if (mode == MODE_ENCODE) {
        return file_to_png(in_fn, out_fn, image_width, make_square, banner_text,
                           password);
    } else if (mode == MODE_DECODE) {
        return png_to_file(in_fn, out_fn, password);
    } else {
        fprintf(stderr, "internal error: unknown mode\n");
        return EX_SOFTWARE;
    }

    return EX_OK;
}

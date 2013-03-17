/*
 *   s2png - "something to png" copyleft (c) 2006 k0wax
 *   Updated in 2013 by dbohdan
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
#include "gd.h"
#include "gdfontt.h"

#define VERSION_STR ("0.02")
#define VERSION_DATE ("20130317")
#define BANNER_HEIGHT 8
#define MODE_ENCODE 1
#define MODE_DECODE 2

int exists(const char *fn)
{
    FILE *t;
    if((t = fopen(fn, "rb"))) { fclose(t); return 1; }
    return 0;
}

int pngtofile(char *finfn, char *foutfn)
{
    FILE *fin, *fout;
    struct stat fin_stat;
    fin = fopen(finfn, "rb");
    gdImagePtr im = gdImageCreateFromPng(fin);
    fclose(fin);

    int c = gdImageGetPixel(im, gdImageSX(im) - 1, gdImageSY(im) - 1);
    int data_size = (gdImageRed(im, c) << 8*2) + (gdImageGreen(im, c) << 8*1) + (gdImageBlue(im, c));

    fout = fopen(foutfn, "wb");
    unsigned char buf[3];
    long written_bytes = 0;
    int x, y;
    int nb = 0;
    for(y=0; y < gdImageSY(im); y++) {
        for(x=0; x < gdImageSX(im); x++) {
            c = gdImageGetPixel(im, x, y);
            buf[0] = gdImageRed(im, c);
            buf[1] = gdImageGreen(im, c);
            buf[2] = gdImageBlue(im, c);
            if(written_bytes >= data_size) {
                break; /* FIXME */
            } else {
                nb = written_bytes + 3 > data_size ?
                    data_size - written_bytes : 3;
                written_bytes += fwrite(buf, 1, nb, fout);
            }
        }
    }
    fclose(fout);

    gdImageDestroy(im);
    return 1;
}

int filetopng(char *finfn, char *foutfn, int im_w)
{
    FILE *fin, *fout;
    struct stat fin_stat;
    gdImagePtr im = NULL; 

    fin = fopen(finfn, "rb");
    fstat(fileno(fin), &fin_stat);
    long data_size = fin_stat.st_size;
    
/*
    // FIXME
    //int im_h = (data_size) % 3 == 0 ?
    //    (data_size/3/im_w + BANNER_HEIGHT) : (((data_size+3)/3/im_w) + BANNER_HEIGHT);
    //printf("data_size %d, im_h %d im_w %d\n", data_size, im_h, im_w);
*/
    int im_h = ceil((float) ((float) data_size / (float) im_w / (float) 3) + (float) BANNER_HEIGHT);
    im = gdImageCreateTrueColor(im_w, im_h);

    unsigned char buf[3];
    long bytes_read = 0;
    long total_bytes = 0;
    int x = 0;
    int y = 0;
    while((bytes_read = fread(buf, 1, 3, fin)) > 0) {
        total_bytes += bytes_read;
        gdImageSetPixel(im, x, y, gdImageColorAllocate(im, buf[0], buf[1], buf[2]));

        if(x + 1 < im_w) {
            x++;
        } else {
            x = 0;
            y++;
        }
    }

    fclose(fin);

    char *s = "This image contains binary data. To extract it use s2png from http://s2png.sf.net";
    gdImageFilledRectangle(im, 0, gdImageSY(im) - BANNER_HEIGHT,
        im_w - 1, gdImageSY(im) + BANNER_HEIGHT, gdImageColorAllocate(im, 255, 255, 255));
    gdImageString(im, (gdFontPtr) gdFontGetTiny(), 5, gdImageSY(im) - BANNER_HEIGHT,
        s, gdImageColorAllocate(im, 0, 0, 0));
    /* store data_size in the last pixel */
    gdImageSetPixel(im, gdImageSX(im) - 1, gdImageSY(im) - 1,
        gdImageColorAllocate(im, (data_size & 0xff0000) >> 8*2, (data_size & 0xff00) >> 8*1, data_size & 0xff));

    fout = fopen(foutfn, "wb");
    gdImagePng(im, fout);
    fclose(fout);

/*
    int c = gdImageGetPixel(im, gdImageSX(im) - 1, gdImageSY(im) - 1);
    int ds = (gdImageRed(im, c) << 8*2) + (gdImageGreen(im, c) << 8*1) + (gdImageBlue(im, c));
    printf("debug: ds %d, data_size %d\n", ds, data_size);
    //printf("c: %d %d %d\n", (data_size & 0xff0000) >> 8*2, (data_size & 0xff00) >> 8*1, data_size & 0xff);
    //printf("d: %d %d %d\n", (gdImageRed(im, c) << 8*2), (gdImageGreen(im, c) << 8*1), (gdImageBlue(im, c)));
*/

    gdImageDestroy(im);
    return 1;
}

int is_png_file(char *filename)
{
    char png_sign[8] = {0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a};
    char buf[8];

    FILE *fp = fopen(filename, "rb"); 
    fread(buf, 8, 1, fp);
    fclose(fp);

    if(memcmp(buf, png_sign, 8) == 0)
        return 1;

    return 0;
}

void usage()
{
    printf("s2png (\"something to png\") version %s\n", VERSION_STR);
    printf("usage: s2png [-h] [-o filename] [-w width (600)] file\n");
}

void help()
{
    usage();
    printf("\n\
  -h            display this message and quit\n\
  -o filename   output the coverted data (image or binary) to filename\n\
  -w            set the width of PNG image output (600 by default)\n\
\n\
See README.md for details.\n");
}

int main(int argc, char **argv)
{
    if(argc < 2) {
        usage();
        exit(1);
    }

    /* opts */
    char *in_fn = NULL;
    char *out_fn = calloc(1, sizeof(char) * 255);
    int im_w = 600;

    int ch;
    while((ch = getopt(argc, argv, "w:o:h")) != -1) {
        switch(ch) {
            case 'w':
                im_w = atoi(optarg);
                break;
            case 'o':
                out_fn = optarg;
                break;
            case 'h':
                help();
                exit(1);
                break;
        }
        
        /*
        argc -= optind;
        argv += optind;
        */
    }

    in_fn = argv[argc - 1];
    if(!exists(in_fn)) { 
        printf("error: can't open file `%s'\n", in_fn);
        exit(-1);
    }

    int mode = is_png_file(in_fn) ? MODE_DECODE : MODE_ENCODE;

    if(!strlen(out_fn)) {
        if(mode == MODE_DECODE) {
            if(strcasecmp((in_fn + strlen(in_fn) - 4), ".png") == 0) {
                strncpy(out_fn, in_fn, strlen(in_fn) - 4);
            } else {
                strcpy(out_fn, in_fn);
                strcat(out_fn, ".orig");
                fprintf(stderr, "warn: file `%s' will be saved as `%s'\n", in_fn, out_fn);
            }
        }
        else if(mode == MODE_ENCODE) {
            strcpy(out_fn, in_fn);
            strcat(out_fn, ".png");
        }
    }
    
    /* printf("in_fn: %s\nout_fn = %s\n", in_fn, out_fn); */

    /* check opts */
    char *err = calloc(1, sizeof(char) * 255);
    if(!im_w && mode == MODE_ENCODE) {
        strcpy(err, "image width not set or invalid");
    } else if(!in_fn) {
        strcpy(err, "no input file");
    } else if(!out_fn) {
        strcpy(err, "no output file");
    }

    if(strlen(err) > 0) {
        fprintf(stderr, "%s, see -h option for help\n", err);
        exit(-1);
    }


    if(mode == MODE_ENCODE) {
        filetopng(in_fn, out_fn, im_w);
    } else if (mode == MODE_DECODE) {
        pngtofile(in_fn, out_fn);
    } else {
        fprintf(stderr, "internal error: unknown mode\n");
        exit(-2);
    }

    return 0;
}

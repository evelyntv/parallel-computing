/*	File: main.c
 *
 * 	Purpose:	Implement histogram equalization to sharpen the quality of an image.
 * 
 *	Compile:	gcc serial.c -o serial
 *	Run:		./serial
 *
 *	Input:		images/lena512.bmp
 * 	Output:		images/lena_copy.bmp (histogram equalized)
 *
 *	Notes:
 *		1. 	The code for reading and writing BMP files was based off of Abhijit Nathwani's work 
 *			(https://abhijitnathwani.github.io/blog/2017/12/20/First-C-Program-for-Image-Processing)
 *		2. 	The algorithm for histogram equalization was adapted from Image Processing in C
 *			(2e) by Dwayne Phillips
 *
 *	Author: Evelyn Evans
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "timer.h"

const int height = 512;
const int width = 512;
const int image_size = 512*512; 
const int bmp_header_size = 54;
const int color_table_size = 1024;
const int nof_gray_shades = 256;
const int bloat = 16384; // 2^14

void initialize_histogram(int * histogram);
void read_bmp(unsigned char * buf, unsigned char * header, unsigned char * colorTable);
void write_bmp(unsigned char * buf, unsigned char * header, unsigned char * colorTable);
void calculate_histogram(unsigned char * buf, int * histogram);
void calculate_pdf(int * histogram, int * pdf);
void cdf(unsigned char * buf, unsigned char * out, int * pdf);

int main(int argc,char *argv[])
{
    unsigned char buf[image_size], out[image_size], header[bmp_header_size], colorTable[color_table_size];
	int histogram[nof_gray_shades], pdf[nof_gray_shades];
	double start_time, finish_time;

	initialize_histogram(histogram);
    read_bmp(buf, header, colorTable);
	calculate_histogram(buf, histogram);
	calculate_pdf(histogram, pdf);

	/* Start Critical Function */

	GET_TIME(start_time);

	for(int i = 0; i < bloat; i++) {
		cdf(buf, out, pdf);
	}

	GET_TIME(finish_time);

	/* End Critical Function */

    write_bmp(out, header, colorTable);
	printf("time elapsed: %f sec\n", finish_time - start_time);

	return 0;
}

void initialize_histogram(int * histogram) {
	for (int i = 0; i < nof_gray_shades; i++) {
		histogram[i] = 0;
	}
}

void read_bmp(unsigned char * buf, unsigned char * header, unsigned char * colorTable) {
	int i;

	FILE *streamIn; 
        streamIn = fopen("images/lena512.bmp", "r"); // Input file name
   
        if (streamIn == (FILE *)0) // check if the input file has not been opened succesfully.
	{
            printf("File opening error ocurred. Exiting program.\n");
            exit(0);
 	}
	
 	int count = 0;
 	for(i=0;i<54;i++) 
 	{
 		header[i] = getc(streamIn);  // strip out BMP header
 		
 	}
 	int width = *(int*)&header[18]; // read the width from the image header
 	int height = *(int*)&header[22]; // read the height from the image header
	int bitDepth = *(int*)&header[28]; // read the bitDepth from the image header

	if(bitDepth <= 8)
		fread(colorTable, sizeof(unsigned char), 1024, streamIn);


	printf("width: %d\n",width);
	printf("height: %d\n",height );

	fread(buf, sizeof(unsigned char), (height * width), streamIn);
	
 	fclose(streamIn);
}

void write_bmp(unsigned char * buf, unsigned char * header, unsigned char * colorTable) {
	int width = *(int*)&header[18]; // read the width from the image header
	int height = *(int*)&header[22]; // read the height from the image header
   	int bitDepth = *(int*)&header[28]; // read the bitDepth from the image header

	FILE *fo = fopen("images/lena_copy.bmp","wb"); // Output File name
	fwrite(header, sizeof(unsigned char), 54, fo); // write the image header to output file
	if(bitDepth <= 8)
		fwrite(colorTable, sizeof(unsigned char), 1024, fo);	
	fwrite(buf, sizeof(unsigned char), (height * width), fo);
	 	
	fclose(fo);
}

void calculate_histogram(unsigned char * buf, int * histogram) {
	int i, j, k;
	for(i = 0; i < height; i++) {
		for(j = 0; j < width; j++) {
			k = (int)buf[(i * width) + j];
			histogram[k]++;
		}
	}
}

void calculate_pdf(int * histogram, int * pdf) {
	int i;
	int sum = 0;
	for(i = 0; i < nof_gray_shades; i++) {
		sum = sum + histogram[i];
		pdf[i] = sum;
	}
}

void cdf(unsigned char * buf, unsigned char * out, int * pdf) {
	int i, j, k;
	float area = image_size;
	float Dm = nof_gray_shades;
	for(i = 0; i < height; i++) {
		for(j = 0; j < width; j++) {
			k = buf[(i * width) + j];
			out[(i * width) + j] = nof_gray_shades*((Dm/area) * (pdf[k]/nof_gray_shades));
		}
	}
}
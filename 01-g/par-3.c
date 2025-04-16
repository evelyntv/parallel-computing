/*	File: par-3.c
 *
 * 	Purpose:	Implement histogram equalization to sharpen the quality of an image.
 * 
 *	Compile:	mpicc -g -Wall -o par par-3.c
 *	Run:		mpiexec -n <number of processes> ./Sum_MPI_v2
 *
 *	Input:					images/lena512.bmp
 * 	output_imageput:		images/lena_copy.bmp (histogram equalized)
 *
 *	Notes:
 *		1. 	The code for reading and writing BMP files was based off of Abhijit Nathwani's work 
 *			(https://abhijitnathwani.github.io/blog/2017/12/20/First-C-Program-for-Image-Processing)
 *		2. 	The algorithm for histogram equalization was adapted from Image Processing in C (2e) by Dwayne Phillips
 * 		3. 	"timer.h" was taken from An Introduction to Parallel Programming (2e) by Pacheco and Malensek
 *
 *	Important:
 *		The number of processes must be a power of 2.
 *
 *	Author: Evelyn Evans
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include "timer.h"

const int height = 512;
const int width = 512;
const int image_size = 512*512; 
const int bmp_header_size = 54;
const int color_table_size = 1024;
const int nof_gray_shades = 256;
const int bloat_serial = 16384;		// 2 ^ 14

void initialize_histogram(int * histogram);
void read_bmp(unsigned char * input_image, unsigned char * header, unsigned char * colorTable);
void write_bmp(unsigned char * input_image, unsigned char * header, unsigned char * colorTable);
void calculate_histogram(unsigned char * input_image, int * histogram);
void calculate_histogram_sum(int * histogram, int * histogram_sum);
void transpose_image(unsigned char * input_image, unsigned char * output_image, int * histogram_sum);
void transpose_image_parallel(
	unsigned char * local_input, 
	unsigned char * local_output,  
	int * histogram_sum, 
	int chunk_size, 
	int process, 
	float Dm, 
	float area,
	int bloat);

int main(int argc,char *argv[])
{
    unsigned char input_image[image_size], output_image[image_size], header[bmp_header_size], colorTable[color_table_size];
	int histogram[nof_gray_shades], histogram_sum[nof_gray_shades];
	int chunk_size, my_rank, comm_sz, bloat;
	double local_start, local_finish, local_elapsed, elapsed; 

	unsigned char *local_input_image, *local_output_image;

	/* Start Parallelization */

	MPI_Init(NULL, NULL);
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

	chunk_size = image_size / comm_sz;
	bloat = bloat_serial * comm_sz;

	local_input_image = malloc(chunk_size * sizeof(unsigned char));
	local_output_image = malloc(chunk_size * sizeof(unsigned char));
	
	if (my_rank == 0) {
		initialize_histogram(histogram);
		read_bmp(input_image, header, colorTable);
		calculate_histogram(input_image, histogram);
		calculate_histogram_sum(histogram, histogram_sum);
	}

	MPI_Bcast(histogram_sum, nof_gray_shades, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&bloat, 1, MPI_INT, 0, MPI_COMM_WORLD);

	MPI_Scatter(input_image, chunk_size, MPI_UNSIGNED_CHAR, local_input_image, chunk_size, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);

	MPI_Barrier(MPI_COMM_WORLD);
	local_start = MPI_Wtime();

	transpose_image_parallel(local_input_image, local_output_image, histogram_sum, chunk_size, my_rank, (float)nof_gray_shades, (float)image_size, bloat);

	local_finish = MPI_Wtime();
	local_elapsed = local_finish - local_start;
	MPI_Reduce(&local_elapsed, &elapsed, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

	MPI_Gather(local_output_image, chunk_size, MPI_UNSIGNED_CHAR, output_image, chunk_size, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);

	free(local_output_image);
	free(local_input_image);

	if(my_rank == 0) {
		write_bmp(output_image, header, colorTable);
		printf("time elapsed: %f sec\n", elapsed);
	}

	MPI_Finalize();

	/* End Parallelization */

	return 0;
}

void initialize_histogram(int * histogram) {
	for (int i = 0; i < nof_gray_shades; i++) {
		histogram[i] = 0;
	}
}

void read_bmp(unsigned char * input_image, unsigned char * header, unsigned char * colorTable) {
	int i;

	FILE *streamIn; 
        streamIn = fopen("images/lena512.bmp", "r"); // Input file name
   
        if (streamIn == (FILE *)0) // check if the input file has not been opened succesfully.
	{
            printf("File opening error ocurred. Exiting program.\n");
            exit(0);
 	}
	
 	// int count = 0;
 	for(i=0;i<54;i++) 
 	{
 		header[i] = getc(streamIn);  // strip output_image BMP header
 		
 	}
 	int width = *(int*)&header[18]; // read the width from the image header
 	int height = *(int*)&header[22]; // read the height from the image header
	int bitDepth = *(int*)&header[28]; // read the bitDepth from the image header

	if(bitDepth <= 8)
		fread(colorTable, sizeof(unsigned char), 1024, streamIn);


	printf("width: %d\n",width);
	printf("height: %d\n",height );

	fread(input_image, sizeof(unsigned char), (height * width), streamIn);
	
 	fclose(streamIn);
}

void write_bmp(unsigned char * input_image, unsigned char * header, unsigned char * colorTable) {
	int width = *(int*)&header[18]; // read the width from the image header
	int height = *(int*)&header[22]; // read the height from the image header
   	int bitDepth = *(int*)&header[28]; // read the bitDepth from the image header

	FILE *fo = fopen("images/lena_copy.bmp","wb"); // output_imageput File name
	fwrite(header, sizeof(unsigned char), 54, fo); // write the image header to output_imageput file
	if(bitDepth <= 8)
		fwrite(colorTable, sizeof(unsigned char), 1024, fo);	
	fwrite(input_image, sizeof(unsigned char), (height * width), fo);
	 	
	fclose(fo);
}

void calculate_histogram(unsigned char * input_image, int * histogram) {
	int i, j, k;
	for(i = 0; i < height; i++) {
		for(j = 0; j < width; j++) {
			k = (int)input_image[(i * width) + j];
			histogram[k]++;
		}
	}
}

void calculate_histogram_sum(int * histogram, int * histogram_sum) {
	int i;
	int sum = 0;
	for(i = 0; i < nof_gray_shades; i++) {
		sum = sum + histogram[i];
		histogram_sum[i] = sum;
	}
}

void transpose_image(unsigned char * input_image, unsigned char * output_image, int * histogram_sum) {
	int i, j, k;
	float area = image_size;
	float Dm = nof_gray_shades;
	for(i = 0; i < height; i++) {
		for(j = 0; j < width; j++) {
			k = input_image[(i * width) + j];
			output_image[(i * width) + j] = nof_gray_shades*((Dm/area) * (histogram_sum[k]/nof_gray_shades));
		}
	}
}

void transpose_image_parallel(
	unsigned char * local_input, 
	unsigned char * local_output, 
	int * histogram_sum, 
	int chunk_size, 
	int process, 
	float Dm, 
	float area,
	int bloat) {

	int k;

	for(int b = 0; b < bloat; b++) {
		for(int i = 0; i < chunk_size; i++) {
			k = local_input[i];
			local_output[i] = (unsigned char)((Dm/area) * (histogram_sum[k]));
		}
	}
}
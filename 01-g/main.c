/**
* @file image_copy.c
* @brief This programs opens a BMP image, reads the header, colortable, and image data
	and stores it in a new file. Makes a copy of the image.
* @author Abhijit Nathwani
* @version v0.1
* @date 2017-12-19
*/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

const int height = 512;
const int width = 512;
const int image_size = 512*512; 
const int bmp_header_size = 54;
const int color_table_size = 1024;

void read_bmp(uint8_t * buf, uint8_t header);
void write_bmp(uint8_t * buf, uint8_t header);

int main(int argc,char *argv[])
{
    uint8_t buf[image_size];
    uint8_t header[bmp_header_size];

    read_bmp(buf, header);
    write_bmp(buf, header);
}


void read_bmp(uint8_t * buf, uint8_t * header) {


	int i;

	FILE *streamIn; 
        streamIn = fopen("images/lena512.bmp", "r"); // Input file name
   
        if (streamIn == (FILE *)0) // check if the input file has not been opened succesfully.
	{
            printf("File opening error ocurred. Exiting program.\n");
            exit(0);
 	}

 	uint8_t header[54]; // to store the image header
	uint8_t colorTable[1024]; // to store the colorTable, if it exists.
	
 	int count = 0;
 	for(i=0;i<54;i++) 
 	{
 		header[i] = getc(streamIn);  // strip out BMP header
 		
 	}
 	int width = *(int*)&header[18]; // read the width from the image header
 	int height = *(int*)&header[22]; // read the height from the image header
	int bitDepth = *(int*)&header[28]; // read the bitDepth from the image header

	if(bitDepth <= 8)
		fread(colorTable, sizeof(uint8_t), 1024, streamIn);


	printf("width: %d\n",width);
	printf("height: %d\n",height );

	fwrite(header, sizeof(uint8_t), 54, fo); // write the image header to output file
  	
 	uint8_t buf[height * width]; // to store the image data

	fread(buf, sizeof(uint8_t), (height * width), streamIn);
	
	if(bitDepth <= 8)
		fwrite(colorTable, sizeof(uint8_t), 1024, fo);	

	fwrite(buf, sizeof(uint8_t), (height * width), fo);
 	
	fclose(fo);
 	fclose(streamIn);
	
}

void write_bmp(uint8_t * buf, uint8_t * header) {
	FILE *fo = fopen("images/lena_copy.bmp","wb"); // Output File name
}
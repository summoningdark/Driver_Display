/*
Copyright (c) 2010 Jennifer Holt

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

//bmp2header_sprite 
// by Jenn Holt
//converts a bitmap picture to a c header file of sprite data for Jenn Holt's sparkfun serial graphic LCD firmware
//outputs to stdout, use redirection to catch it in a file

#include <stdlib.h>
#include <stdio.h>
#include "qdbmp.h"
#include <string.h>


int main(int argc, char **argv)
{
	BMP* bmp;
	unsigned int y,w,h,Nrows,i,j,k;
	unsigned char buff[128]; 	//buffer to hold a row of image data
	unsigned char r,g,b;

  if(argc==2)
  {	

	bmp = BMP_ReadFile(argv[1]);
	BMP_CHECK_ERROR(stderr, -1);     

	/* get the height and width of the image. */
        w = BMP_GetWidth(bmp);
        h = BMP_GetHeight(bmp);

	if ((w>128) || (h>64))
	{
		fprintf(stderr,"Image is too large. max size is 128x64\n");
		return -1;
	}
	
	//calculate the number of rows necessary(each row is 8 pixels tall)
	Nrows = h/8;			//number of whole rows necessary
	if (h%8!=0)
		Nrows++;		//a partial row counts too

	//build sprite

	//first 2 bytes are width and height		
	buff[0]=w;
	buff[1]=h;
 
	printf("static unsigned char sprite[]={0x%.2x, 0x%.2x", buff[0],buff[1]);	//print the array definition

	//now build image data
	for (i=0;i<Nrows;i++)	//loop for each row
	{
		memset(buff,0x00,128);	//clear all bytes in buffer for each new row
		for (j=0;j<w;j++)	//loop for each byte in row
		{
			for(k=0;k<8;k++) //loop for each pixel in byte
			{
				//calculate the y coordinate of this bit(x coordinate is j)
				y = i*8+k;
				BMP_GetPixelRGB(bmp,j,y,&r,&g,&b);	//get pixel 	
				if ((r+g+b)==0)	//check if pixel is set
				{
					buff[j] |= ((unsigned char)1 << k);		//set the pixel
				}
		 	}
		}
		//output row of data
		for (y=0;y<w;y++)		
			printf(", 0x%.2x",buff[y]); //write w bytes(1 for each x coordinate)	
	}
	printf("};\n");		//finish array definition 
  }
  else
  {
	printf("Usage: bmp2bin_sprite [infile.bmp] > sprite.h\n");
	printf("Converts a 24bbp bitmap image into a sprite\n");
	printf("any pixel in the source exactly equal to 0 is considered set in the sprite.\n");
  }
	return 0;
}

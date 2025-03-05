
#include "bitmap.h"

#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <errno.h>
#include <string.h>
#include <pthread.h> //for multi-threading

//struct storing information for threads
typedef struct {
	struct bitmap *bm; 					//pointer to bitmap image
	double xMin, xMax, yMin, yMax;     // coordinate range of mandelbrot
	int maxIterations;				  // maximum iterations per pixel
	int startRow, endRow;			 // rows that the thread will compute
} ThreadData;

int iteration_to_color( int i, int max );
int iterations_at_point( double x, double y, int max );
void *compute_image_thread(void *arg);  


void show_help()
{
	printf("Use: mandel [options]\n");
	printf("Where options are:\n");
	printf("-m <max>    The maximum number of iterations per point. (default=1000)\n");
	printf("-x <coord>  X coordinate of image center point. (default=0)\n");
	printf("-y <coord>  Y coordinate of image center point. (default=0)\n");
	printf("-s <scale>  Scale of the image in Mandlebrot coordinates. (default=4)\n");
	printf("-W <pixels> Width of the image in pixels. (default=500)\n");
	printf("-H <pixels> Height of the image in pixels. (default=500)\n");
	printf("-o <file>   Set output file. (default=mandel.bmp)\n");
	printf("-h          Show this help text.\n");
	printf("\nSome examples are:\n");
	printf("mandel -x -0.5 -y -0.5 -s 0.2\n");
	printf("mandel -x -.38 -y -.665 -s .05 -m 100\n");
	printf("mandel -x 0.286932 -y 0.014287 -s .0005 -m 1000\n\n");
}

int main( int argc, char *argv[] )
{
	char c;

	// These are the default configuration values used
	// if no command line arguments are given.

	const char *outfile = "mandel.bmp";
	double xcenter = 0;
	double ycenter = 0;
	double scale = 4;
	int    image_width = 500;
	int    image_height = 500;
	int    max = 1000;

	// For each command line argument given,
	// override the appropriate configuration value.
	int numberOfThreads = 1; //defaulting the number of threads to 1 so if we don't specify it does single thread
	while((c = getopt(argc,argv,"x:y:s:W:H:m:o:h:n:"))!=-1) {
		switch(c) {
			case 'x':
				xcenter = atof(optarg);
				break;
			case 'y':
				ycenter = atof(optarg);
				break;
			case 's':
				scale = atof(optarg);
				break;
			case 'W':
				image_width = atoi(optarg);
				break;
			case 'H':
				image_height = atoi(optarg);
				break;
			case 'm':
				max = atoi(optarg);
				break;
			case 'o':
				outfile = optarg;
				break;
			case 'h':
				show_help();
				exit(1);
				break;
			case 'n': //added case n so user can set the number of threads to use
				numberOfThreads = atoi(optarg);
				break;
		}
	}

	// Display the configuration of the image.
	printf("mandel: x=%lf y=%lf scale=%lf max=%d outfile=%s\n",xcenter,ycenter,scale,max,outfile);

	// Create a bitmap of the appropriate size.
	struct bitmap *bm = bitmap_create(image_width,image_height);

	// Fill it with a dark blue, for debugging
	bitmap_reset(bm,MAKE_RGBA(0,0,255,0));

	//here we make a thread array and structure for each thread
	pthread_t threads[numberOfThreads];
	ThreadData thread_data[numberOfThreads];

	//here we loop over the number of threads and distribute the work among them
	for (int i = 0; i < numberOfThreads; i++) {
		thread_data[i].bm = bm;
		thread_data[i].xMin = xcenter - scale;
		thread_data[i].xMax = xcenter + scale;
		thread_data[i].yMin = ycenter - scale;
		thread_data[i].yMax = ycenter + scale;
		thread_data[i].maxIterations = max;
		thread_data[i].startRow = (i * image_height) / numberOfThreads; //here we are getting the range of the rows
		thread_data[i].endRow = ((i + 1) * image_height) / numberOfThreads;

		// here we launch the thread to process its assigned part of the image
		pthread_create(&threads[i], NULL, compute_image_thread, &thread_data[i]);
	}

	//  here we wait for all the threads to finish their work
	for (int i = 0; i < numberOfThreads; i++) {
		pthread_join(threads[i], NULL);
	}


	
	// Save the image in the stated file.
	if(!bitmap_save(bm,outfile)) {
		fprintf(stderr,"mandel: couldn't write to %s: %s\n",outfile,strerror(errno));
		return 1;
	}

	return 0;
}

/*
Compute an entire Mandelbrot image, writing each point to the given bitmap.
Scale the image to the range (xmin-xmax,ymin-ymax), limiting iterations to "max"
*/

//here we modified the compute image function so it can do multi threading and each one can compute its assigned part
void *compute_image_thread(void *arg)
{
	ThreadData *data = (ThreadData *)arg; //getting the data assigned for thread
	int i,j;

	int width = bitmap_width(data->bm);
	int height = bitmap_height(data->bm);

	// For every pixel in the image...
	//we iterate over the assigned rows for the thread
	for(j= data->startRow; j<data->endRow; j++) 
	{

		for(i=0; i<width; i++) 
		{

			// Determine the point in x,y space for that pixel.
			double x = data->xMin + i*(data->xMax - data->xMin) / width;
			double y = data->yMin + j*(data->yMax - data->yMin)/height;

			// Compute the iterations at that point.
			int iters = iterations_at_point(x,y,data->maxIterations);

			// Set the pixel in the bitmap.
			bitmap_set(data->bm,i,j,iters);
		}
	}
	return NULL;
}

/*
Return the number of iterations at point x, y
in the Mandelbrot space, up to a maximum of max.
*/

int iterations_at_point( double x, double y, int max )
{
	double x0 = x;
	double y0 = y;

	int iter = 0;

	while( (x*x + y*y <= 4) && iter < max ) {

		double xt = x*x - y*y + x0;
		double yt = 2*x*y + y0;

		x = xt;
		y = yt;

		iter++;
	}

	return iteration_to_color(iter,max);
}

/*
Convert a iteration number to an RGBA color.
Here, we just scale to gray with a maximum of imax.
Modify this function to make more interesting colors.
*/

int iteration_to_color( int i, int max )
{
	int gray = 255*i/max;
	return MAKE_RGBA(gray,gray,gray,0);
}





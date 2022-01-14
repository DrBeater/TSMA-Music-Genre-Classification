#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <math.h>

#include <sndfile.h>

#include "mfcc.h"

static void
print_usage (char *progname)
{	printf ("\nUsage : %s <input file> <output file>\n", progname) ;
	puts ("\n"
		"    Where the output file will contain MFCC"
		) ;

} 

static int
read_all_samples(SNDFILE * infile, double * buffer, int channels)
{
  if (channels == 1)
    {
      /* MONO */
      int i = 0;
      double tmp;
      int readcount = sf_readf_double (infile, &tmp, 1);
      while (readcount == 1)
	{
	  buffer[i] = tmp;
	  i++;
	  readcount = sf_readf_double (infile, &tmp, 1);
	}
      
      return i;
    }
  else if (channels == 2)
    {
      /* STEREO */
      double buf [2], tmp ;
      int readcount, k=0 ;
      readcount = sf_readf_double (infile, buf, 1);
      tmp = (buf [0]+buf [1])/2.0 ;
      while (readcount == 1)
	{
	  buffer[k] = tmp;
	  k++;
	  readcount = sf_readf_double (infile, buf, 1);
	  tmp = (buf [0]+buf [1])/2.0 ;
      	}

      return k;
    }
  else
    {
      /* FORMAT ERROR */
      printf ("Channel format error %d.\n", channels);
    }
  
  return 0;
}

int
main (int argc, char * argv [])
{	char 		*progname, *infilename, *outfilename ;
	SNDFILE	 	*infile = NULL ;
	FILE		*outfile = NULL ;
	SF_INFO	 	sfinfo ;

	progname = strrchr (argv [0], '/') ;
	progname = progname ? progname + 1 : argv [0] ;

	if (argc != 3)
	{	print_usage (progname) ;
		return 1 ;
		} ;

	infilename = argv [1] ;
	outfilename = argv [2] ;
	
	if (strcmp (infilename, outfilename) == 0)
	{	printf ("Error : Input and output filenames are the same.\n\n") ;
		print_usage (progname) ;
		return 1 ;
		} ;
	
	
	if ((infile = sf_open (infilename, SFM_READ, &sfinfo)) == NULL)
	{	printf ("Not able to open input file %s.\n", infilename) ;
		puts (sf_strerror (NULL)) ;
		return 1 ;
		} ;
	
	/* Open the output file. */
	if ((outfile = fopen (outfilename, "w")) == NULL)
	{	printf ("Not able to open output file %s : %s\n", outfilename, sf_strerror (NULL)) ;
		return 1 ;
		} ;
	
	printf("Processing : %s\n",infilename);
	
	/* verify sampling rate */
	printf("Sampling rate : %d\n",sfinfo.samplerate);
	printf("Channels : %d\n",sfinfo.channels);
	printf("Size : %d\n",(int)sfinfo.frames);
	
 
	/* Read WAV and Compute Chromas */
	double *buffer= malloc(sfinfo.frames*sizeof(double));
	assert(buffer);
		
	int i;
	
	int frame_size = read_all_samples(infile,buffer, sfinfo.channels);

	mfcc_ctrl* mfcc_c=mfcc_init(frame_size, MFCC_SIZE, 40, sfinfo.samplerate);
	double mfcc[MFCC_SIZE];

	/* Process */
	mfcc_compute(mfcc_c, buffer, frame_size, mfcc);


	/* MFCC Normalization */
	/* Mean Variance */
	double mean = 0.0;
	double std = 0.0;
		
	for (i = 2; i < MFCC_SIZE; i++)
	  {
	    mean += mfcc[i];
	  }
	mean/= (MFCC_SIZE-2);
	
	for (i = 2; i < MFCC_SIZE; i++)
	  {
	    mfcc[i] -= mean;
	  }

	mean = 0.0;

	for (i = 2; i < MFCC_SIZE; i++)
	  {
	    std+=pow(mfcc[i]-mean,2);
	  }
	
	std=sqrt(std/(MFCC_SIZE-2));
	
	for (i = 2; i < MFCC_SIZE; i++)
	  {
	    mfcc[i] /= std;
	  }
	
	for (i = 0; i < MFCC_SIZE; i++)
	  fprintf(outfile, "%f ", mfcc[i]);
	fprintf(outfile,"\n");

	/* exit */
	mfcc_exit(mfcc_c);
	
	sf_close (infile) ;
	fclose (outfile) ;

	free(buffer);
	return EXIT_SUCCESS ;
} /* main */

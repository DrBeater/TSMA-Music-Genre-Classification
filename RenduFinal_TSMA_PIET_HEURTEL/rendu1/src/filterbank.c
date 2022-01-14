#include <stdio.h>
#include <malloc.h>
#include <math.h>
#include <assert.h>

#include "filterbank.h"

filterbank_ctrl* filterbank_init(int frame_size, int n_filters)
{
  /* memory allocation */
  filterbank_ctrl *fb_c = malloc(sizeof(struct filterbank_ctrl));
  assert(fb_c);

  fb_c->n_filters = n_filters;
  fb_c->frame_size = frame_size;

  fb_c->filters = malloc(sizeof(double*)*(frame_size/2+1));
  assert(fb_c->filters);
  
  int i;

  for(i = 0; i < fb_c->frame_size/2 + 1 ; i++)
    {
      fb_c->filters[i] = malloc(sizeof(double)*n_filters);
      assert(fb_c->filters[i]);
    }
  
  return fb_c;
}

int filterbank_compute(filterbank_ctrl *fb_c, double *in, double *out)
{
  int j, fn;

  int max_filters = fmin(fb_c->n_filters, fb_c->frame_size);
  int max_length  = fmin(fb_c->frame_size,fb_c->frame_size/2+1);
  
  // reset output values
  for (j = 0; j < fb_c->n_filters; j++)
    out[j] =  0.0;
  
  for (fn = 0; fn < max_filters ; fn++)
    for (j = 0; j < max_length ; j++)
      {
	out[fn]
	  += in[j] * fb_c->filters[j][fn];
	// DEBUG
	//printf("%d %d : %f\n", fn, j,  fb_c->filters[j][fn]);
	//printf("%d %d : %f\n", fn, j,  out[fn]);
	//printf("%d : %f\n", j, in[j]);
				
	
      }
  
  return 0;
}

void filterbank_exit(filterbank_ctrl * fb_c)
{
  
  int i;

  for(i = 0; i < fb_c->frame_size/2 + 1 ; i++)
    {
      free(fb_c->filters[i]);
    }
  free(fb_c->filters);

  free(fb_c);
}


int filterbank_set_triangle_bands (filterbank_ctrl * fb_c, int samplerate, float * freqs, int n_filters_required)
{
	
  double **filters = fb_c->filters;
  int n_filters = fb_c->n_filters, frame_size = fb_c->frame_size/2+1;

  //printf("frame_size %d\n", frame_size);
  //printf("n_filters %d\n", n_filters);
  

  int fn;                    /* filter counter */
  int bin;                   /* bin counter */
  int i,j;
  
  /* freqs define the bands of triangular overlapping windows.
     throw a warning if filterbank object fb is too short. */
  if (n_filters_required - 2 > n_filters)
    {
      printf("not enough filters, %d allocated but %d requested\n", n_filters, n_filters_required - 2);
    }

  if ( n_filters_required - 2 < n_filters)
    {
      printf("too many filters, %d allocated but %d requested\n", n_filters, n_filters_required - 2);
    }
  
  if (freqs[n_filters_required - 1] > samplerate / 2)
    {
      printf("Nyquist frequency is %fHz, but highest frequency band ends at %fHz\n", (float)samplerate / 2, freqs[n_filters_required - 1]);
    }
	
  /* convenience reference to lower/center/upper frequency for each triangle */
  float *lower_freqs = malloc(n_filters*sizeof(float));
  assert(lower_freqs);
  float *upper_freqs = malloc(n_filters*sizeof(float));
  assert(upper_freqs);
  float *center_freqs = malloc(n_filters*sizeof(float));
  assert(center_freqs);
  
  /* height of each triangle */
  float *triangle_heights = malloc(n_filters*sizeof(float));
  assert(triangle_heights);

  /* lookup table of each bin frequency in hz */
  float *fft_freqs = malloc(frame_size*sizeof(float));
  assert(fft_freqs);

  /* fill up the lower/center/upper */
  for (fn = 0; fn < n_filters; fn++)
    {
      lower_freqs[fn] = freqs[fn];
      center_freqs[fn] = freqs[fn + 1];
      upper_freqs[fn] = freqs[fn + 2];
    }
	
  /* compute triangle heights so that each triangle has unit area */
  for (fn = 0; fn < n_filters; fn++)
    {
      triangle_heights[fn] = 2. / (upper_freqs[fn] - lower_freqs[fn]);
      // DEBUG OK
    }
	
  /* fill fft_freqs lookup table, which assigns the frequency in hz to each bin */
  for (bin = 0; bin < frame_size; bin++)
    {
      fft_freqs[bin] = (float)bin * (float)samplerate / ((frame_size - 1) * 2);
      //DEBUG OK
      //printf("%d %f\n", bin, fft_freqs[bin]);
      
    }
	
  /* zeroing of all filters */
  for(i = 0; i < frame_size ; i++)
    {
      for (j = 0 ; j < n_filters ; j++)
	fb_c->filters[i][j] = 0.0;
    }
  
	
  if (fft_freqs[1] >= lower_freqs[0]) {
    /* - 1 to make sure we don't miss the smallest power of two */
    printf("Lowest frequency bin (%.2fHz) is higher than lowest frequency band (%.2f-%.2fHz). Consider increasing the window size from %d to next power of two.\n",
	   fft_freqs[1], lower_freqs[0],
	   upper_freqs[0], (frame_size - 1) * 2);
  }
	
  /* building each filter table */
  for (fn = 0; fn < n_filters; fn++) {
	
    /* skip first elements */
    for (bin = 0; bin < frame_size - 1; bin++) {
      if (fft_freqs[bin] <= lower_freqs[fn] &&
	  fft_freqs[bin + 1] > lower_freqs[fn]) {
	bin++;
	break;
      }
    }
    
    /* compute positive slope step size */
    float riseInc = triangle_heights[fn] / (center_freqs[fn] - lower_freqs[fn]);
	
    /* compute coefficients in positive slope */
    for (; bin < frame_size - 1; bin++) {
      filters[bin][fn] = (fft_freqs[bin] - lower_freqs[fn]) * riseInc;
	
      if (fft_freqs[bin + 1] >= center_freqs[fn]) {
	bin++;
	break;
      }
    }
	
    /* compute negative slope step size */
    float downInc = triangle_heights[fn] / (upper_freqs[fn] - center_freqs[fn]);
	
    /* compute coefficents in negative slope */
    for (; bin < frame_size - 1; bin++) {
      filters[bin][fn] += (upper_freqs[fn] - fft_freqs[bin]) * downInc;
	
      if (filters[bin][fn] < 0.) {
	filters[bin][fn] = 0.;
      }
	
      if (fft_freqs[bin + 1] >= upper_freqs[fn])
	break;
    }
    /* nothing else to do */
	
  }

  // DEBUG
  /*
  for(i = 0; i < frame_size ; i++)
    {
      for (j = 0 ; j < n_filters ; j++)
	printf("%d, %d : %f\n", i, j, fb_c->filters[i][j]);
    }
  */
  /* destroy temporarly allocated vectors */
  free (lower_freqs);
  free (upper_freqs);
  free (center_freqs);
  
  free (triangle_heights);
  free (fft_freqs);
  
  return 0;
}



int filterbank_set_mel_coeffs_slaney (filterbank_ctrl * fb_c, int samplerate)
{
  int retval;

  /* Malcolm Slaney parameters */
  float lowestFrequency = 133.3333*2;
  float linearSpacing = 66.66666666;
  float logSpacing = 1.0711703;
  
  short linearFilters = 13;
  short logFilters = 27;
  short n_filters = linearFilters + logFilters;
  
  int fn;                    /* filter counter */

  /* buffers to compute filter frequencies */
  float *freqs = malloc(sizeof(float)*(n_filters+2));
  
  /* first step: fill all the linear filter frequencies */
  for (fn = 0; fn < linearFilters; fn++)
    {
      freqs[fn] = lowestFrequency + fn * linearSpacing;
    }

  float lastlinearCF = freqs[fn - 1];

  /* second step: fill all the log filter frequencies */
  for (fn = 0; fn < logFilters + 2; fn++)
    {
      freqs[fn + linearFilters] = lastlinearCF * (pow (logSpacing, fn + 1));
    }

  /* now compute the actual coefficients */
  retval = filterbank_set_triangle_bands (fb_c, samplerate, freqs, n_filters+2);
	
  /* destroy vector used to store frequency limits */
  free (freqs);
  
  return retval;
}

    

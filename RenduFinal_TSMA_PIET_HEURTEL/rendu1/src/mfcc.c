#include <complex.h>
#include <math.h>
#include <malloc.h>
#include <assert.h>

#include <math.h>
#include <complex.h>
#include <fftw3.h>


#include "mfcc.h"

static fftw_plan plan = NULL;

static double hamming_window(int i, int N)
{
  return (0.54-0.46*cos(2*3.1415926*(double)i/(double)(N-1)));
}


void
fft_init (fftw_complex *input, fftw_complex *spec, int size)
{
  plan = fftw_plan_dft_1d (size, input, spec, FFTW_FORWARD, FFTW_ESTIMATE);
}

void
fft_exit (void)
{
  fftw_destroy_plan (plan);
}

void
fft_process (void)
{
  fftw_execute (plan);
}

/* INIT */
/* ---- */

mfcc_ctrl* mfcc_init(int frame_size, int n_coefs, int n_filters, int samplerate)
{
  /* memory allocation */
  mfcc_ctrl *mfcc_c = (mfcc_ctrl*)malloc(sizeof(struct mfcc_ctrl));
  assert(mfcc_c);

   /* variables init */
  mfcc_c->num_frames=0;
  mfcc_c->samplerate=samplerate;
  mfcc_c->frame_size=frame_size;

  mfcc_c->n_coefs=n_coefs;
  mfcc_c->n_filters=n_filters;

  /* filterbank */
  mfcc_c->fb_c = filterbank_init(frame_size,n_filters);
  filterbank_set_mel_coeffs_slaney (mfcc_c->fb_c, samplerate);

  int i;
  mfcc_c->dct_coefs = malloc(n_coefs*sizeof(double*));
  assert(mfcc_c->dct_coefs);
  for (i = 0; i < n_coefs; i++)
    {
      mfcc_c->dct_coefs[i] = malloc(n_filters*sizeof(double));
      assert(mfcc_c->dct_coefs[i]);
    }

  int j;
  float scaling = 1. / sqrt(n_filters/2.0);
  for (i = 0; i <  n_filters; i++)
    {
      
      for (j = 0; j < n_coefs; j++)
	mfcc_c->dct_coefs[j][i] = scaling* cos(j *(i+0.5) * M_PI / n_filters);

      mfcc_c->dct_coefs[0][i] *= sqrt(2.0)/2.0;
    }
  
  return mfcc_c;
}


/* EXIT */
/* ---- */

void mfcc_exit(mfcc_ctrl *mfcc_c)
{
  int i;
  
  for (i = 0; i < mfcc_c->n_coefs; i++)
    free(mfcc_c->dct_coefs[i]);

  free(mfcc_c->dct_coefs);
  free(mfcc_c);
}


/* COMPUTATION */
/* ----------- */

int mfcc_compute(mfcc_ctrl *mfcc_c, double *samples, int sample_size, double *mfcc)
{

  /* Verif*/
  if (sample_size <=0)
    {
      printf("Error mfcc.c : sample_size");
      return 1;
    }

  if (!samples)
    {
      printf("Error mfcc.c : samples");
      return 1;
    }

  if (!mfcc)
    {
      printf("Error mfcc.c : mfcc_c");
      return 1;
    }
    
  double *in_dct = malloc(sizeof(double)*mfcc_c->n_filters);
  assert(in_dct);
  
  int i;


  
  /* ProcessFFT */
  complex *infft = malloc(sizeof(complex)*sample_size);
  assert(infft);
  complex *spectrum = malloc(sizeof(complex)*sample_size);
  assert(spectrum);
  double *amplitude = malloc(sizeof(double)*(sample_size/2+1));
  assert(amplitude);

  /* FFT init */
  fft_init(infft, spectrum, sample_size);

  for (i=0;i<sample_size;i++)
    {
      infft[i] = (samples[i]*hamming_window(i,sample_size));
    }
  
  /* ProcessFFT */
  fft_process();

  for (i=0;i<sample_size/2+1;i++)
    {
      amplitude[i] = cabs(spectrum[i]);///sample_size;
    }
  
  /* Compute MFCC */
  int j,k;

  filterbank_compute(mfcc_c->fb_c, amplitude, in_dct);

  for (i=0;i<mfcc_c->n_filters;i++)
    {
      in_dct[i] = log10(in_dct[i]);
    }

  for (i = 0; i < mfcc_c->n_coefs; i++)
    mfcc[i] = 0.0;

  for (j = 0; j < mfcc_c->n_filters; j++)
    for (k = 0; k < mfcc_c->n_coefs; k++)
      {
	
	mfcc[k] += in_dct[j] * mfcc_c->dct_coefs[k][j];
      }
  
  /* --- */
  
  mfcc_c->num_frames++;
  mfcc_c->total_samples += sample_size;

  free(in_dct);
  free(spectrum);
  free(amplitude);
  free(infft);

  /* FFT exit */
  fft_exit();

  return 0;
}


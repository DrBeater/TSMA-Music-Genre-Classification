#ifndef MFCC_H
#define MFCC_H

#include "filterbank.h"

#define MFCC_SIZE 22

typedef
struct mfcc_ctrl
{
  int frame_size;// taille fenêtre analyse
  int num_frames;
  int samplerate;

  int n_coefs;
  int n_filters;

  double **dct_coefs;

  filterbank_ctrl *fb_c;
    
  int total_samples;
  
} mfcc_ctrl;


/*MFCC computation initialiser*/
mfcc_ctrl* mfcc_init(int frame_size, int n_coefs, int n_filters, int samplerate);

/* MFCC computation */
int mfcc_compute(mfcc_ctrl *mfcc_c, double *samples, int sample_size, double *mfcc);

/* MFCC controler free */
void mfcc_exit(mfcc_ctrl *mfcc_c);


#endif

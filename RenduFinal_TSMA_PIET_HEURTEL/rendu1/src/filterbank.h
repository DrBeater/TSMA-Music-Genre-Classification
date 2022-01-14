#ifndef FILTERBANK_H
#define FILTERBANK_H

typedef
struct filterbank_ctrl
{
  int frame_size;
  int n_filters;
  double** filters;
} filterbank_ctrl;

filterbank_ctrl* filterbank_init();

int filterbank_compute(filterbank_ctrl *fb_c, double *in, double *out);

void filterbank_exit(filterbank_ctrl * fb_c);

int filterbank_set_mel_coeffs_slaney (filterbank_ctrl * fb_c, int samplerate);


#endif

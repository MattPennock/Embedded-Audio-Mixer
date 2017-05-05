#ifndef __BQFAC_H
#define __BQFAC_H

#include "biquad.h"
#define status_t int

status_t lowpass(biquad_t *filter, float center_freq, float Q);
status_t highpass(biquad_t *filter, float center_freq, float Q);
status_t bandpass(biquad_t *filter, float center_freq, float Q);
status_t notch(biquad_t *filter, float center_freq, float Q);
status_t APF(biquad_t *filter, float center_freq, float Q);
status_t peakingEQ(biquad_t *filter, float center_freq, float Q, float dB);
status_t lowshelf(biquad_t *filter, float center_freq, float Q, float dB);
status_t highshelf(biquad_t *filter, float center_freq, float Q, float dB);


#endif

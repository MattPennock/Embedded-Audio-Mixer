#ifndef __STATE_H
#define __STATE_H

#include <stdbool.h>
#include "Interaction.h"
#include "fsl_i2s.h"
#include "fsl_wm8904.h"
#include "fsl_ft5406.h"
#include "BiquadFactory.h"
#include "Biquad.h"

#define N_BIQUADS 3

/*******************************
*  Global State Variables
*******************************/

Audio mode[N_BIQUADS];

/* Slider Control Variables */
float prev_volume;
float volume;
int volume_target;

float center_freq[N_BIQUADS];
float db_gain[N_BIQUADS];
float q_factor[N_BIQUADS];
int CFX[N_BIQUADS];
int DBX[N_BIQUADS];
int QX[N_BIQUADS];

/* Button Flags */
bool play;
bool loop_back;
bool low_pass;
bool high_pass;
bool band_pass;
bool notch_filt;
bool peaking_EQ;
bool filterA;
bool filterB;

/* Touchscreen Controls */
bool touched;
int touchX, touchY;

/*******************************
*  Biquads
*******************************/

biquad_t Right[N_BIQUADS];
biquad_t Left[N_BIQUADS];

/* Filter currently under configuration */
int active;
biquad_t* ActiveLeft;
biquad_t* ActiveRight;

/*******************************
*  Handles and Events
*******************************/
ft5406_handle_t touch_handle;
touch_event_t touch_event;
wm8904_handle_t codecHandle;

/*******************************
*  Buttons
*******************************/
#define N_BUTTONS 9

Button_t *Button_List[N_BUTTONS];

Button_t play_button;
Button_t loop_back_button;
Button_t low_pass_button;
Button_t high_pass_button;
Button_t band_pass_button;
Button_t notch_filt_button;
Button_t peaking_EQ_button;
Button_t FilterA_button;
Button_t FilterB_button;

/*******************************
*  Sliders
*******************************/
#define N_SLIDERS 4

Slider_t *Slider_List[N_SLIDERS];

Slider_t volume_slider;
Slider_t center_freq_slider;
Slider_t db_gain_slider;
Slider_t q_factor_slider;

/*******************************
*  State Machine Process
*******************************/
void State_Machine(void)
{

	if(filterA)
	{
		active = 0;
		filterA = false;
	}

	if(filterB)
	{
		active = 1;
		filterB = false;
	}

    /* Update Sliders to active biquad */
    center_freq_slider.Control   = &center_freq[active];
    center_freq_slider.xPosition = &CFX[active];

    q_factor_slider.Control      = &q_factor[active];
    q_factor_slider.xPosition    = &QX[active];

    db_gain_slider.Control       = &db_gain[active];
    db_gain_slider.xPosition     = &DBX[active];

    /* Change active Biquads */
    ActiveLeft  = &Left[active];
    ActiveRight = &Right[active];

    /* turn off all filter buttons */
    for(int i = 1; i < N_BUTTONS; i++)
        Button_List[i]->isFilled = false;

    /* play button behavior */
    if(play)
    {
        if(play_button.isFilled)
        {
            /* pause the playback */
            I2S_Disable(I2S0);
            I2S_Disable(I2S1);
            play_button.isFilled = false;
        }

        else
        {
            /* start the playback */
            I2S_Enable(I2S0);
            I2S_Enable(I2S1);
            play_button.isFilled = true;
        }

        play = false;
    }

    /* update the volume */
    if(prev_volume != volume)
    {
        prev_volume = volume;
        WM8904_SetVolume(&codecHandle, volume, volume);

    }

    if(loop_back)
    {
        mode[active] = k_LoopBack;
        loop_back = false;
    }

    else if(low_pass)
    {
        mode[active] = k_LowPass;
        low_pass = false;
    }

    else if(high_pass)
    {
        mode[active] = k_HighPass;
        high_pass = false;
    }

    else if(band_pass)
    {
        mode[active] = k_BandPass;
        band_pass = false;
    }

    else if(notch_filt)
    {
        mode[active] = k_Notch;
        notch_filt = false;
    }

    else if(peaking_EQ)
    {
        mode[active] = k_Peaking;
        peaking_EQ = false;
    }

    /* update filter coeffecients */
    switch(mode[active])
    {
        case k_LoopBack:
            loop_back_button.isFilled = true;
            break;

        case k_LowPass:
            lowpass(&Left[active], center_freq[active], q_factor[active]);
            lowpass(&Left[active],  center_freq[active], q_factor[active]);
            low_pass_button.isFilled = true;
            break;

        case k_HighPass:
            highpass(ActiveRight, center_freq[active], q_factor[active]);
            highpass(ActiveLeft,  center_freq[active], q_factor[active]);
            high_pass_button.isFilled = true;
            break;

        case k_BandPass:
            bandpass(ActiveRight, center_freq[active], q_factor[active]);
            bandpass(ActiveLeft,  center_freq[active], q_factor[active]);
            band_pass_button.isFilled = true;
            break;

        case k_Notch:
            notch(ActiveRight, center_freq[active], q_factor[active]);
            notch(ActiveLeft,  center_freq[active], q_factor[active]);
            notch_filt_button.isFilled = true;
            break;

        case k_Peaking:
            peakingEQ(ActiveRight, center_freq[active], q_factor[active], db_gain[active]);
            peakingEQ(ActiveLeft,  center_freq[active], q_factor[active], db_gain[active]);
            peaking_EQ_button.isFilled = true;
            break;
    }

    switch(active)
    {
    	case 0:
    		FilterA_button.isFilled = true;
    		break;

    	case 1:
    	    FilterB_button.isFilled = true;
    	    break;
    }
}

#endif

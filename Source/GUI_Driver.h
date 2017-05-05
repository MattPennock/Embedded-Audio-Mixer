#include <stateOLD.h>
#include "Interaction.h"
#include "eGFX_DataTypes.h"

#define BUTTON_WIDTH    62
#define BUTTON_HEIGHT   32
#define BUTTON_Y_ALIGN  16

#define SLIDER_WIDTH    340
#define SLIDER_HEIGHT   20
#define SLIDER_X_ALIGN  80

void Init_GUI()
{
    /* ;-) */
    ActiveLeft  = &Left[active];
    ActiveRight = &Right[active];

    for(int i = 0; i < N_BIQUADS; i++)
    {
    	CFX[i] = 256;
    	QX[i]  = 256;
    	DBX[i] = 256;
    	center_freq[i] = 440;
    	q_factor[i] = 1;
    	db_gain[i] = 0;
    }

    /* Layout all Registered Buttons and Sliders */

    eGFX_Box tmp;

    /* Step 1: assign object pointers to lists */

    Button_List[0] = &play_button;
	Button_List[1] = &loop_back_button;
    Button_List[2] = &low_pass_button;
    Button_List[3] = &high_pass_button;
	Button_List[4] = &band_pass_button;
    Button_List[5] = &notch_filt_button;
    Button_List[6] = &peaking_EQ_button;

    Button_List[7] = &FilterA_button;
    Button_List[8] = &FilterB_button;

    Slider_List[0] = &center_freq_slider;
	Slider_List[1] = &q_factor_slider;
    Slider_List[2] = &db_gain_slider;
    Slider_List[3] = &volume_slider;

    /* Step 2: init objects / perform layout */

    /* PLAY BUTTON */
    Set_Box(&tmp, 16, 230, 128, BUTTON_HEIGHT);
    Init_Button(&play_button, tmp, k_Green, "PLAY",
                &play, k_PlayGroup);

    /* LOOP BACK */
    Set_Box(&tmp, 16, BUTTON_Y_ALIGN, BUTTON_WIDTH, BUTTON_HEIGHT);
    Init_Button(&loop_back_button, tmp, k_YellowOrange, "NOFILT",
                &loop_back, k_FilterGroup);

    /* LOW PASS */
    Set_Box(&tmp, 94, BUTTON_Y_ALIGN, BUTTON_WIDTH, BUTTON_HEIGHT);
    Init_Button(&low_pass_button, tmp, k_YellowOrange, "LOWPASS",
                &low_pass, k_FilterGroup);

    /* HIGH PASS */
    Set_Box(&tmp, 172, BUTTON_Y_ALIGN, BUTTON_WIDTH, BUTTON_HEIGHT);
    Init_Button(&high_pass_button, tmp, k_Orange, "HIGHPASS",
                &high_pass, k_FilterGroup);

    /* BAND PASS */
    Set_Box(&tmp, 250, BUTTON_Y_ALIGN, BUTTON_WIDTH, BUTTON_HEIGHT);
    Init_Button(&band_pass_button, tmp, k_RedOrange, "BANDPASS",
                &band_pass, k_FilterGroup);

    /* NOTCH */
    Set_Box(&tmp, 328, BUTTON_Y_ALIGN, BUTTON_WIDTH, BUTTON_HEIGHT);
    Init_Button(&notch_filt_button, tmp, k_Red, "NOTCH",
                &notch_filt, k_FilterGroup);

    /* PEAKING EQ */
    Set_Box(&tmp, 406, BUTTON_Y_ALIGN, BUTTON_WIDTH, BUTTON_HEIGHT);
    Init_Button(&peaking_EQ_button, tmp, k_RedPurple, "PEAKINGEQ",
                &peaking_EQ, k_FilterGroup);

    /* Cascade A */
    Set_Box(&tmp, 172, 230, BUTTON_WIDTH, BUTTON_HEIGHT);
    Init_Button(&FilterA_button, tmp, k_Lime, "FILT A",
                &filterA, k_SelectGroup);

    /* Cascade B */
    Set_Box(&tmp, 250, 230, BUTTON_WIDTH, BUTTON_HEIGHT);
    Init_Button(&FilterB_button, tmp, k_HotPink, "FILT B",
                &filterB, k_SelectGroup);

    /* CENTER FREQ SLIDER */
    Set_Box(&tmp, SLIDER_X_ALIGN, 70, SLIDER_WIDTH, SLIDER_HEIGHT);
	Init_Slider(&center_freq_slider, tmp, k_Cyan, 
				"CENTER FREQ:", &center_freq[active], 25, 1000, &CFX[active]);

    /* Q-FACTOR SLIDER */
    Set_Box(&tmp, SLIDER_X_ALIGN, 110, SLIDER_WIDTH, SLIDER_HEIGHT);
	Init_Slider(&q_factor_slider, tmp, k_Cyan, 
				"Q-FACTOR:", &q_factor[active], 0.5, 5, &QX[active]);

    /* DB GAIN SLIDER */
    Set_Box(&tmp, SLIDER_X_ALIGN, 150, SLIDER_WIDTH, SLIDER_HEIGHT);
	Init_Slider(&db_gain_slider, tmp, k_Cyan, 
				"dB GAIN:", &db_gain[active], -10, 30, &DBX[active]);

    /* VOLUME SLIDER */
    Set_Box(&tmp, SLIDER_X_ALIGN, 190, SLIDER_WIDTH, SLIDER_HEIGHT);
	Init_Slider(&volume_slider, tmp, k_Cyan, 
				"VOLUME:", &volume, 0, 63, &volume_target);
}

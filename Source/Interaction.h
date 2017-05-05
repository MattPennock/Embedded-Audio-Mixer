#ifndef __INTERACT_H
#define __INTERACT_H

#include <stdbool.h>
#include "eGFX_DataTypes.h"

/*******************************
 *  Enumerations
 *******************************/

typedef enum
{
	k_Black         = 0x0000u,
	k_White         = 0xFFFFu,
	k_Yellow        = 0x07DFu,
	k_YellowOrange  = 0x067Fu,
	k_Orange        = 0x053Fu,
	k_RedOrange     = 0x03FFu,
	k_Red           = 0x011Fu,
	k_RedPurple     = 0x913Eu,
	k_Purple        = 0xB911u,
    k_Blue          = 0xB920u,
	k_LightBlue     = 0xC3C0u,
	k_Cyan          = 0xD580u,
	k_Green         = 0x05C0u,
	k_Lime          = 0x0670u,
	k_HotPink       = 0xED9Fu,
	k_Gray          = 0xC618u
} Color_Wheel;

typedef enum
{
	k_LoopBack,
	k_LowPass,
	k_HighPass,
	k_BandPass,
	k_Notch,
	k_Peaking
} Audio;

typedef enum
{
    k_PlayGroup,    /* play/pause */
    k_FilterGroup,  /* select a type of filter */
    k_SelectGroup   /* biquad struct to modify */
} Button_Group;

/*******************************
 *  Interaction Data Types
 *******************************/

typedef struct {
    eGFX_Box    	Box;
    Color_Wheel     Color;
    char*       	Label;
	bool			isFilled;
    bool*           Flag;
    Button_Group    Group;  
} Button_t;

typedef struct {
    eGFX_Box        Box;
    Color_Wheel     Color;
    char*           Label;
    int             *xPosition;
    float           *Control;
    float           lowerBound;
    float           upperBound;
    float           scaleFactor;
} Slider_t;

/*******************************
 *  Button API
 *******************************/

/* Buttons set their associated Flag to true when touched */

void Init_Button(Button_t *B, eGFX_Box Box,
                 Color_Wheel color, char* label, bool* flag,
                 Button_Group group);

void Draw_Button(Button_t *B);

/* Must call to refresh button's control value */
void Update_Button(Button_t *B);

/*******************************
 *  Slider API
 *******************************/

/* Sliders update the value pointed to by Control when touched */

void Init_Slider(Slider_t *S, eGFX_Box Box,
                 Color_Wheel color, char* label, float* control,
                 float lower_bound, float upper_bound, int* xPosition);

void Draw_Slider(Slider_t *S);

/* Must call to refresh slider's control value */
void Update_Slider(Slider_t *S, int x);

/*******************************
 *  Detect Touch
 *******************************/
 
 /* Detect_Touch returns true if a touch event is within the Box,
        false otherwise */
 bool Detect_Touch(eGFX_Box *Box, int x, int y);

/* Set_Box modifies a Box struct, commonly used when building the GUI */
 void Set_Box(eGFX_Box *Box, int x, int y, int width, int height);

 #endif

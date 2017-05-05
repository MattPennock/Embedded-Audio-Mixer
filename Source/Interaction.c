
#include "Interaction.h"
#include "eGFX_Driver.h"
#include "eGFX.h"
#include "FONT_5_7_8BPP.h"

/*  Interaction.c contains the API to act on sliders and buttons:
 *
 *  Init_Button
 *  Draw_Button
 *  Update_Button
 *
 *  Init_Slider
 *  Draw_Slider
 *  Update_Slider
 *
 *  Detect_Touch 
 *  Set_Box 
 *
 */

void Init_Button(Button_t *B, eGFX_Box Box,
                 Color_Wheel color, char* label, bool* flag,
                 Button_Group group)
{
    B-> Box      = Box;
    B-> Color    = color;
    B-> Label    = label;
    B-> isFilled = false;
    B-> Flag     = flag;
    B-> Group    = group;
}

void Draw_Button(Button_t *B)
{
    int textX = B->Box.P1.X + 4;
    int textY = (int) ((B->Box.P2.Y + B->Box.P1.Y) / 2) + 4;

    if(B->isFilled)
    {
        /* Draw filled box with black text */
        eGFX_DrawFilledBox(&eGFX_BackBuffer, &B->Box, B->Color);
        eGFX_printf_Colored(&eGFX_BackBuffer,
                            textX, textY, &FONT_5_7_8BPP,
                            k_Black, B->Label);
    }

    else
    {
        eGFX_DrawBox(&eGFX_BackBuffer, &B->Box, B->Color);
        eGFX_printf_Colored(&eGFX_BackBuffer,
                            textX, textY, &FONT_5_7_8BPP,
                            B->Color, B->Label);
    }

}

void Update_Button(Button_t *B)
{
    *B->Flag = true;
}

void Init_Slider(Slider_t *S, eGFX_Box Box,
                 Color_Wheel color, char* label, float* control,
                 float lower_bound, float upper_bound, int *xPosition)
{
    S-> Box         = Box;
    S-> Color       = color;
    S-> Label       = label;
    S-> Control     = control;
    S-> lowerBound  = lower_bound;
    S-> upperBound  = upper_bound;
    S->xPosition   = xPosition;
    *S-> xPosition  = (int) (Box.P2.X + Box.P1.X) / 2;
    S-> scaleFactor = (float) (upper_bound - lower_bound) /  (Box.P2.X - Box.P1.X);

    /* set inital value of slider's control */
    *S->Control = (float) ((*S->xPosition - lower_bound) * S->scaleFactor) + lower_bound;
}

void Draw_Slider(Slider_t *S)
{
    eGFX_Box colored;       /* Active Slider Portion */
    eGFX_Box gray;          /* Inactive Slider Portion */

    colored.P1.X = S->Box.P1.X;
    colored.P1.Y = S->Box.P1.Y;

    colored.P2.X = *S->xPosition;
    colored.P2.Y = S->Box.P2.Y;

    gray.P1.Y = S->Box.P1.Y;
    gray.P1.X = *S->xPosition;

    gray.P2.X = S->Box.P2.X;
    gray.P2.Y = S->Box.P2.Y;

    /* Active Region */
    eGFX_DrawFilledBox(&eGFX_BackBuffer, &colored, S->Color);

    /* Inactive Region */
    eGFX_DrawFilledBox(&eGFX_BackBuffer, &gray, k_Gray);

    /* Label */
    eGFX_printf_Colored(&eGFX_BackBuffer,
                        colored.P1.X - eGFX_GetStringWidth(S->Label, &FONT_5_7_8BPP) - 4,
                        colored.P1.Y + 4,
                        &FONT_5_7_8BPP, k_White, S->Label);
    
    /* Control Value */
    eGFX_printf_Colored(&eGFX_BackBuffer,
                        gray.P2.X + 4,
						colored.P1.Y + 4,
						&FONT_5_7_8BPP, k_White, "%0.1f", *S->Control);
}

void Update_Slider(Slider_t *S, int x)
{
    *S->xPosition = x;
    *S->Control = (float) ((*S->xPosition - S->Box.P1.X) * S->scaleFactor) + S->lowerBound;
}

 bool Detect_Touch(eGFX_Box *Box, int x, int y)
 {
    if(x >= Box->P1.X && x <= Box->P2.X &&
       y >= Box->P1.Y && y <= Box->P2.Y)
    {
        return true;
    }

    return false;
 }

  void Set_Box(eGFX_Box *Box, int x, int y, int width, int height)
  {
      Box->P1.X = x;
      Box->P1.Y = y;
      Box->P2.X = x + width;
      Box->P2.Y = y + height;
  }


/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <state.h>				/* State Machine Global State */
#include <stdio.h>
#include <string.h>

#include "fsl_device_registers.h"
#include "cr_section_macros.h"
#include "board.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "fsl_debug_console.h"
#include "fsl_iocon.h"
#include "fsl_i2c.h"
#include "fsl_i2s.h"
#include "fsl_wm8904.h"
#include "fsl_ft5406.h"
#include "fsl_lcdc.h"
#include "fsl_sctimer.h"
#include "eGFX.h"
#include "eGFX_Driver.h"
#include "FONT_5_7_8BPP.h"

#include "biquad.h"				/* Biquad Structure */
#include "BiquadFactory.h"		/* Calculate Biquad Polynomials */
#include "Interaction.h"		/* GUI API */
#include "GUI_Driver.h"			/* GUI Init Routine */

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/* GPIO Pins for measuring interrupt duration */
#define IOCON_GPIO 	0x80u
#define TestPortA 	   4u
#define TestPortB 	   3u
#define TestPinA 	   6u
#define TestPinB 	  16u
#define ToggleA GPIO->NOT[TestPortA] |= 1 << TestPinA
#define ToggleB GPIO->NOT[TestPortB] |= 1 << TestPinB

/* Screen Size */
#define IMG_WIDTH 480
#define IMG_HEIGHT 272

/* I2S Base addresses & clocks */
#define CODEC_I2C 				 	 I2C2
#define I2C_MCLK_FREQUENCY  	12000000u
#define I2S_TX 						 I2S0
#define I2S_RX 					     I2S1
#define I2S_CLOCK_DIV (CLOCK_GetAudioPllOutFreq() / 48000U / 16U / 2U)

#define I16_TO_F(x) (float)(x) / 32768.0f
#define F_TO_I16(y) (int16_t)(y * 32768.0f)

/*******************************************************************************
 * Data Structures
 ******************************************************************************/

typedef union{
	uint32_t data;
	int16_t channel[2];
} sample;

/*******************************************************************************
 * Variables
 ******************************************************************************/
static volatile sample sampleIn;
static volatile sample sampleOut;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

void FLEXCOMM6_IRQHandler(void);	// Tx Interrupt
void FLEXCOMM7_IRQHandler(void);	// Rx Interrupt

/*******************************************************************************
 * Interrupts
 ******************************************************************************/
void FLEXCOMM6_IRQHandler(void)
{
	ToggleB;

	if (I2S_TX->FIFOINTSTAT & I2S_FIFOINTSTAT_TXLVL_MASK)
	{
		/* Write Data */
		I2S_TX->FIFOWR = sampleOut.data;

		/* Clear TX level interrupt flag */
		I2S_TX->FIFOSTAT = I2S_FIFOSTAT_TXLVL(1U);
	}

	ToggleB;
}

void FLEXCOMM7_IRQHandler(void)
{
	ToggleA;
	sample S;
	float l;
	float r;

	/* Clear RX level interrupt flag */
	I2S_RX->FIFOSTAT = I2S_FIFOSTAT_RXLVL(1U);

	/* Read Data */
	S.data = I2S_RX->FIFORD;

	/* int16_t -> float */
	l = I16_TO_F(S.channel[0]);
	r = I16_TO_F(S.channel[1]);

	// CASCADE
	for(int i = 0; i < N_BIQUADS; i++)
	{
		if(mode[i] != k_LoopBack)
		{
			/* process both channels */
			l = BQ_process(&Left[i],  l);
			r = BQ_process(&Right[i], r);
		}
	}

	/* float -> int16_t */
	S.channel[0] = F_TO_I16(l);
	S.channel[1] = F_TO_I16(r);

	/* Write for output */
	sampleOut.data = S.data;

	ToggleA;
}

/*******************************************************************************
 * Code
 ******************************************************************************/

void APP_Clock_Setup()
{
	/* Enable Clocks on AHB Bus */
	CLOCK_EnableClock(kCLOCK_InputMux);
	CLOCK_EnableClock(kCLOCK_Iocon);
	CLOCK_EnableClock(kCLOCK_Gpio0);
	CLOCK_EnableClock(kCLOCK_Gpio1);
	CLOCK_EnableClock(kCLOCK_Gpio2);
	CLOCK_EnableClock(kCLOCK_Gpio3);
	CLOCK_EnableClock(kCLOCK_Gpio4);

	/* Init Audio PLL */
	pll_config_t audio_pll_config = {
				.desiredRate = 24576000U,
				.inputRate   = 12000000U,
	};

	pll_setup_t audio_pll_setup;

	CLOCK_SetupAudioPLLData(&audio_pll_config, &audio_pll_setup);
	audio_pll_setup.flags = PLL_SETUPFLAG_POWERUP | PLL_SETUPFLAG_WAITLOCK;
	CLOCK_SetupAudioPLLPrec(&audio_pll_setup, audio_pll_setup.flags);

	/* Attach USART0 and I2C Clocks */
	CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);		/* Debug USART */
	CLOCK_AttachClk(kFRO12M_to_FLEXCOMM2);				/* I2C to Codec */
	CLOCK_AttachClk(kAUDIO_PLL_to_FLEXCOMM6);			/* I2S Transmit */
	CLOCK_AttachClk(kAUDIO_PLL_to_FLEXCOMM7);			/* I2S Receive */
	CLOCK_AttachClk(kAUDIO_PLL_to_MCLK);				/* Codec MCLK */
	SYSCON->MCLKDIV = SYSCON_MCLKDIV_DIV(0U);			/* No Divider */
	SYSCON->MCLKIO = 1U;								/* Enable */


	/* reset FLEXCOMM for I2C */
	RESET_PeripheralReset(kFC2_RST_SHIFT_RSTn);

	/* reset FLEXCOMM for I2S */
	RESET_PeripheralReset(kFC6_RST_SHIFT_RSTn);
	RESET_PeripheralReset(kFC7_RST_SHIFT_RSTn);
}

void APP_Audio_Setup()
{
	i2s_config_t s_TxConfig;
	i2s_config_t s_RxConfig;
	i2c_master_config_t i2cConfig;
	wm8904_config_t codecConfig;

	/********************************
	*	Configure Codec over I2C	*
	*********************************/

	/*
	 * enableMaster = true;
	 * baudRate_Bps = 100000U;
	 * enableTimeout = false;
	 */
	I2C_MasterGetDefaultConfig(&i2cConfig);
	i2cConfig.baudRate_Bps = WM8904_I2C_BITRATE;
	I2C_MasterInit(CODEC_I2C, &i2cConfig, I2C_MCLK_FREQUENCY);

	PRINTF("Configure WM8904 codec\r\n");

	WM8904_GetDefaultConfig(&codecConfig);
	codecHandle.i2c = CODEC_I2C;
	if (WM8904_Init(&codecHandle, &codecConfig) != kStatus_Success)
	{
		PRINTF("WM8904_Init failed!\r\n");
	}

	/* Initial volume kept low for hearing safety. */
	/* Adjust it to your needs, 0x0006 for -51 dB, 0x0039 for 0 dB etc. */
	WM8904_SetVolume(&codecHandle, 0x30u, 0x30u);

	/****************************
	*	Configure I2S Rx & Tx	*
	*****************************/

	I2S_TxGetDefaultConfig(&s_TxConfig);
	I2S_RxGetDefaultConfig(&s_RxConfig);

	/* Output samples at 48 KHz */
	s_TxConfig.divider = I2S_CLOCK_DIV;

	/* sample-by-sample water mark level */
	s_TxConfig.watermark = 2;
	s_RxConfig.watermark = 2;

	I2S_TxInit(I2S_TX, &s_TxConfig);
	I2S_RxInit(I2S_RX, &s_RxConfig);

	/************************
	*	Interrupt Config	*
	*************************/
	NVIC_SetPriority(FLEXCOMM6_IRQn,0);
	NVIC_SetPriority(FLEXCOMM7_IRQn,0);

	EnableIRQ(FLEXCOMM6_IRQn);
	EnableIRQ(FLEXCOMM7_IRQn);

	/* Write some data to have initial  */
	I2S_TX->FIFOWR = 0xFFFFFFFF;
	I2S_TX->FIFOWR = 0xFFFFFFFF;

	/* Trigger on FIFO watermark level */
	I2S_EnableInterrupts(I2S_TX, kI2S_TxLevelFlag);
	I2S_EnableInterrupts(I2S_RX, kI2S_RxLevelFlag);
}

void APP_STARTUP()
{
	APP_Clock_Setup();
	BOARD_InitPins();
	BOARD_InitBootClocks();		/* Running at 96 MHz */
	SystemCoreClockUpdate();
	BOARD_InitDebugConsole();
	BOARD_InitSDRAM();
	APP_Audio_Setup();

	eGFX_InitDriver();

	/* GPIO Pins */
	IOCON_PinMuxSet(IOCON, TestPortA, TestPinA, IOCON_FUNC0 | IOCON_DIGITAL_EN | IOCON_INPFILT_OFF);
	IOCON_PinMuxSet(IOCON, TestPortB, TestPinB, IOCON_FUNC0 | IOCON_DIGITAL_EN | IOCON_INPFILT_OFF);
	GPIO->DIR[TestPortA] |= (1<<TestPinA);
	GPIO->DIR[TestPortB] |= (1<<TestPinB);
	GPIO->CLR[TestPortA] |= (1<<TestPinA);
	GPIO->CLR[TestPortB] |= (1<<TestPinB);

	/* Touch Controller */
	FT5406_Init(&touch_handle, I2C2);
}

int main(void) {

	ActiveLeft  = &Left[0];
	ActiveRight = &Right[0];

	for(int i = 0; i < N_BIQUADS; i++)
	{
		BQ_init(&Left[i]);
		BQ_init(&Right[i]);
	}

	APP_STARTUP();	/* Clock, Power, Peripheral and Register Configuration */
	Init_GUI();		/* Initialization of predefined GUI */

    while(1)
    {
    	eGFX_ImagePlane_Clear(&eGFX_BackBuffer);

    	/* INPUT: Poll for Touch Event */
    	if(kStatus_Success == FT5406_GetSingleTouch(&touch_handle, &touch_event, &touchY, &touchX))
    	{
    		if(touch_event == kTouch_Contact)
    		{
    			touched = true;
    		}
    	}

    	/* PROCESS: Check if an object was interacted with */
    	if(touched)
    	{
			for(int i = 0; i < N_BUTTONS; i++)
			{
				if(Detect_Touch(&Button_List[i]->Box, touchX, touchY))
				{
					Update_Button(Button_List[i]);
				}
			}

			for(int i = 0; i < N_SLIDERS; i++)
			{
				if(Detect_Touch(&Slider_List[i]->Box, touchX, touchY))
				{
					Update_Slider(Slider_List[i], touchX);
				}
			}


			State_Machine();
			touched = false;
    	}

    	/* OUTPUT: Draw the Screen */

    	for(int i = 0; i < N_BUTTONS; i++)
    	{
    		Draw_Button(Button_List[i]);
    	}

    	for(int i = 0; i < N_SLIDERS; i++)
		{
			Draw_Slider(Slider_List[i]);
		}

		eGFX_Dump(&eGFX_BackBuffer);
    }
}

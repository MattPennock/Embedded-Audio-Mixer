![Embedded Audio Mixer](/Images/LPC.jpg)

On Behalf of Penn State's Team ECHO I'm proud to share our senior design project! We built an Embedded Audio Mixer for use with NXP's [LPCXpresso 54608](http://www.nxp.com/products/microcontrollers-and-processors/arm-processors/lpc-cortex-m-mcus/lpc54000-series-cortex-m4-mcus/lpcxpresso-development-board-for-lpc5460x-mcus:OM13092) development board. The DSP engine performs sample-by-sample processing at 48 kHz using second order IIR filters (aka. 'Biquads'). Our most recent version supports cascading two filters, with more coming in future updates once we rework the underlying Biquad implementation.

To see the project in action check out our [YouTube Demo](https://youtu.be/au6VgSrRr1A), or follow the instructions below to run the project on your own devboard!

Aditionally, check out our [Biquad](https://github.com/MattPennock/Biquad) and [Interaction](https://github.com/MattPennock/Interaction) libraries for the audio processing algorithms and Touch GUI implementation well suited to embedded applications.

## Flashing a Precompiled Project

For users familiar with embedded projects, a prebuilt binary is included that can be flashed using a wide variety of tools.

## Building the Project within MCUXpresso

### Download the SDK & MCUXpresso
Follow these [instructions](http://www.nxp.com/video/getting-started-with-the-mcuxpresso-sdk:GS-MCUXPRESSO-SDK) to download NXP's LPCXpresso SDKv2.2.0. We will be using the GNU toolchain to build and compile with MCUXpresso.

A copy of the MCUXpresso IDE is freely avaliable on NXP's [servers](http://www.nxp.com/products/software-and-tools/run-time-software/mcuxpresso-software-and-tools/mcuxpresso-integrated-development-environment-ide:MCUXpresso-IDE) along with [instructions](http://www.nxp.com/assets/documents/data/en/quick-reference-guides/MCUXpresso_IDE_Installation_Guide.pdf) for installation and importing the SDK downloaded prior, although it is largely self explanatory. 

### Project Setup
Let's start by creating a new MCUXpresso c/c++ project:

![](/images/NewProject.PNG)

Select the LPC54608 devboard in the Device Selection Page and hit 'Next' to configure the compiler.

On the first page of project settings choose an appropriate name, deactiate semi-hosting and select all Freescale drivers, with no OS sources to be included.

![](/images/ProjectSettings.PNG)

Hit Finish and continue to import our sources!

### Importing the Embedded Audio Project

Delete the pre-made contents in the projects source folder and drag our code into the project directory. Choose to copy files into the project. Press F5 to refresh the project's filesystem within MCUXpresso.

![](/images/files.PNG)

eFGX is an embedded graphics library with drivers for the LPC54608 provided by Eli Hughes [here](https://github.com/ehughes/eGFX) and pre-included within this project.

Within MCUXpresso's file browser update the project settings as follows:

Navigate to C/C++ Build -> Settings -> Preprocessor

* Delete the defined symbol `CR_INTEGER_PRINTF` to enable floating point printf statements

Navigate to C/C++ Build -> Settings -> Includes


* Add `"${workspace_loc:/${ProjName}/LPC54608}"`
* Add `"${workspace_loc:/${ProjName}/source/eGFX}"`
* Add `"${workspace_loc:/${ProjName}/source/eGFX/Fonts}"`

Navigate to C/C++ Build -> Settings -> Optimization

* Change `Optimization Level` from None (-O0) to Optimize Most (-O3). Running at 48 kHz leaves 20 us per sample, and having a high optimization level makes a major difference for cascaded secions and responsiveness.

Press OK and apply all changes.

### Build and Debug

In the bottom left of MCUXpresso press the purple bug `Debug 'Embedded Audio Mixer' [Debug]` with your MCUXpresso plugged in and enjoy!

## Future Changes
* Speed up the Biquad implementation
* Move touch events from polling model to interrupt model
* Up clock speed from 96 Mhz to 180 MHz
* Add additional features: filter magnitude response plotting, additional cascades
* Seperate touch & audio codec I2C channels (embarassingly, both use the same Flexcomm to communicate with their respective peripherals.)

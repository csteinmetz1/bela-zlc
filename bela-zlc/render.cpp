/*

This project is based on the code from:
Lecture 20: Phase vocoder, part 3 fft-robotisation

*/

#include <Bela.h>
#include <libraries/Scope/Scope.h>
#include <libraries/math_neon/math_neon.h>
#include <libraries/AudioFile/AudioFile.h>
#include <libraries/Gui/Gui.h>
#include <libraries/GuiController/GuiController.h>
#include "MonoFilePlayer.h"
#include "DirectConvolver.h"
#include "FFTConvolver.h"
#include "ZLConvolver.h"

#include <vector>
#include <cmath>
#include <cstring>
#include <chrono>

// Setup for the impulse response and audio data
MonoFilePlayer gPlayer;
std::string gAudioFilename = "audio/riff.wav";
std::vector<std::string> gImpulseFilenames = {
	"audio/large_room.wav",
	"audio/drum_room.wav",
	"audio/studio.wav",
	"audio/room.wav",
	"audio/plate.wav",
	"audio/church.wav"};

// zero-latency convolvers
ZLConvolver zlConvolverA,
	zlConvolverB,
	zlConvolverC,
	zlConvolverD,
	zlConvolverE,
	zlConvolverF;

// Browser-based GUI to adjust parameters
Gui gGui;
GuiController gGuiController;

unsigned int gRoomSlider;
unsigned int gMaxBlocksSlider;
unsigned int gSparsitySlider;

// global variables that track slider states
int gRoom = 0;
bool gRandom = 0;
int gKernelSize = 0;

unsigned int gWetSlider;
unsigned int gDrySlider;
unsigned int gTanhSlider;
unsigned int gInGainSlider;
unsigned int gOutGainSlider;

/* variables for speed testing
int k = 0;
int blockSize = 1024;
std::vector<float> h;
std::vector<float> gInputBuffer, gOutputBuffer;
std::vector<std::chrono::microseconds> timings;
int timingsCount = 0;
int totalTimings = 100;

FFTConvolver fftConvolver;
DirectConvolver directConvolver;
*/

bool setup(BelaContext *context, void *userData)
{
	// Load the audio file
	if (!gPlayer.setup(gAudioFilename))
	{
		rt_printf("Error loading audio file '%s'\n", gAudioFilename.c_str());
		return false;
	}

	// Print some useful info
	rt_printf("Loaded the audio file '%s' with %d frames (%.1f seconds)\n",
			  gAudioFilename.c_str(), gPlayer.size(),
			  gPlayer.size() / context->audioSampleRate);

	// Set up the GUI
	gGui.setup(context->projectName);

	// and attach to it
	gGuiController.setup(&gGui, "Controls");

	// Arguments: name, default value, minimum, maximum, increment
	// store the return value to read from the slider later on
	gRoomSlider = gGuiController.addSlider("Room", 0.0, 0.0, gImpulseFilenames.size(), 1.0);
	gMaxBlocksSlider = gGuiController.addSlider("Max blocks", 10.0, 0.0, 30.0, 1.0);
	gSparsitySlider = gGuiController.addSlider("Sparsity (%)", 0.0, 0.0, 1.0, 0.1);

	gTanhSlider = gGuiController.addSlider("Tanh (on/off)", 0.0, 0.0, 1.0, 1.0);
	gWetSlider = gGuiController.addSlider("Wet", 0.7, 0.0, 1.0, 0.01);
	gDrySlider = gGuiController.addSlider("Dry", 0.0, 0.0, 1.0, 0.01);
	gInGainSlider = gGuiController.addSlider("In gain (dB)", 0.0, -12.0, 12.0, 0.1);
	gOutGainSlider = gGuiController.addSlider("Out gain (dB)", 0.0, -12.0, 12.0, 0.1);

	// setup/configure the zero-latency convolvers
	zlConvolverA.setup(context->audioFrames, context->audioSampleRate, gImpulseFilenames[0], false, 0);
	zlConvolverB.setup(context->audioFrames, context->audioSampleRate, gImpulseFilenames[1], false, 0);
	zlConvolverC.setup(context->audioFrames, context->audioSampleRate, gImpulseFilenames[2], false, 0);
	zlConvolverD.setup(context->audioFrames, context->audioSampleRate, gImpulseFilenames[3], false, 0);
	zlConvolverE.setup(context->audioFrames, context->audioSampleRate, gImpulseFilenames[4], false, 0);
	zlConvolverF.setup(context->audioFrames, context->audioSampleRate, gImpulseFilenames[5], false, 0);

	/* // convolvers for speed testing
	for (int n = 0; n < blockSize; n++)
		h.push_back(0.0);
		
	gInputBuffer.resize(131072);
	gOutputBuffer.resize(131072);
	timings.resize(totalTimings);

	directConvolver.setup(h, k, gInputBuffer, gOutputBuffer);
	fftConvolver.setup(blockSize *2, h, k, gInputBuffer, gOutputBuffer);
	*/

	return true;
}

void render(BelaContext *context, void *userData)
{
	// Access the sliders specifying the index we obtained when creating then
	int room = (int)gGuiController.getSliderValue(gRoomSlider);
	int maxBlocks = (int)gGuiController.getSliderValue(gMaxBlocksSlider);
	float sparsity = gGuiController.getSliderValue(gSparsitySlider);

	// access other sliders which do not effect state
	float nl = gGuiController.getSliderValue(gTanhSlider);
	float wet = gGuiController.getSliderValue(gWetSlider);
	float dry = gGuiController.getSliderValue(gDrySlider);
	float inGain = gGuiController.getSliderValue(gInGainSlider);
	float outGain = gGuiController.getSliderValue(gOutGainSlider);

	for (unsigned int n = 0; n < context->audioFrames; n++)
	{

		float in = gPlayer.process();
		float out = in * powf(10, inGain / 20);

		switch (room)
		{
		case 0: // large_room
			out = zlConvolverA.process(out, 1.0, 1.0, nl, maxBlocks, sparsity);
			break;
		case 1: // drum_room
			out = zlConvolverB.process(out, 1.0, 1.0, nl, maxBlocks, sparsity);
			break;
		case 2: // studio
			out = zlConvolverC.process(out, 1.0, 1.0, nl, maxBlocks, sparsity);
			break;
		case 3: // room
			out = zlConvolverD.process(out, 1.0, 1.0, nl, maxBlocks, sparsity);
			break;
		case 4: // plate
			out = zlConvolverE.process(out, 1.0, 1.0, nl, maxBlocks, sparsity);
			break;
		case 5: // church
			out = zlConvolverF.process(out, 1.0, 1.0, nl, maxBlocks, sparsity);
			break;
		}

		// wet dry mix
		out = out * wet + in * dry;

		// scale the output mix
		out = out * powf(10, outGain / 20);

		audioWrite(context, n, 0, out);
		audioWrite(context, n, 1, out);
	}

	/* // compute timings (ignore)
    auto start = std::chrono::high_resolution_clock::now();
    
    //for (int n = 0; n < blockSize; n++)
    //	directConvolver.process(0);
    
    fftConvolver.queue(0, false);
    fftConvolver.process();
    
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    timings[timingsCount++] = duration;
    
    // compute average timings
    if (timingsCount == totalTimings){
    	float avgDuration = 0;
	    for (int n = 0; n < timings.size(); n++)
	    	avgDuration += timings[n].count();
	    rt_printf("audioFrames: %0d  blockSize: %d  Avg. %0.3f us\n", context->audioFrames, blockSize, (avgDuration/totalTimings));
	    std::exit(0);
    }
    */
}

void cleanup(BelaContext *context, void *userData)
{
}

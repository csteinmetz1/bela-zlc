/***** ZLConvolver.cpp *****/

#include "ZLConvolver.h"

// Constructor taking the path of a file to load
ZLConvolver::ZLConvolver(int blockSize, int audioSampleRate, std::string impulseFilename, bool random, int kernelSize)
{
	setup(blockSize, audioSampleRate, impulseFilename, random, kernelSize);
}

bool ZLConvolver::setup(int blockSize, int audioSampleRate, std::string impulseFilename, bool random, int kernelSize)
{
	random_ = random;
	MonoFilePlayer impulsePlayer;

	if (!random)
	{
		// Load our impulse response from file
		if (!impulsePlayer.setup(impulseFilename))
		{
			rt_printf("Error loading impulse response file '%s'\n", impulseFilename.c_str());
			return false;
		}

		// Print some useful info
		rt_printf("Loaded the impulse response file '%s' with %d frames (%.1f seconds)\n",
				  impulseFilename.c_str(), impulsePlayer.size(),
				  impulsePlayer.size() / audioSampleRate);

		kernelSize = impulsePlayer.size();
	}

	// Set up the FFT and buffers
	inputBuffer_.resize(bufferSize_);
	outputBuffer_.resize(bufferSize_);

	// Here we create an array of fftConvolvers
	// each has a separate block of the impulse response
	int k = 0; // starting position in the impulse response
	int samplesRead = 0;
	blocks_ = 0;
	std::vector<float> h;

	N_ = 32;
	// we select N to be the smallest N such that
	// the FFT is faster than the direct form convolution

	while (samplesRead < kernelSize)
	{
		int fftSize = 0;
		bool direct = false; // use direct form conv. for first block
		float value;

		// follow pattern: 2N, N, N, 2N, 2N, 4N, 4N, ...
		if (blocks_ == 0)
		{
			fftSize = 2 * N_;
			direct = true;
		}
		else if (blocks_ % 2 != 0)
			fftSize = (int)powf(2, (blocks_ / 2)) * N_;
		else
			fftSize = (int)powf(2, (blocks_ / 2) - 1) * N_;

		if (!random)
			value = impulsePlayer.process();
		else
			value = randFloat(-0.1, 0.1);

		h.push_back(value);
		samplesRead++;

		// when we read enough samples, create a convolver
		if ((samplesRead - k) == (fftSize / 2))
		{
			int priority = (int)basePriority_ - (3 * blocks_);

			if (direct)
			{
				directConvolver_.setup(h, k, inputBuffer_, outputBuffer_);
			}
			else
			{
				// Note: actual FFT size is always twice as large as block size
				FFTConvolver convolver(fftSize, h, k, inputBuffer_, outputBuffer_);
				fftConvolvers_.push_back(convolver);
				convolverBufferSamples_.push_back(0);
				convolverPriority_.push_back(priority);
				//rt_printf("n: %d  fftSize: %d. priority: %d samplesRead: %d  k: %d\n", blocks_, fftSize, priority, samplesRead, k);
			}

			blocks_++;
			h.clear(); // remove all elements from h
			k = samplesRead;
		}
	}

	// create threads for FFT convolutions
	// each convolver will have its own thread
	for (int n = 0; n < fftConvolvers_.size(); n++)
	{
		convolverThreads_.push_back(
			Bela_createAuxiliaryTask(
				convolverLauncher,
				convolverPriority_[n],
				"convolverLauncher",
				&fftConvolvers_[n]));
	}

	rt_printf("Splitting impulse into %d blocks.\n", blocks_);

	return true;
}

float ZLConvolver::process(float in, float wet, float dry, bool nl, int maxBlocks, float sparsity)
{
	// store input sample into input circular buffer
	inputBuffer_[inputBufferPointer_++] = in;
	if (inputBufferPointer_ >= bufferSize_)
	{
		inputBufferPointer_ = 0;
	}

	// direct convolution
	directConvolver_.process(inputBufferPointer_);

	// iterate over FFT convolutions
	for (int n = 0; n < fftConvolvers_.size(); n++)
	{
		// based on the GUI controls we may ignore some blocks in the filter
		bool bypass = false;
		if (n % (int)(((1 - sparsity) * (blocks_ / 2)) + 1) == 0 || n > maxBlocks)
			bypass = true;

		// when enough samples are loaded, we will launch the correct convolver threads
		if (++convolverBufferSamples_[n] == (fftConvolvers_[n].getFftSize() / 2))
		{
			fftConvolvers_[n].queue(inputBufferPointer_, bypass);
			Bela_scheduleAuxiliaryTask(convolverThreads_[n]);
			convolverBufferSamples_[n] = 0; // reset this convolver until buffer is full
		}
	}

	// Get the output sample from the output buffer
	float out = outputBuffer_[outputBufferReadPointer_];

	// Then clear the output sample in the buffer so it is ready for the next overlap-add
	outputBuffer_[outputBufferReadPointer_] = 0;

	// Increment the read pointer in the output circular buffer
	outputBufferReadPointer_++;
	if (outputBufferReadPointer_ >= bufferSize_)
		outputBufferReadPointer_ = 0;

	// create the wet/dry mix
	out = wet * out + dry * in;

	// apply nonlinearity
	if (nl)
		out = tanhf_neon(out);

	return out;
}
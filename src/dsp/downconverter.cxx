#include <math.h>
#include <stdint.h>

#include "debug.h"
#include "downconverter.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define PHASE_BITS		31
#define LOOKUP_BITS		16

#define PHASE_MASK		((1UL << PHASE_BITS) - 1)
#define LOOKUP_MASK		((1UL << LOOKUP_BITS) - 1)
#define LOOKUP_SHIFT	(PHASE_BITS - LOOKUP_BITS)

DownConverter::DownConverter(const string &name) :
	DspBlock(name, "DownConverter"),
	filter(new LowPass(name)),
	_if(0),
	phase(0), phaseStep(0)
{
	/* Calculate sine table for fast NCO */
	sinTable.resize(1UL << LOOKUP_BITS);
	for(unsigned int n = 0; n < sinTable.size(); n++)
		sinTable[n] = sinf((float)n * 2 * M_PI / (float)(1UL << LOOKUP_BITS));
}

DownConverter::~DownConverter()
{
	delete filter;
}

void DownConverter::setIF(int hz)
{
	_if = hz;

	if (isRunning()) {
		/* Update phase accumulator step */
		phaseStep = (int)((int64_t)hz * (int64_t)(1UL << PHASE_BITS) / (int64_t)inputSampleRate());
	}
}

bool DownConverter::init()
{
	if (inputChannels() != 2) {
		LOG_ERROR("Expect IQ input\n");
		return false;
	}

	_outputSampleRate = inputSampleRate();
	_outputChannels = inputChannels();

	/* Calculate initial phase step */
	phaseStep = (int)((int64_t)_if * (int64_t)(1UL << PHASE_BITS) / (int64_t)inputSampleRate());
	LOG_DEBUG("phaseStep = %d for %d Hz\n", phaseStep, _if);

	return true;
}

void DownConverter::deinit()
{

}

bool DownConverter::process(const vector<sample_t> &inBuffer, vector<sample_t> &outBuffer)
{
	const float *in = (const float*)inBuffer.data();
	float *out = (float*)outBuffer.data();
	unsigned int nframes = inBuffer.size() / inputChannels();

	while (nframes--) {
		/* NCO */
		unsigned int sinidx, cosidx;
		sinidx = phase >> LOOKUP_SHIFT;
		cosidx = (sinidx + (1UL << LOOKUP_BITS) / 4) & LOOKUP_MASK;
//		LOG_DEBUG("phase = %u sinidx = %u sin = %f cosidx = %u cos = %f\n", phase, sinidx, sinTable[sinidx], cosidx, sinTable[cosidx]);
		phase = (phase + phaseStep) & PHASE_MASK;

		/* Complex mixer - signal is multiplied by the complex conjugate of the
		 * local oscillator */
		float i = *in++;
		float q = *in++;
		*out++ = i * sinTable[cosidx] + q * sinTable[sinidx]; // I
		*out++ = q * sinTable[cosidx] - i * sinTable[sinidx]; // Q
	}

	return true;
}


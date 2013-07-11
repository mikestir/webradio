/*
 * fm.cxx
 *
 *  Created on: 29 Jun 2013
 *      Author: mike
 */

#include <math.h>

#include "debug.h"
#include "amdemod.h"

AMDemod::AMDemod(const string &name) : DspBlock(name, "AMDemod")
{

}

AMDemod::~AMDemod()
{

}

bool AMDemod::init()
{
	if (inputChannels() != 2) {
		LOG_ERROR("Expect IQ input\n");
		return false;
	}

	_outputSampleRate = inputSampleRate();
	_outputChannels = 1;
	return true;
}

void AMDemod::deinit()
{

}

bool AMDemod::process(const vector<sample_t> &inBuffer, vector<sample_t> &outBuffer)
{
//	static float prev_i, prev_q; // FIXME: not thread safe

	const float *in = (const float*)inBuffer.data();
	float *out = (float*)outBuffer.data();
	unsigned int nframes = inBuffer.size() / inputChannels();

	while(nframes--) {
		float i = *in++;
		float q = *in++;
#if 0
		// multiply conjugate of previous sample
		float ii, qq;
		ii = i * prev_i + q * prev_q;
		qq = q * prev_i - i * prev_q;

		*out++ = atan2f(ii,qq) / M_PI / 2.0;
		prev_i = i;
		prev_q = q;

#else
		*out++ = sqrt(i*i + q*q);
#endif
	}

	return true;
}

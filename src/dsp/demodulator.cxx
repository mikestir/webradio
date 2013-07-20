/*
 * demodulator.cxx
 *
 *  Created on: 29 Jun 2013
 *      Author: mike
 */

#include <cmath>

#include "debug.h"
#include "demodulator.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Demodulator::Demodulator(const string &name) : DspBlock(name, "AMDemod"),
	_mode(AM),
	prev_i(0.0), prev_q(0.0)
{
	/* In same order as enum */
	_modeStrings.push_back("AM");
	_modeStrings.push_back("FM");
	_modeStrings.push_back("USB");
	_modeStrings.push_back("LSB");
}

Demodulator::~Demodulator()
{

}

bool Demodulator::setModeString(const string &mode)
{
	for (unsigned int n = 0; n < _modeStrings.size(); n++) {
		if (_modeStrings[n] == mode) {
			_mode = (Demodulator::Mode)n;
			return true;
		}
	}
	return false;
}

bool Demodulator::init()
{
	if (inputChannels() != 2) {
		LOG_ERROR("Expect IQ input\n");
		return false;
	}

	_outputSampleRate = inputSampleRate();
	_outputChannels = 1;
	return true;
}

void Demodulator::deinit()
{

}

bool Demodulator::process(const vector<sample_t> &inBuffer, vector<sample_t> &outBuffer)
{
	const float *in = (const float*)inBuffer.data();
	float *out = (float*)outBuffer.data();
	unsigned int nframes = inBuffer.size() / inputChannels();

	while(nframes--) {
		float i = *in++;
		float q = *in++;

		switch (_mode) {
		case AM:
			*out++ = sqrt(i*i + q*q);
			break;
		case FM:
			// multiply conjugate of previous sample
			float ii, qq;
			ii = i * prev_i + q * prev_q;
			qq = q * prev_i - i * prev_q;

			*out++ = atan2f(ii,qq) / M_PI / 2.0;
			break;
		case USB:
			*out++ = i + q;
			break;
		case LSB:
			*out++ = i - q;
			break;
		default:
			LOG_ERROR("Bad mode\n");
			return false;
		}

		prev_i = i;
		prev_q = q;
	}

	return true;
}

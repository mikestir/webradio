/*
 * WebRadio web-based Software Defined Radio
 *
 * Copyright (C) 2013 Mike Stirling
 *
 * This file is part of WebRadio (http://www.mike-stirling.com/webradio)
 *
 * All rights reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <cmath>

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
	DspBlock(
			DspData::Int16, DspData::Float,
			name, "DownConverter"),
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
		LOG_ERROR("Expected IQ input\n");
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

bool DownConverter::process(const DspData &in, DspData &out)
{
	out.resize(in.size());

	const short *inptr = (const short*)in.data();
	float *outptr = (float*)out.data();

	for (unsigned int n = 0; n < in.size() / 2; n++) {
		/* NCO */
		unsigned int sinidx, cosidx;
		sinidx = phase >> LOOKUP_SHIFT;
		cosidx = (sinidx + (1UL << LOOKUP_BITS) / 4) & LOOKUP_MASK;
//		LOG_DEBUG("phase = %u sinidx = %u sin = %f cosidx = %u cos = %f\n", phase, sinidx, sinTable[sinidx], cosidx, sinTable[cosidx]);
		phase = (phase + phaseStep) & PHASE_MASK;

		/* Complex mixer - signal is multiplied by the complex conjugate of the
		 * local oscillator */
		float i = (float)(*inptr++) / 32768.0;
		float q = (float)(*inptr++) / 32768.0;
		*outptr++ = i * sinTable[cosidx] + q * sinTable[sinidx]; // I
		*outptr++ = q * sinTable[cosidx] - i * sinTable[sinidx]; // Q
	}

	return true;
}


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

#include <stdlib.h>

#include "testsource.h"
#include "debug.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* NCO */
#define PHASE_BITS		31
#define LOOKUP_BITS		16

#define PHASE_MASK		((1UL << PHASE_BITS) - 1)
#define LOOKUP_MASK		((1UL << LOOKUP_BITS) - 1)
#define LOOKUP_SHIFT	(PHASE_BITS - LOOKUP_BITS)
#define TONE1_FREQ		1000
#define TONE2_FREQ		30600 // for alias at 1400 Hz at 32 kHz sampling


TestSource::TestSource(const string &name) :
	Tuner(DspData::Int16, name, "TestSource")
{
	setChannels(2); // only support I/Q sampling
	
	phase[0] = phase[1] = 0;
	phaseStep[0] = phaseStep[1] = 0;
	
	/* Calculate sine table for fast NCO */
	sinTable.resize(1UL << LOOKUP_BITS);
	for(unsigned int n = 0; n < sinTable.size(); n++)
		sinTable[n] = (int16_t)(sinf((float)n * 2 * M_PI / (float)(1UL << LOOKUP_BITS)) * 32767.0);
}

TestSource::~TestSource()
{

}

bool TestSource::init()
{
	if (inputChannels() != 2) {
		LOG_ERROR("Expected IQ input\n");
		return false;
	}
	
	/* Output is always whatever was asked for */
	_outputSampleRate = inputSampleRate();
	_outputChannels = inputChannels();

	/* Calculate initial phase step */
	phaseStep[0] = (int)((int64_t)TONE1_FREQ * (int64_t)(1UL << PHASE_BITS) / (int64_t)inputSampleRate());
	phaseStep[1] = (int)((int64_t)TONE2_FREQ * (int64_t)(1UL << PHASE_BITS) / (int64_t)inputSampleRate());

	return true;
}

void TestSource::deinit()
{

}

bool TestSource::process(const DspData &in, DspData &out)
{
	out.resize(in.size());

	short *outptr = (short*)out.data();

	for (unsigned int n = 0; n < out.size() / 2; n++) {
		short out = 0;
		
		for (int n = 0; n < 2; n++) {
			unsigned int sinidx;
			sinidx = phase[n] >> LOOKUP_SHIFT;
			phase[n] = (phase[n] + phaseStep[n]) & PHASE_MASK;
			out += sinTable[sinidx] / 4;
		}
		
		*outptr++ = out;
		*outptr++ = 0;
	}

	return true;
}

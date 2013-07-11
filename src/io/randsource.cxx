/*
 * randsource.cxx
 *
 *  Created on: 29 Jun 2013
 *      Author: mike
 */

#include <stdlib.h>

#include "randsource.h"

RandSource::RandSource(const string &name) : SampleSource(name, "RandSource")
{

}

RandSource::~RandSource()
{

}

bool RandSource::init()
{
	/* Output is always whatever was asked for */
	_outputSampleRate = inputSampleRate();
	_outputChannels = inputChannels();

	return true;
}

void RandSource::deinit()
{

}

bool RandSource::process(const vector<sample_t> &inBuffer, vector<sample_t> &outBuffer)
{
	for (unsigned int n = 0; n < outBuffer.size(); n++)
		outBuffer[n] = (float)(random() - RAND_MAX / 2) / (float(RAND_MAX / 2));

	return true;
}

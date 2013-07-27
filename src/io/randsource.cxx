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

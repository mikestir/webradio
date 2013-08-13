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

#ifndef PULSEAUDIO_H_
#define PULSEAUDIO_H_

#include <vector>
#include <string>

#include <pulse/simple.h>
#include <pulse/error.h>

#include "ioblock.h"

using namespace std;

class PulseAudioSource : public SourceBlock
{
public:
	PulseAudioSource(const string &name = "<undefined>");
	~PulseAudioSource();

private:
	Type outputType() { return DspBlock::Float; }

	bool init();
	void deinit();
	int process(const void *inbuffer, unsigned int inframes, void *outbuffer, unsigned int outframes);

	pa_sample_spec ss;
	pa_simple *pa;
};

class PulseAudioSink : public IOBlock
{
public:
	PulseAudioSink(const string &name = "<undefined>");
	~PulseAudioSink();

private:
	Type inputType() { return DspBlock::Float; }
	Type outputType() { return DspBlock::None; }

	bool init();
	void deinit();
	int process(const void *inbuffer, unsigned int inframes, void *outbuffer, unsigned int outframes);

	pa_sample_spec ss;
	pa_simple *pa;
};

#endif /* PULSEAUDIO_H_ */

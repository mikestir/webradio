/*
 * pulseaudio.h
 *
 *  Created on: 24 Jun 2013
 *      Author: mike
 */

#ifndef PULSEAUDIO_H_
#define PULSEAUDIO_H_

#include <vector>
#include <string>

#include <pulse/simple.h>
#include <pulse/error.h>

#include "samplesource.h"
#include "samplesink.h"

using namespace std;

class PulseAudioSource : public SampleSource
{
public:
	PulseAudioSource(const string &name = "<undefined>");
	~PulseAudioSource();

private:
	bool init();
	void deinit();
	bool process(const vector<sample_t> &inBuffer, vector<sample_t> &outBuffer);

	pa_sample_spec ss;
	pa_simple *pa;
};

class PulseAudioSink : public SampleSink
{
public:
	PulseAudioSink(const string &name = "<undefined>");
	~PulseAudioSink();

private:
	bool init();
	void deinit();
	bool process(const vector<sample_t> &inBuffer, vector<sample_t> &outBuffer);

	pa_sample_spec ss;
	pa_simple *pa;
};

#endif /* PULSEAUDIO_H_ */

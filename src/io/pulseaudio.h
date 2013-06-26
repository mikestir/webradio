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
	PulseAudioSource(
			unsigned int samplerate = DEFAULT_SOURCE_SAMPLE_RATE,
			unsigned int channels = DEFAULT_SOURCE_CHANNELS,
			const string &subdevice = "");
	~PulseAudioSource();
	static SampleSource* factory() {
		return new PulseAudioSource();
	}

	bool open();
	void close();

	void setSamplerate(unsigned int samplerate);
	void setChannels(unsigned int channels);
	void setSubdevice(const string &subdevice);

	unsigned int pull(short *samples, unsigned int maxframes);
private:
	pa_sample_spec ss;
	pa_simple *pa;
};

class PulseAudioSink : public SampleSink
{
public:
	PulseAudioSink(
			unsigned int samplerate = DEFAULT_SINK_SAMPLE_RATE,
			unsigned int channels = DEFAULT_SINK_CHANNELS,
			const string &subdevice = "");
	~PulseAudioSink();
	static SampleSink* factory() {
		return new PulseAudioSink();
	}

	bool open();
	void close();

	void setSamplerate(unsigned int samplerate);
	void setChannels(unsigned int channels);
	void setSubdevice(const string &subdevice);

	void push(short *samples, unsigned int nframes);
private:
	pa_sample_spec ss;
	pa_simple *pa;
};

#endif /* PULSEAUDIO_H_ */

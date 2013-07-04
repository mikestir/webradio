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
	PulseAudioSource();
	~PulseAudioSource();

	bool start();
	void stop();

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
	PulseAudioSink();
	~PulseAudioSink();
	static SampleSink* factory() {
		return new PulseAudioSink();
	}

	bool start();
	void stop();

	void setSamplerate(unsigned int samplerate);
	void setChannels(unsigned int channels);
	void setSubdevice(const string &subdevice);

	void push(short *samples, unsigned int nframes);
private:
	pa_sample_spec ss;
	pa_simple *pa;
};

#endif /* PULSEAUDIO_H_ */

/*
 * playbackdevice.h
 *
 *  Created on: 24 Jun 2013
 *      Author: mike
 */

#ifndef SAMPLESINK_H_
#define SAMPLESINK_H_

#include <vector>
#include <string>

#define DEFAULT_SINK_SAMPLE_RATE		48000
#define DEFAULT_SINK_CHANNELS			2

using namespace std;

class SampleSink
{
public:
	SampleSink() :
			_samplerate(DEFAULT_SINK_SAMPLE_RATE),
			_channels(DEFAULT_SINK_CHANNELS),
			_subdevice(""),
			_subdevices() {}
	virtual ~SampleSink() {}

	virtual bool start() { return false; };
	virtual void stop() {};

	unsigned int samplerate() const { return _samplerate; }
	virtual void setSamplerate(unsigned int samplerate) {}
	unsigned int channels() const { return _channels; }
	virtual void setChannels(unsigned int channels) {}
	const string& subdevice() const { return _subdevice; }
	virtual void setSubdevice(const string &subdevice) {}
	const vector<string>& subdevices() const { return _subdevices; }

	virtual void push(short *samples, unsigned int nframes);
	virtual void push(float *samples, unsigned int nframes);
protected:
	unsigned int _samplerate;
	unsigned int _channels;
	string _subdevice;
	vector<string> _subdevices;
};

#endif /* SAMPLESINK_H_ */

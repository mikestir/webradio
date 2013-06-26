/*
 * capturedevice.h
 *
 *  Created on: 24 Jun 2013
 *      Author: mike
 */

#ifndef SAMPLESOURCE_H_
#define SAMPLESOURCE_H_

#include <vector>
#include <string>

#define DEFAULT_SOURCE_SAMPLE_RATE		48000
#define DEFAULT_SOURCE_CHANNELS			2

using namespace std;

class SampleSource
{
public:
	SampleSource(
			unsigned int samplerate = DEFAULT_SOURCE_SAMPLE_RATE,
			unsigned int channels = DEFAULT_SOURCE_CHANNELS,
			const string &subdevice = "") :
			_samplerate(samplerate),
			_channels(channels),
			_subdevice(subdevice),
			_subdevices() {}
	virtual ~SampleSource() {}
	static SampleSource* factory() {
		return new SampleSource();
	}

	virtual bool open() { return false; };
	virtual void close() {};

	unsigned int samplerate() const { return _samplerate; }
	virtual void setSamplerate(unsigned int samplerate) {}
	unsigned int channels() const { return _channels; }
	virtual void setChannels(unsigned int channels) {}
	const string& subdevice() const { return _subdevice; }
	virtual void setSubdevice(const string &subdevice) {}
	const vector<string>& subdevices() const { return _subdevices; }

	virtual unsigned int pull(short *samples, unsigned int maxframes);
	virtual unsigned int pull(float *samples, unsigned int maxframes);
protected:
	unsigned int _samplerate;
	unsigned int _channels;
	string _subdevice;
	vector<string> _subdevices;
};

#endif /* SAMPLESOURCE_H_ */

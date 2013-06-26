/*
 * tuner.h
 *
 *  Created on: 25 Jun 2013
 *      Author: mike
 */

#ifndef TUNER_H_
#define TUNER_H_

#include <string>

#include "samplesource.h"

using namespace std;

#define DEFAULT_TUNER_SAMPLE_RATE	1200000
#define DEFAULT_TUNER_CHANNELS		2

class Tuner : public SampleSource
{
public:
	Tuner(
			unsigned int samplerate = DEFAULT_TUNER_SAMPLE_RATE,
			unsigned int channels = DEFAULT_TUNER_CHANNELS,
			const string &subdevice = "") :
			SampleSource(samplerate, channels, subdevice),
			_centreFrequency(100000000),
			_offsetPPM(0),
			_AGC(true),
			_gainDB(0) {}
	virtual ~Tuner() {}
	static Tuner* factory() {
		return new Tuner();
	}

	unsigned int centreFrequency() const { return _centreFrequency; }
	virtual void setCentreFrequency(unsigned int hz) {}
	int offsetPPM() const { return _offsetPPM; }
	virtual void setOffsetPPM(int ppm) {}
	bool AGC() const { return _AGC; }
	virtual void setAGC(bool agc) {}
	int gainDB() const { return _gainDB; }
	virtual void setGainDB(int gain) {}

protected:
	unsigned int	_centreFrequency;
	int				_offsetPPM;
	bool			_AGC;
	int				_gainDB;
};

#endif /* TUNER_H_ */

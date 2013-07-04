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
	Tuner() :
			SampleSource(),
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
	virtual float gainDB() const { return _gainDB; }
	virtual void setGainDB(float gain) {}

protected:
	unsigned int	_centreFrequency;
	int				_offsetPPM;
	bool			_AGC;
	float			_gainDB;
};

#endif /* TUNER_H_ */

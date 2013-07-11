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
			const string &name = "<undefined>",
			const string &type = "Tuner") :
			SampleSource(name, type),
			_centreFrequency(100000000),
			_offsetPPM(0),
			_AGC(true),
			_gainDB(0) {}
	virtual ~Tuner() {}

	unsigned int centreFrequency() const { return _centreFrequency; }
	int offsetPPM() const { return _offsetPPM; }
	bool AGC() const { return _AGC; }
	// Subclass can provide live access to AGC gain if desired
	virtual float gainDB() const { return _gainDB; }

	virtual void setCentreFrequency(unsigned int hz) {}
	virtual void setOffsetPPM(int ppm) {}
	virtual void setAGC(bool agc) {}
	virtual void setGainDB(float gain) {}

protected:
	unsigned int	_centreFrequency;
	int				_offsetPPM;
	bool			_AGC;
	float			_gainDB;
};

#endif /* TUNER_H_ */

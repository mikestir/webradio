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

	const string& name() const { return _name; }
	const string& manufacturer() const { return _manufacturer; }
	const string& product() const { return _product; }
	const string& serial() const { return _serial; }

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
	string			_name;
	string			_manufacturer;
	string			_product;
	string			_serial;

	unsigned int	_centreFrequency;
	int				_offsetPPM;
	bool			_AGC;
	float			_gainDB;
};

typedef Tuner* (*TunerFactory)(const string&);

#endif /* TUNER_H_ */

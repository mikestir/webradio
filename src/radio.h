/*
 * radio.h
 *
 *  Created on: 16 Jul 2013
 *      Author: mstirling
 */

#ifndef RADIO_H_
#define RADIO_H_

#include <map>
#include <string>

#include "tuner.h"
#include "spectrumsink.h"
#include "lowpass.h"
#include "downconverter.h"
#include "demodulator.h"
#include "audiostream.h"

class Receiver;
class FrontEnd;

class Receiver {
public:
	Receiver();
	~Receiver();

	FrontEnd* frontEnd() { return _frontEnd; }
	void setFrontEnd(FrontEnd *frontend);

	DspBlock* input() { return _downconverter; }

	DownConverter* downconverter() { return _downconverter; }
	LowPass* channelFilter() { return _channelFilter; }
	Demodulator* demodulator() { return _demodulator; }
	LowPass* audioFilter() { return _audioFilter; }
	AudioStreamManager* stream() { return _stream; }

	const string& uuid() { return _uuid; }

private:
	DownConverter* _downconverter;
	LowPass* _channelFilter;
	Demodulator* _demodulator;
	LowPass* _audioFilter;
	AudioStreamManager* _stream;

	string _uuid;
	FrontEnd *_frontEnd;
};

class FrontEnd {
public:
	friend class Receiver; // for add/removeReceiver

	FrontEnd(TunerFactory factory);
	~FrontEnd();

	Tuner* tuner() { return _tuner; }
	SpectrumSink* spectrum() { return _spectrum; }

	const string& uuid() const { return _uuid; }
	const map<string, Receiver*>& receivers() const { return _receivers; }
private:
	void addReceiver(Receiver *rx);
	void removeReceiver(Receiver *rx);

	Tuner *_tuner;
	SpectrumSink *_spectrum;

	string _uuid;
	map<string, Receiver*> _receivers;
};

namespace Radio {
	/* These functions export the global maps updated when creating/destroying
	 * a front end or receiver */
	const map<string, FrontEnd*>& frontEnds();
	const map<string, Receiver*>& receivers();

	void run();
}

#endif /* RADIO_H_ */

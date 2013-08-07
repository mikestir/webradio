/*
 * WebRadio web-based Software Defined Radio
 *
 * Copyright (C) 2013 Mike Stirling
 *
 * This file is part of WebRadio (http://www.mike-stirling.com/webradio)
 *
 * All rights reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef RADIO_H_
#define RADIO_H_

#include <map>
#include <string>

#include <stdint.h>

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

	void profile();
	void run();
}

#endif /* RADIO_H_ */

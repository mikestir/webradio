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

#include <map>
#include <string>
#include <cstdio>

#include "radio.h"
#include "debug.h"

/* Global maps of UUIDs to front-end/receiver objects */
static map<string, FrontEnd*> g_frontEnds;
static map<string, Receiver*> g_receivers;

static string tohex(unsigned int n)
{
	char str[5];
	snprintf(str, sizeof(str), "%04X", n);
	return string(str);
}

namespace Radio {
	const map<string, FrontEnd*>& frontEnds() {
		return g_frontEnds;
	}

	const map<string, Receiver*>& receivers() {
		return g_receivers;
	}

	void profile() {
		for (map<string, FrontEnd*>::iterator it = g_frontEnds.begin(); it != g_frontEnds.end(); ++it) {
			float busyms = (float)it->second->tuner()->nsPerSecond() / 1000000.0;
			LOG_DEBUG("Front end %s busy %f ms/s\n", it->second->uuid().c_str(), busyms);
		}
	}

	void run() {
		for (map<string, FrontEnd*>::iterator it = g_frontEnds.begin(); it != g_frontEnds.end(); ++it)
			it->second->tuner()->run();
	}
}

Receiver::Receiver()
{
	/* Generate UUID */
	_uuid = tohex(g_receivers.size());

	/* Create components */
	_downconverter = new DownConverter(_uuid);
	_channelFilter = new LowPass(_uuid);
	_demodulator = new Demodulator(_uuid);
	_audioFilter = new LowPass(_uuid);
	_stream = new AudioStreamManager(_uuid);
	_downconverter->connect(_channelFilter);
	_channelFilter->connect(_demodulator);
	_demodulator->connect(_audioFilter);
	_audioFilter->connect(_stream);

	_downconverter->setOutputSampleRate(240000);
	_channelFilter->setPassband(25000);
	_channelFilter->setOutputSampleRate(48000);
	_audioFilter->setPassband(4000);
	_audioFilter->setOutputSampleRate(8000);
	_demodulator->setMode(Demodulator::AM);
	_stream->setSubdevice(_uuid);

	_frontEnd = NULL; // starts disconnected

	/* Add to map */
	g_receivers[_uuid] = this;
	LOG_DEBUG("Created receiver %s\n", _uuid.c_str());
}

Receiver::~Receiver()
{
	/* Disconnect */
	if (_frontEnd != NULL)
		setFrontEnd(NULL);

	/* Remove from map */
	g_receivers.erase(_uuid);

	/* Delete components */
	delete _downconverter;
	delete _channelFilter;
	delete _demodulator;
	delete _audioFilter;
	delete _stream;
	LOG_DEBUG("Destroyed receiver %s\n", _uuid.c_str());
}

void Receiver::setFrontEnd(FrontEnd *frontend)
{
	if (_frontEnd)
		_frontEnd->removeReceiver(this);

	_frontEnd = frontend;
	if (_frontEnd)
		_frontEnd->addReceiver(this);
}

FrontEnd::FrontEnd(TunerFactory factory)
{
	/* Generate UUID */
	_uuid = tohex(g_frontEnds.size());

	/* Create components */
	_tuner = factory(_uuid);
	_spectrum = new SpectrumSink(_uuid);
	_tuner->connect(_spectrum);

	/* Add to map */
	g_frontEnds[_uuid] = this;
	LOG_DEBUG("Created front-end %s\n", _uuid.c_str());
}

FrontEnd::~FrontEnd()
{
	/* Detach all receivers */
	for (map<string, Receiver*>::iterator it = _receivers.begin(); it != _receivers.end(); ++it)
		_tuner->disconnect(it->second->input());
	_receivers.clear();

	/* Remove from map */
	g_frontEnds.erase(_uuid);

	/* Delete components */
	delete _tuner;
	delete _spectrum;
	LOG_DEBUG("Destroyed front-end %s\n", _uuid.c_str());
}

void FrontEnd::addReceiver(Receiver *rx)
{
	_tuner->connect(rx->input());
	_receivers[rx->uuid()] = rx; // add to map
	LOG_DEBUG("Added rx %s to front-end %s\n", rx->uuid().c_str(), _uuid.c_str());
}

void FrontEnd::removeReceiver(Receiver *rx)
{
	_receivers.erase(rx->uuid()); // remove from map
	_tuner->disconnect(rx->input());
	LOG_DEBUG("Removed rx %s from front-end %s\n", rx->uuid().c_str(), _uuid.c_str());
}

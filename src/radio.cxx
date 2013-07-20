/*
 * radio.cxx
 *
 *  Created on: 16 Jul 2013
 *      Author: mstirling
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

	_channelFilter->setPassband(80000);
	_channelFilter->setOutputSampleRate(480000);
	_audioFilter->setPassband(8000);
	_audioFilter->setOutputSampleRate(48000);
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

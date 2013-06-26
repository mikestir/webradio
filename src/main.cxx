/*
 * main.cxx
 *
 *  Created on: 24 Jun 2013
 *      Author: mike
 */
#include <fstream>
#include <vector>

#include "httpserver.h"
#include "audiostream.h"
#include "filehandler.h"
#include "debug.h"

#include "pulseaudio.h"
#include "rtlsdrtuner.h"

int main(int argc, char **argv)
{
	HttpServer *server;

	SampleSource *cp = PulseAudioSource::factory();
	cp->open();

	server = new HttpServer(8080);

	server->registerHandler("/static", FileHandler::factory);
	server->registerHandler("/audio", AudioStreamHandler::factory);
	server->start();

	SampleSink *stream = AudioStreamManager::factory();
	stream->setSubdevice("/teststream1");
	stream->open();

	SampleSink *stream2 = AudioStreamManager::factory();
	stream2->setSubdevice("/teststream2");
	stream2->open();

	Tuner *tuner = RtlSdrTuner::factory();
	tuner->setCentreFrequency(98500000);
	tuner->setOffsetPPM(25);
	tuner->setAGC(true);
	tuner->setSamplerate(1200000);
	tuner->open();

	while (1) {
		short buf[1024];
		cp->pull(buf, sizeof(buf) / sizeof(short) / cp->channels());
		stream->push(buf, sizeof(buf) / sizeof(short) / cp->channels());
		stream2->push(buf, sizeof(buf) / sizeof(short) / cp->channels());
	}

	delete server;
	return 0;
}

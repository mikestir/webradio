/*
 * main.cxx
 *
 *  Created on: 24 Jun 2013
 *      Author: mike
 */
#include <fstream>
#include <vector>

#include <signal.h>
#include <sys/time.h>

#include "httpserver.h"
#include "audiostream.h"
#include "filehandler.h"
#include "apihandler.h"
#include "debug.h"

#include "pulseaudio.h"
#include "rtlsdrtuner.h"
#include "spectrumsink.h"
#include "filter.h"
#include "amdemod.h"
#include "randsource.h"

static volatile bool quit = false;

SpectrumSink *spectrum;
Tuner *tuner;
Filter *filter;

static void sighandler(int signum)
{
	quit = true;
}

int main(int argc, char **argv)
{
	struct sigaction sa;
	HttpServer *server;
	
	sa.sa_handler = sighandler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sigaction(SIGINT, &sa, NULL);

#if 0
	SampleSource *rand = new RandSource();
	rand->setSamplerate(48000);
	rand->setChannels(2);
	rand->start();

	SampleSource *cp = new PulseAudioSource();
	cp->setSamplerate(48000);
	cp->setChannels(2);
	cp->start();
#endif
	SampleSink *pb = new PulseAudioSink();
	pb->setSamplerate(48000);
	pb->setChannels(1);
	pb->start();
	
	SampleSink *stream = new AudioStreamManager();
	stream->setSubdevice("/stream1");
	stream->setChannels(1);
	stream->start();


	tuner = new RtlSdrTuner();
	tuner->setCentreFrequency(124325000);
	tuner->setOffsetPPM(25);
	tuner->setAGC(true);
	tuner->setSamplerate(1200000);
	tuner->start();
	LOG_DEBUG("gain is %f dB\n", tuner->gainDB());
	
	spectrum = new SpectrumSink();
	spectrum->setSamplerate(tuner->samplerate());
	spectrum->setChannels(tuner->channels());
	spectrum->start();
	
	/* channel filter */
	filter = new Filter();
	filter->setInputSamplerate(tuner->samplerate());
	filter->setOutputSamplerate(pb->samplerate());
	filter->setChannels(tuner->channels());
	filter->setPassband(20000);
	filter->start();

	AMDemod *demod = new AMDemod();
	demod->setInputSamplerate(filter->outputSamplerate());
	demod->start();

	server = new HttpServer(8080);

	server->registerHandler("/static", FileHandler::factory);
	server->registerHandler("/audio", AudioStreamHandler::factory);
	server->registerHandler("/api", ApiHandler::factory);
	server->start();

	struct timeval last;
	unsigned int incount = 0, outcount = 0;
	gettimeofday(&last, NULL);
	while (!quit) {

		/* 2.4 MHz / 50 = 48 kHz */
		static float buf[51200];
		static float buf2[51200];
		unsigned int frames1, frames2;
		frames1 = tuner->pull(buf, sizeof(buf) / sizeof(float) / tuner->channels());
		incount += frames1;
		spectrum->push(buf, frames1);
		frames2 = filter->process(buf, buf2, frames1);
		outcount += frames2;
		demod->process(buf2, buf, frames2);
		stream->push(buf, frames2);

		struct timeval now;
		gettimeofday(&now, NULL);
		if (now.tv_sec != last.tv_sec) {
			last = now;
			LOG_DEBUG("inframes = %u, outframes = %u\n", incount, outcount);
			incount = 0;
			outcount = 0;
		}

//		struct timeval t;
//		gettimeofday(&t, NULL);
//		LOG_DEBUG("time %u.%05u: %u, %u\n", t.tv_sec,t.tv_usec,frames1,frames2);
	}

#if 0
	LOG_INFO("cleaning up\n");
	delete server;
	delete spectrum;
	delete tuner;
	delete stream2;
	delete stream;
	delete cp;
#endif

	return 0;
}

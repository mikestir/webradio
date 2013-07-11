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
#include "lowpass.h"
#include "randsource.h"
#include "downconverter.h"
#include "amdemod.h"

static volatile bool quit = false;

static void sighandler(int signum)
{
	quit = true;
}

SpectrumSink *spectrum;
Tuner *tuner;
DownConverter *dc1, *dc2;

int main(int argc, char **argv)
{
	struct sigaction sa;
	struct timeval now,last;
	
	sa.sa_handler = sighandler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sigaction(SIGINT, &sa, NULL);
	
	//SampleSource *src = new PulseAudioSource();
	//src->setSampleRate(48000);
	//src->setChannels(2);
	//src->setBlockSize(32768);

	tuner = new RtlSdrTuner();
	tuner->setCentreFrequency(124000000);
	tuner->setOffsetPPM(25);
	tuner->setSampleRate(2400000);
	tuner->setBlockSize(204800);
	tuner->setAGC(true);

	spectrum = new SpectrumSink();
	tuner->connect(spectrum);


	dc1 = new DownConverter();
	dc1->setIF(325000);

	LowPass *filt1 = new LowPass();
	filt1->setPassband(80000);
	filt1->setOutputSampleRate(240000);

	AMDemod *demod = new AMDemod();

	LowPass *filt2 = new LowPass();
	filt2->setPassband(8000);
	filt2->setOutputSampleRate(48000);

	SampleSink *sink = new AudioStreamManager();
	sink->setSubdevice("/stream1");

	tuner->connect(dc1);
	dc1->connect(filt1);
	filt1->connect(demod);
	demod->connect(filt2);
	filt2->connect(sink);

#if 0
	dc2 = new DownConverter();
	dc2->setIF(1010000);

	LowPass *filt3 = new LowPass();
	filt3->setPassband(80000);
	filt3->setOutputSampleRate(240000);

	AMDemod *demod2 = new AMDemod();

	LowPass *filt4 = new LowPass();
	filt4->setPassband(8000);
	filt4->setOutputSampleRate(248000);

	SampleSink *sink2 = new AudioStreamManager();
	sink2->setSubdevice("/stream2");

	tuner->connect(dc2);
	dc2->connect(filt3);
	filt3->connect(demod2);
	demod2->connect(filt4);
	filt4->connect(sink2);
#endif


	tuner->start();

	HttpServer *h = new HttpServer(8080);
	h->registerHandler("/audio", AudioStreamHandler::factory);
	h->registerHandler("/static", FileHandler::factory);
	h->registerHandler("/api", ApiHandler::factory);
	h->start();


	gettimeofday(&last, NULL);
	while (!quit) {
		static unsigned int lastin = 0;
		gettimeofday(&now, NULL);
		tuner->run();
		if (now.tv_sec >= last.tv_sec + 5) {
			LOG_DEBUG("%u Hz\n", (tuner->totalOut() - lastin) / 5);
			last = now;
			lastin = tuner->totalOut();
		}
	}

	delete h;
	delete tuner;
	//delete filt1;
	delete filt2;
	delete demod;
	delete sink;

	return 0;
}

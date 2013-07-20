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

#include "debug.h"

#include "httpserver.h"
#include "audiostream.h"
#include "confighandler.h"
#include "filehandler.h"
#include "redirecthandler.h"
#include "tunerhandler.h"
#include "tunercontrolhandler.h"
#include "receiverhandler.h"
#include "waterfallhandler.h"

#include "rtlsdrtuner.h"
#include "spectrumsink.h"
#include "radio.h"

static volatile bool quit = false;

static void sighandler(int signum)
{
	quit = true;
}

int main(int argc, char **argv)
{
	struct sigaction sa;
	struct timeval last;
	string tunerid = "00000001";
	
	sa.sa_handler = sighandler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sigaction(SIGINT, &sa, NULL);

	if (argc > 1) {
		tunerid = string(argv[1]);
	}

	/* FIXME: This just builds a simple single front-end, single
	 * receiver stack.  We can handle way more than this!
	 */
	FrontEnd *fe = new FrontEnd(RtlSdrTuner::factory);
	fe->tuner()->setSubdevice(tunerid);
	Receiver *rx = new Receiver();
	rx->setFrontEnd(fe);
	fe->tuner()->setCentreFrequency(124325000);
	fe->tuner()->setSampleRate(2400000);
	fe->tuner()->setBlockSize(204800);
	//fe->tuner()->setGainDB(50);
	fe->tuner()->setAGC(true);
	rx->downconverter()->setIF(0);
	rx->demodulator()->setMode(Demodulator::AM);

	HttpServer *h = new HttpServer(8080);
	h->registerHandler("", RedirectHandler::factory, new string("/static/ui.html"));
	h->registerHandler("static/**", FileHandler::factory);
	h->registerHandler("audio/*", AudioStreamHandler::factory);
	h->registerHandler("config", ConfigHandler::factory);
	h->registerHandler("tuners", TunerHandler::factory);
	h->registerHandler("tuners/*", TunerHandler::factory);
	h->registerHandler("tuners/*/control", TunerControlHandler::factory);
//	h->registerHandler("tuners/*/peaks", PeaksHandler::factory);
	h->registerHandler("tuners/*/waterfall", WaterfallHandler::factory);
	h->registerHandler("tuners/*/receivers", RedirectHandler::factory, new string("/receivers?tuner_id=$1"));
	h->registerHandler("receivers", ReceiverHandler::factory);
	h->registerHandler("receivers/*", ReceiverHandler::factory);
	h->registerHandler("receivers/*/audio.mp3", RedirectHandler::factory, new string("/audio/$1.mp3"));
	h->registerHandler("receivers/*/audio.ogg", RedirectHandler::factory, new string("/audio/$1.ogg"));
	h->start();

	if (!fe->tuner()->start()) { // FIXME: Maybe there should be shortcut in Radio to do this
		LOG_ERROR("Pipeline failed to start\n");
		goto exit;
	}

	gettimeofday(&last, NULL);
	while (!quit) {
		Radio::run();
#if 0
//		static unsigned int lastin = 0;
		struct timeval now;
		gettimeofday(&now, NULL);

		if (now.tv_sec >= last.tv_sec + 5) {
			LOG_DEBUG("%u Hz\n", (tuner->totalOut() - lastin) / 5);
			last = now;
			lastin = tuner->totalOut();
			LOG_DEBUG("capture %lu ns/frame\n", tuner->totalNanoseconds() / tuner->totalIn());
			LOG_DEBUG("waterfall %lu ns/frame\n", spectrum->totalNanoseconds() / spectrum->totalIn());
			LOG_DEBUG("downconverter %lu ns/frame\n", rx->downconverter()->totalNanoseconds() / rx->downconverter()->totalIn());
			LOG_DEBUG("channel filter %lu ns/frame\n", rx->channelFilter()->totalNanoseconds() / rx->channelFilter()->totalIn());
			LOG_DEBUG("demod %lu ns/frame\n", rx->demodulator()->totalNanoseconds() / rx->demodulator()->totalIn());
			LOG_DEBUG("audio filter %lu ns/frame\n", rx->audioFilter()->totalNanoseconds() / rx->audioFilter()->totalIn());

		}
#endif
	}

exit:
	delete h;
	delete rx;
	delete fe;

	LOG_DEBUG("bye bye!\n");

	return 0;
}

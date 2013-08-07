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
	time_t t = 0, tn = 0;
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
	fe->tuner()->setCentreFrequency(124325000);
	fe->tuner()->setSampleRate(2400000);
	fe->tuner()->setBlockSize(204800);
	//fe->tuner()->setGainDB(50);
	fe->tuner()->setAGC(true);
	fe->tuner()->setOffsetPPM(25);

	Receiver *rx = new Receiver();
	rx->setFrontEnd(fe);
	rx->downconverter()->setIF(0);
	rx->demodulator()->setMode(Demodulator::AM);

#if 0
	rx = new Receiver();
	rx->setFrontEnd(fe);
	rx->downconverter()->setIF(100000);
	rx->demodulator()->setMode(Demodulator::FM);
#endif

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

	while (!quit) {
		Radio::run();

		t = time(NULL);
		if (t >= tn) {
			Radio::profile();
			tn = t + 5;
		}
	}

exit:
	delete h;
	delete rx;
	delete fe;

	LOG_DEBUG("bye bye!\n");

	return 0;
}

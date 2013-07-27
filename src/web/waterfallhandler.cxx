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

#include <vector>
#include <string>
#include <sstream>
#include <cmath>

#include <json/json.h>

#include "waterfallhandler.h"
#include "debug.h"

WaterfallHandler::WaterfallHandler() : HttpRequestHandler()
{

}

WaterfallHandler::~WaterfallHandler()
{

}

unsigned short WaterfallHandler::doGet(const vector<string> &wildcards, const vector<char> &requestData)
{
	if (wildcards.size() < 1)
		return MHD_HTTP_NOT_FOUND;

	string uuid = wildcards[0];
	if (Radio::frontEnds().count(uuid) == 0)
		return MHD_HTTP_NOT_FOUND;

	FrontEnd *fe = Radio::frontEnds().at(uuid);
	vector<float> magn;

	magn.resize(fe->spectrum()->fftSize());
	fe->spectrum()->getSpectrum(magn.data());

	Json::Value root;
	root["centre_frequency"] = fe->tuner()->centreFrequency();
	root["sample_rate"] = fe->spectrum()->inputSampleRate();

	for (vector<float>::iterator it = magn.begin(); it != magn.end(); it++) {
		// JSON can't represent inf or nan so convert to a large -ve number
		if (isfinite(*it))
			root["data"].append(*it);
		else
			root["data"].append(-10000.0);
	}

	Json::StyledWriter writer;
	string output = writer.write(root);
	_contentType = "application/json";
	_data.insert(_data.end(), output.begin(), output.end());
	return MHD_HTTP_OK;
}


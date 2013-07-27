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

#include <json/json.h>

#include "tunerhandler.h"
#include "debug.h"

TunerHandler::TunerHandler() : HttpRequestHandler()
{

}

TunerHandler::~TunerHandler()
{

}

unsigned short TunerHandler::doGet(const vector<string> &wildcards, const vector<char> &requestData)
{
	Json::Value root;

	if (wildcards.size() == 0) {
		/* Return data for all registered front-ends */
		for (map<string, FrontEnd*>::const_iterator it = Radio::frontEnds().begin(); it != Radio::frontEnds().end(); ++it)
			root.append(buildTunerInfo(it->second));
	} else {
		string uuid = wildcards[0];

		/* Return data for specific front-end */
		if (Radio::frontEnds().count(uuid) == 0)
			return MHD_HTTP_NOT_FOUND;
		root = buildTunerInfo(Radio::frontEnds().at(uuid));
	}

	Json::StyledWriter writer;
	string output = writer.write(root);
	_contentType = "application/json";
	_data.insert(_data.end(), output.begin(), output.end());
	return MHD_HTTP_OK;
}

Json::Value TunerHandler::buildTunerInfo(FrontEnd *fe)
{
	Json::Value root;

	root["uri"] = "/tuners/" + fe->uuid();
	root["name"] = fe->tuner()->name();
	root["driver"] = fe->tuner()->type();
	root["port"] = "";
	root["serial_nr"] = fe->tuner()->serial();
	root["manufacturer"] = fe->tuner()->manufacturer();
	root["product"] = fe->tuner()->product();
	root["sample_rate"] = fe->tuner()->outputSampleRate();
	root["iq"] = (fe->tuner()->outputChannels() == 2) ? "true" : "false";
	root["control"] = "/tuners/" + fe->uuid() + "/control";
	root["peaks"] = "/tuners/" + fe->uuid() + "/peaks";
	root["receivers"] = "/tuners/" + fe->uuid() + "/receivers";
	root["waterfall"] = "/tuners/" + fe->uuid() + "/waterfall";
	return root;
}

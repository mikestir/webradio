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

#include "tunercontrolhandler.h"
#include "debug.h"

TunerControlHandler::TunerControlHandler() : HttpRequestHandler()
{

}

TunerControlHandler::~TunerControlHandler()
{

}

unsigned short TunerControlHandler::doGet(const vector<string> &wildcards, const vector<char> &requestData)
{
	Json::Value root;

	if (wildcards.size() == 0)
		return MHD_HTTP_NOT_FOUND;

	string uuid = wildcards[0];
	if (Radio::frontEnds().count(uuid) == 0)
		return MHD_HTTP_NOT_FOUND;

	root = buildTunerControl(Radio::frontEnds().at(uuid));

	Json::StyledWriter writer;
	string output = writer.write(root);
	_contentType = "application/json";
	_data.insert(_data.end(), output.begin(), output.end());
	return MHD_HTTP_OK;
}

unsigned short TunerControlHandler::doPut(const vector<string> &wildcards, const vector<char> &requestData)
{
	if (wildcards.size() == 0)
		return MHD_HTTP_NOT_FOUND;

	string uuid = wildcards[0];
	if (Radio::frontEnds().count(uuid) == 0)
		return MHD_HTTP_NOT_FOUND;

	Json::Reader reader;
	Json::Value root;
	bool ok = reader.parse(requestData.data(), root);
	if (!ok)
		return MHD_HTTP_BAD_REQUEST;

	unsigned short status = parseTunerControl(Radio::frontEnds().at(uuid), root);

	_contentType = "application/json";
	return status;
}

Json::Value TunerControlHandler::buildTunerControl(FrontEnd *fe)
{
	Json::Value root;

	/* FIXME: IF gain */
	root["centre_frequency"] = fe->tuner()->centreFrequency();
	root["agc"] = fe->tuner()->AGC();
	root["rf_gain"] = fe->tuner()->gainDB();
	root["if_gain"] = 0;
	root["offset"] = fe->tuner()->offsetPPM();

	return root;
}

unsigned short TunerControlHandler::parseTunerControl(FrontEnd *fe, Json::Value &root)
{
	/* FIXME: IF gain, some kind of validation */
	if (root.isMember("centre_frequency"))
		fe->tuner()->setCentreFrequency(root["centre_frequency"].asUInt());
	if (root.isMember("agc"))
		fe->tuner()->setAGC(root["agc"].asBool());
	if (root.isMember("rf_gain"))
		fe->tuner()->setGainDB(root["rf_gain"].asInt());
	if (root.isMember("offset"))
		fe->tuner()->setOffsetPPM(root["offset"].asInt());

	return MHD_HTTP_NO_CONTENT;
}


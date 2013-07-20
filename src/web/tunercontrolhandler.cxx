/*
 * tunercontrolhandler.cxx
 *
 *  Created on: 20 Jul 2013
 *      Author: mike
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


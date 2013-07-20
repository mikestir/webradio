/*
 * tunerhandler.cxx
 *
 *  Created on: 16 Jul 2013
 *      Author: mike
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

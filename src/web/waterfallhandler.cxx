/*
 * waterfallhandler.cxx
 *
 *  Created on: 16 Jul 2013
 *      Author: mike
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


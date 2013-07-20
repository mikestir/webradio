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

//#include <json/json.h>

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

	// FIXME: Do this with jsoncpp?
	stringstream s;
	vector<float>::iterator it;
	s << "[";
	for (it = magn.begin(); it != magn.end(); it++) {
		// inf and nan are not allowed in JSON so serialise as a big negative number
		if (isfinite(*it))
			s << *it;
		else
			s << "-999.9";
		if (it+1 != magn.end())
			s << ", ";
	}
	s << "]";
	string out = s.str();

	_contentType = "application/json";
	_data.insert(_data.end(), out.begin(), out.end());
	return MHD_HTTP_OK;
}


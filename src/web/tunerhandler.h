/*
 * tunerhandler.h
 *
 *  Created on: 16 Jul 2013
 *      Author: mike
 */

#ifndef TUNERHANDLER_H_
#define TUNERHANDLER_H_

#include <vector>
#include <string>
#include <json/json.h>

#include "httpserver.h"
#include "radio.h"

class TunerHandler : public HttpRequestHandler
{
public:
	TunerHandler();
	~TunerHandler();
	static HttpRequestHandler *factory() {
		return new TunerHandler();
	}

	const string allows(const vector<string> &wildcards) { return "GET"; }

	unsigned short doGet(const vector<string> &wildcards, const vector<char> &requestData);
private:
	Json::Value buildTunerInfo(FrontEnd *fe);
};

#endif /* TUNERHANDLER_H_ */

/*
 * tunercontrolhandler.h
 *
 *  Created on: 20 Jul 2013
 *      Author: mike
 */

#ifndef TUNERCONTROLHANDLER_H_
#define TUNERCONTROLHANDLER_H_

#include <vector>
#include <string>
#include <json/json.h>

#include "httpserver.h"
#include "radio.h"

class TunerControlHandler : public HttpRequestHandler
{
public:
	TunerControlHandler();
	~TunerControlHandler();
	static HttpRequestHandler *factory() {
		return new TunerControlHandler();
	}

	const string allows(const vector<string> &wildcards) { return "GET, PUT"; }

	unsigned short doGet(const vector<string> &wildcards, const vector<char> &requestData);
	unsigned short doPut(const vector<string> &wildcards, const vector<char> &requestData);
private:
	Json::Value buildTunerControl(FrontEnd *fe);
	unsigned short parseTunerControl(FrontEnd *fe, Json::Value &root);
};

#endif /* TUNERCONTROLHANDLER_H_ */

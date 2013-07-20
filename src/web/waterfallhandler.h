/*
 * waterfallhandler.h
 *
 *  Created on: 16 Jul 2013
 *      Author: mike
 */

#ifndef WATERFALLHANDLER_H_
#define WATERFALLHANDLER_H_

#include <vector>
#include <string>

#include "httpserver.h"
#include "radio.h"

class WaterfallHandler : public HttpRequestHandler
{
public:
	WaterfallHandler();
	~WaterfallHandler();
	static HttpRequestHandler *factory() {
		return new WaterfallHandler();
	}

	const string allows(const vector<string> &wildcards) { return "GET"; }

	unsigned short doGet(const vector<string> &wildcards, const vector<char> &requestData);
};

#endif /* WATERFALLHANDLER_H_ */

/*
 * redirecthandler.h
 *
 *  Created on: 15 Jul 2013
 *      Author: mike
 */

#ifndef REDIRECTHANDLER_H_
#define REDIRECTHANDLER_H_

#include <string>
#include <vector>
#include <map>

#include "httpserver.h"

class RedirectHandler : public HttpRequestHandler
{
public:
	RedirectHandler();
	~RedirectHandler();
	static HttpRequestHandler *factory() {
		return new RedirectHandler();
	}

	const string allows() { return "GET"; }

	unsigned short doGet(const vector<string> &wildcards, const vector<char> &requestData);
};

#endif /* REDIRECTHANDLER_H_ */

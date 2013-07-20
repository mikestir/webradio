/*
 * confighandler.h
 *
 *  Created on: 16 Jul 2013
 *      Author: mike
 */

#ifndef CONFIGHANDLER_H_
#define CONFIGHANDLER_H_

#include <vector>
#include <string>

#include "httpserver.h"

class ConfigHandler : public HttpRequestHandler
{
public:
	ConfigHandler();
	~ConfigHandler();
	static HttpRequestHandler *factory() {
		return new ConfigHandler();
	}

	const string allows(const vector<string> &wildcards) { return "GET"; }

	unsigned short doGet(const vector<string> &wildcards, const vector<char> &requestData);
};

#endif /* CONFIGHANDLER_H_ */

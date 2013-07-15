/*
 * redirecthandler.cxx
 *
 *  Created on: 15 Jul 2013
 *      Author: mike
 */

#include <string>
#include <vector>

#include "redirecthandler.h"
#include "debug.h"

RedirectHandler::RedirectHandler() : HttpRequestHandler()
{

}

RedirectHandler::~RedirectHandler()
{

}

unsigned short RedirectHandler::doGet(const vector<string> &wildcards, const vector<char> &requestData)
{
	/* Destination is stored in arg when the handler is registered */
	_location = *((string*)arg());

	/* Replace any wildcard refs */
	int n = 1;
	for (vector<string>::const_iterator it = wildcards.begin(); it != wildcards.end(); ++it) {
		char buf[4];
		int markerlength = snprintf(buf, 4, "$%d", n++);
		size_t pos = _location.find(string(buf));
		if (pos != string::npos)
			_location.replace(pos, markerlength, *it);
	}

	LOG_DEBUG("Redirecting to: %s\n", _location.c_str());
	return MHD_HTTP_FOUND;
}

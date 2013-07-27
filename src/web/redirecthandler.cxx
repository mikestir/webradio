/*
 * WebRadio web-based Software Defined Radio
 *
 * Copyright (C) 2013 Mike Stirling
 *
 * This file is part of WebRadio (http://www.mike-stirling.com/webradio)
 *
 * All rights reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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

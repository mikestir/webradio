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

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

#ifndef RECEIVERHANDLER_H_
#define RECEIVERHANDLER_H_

#include <vector>
#include <string>
#include <json/json.h>

#include "httpserver.h"
#include "radio.h"

class ReceiverHandler : public HttpRequestHandler
{
public:
	ReceiverHandler();
	~ReceiverHandler();
	static HttpRequestHandler *factory() {
		return new ReceiverHandler();
	}

	const string allows(const vector<string> &wildcards);

	unsigned short doGet(const vector<string> &wildcards, const vector<char> &requestData);
	unsigned short doPut(const vector<string> &wildcards, const vector<char> &requestData);
	unsigned short doPost(const vector<string> &wildcards, const vector<char> &requestData);
	unsigned short doDelete(const vector<string> &wildcards, const vector<char> &requestData);
private:
	Json::Value buildReceiverInfo(Receiver *rx);
	unsigned short parseReceiverInfo(Receiver *rx, Json::Value &root);
};

#endif /* RECEIVERHANDLER_H_ */

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

#include <vector>
#include <string>

#include <json/json.h>

#include "confighandler.h"

ConfigHandler::ConfigHandler() : HttpRequestHandler()
{

}

ConfigHandler::~ConfigHandler()
{

}

unsigned short ConfigHandler::doGet(const vector<string> &wildcards, const vector<char> &requestData)
{
	Json::Value root;

	root["htmlpath"] = "html";
	root["version"] = "1.0";
	root["blah"]["test"] = "foo";
	root["blah"]["test2"] = "bar";

	Json::StyledWriter writer;
	string output = writer.write(root);
	_contentType = "application/json";
	_data.insert(_data.end(), output.begin(), output.end());
	return MHD_HTTP_OK;
}

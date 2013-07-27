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
#include <fstream>
#include <map>
#include <algorithm>

#include "filehandler.h"
#include "debug.h"

#define DEFAULT_DIR		"html"

FileHandler::FileHandler() : HttpRequestHandler()
{
	/* Init MIME type list */
	mimeTypes[".html"] = "text/html";
	mimeTypes[".htm"] = "text/html";
	mimeTypes[".js"] = "application/javascript";
	mimeTypes[".css"] = "text/css";
	mimeTypes[".txt"] = "text/plain";
	mimeTypes[".jpg"] = "image/jpeg";
	mimeTypes[".png"] = "image/png";
}

FileHandler::~FileHandler()
{

}

unsigned short FileHandler::doGet(const vector<string> &wildcards, const vector<char> &requestData)
{
	string ct, mypath; // writeable copy
	size_t pos;

	/* Prepend local path */
	mypath = DEFAULT_DIR "/" + wildcards[0];

	/* Remove all .. in path */
	while ((pos = mypath.find("..")) < mypath.length()) {
		mypath.replace(pos, 2, "");
	}

	/* Determine content type from extension */
	pos = mypath.find_last_of('.');
	if (pos < mypath.length())
		ct = mimeTypes[mypath.substr(pos)];
	if (ct.empty())
		ct = "application/octet-stream";

	LOG_DEBUG("path: %s, content-type: %s\n", mypath.c_str(), ct.c_str());

	ifstream file;
	file.exceptions(ifstream::failbit | ifstream::badbit);
	try {
		file.open(mypath.c_str());
		_data.assign((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
		file.close();
	}
	catch (ifstream::failure &e) {
		LOG_DEBUG("Error opening file: %s\n", e.what());
		return MHD_HTTP_NOT_FOUND;
	}

	_contentType = ct;
	return MHD_HTTP_OK;
}

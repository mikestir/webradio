/*
 * filehandler.cxx
 *
 *  Created on: 20 Jun 2013
 *      Author: mike
 */

#include <string>
#include <vector>
#include <fstream>
#include <map>
#include <algorithm>

#include "filehandler.h"
#include "debug.h"

#define DEFAULT_DIR		"html"

FileHandler::FileHandler()
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

HttpRequestHandler* FileHandler::factory()
{
	return new FileHandler();
}

unsigned short FileHandler::handleRequest(const string &method, const string &path,
	const vector<char> &requestData, unsigned short status)
{
	string ct, mypath; // writeable copy
	size_t pos;

	if (method != "GET")
		return MHD_HTTP_METHOD_NOT_ALLOWED;

	/* Prepend local path */
	mypath = DEFAULT_DIR + path;

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

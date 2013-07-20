/*
 * confighandler.cxx
 *
 *  Created on: 16 Jul 2013
 *      Author: mike
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
